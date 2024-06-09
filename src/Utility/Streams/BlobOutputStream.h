#pragma once

#include <string>

#include "Utility/Embedded.h"
#include "Utility/Memory/Blob.h"

#include "StringOutputStream.h"

class BlobOutputStream : private Embedded<std::string>, public StringOutputStream {
    using base_type = StringOutputStream;
 public:
    BlobOutputStream();
    explicit BlobOutputStream(Blob *target);
    virtual ~BlobOutputStream();

    void open(Blob *target);

    virtual void flush() override;
    virtual void close() override;

 private:
    void closeInternal();

 private:
    Blob *_target = nullptr;
};
