#pragma once

namespace ui {

/** Draw the static sonar/radar grid (black disc, green overlay, labels). */
void radarDisplayDraw();

/** Redraw aircraft only (blits cached grid; no full-screen clear). */
void radarDisplayRefreshAircraft();

/** Toggle stale-data warning overlay and show age in seconds. */
void radarDisplaySetStale(bool stale, unsigned long age_seconds);

}  // namespace ui
