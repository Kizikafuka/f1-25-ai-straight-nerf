// SPDX-License-Identifier: MIT
// A small mouse panel (Swofty overlay, input v3) with two sliders for the AI cuts. F8 opens / closes it; Esc or the X
// closes it too. While it is open it holds input focus: the core keeps the mouse and keys from the game and draws a cursor.
// With the "open on pause" option it opens by itself when the game is paused and closes when the game resumes.
#pragma once
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <span>
#include <string>
#include <swofty/events.hpp>
#include <swofty/overlay.hpp>

namespace kizi::panel {

namespace ov = swofty::overlay;

inline std::atomic<int> g_power{0}, g_rpm{0};          // the values the nerf uses (settings and the panel both write them)
inline std::atomic<bool> g_open_on_pause{false};

struct Slider {
    const char *label;
    const char *unit;
    std::atomic<int> *value;
    int lo, hi, step;
};

class Panel {
public:
    void start()
    {
        auto r1 = swofty::events::on("game.pause").after(
            [this](std::span<const std::byte>, const swofty::events::Context &) { if (g_open_on_pause) want_open_ = true; });
        auto r2 = swofty::events::on("game.resume").after(
            [this](std::span<const std::byte>, const swofty::events::Context &) { if (g_open_on_pause) want_close_ = true; });
        uint32_t token = 0;
        auto r3 = ov::on_paint(&Panel::paint_entry, this, &token);
        swofty::log("[ai-straight-nerf] panel: game.pause %s, game.resume %s, overlay paint %s, input v3 %s",
                    swofty::events::name(r1), swofty::events::name(r2), ov::name(r3), ov::v3() ? "yes" : "NO (no mouse)");
    }

private:
    static void paint_entry(SwoftyPaint *p, void *self) { static_cast<Panel *>(self)->paint(p); }

    void set_open(bool on)
    {
        if (open_ == on)
            return;
        open_ = on;
        drag_ = -1;
        auto r = ov::focus(on);
        swofty::log("[ai-straight-nerf] panel %s (focus %s)", on ? "open" : "closed", ov::name(r));
    }

    void paint(SwoftyPaint *p)
    {
        ov::Input in = ov::input(p);
        if (in.key_pressed(VK_F8))
            set_open(!open_);
        if (want_open_.exchange(false))
            set_open(true);
        if (want_close_.exchange(false))
            set_open(false);
        if (!open_)
            return;
        if (in.key_pressed(VK_ESCAPE)) {
            set_open(false);
            return;
        }

        float W = 0, H = 0;
        ov::size(p, &W, &H);
        const float s = ov::ui_scale(p);
        const float pw = 560 * s, ph = 330 * s;
        const float px = std::round((W - pw) / 2), py = std::round((H - ph) / 2);

        ov::rect_rounded(p, px, py, pw, ph, 14 * s, 0xE6101418);
        ov::rect_rounded_outline(p, px, py, pw, ph, 14 * s, 2 * s, 0xFF3A4654);
        ov::TextStyle title;
        title.size = 26 * s, title.rgba = 0xFFFFFFFF;
        ov::text(p, px + 24 * s, py + 18 * s, title, "AI Straight-line Nerf  -  EXPERIMENTAL");
        ov::TextStyle hint;
        hint.size = 15 * s, hint.rgba = 0xFF9AA7B5;
        ov::text(p, px + 24 * s, py + 52 * s, hint, "EXPERIMENTAL and barely tested. Drag the sliders; applies to AI cars within a second. F8 / Esc closes.");

        // close button
        const float bx = px + pw - 44 * s, by = py + 14 * s, bs = 28 * s;
        const bool over_close = in.inside(bx, by, bs, bs);
        ov::rect_rounded(p, bx, by, bs, bs, 6 * s, over_close ? 0xFFB03A3A : 0xFF3A4654);
        ov::TextStyle x;
        x.size = 18 * s, x.align = ov::align::center | ov::align::middle;
        ov::text(p, bx + bs / 2, by + bs / 2, x, "X");
        if (in.clicked() && over_close) {
            set_open(false);
            return;
        }

        Slider sl[2] = {{"Engine power cut", "%", &g_power, 0, 10, 1}, {"Top speed cut", " rpm", &g_rpm, 0, 600, 100}};
        for (int i = 0; i < 2; i++) {
            const float ry = py + 100 * s + i * 90 * s;
            const float tx = px + 24 * s, tw = pw - 48 * s, ty = ry + 42 * s, th = 10 * s;
            const int v = sl[i].value->load();
            char buf[64];
            std::snprintf(buf, sizeof buf, "%s: %d%s%s", sl[i].label, v, sl[i].unit, v == 0 ? "  (off)" : "");
            ov::TextStyle lab;
            lab.size = 20 * s, lab.rgba = v ? 0xFFFFD27A : 0xFFE6EDF3;
            ov::text(p, tx, ry, lab, buf);

            // mouse: press on the track (a taller hit area than the bar) starts a drag, release ends it
            const bool hit = in.inside(tx - 10 * s, ty - 14 * s, tw + 20 * s, th + 28 * s);
            if (in.clicked() && hit)
                drag_ = i;
            if (!in.held())
                drag_ = -1;
            if (drag_ == i) {
                float f = std::clamp((in.x - tx) / tw, 0.0f, 1.0f);
                int steps = int(std::lround(f * (sl[i].hi - sl[i].lo) / float(sl[i].step)));
                sl[i].value->store(sl[i].lo + steps * sl[i].step);
            }

            const float f = float(sl[i].value->load() - sl[i].lo) / float(sl[i].hi - sl[i].lo);
            ov::rect_rounded(p, tx, ty, tw, th, th / 2, 0xFF2A323C);
            ov::rect_rounded(p, tx, ty, std::max(th, tw * f), th, th / 2, 0xFFE0A030);
            const float kr = 12 * s;
            ov::rect_rounded(p, tx + tw * f - kr, ty + th / 2 - kr, 2 * kr, 2 * kr, kr, drag_ == i || hit ? 0xFFFFFFFF : 0xFFD9E1E8);
        }

        // reset button
        const float rx = px + 24 * s, rw = 120 * s, rh = 34 * s, rby = py + ph - 56 * s;
        const bool over_reset = in.inside(rx, rby, rw, rh);
        ov::rect_rounded(p, rx, rby, rw, rh, 8 * s, over_reset ? 0xFF4C5A6B : 0xFF3A4654);
        ov::TextStyle rt;
        rt.size = 18 * s, rt.align = ov::align::center | ov::align::middle;
        ov::text(p, rx + rw / 2, rby + rh / 2, rt, "Reset to 0");
        if (in.clicked() && over_reset)
            g_power = 0, g_rpm = 0;

        ov::text(p, rx + rw + 20 * s, rby + 9 * s, hint, "Not saved: the launcher settings load again next start.");
    }

    bool open_ = false;
    int drag_ = -1;
    std::atomic<bool> want_open_{false}, want_close_{false};
};

}
