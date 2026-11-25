#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

#include <gtest/gtest.h> // NOLINT: not a C system header.

#include "Library/Image/Image.h"

struct FrameStats {
    double mean = 0;
    double std = 0;
};

// A frame is blank when the standard deviation over its RGB channels is below 1.0. A real MM8 frame
// measures about 35-39. A blank frame is `NullRenderer`'s all-zero frame, or a screenshot that comes
// back uniformly black.
inline FrameStats computeFrameStats(RgbaImageView image) {
    double sum = 0;
    double sumSquares = 0;
    int64_t sampleCount = 0;
    for (Color pixel : image.pixels()) {
        for (uint8_t channel : {pixel.r, pixel.g, pixel.b}) {
            sum += channel;
            sumSquares += static_cast<double>(channel) * channel;
            sampleCount++;
        }
    }
    if (sampleCount == 0)
        return {};
    double mean = sum / sampleCount;
    double variance = sumSquares / sampleCount - mean * mean;
    return {mean, std::sqrt(std::max(variance, 0.0))};
}

// Use as `ASSERT_TRUE(assertFrameNotBlank(frame))` / `EXPECT_TRUE(assertFrameNotBlank(frame))` -
// fails with the measured mean/std instead of letting a blank frame
// pass as "no pixels changed" in an unrelated diff assertion.
inline testing::AssertionResult assertFrameNotBlank(RgbaImageView image) {
    FrameStats stats = computeFrameStats(image);
    if (stats.std < 1.0) {
        return testing::AssertionFailure()
            << "frame is blank (mean=" << stats.mean << " std=" << stats.std << ", need std >= 1.0) - "
               "no real GL frame was captured.";
    }
    return testing::AssertionSuccess() << "mean=" << stats.mean << " std=" << stats.std;
}
