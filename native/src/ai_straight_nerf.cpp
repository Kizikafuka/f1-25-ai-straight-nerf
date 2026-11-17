// SPDX-License-Identifier: MIT
// AI Straight-line Nerf, in-game part: live power / top-speed cut for AI cars (ai_nerf.hpp) and a mouse panel to change the two
// values while playing or paused (ai_panel.hpp, F8). The ERS option of this addon is a database edit (db/ers.json)
// applied while the game boots.
#include <windows.h>

#include <swofty/mixin.hpp>

#include "ai_nerf.hpp"
#include "ai_panel.hpp"

SWOFTY_SETTING(int, ai_power_cut, 0);
SWOFTY_SETTING(int, ai_topspeed_cut, 0);
SWOFTY_SETTING(bool, open_on_pause, false);

// the launcher's settings and the panel write the same two numbers; the last change wins
SWOFTY_ON_CHANGE(ai_power_cut, int v)
{
    kizi::panel::g_power = v;
}

SWOFTY_ON_CHANGE(ai_topspeed_cut, int v)
{
    kizi::panel::g_rpm = v;
}

SWOFTY_ON_CHANGE(open_on_pause, bool on)
{
    kizi::panel::g_open_on_pause = on;
}

SWOFTY_ON_LOAD
{
    kizi::panel::g_power = ai_power_cut.get();
    kizi::panel::g_rpm = ai_topspeed_cut.get();
    kizi::panel::g_open_on_pause = open_on_pause.get();
    (new kizi::nerf::Nerf([] { return std::pair<int, int>{kizi::panel::g_power.load(), kizi::panel::g_rpm.load()}; }))->start();
    (new kizi::panel::Panel)->start();
    swofty::log("[ai-straight-nerf] EXPERIMENTAL addon loaded: power cut %d%%, top speed cut %d rpm, panel on F8%s", ai_power_cut.get(),
                ai_topspeed_cut.get(), open_on_pause.get() ? " and on pause" : "");
}

SWOFTY_ADDON()
