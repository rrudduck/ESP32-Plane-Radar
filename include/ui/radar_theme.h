#pragma once

#include <cstdint>

namespace ui::radar {

constexpr int kSize = 240;
constexpr int kCenterX = kSize / 2;
constexpr int kCenterY = kSize / 2;

/** Inset for N/S/E/W from top, bottom, left, right of the 240×240 buffer. */
constexpr int kEdgeInset = 4;

/** Outermost grid ring (inside edge labels). */
constexpr int kGridOuterRadius = 103;

/** Extra label height (px) for N/S/E/W and scale text. */
constexpr int kLabelExtraPx = 4;

/** Shift scale label left from ring-3 spoke (px). */
constexpr int kScaleLabelOffsetX = -11;

/** Extra downward offset for S label (px). */
constexpr int kCardinalSouthYOffset = 4;
/** W/E inset from left/right edge (px); smaller = closer to sides. */
constexpr int kCardinalSideInset = 3;

constexpr int kRingCount = 4;

constexpr int kCenterDotRadius = 2;

constexpr float kCardinalLabelSize = 0.55f;
constexpr float kScaleLabelSize = 0.5f;

extern uint16_t kColorBackground;
extern uint16_t kColorGrid;
extern uint16_t kColorLabel;
extern uint16_t kColorCenter;

}  // namespace ui::radar
