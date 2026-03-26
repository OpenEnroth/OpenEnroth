#pragma once

#include <memory>
#include <string_view>

#include "EncodingEnums.h"

struct uchardet;

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
     * @return                          Detected encoding, or `ENCODING_BYTES` in case of a failure.
     */
    TextEncoding finish();

 private:
    std::unique_ptr<uchardet, void (*)(uchardet *)> _handle;
};
