// SPDX-License-Identifier: MIT
// Live layers on the AI cars' virtual fields engine.power_scale (x 1 - cut%) and engine.limiter_offset_rpm (- rpm). The core's
// handler on the engine drive torque reads both per car, so neither touches the AI's corner-speed planning. Layers are refreshed
// once a second on the flow thread: a changed setting, new cars, and cars that change between AI and you (AI Drive) are handled.
// Make one Nerf with `new` and never delete it (the layers must not call the core while the game unloads).
#pragma once
#include <atomic>
#include <functional>
#include <map>
#include <span>
#include <utility>
#include <swofty/events.hpp>
#include <swofty/systems/cars.hpp>

namespace kizi::nerf {

namespace live = swofty::cars::live;

class Nerf {
public:
    // cut() -> {power cut in percent, torque cut in rpm under the limiter}
    explicit Nerf(std::function<std::pair<int, int>()> cut) : cut_(std::move(cut)) {}

    void start()
    {
        auto r1 = swofty::events::on("session.start").after(
            [this](std::span<const std::byte>, const swofty::events::Context &) { new_session_ = true; });
        auto r2 = swofty::events::on_frame(&Nerf::frame_entry, this);
        swofty::log("[ai-straight-nerf] layers: session.start %s, frame callback %s", swofty::events::name(r1),
                    swofty::events::name(r2));
    }

private:
    struct Car {
        int power, rpm;
        live::Layer power_layer, rpm_layer;
    };

    static void frame_entry(double dt, void *self) { static_cast<Nerf *>(self)->frame(dt); }

    void frame(double dt)
    {
        acc_ += dt;
        if (acc_ < 1.0)
            return;
        acc_ = 0;
        update();
    }

    void update()
    {
        if (new_session_.exchange(false))
            cars_.clear();
        if (refused_)
            return;
        auto [power, rpm] = cut_();
        if (!live::available()) {
            if (!missing_logged_ && (power || rpm)) {
                missing_logged_ = true;
                swofty::log("[ai-straight-nerf] the live car operations are not available (needs Swofty Launcher 1.7 and this game build)");
            }
            return;
        }
        std::map<uint32_t, const live::Car *> ai;
        auto list = live::cars();
        for (const auto &c : list)
            if (c.is_ai())
                ai[c.index()] = &c;
        for (auto it = cars_.begin(); it != cars_.end();)
            it = ai.count(it->first) ? std::next(it) : cars_.erase(it);
        int made = 0;
        for (auto &[idx, car] : ai) {
            auto it = cars_.find(idx);
            if (it != cars_.end() && it->second.power == power && it->second.rpm == rpm)
                continue;
            if (it != cars_.end())
                cars_.erase(it);
            if (!power && !rpm)
                continue;
            auto sel = live::Cars::car(*car);
            live::Layer lp, lr;
            if (power) {
                lp = live::Layer::scale(sel, "engine.power_scale", 1.0f - power / 100.0f);
                if (!lp.ok() && refuse(lp.result()))
                    return;
            }
            if (rpm) {
                lr = live::Layer::add(sel, "engine.limiter_offset_rpm", -float(rpm));
                if (!lr.ok() && refuse(lr.result()))
                    return;
            }
            if ((power && !lp.ok()) || (rpm && !lr.ok()))
                continue;                          // late (the game is rebuilding): tried again next second
            cars_.emplace(idx, Car{power, rpm, std::move(lp), std::move(lr)});
            made++;
        }
        if (made)
            swofty::log("[ai-straight-nerf] %d AI car(s): power -%d%%, torque cut %d rpm under the limiter", made, power, rpm);
        if (cars_.empty() && was_set_)
            swofty::log("[ai-straight-nerf] back to the game's");
        was_set_ = !cars_.empty();
    }

    bool refuse(swofty::Result r)
    {
        if (r == swofty::Result::unavailable || r == swofty::Result::invalid) {
            refused_ = true;
            swofty::log("[ai-straight-nerf] the core refused the engine field (%s): not used", swofty::result_name(r));
            return true;
        }
        return false;
    }

    std::function<std::pair<int, int>()> cut_;
    std::map<uint32_t, Car> cars_;
    std::atomic<bool> new_session_{false};
    bool refused_ = false, missing_logged_ = false, was_set_ = false;
    double acc_ = 0;
};

}
