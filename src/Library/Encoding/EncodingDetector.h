#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "EncodingEnums.h"

struct uchardet;

struct EncodingDetectionResult {
    TextEncoding encoding = ENCODING_ASCII;
    float confidence = 0.0f;
};

/**
 * RAII wrapper for uchardet charset detection library.
 */
class EncodingDetector {
 public:
    EncodingDetector();
    ~EncodingDetector();

    EncodingDetector(const EncodingDetector &) = delete;
    EncodingDetector &operator=(const EncodingDetector &) = delete;

    EncodingDetector(EncodingDetector &&other) noexcept = default;
    EncodingDetector &operator=(EncodingDetector &&other) noexcept = default;

    void write(std::string_view data);

    /**
     * @return                          Detection result, sorted by confidence (most probable encoding in `front()`).
     *                                  Empty in case of failure.
     */
    std::vector<EncodingDetectionResult> finish();

 private:
    std::unique_ptr<uchardet, void (*)(uchardet *)> _handle;
};
