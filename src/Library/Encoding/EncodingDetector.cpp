#include "EncodingDetector.h"

#include <vector>

#include <uchardet.h> // NOLINT: not a C system include.

EncodingDetector::EncodingDetector() : _handle(uchardet_new(), &uchardet_delete) {}

EncodingDetector::~EncodingDetector() = default;

void EncodingDetector::write(std::string_view data) {
    int result = uchardet_handle_data(_handle.get(), data.data(), data.size());
    if (result != 0) // The only possible error is an out of memory error, we rethrow in this case.
        throw std::bad_alloc();
}

std::vector<EncodingDetectionResult> EncodingDetector::finish() {
    uchardet_data_end(_handle.get());

    std::vector<EncodingDetectionResult> results;
    for (size_t i = 0, count = uchardet_get_n_candidates(_handle.get()); i < count; i++) {
        EncodingDetectionResult result;

        // Some of the uchardet encodings are not present in std::text_encoding or not supported by ztd.text,
        // we just skip these.
        if (!tryDeserialize(uchardet_get_encoding(_handle.get(), i), &result.encoding))
            continue;
        result.confidence = uchardet_get_confidence(_handle.get(), i);

        results.push_back(result);
    }

    uchardet_reset(_handle.get());

    return results;
}
