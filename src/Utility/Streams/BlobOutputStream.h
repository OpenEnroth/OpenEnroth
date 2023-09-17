#pragma once

#include <string>

#include "Utility/Embedded.h"
#include "Utility/Memory/Blob.h"

#include "StringOutputStream.h"

class BlobOutputStream : private Embedded<std::string>, public StringOutputStream {
    using base_type = StringOutputStream;
 public:
    explicit BlobOutputStream(Blob *target);
    virtual ~BlobOutputStream();

    virtual void flush() override;
    virtual void close() override;

 private:
    void closeInternal();

 private:
    Blob *_target = nullptr;
};
