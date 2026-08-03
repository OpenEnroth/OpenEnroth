#include <cstddef>
#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/OutputStream.h"
#include "Utility/Exception.h"

/**
 * An output stream that leaves a mangled buffer behind and then throws, from `_overflow` or from `_flush`. Position
 * accounting must not depend on what a failing implementation does to the buffer.
 */
class ThrowingOutputStream : public OutputStream {
 public:
    explicit ThrowingOutputStream(bool throwOnFlush) : _throwOnFlush(throwOnFlush) {
        open({}, {});
    }

    virtual ~ThrowingOutputStream() {
        destroy();
    }

 protected:
    virtual void _overflow(Buffer *buffer, const void *data, size_t size) override {
        if (_throwOnFlush) {
            buffer->reset(_scratch, _scratch, _scratch + sizeof(_scratch));
            buffer->write(data, size);
            return;
        }

        buffer->reset(_scratch, _scratch + 7, _scratch + sizeof(_scratch)); // `used()` out of nowhere.
        throw Exception("overflow failed");
    }

    virtual void _flush(Buffer *buffer) override {
        buffer->reset(_scratch, _scratch + 7, _scratch + sizeof(_scratch));
        throw Exception("flush failed");
    }

    virtual void _close(Buffer *buffer, bool canThrow) override {
        OutputStream::_close(buffer, canThrow);
    }

 private:
    bool _throwOnFlush = false;
    char _scratch[64] = {};
};

UNIT_TEST(OutputStream, PositionAfterThrowingOverflow) {
    ThrowingOutputStream out(false);

    EXPECT_EQ(out.position(), 0u);
    EXPECT_THROW(out.write("hello", 5), Exception);
    EXPECT_EQ(out.position(), 0u); // Nothing was accepted, so it must not have moved.
    out.close();
}

UNIT_TEST(OutputStream, PositionAfterThrowingFlush) {
    ThrowingOutputStream out(true);

    out.write("hello", 5);
    EXPECT_EQ(out.position(), 5u);
    EXPECT_THROW(out.flush(), Exception);
    EXPECT_EQ(out.position(), 5u); // Flushing consumes nothing, so it must not have moved either.
    out.close();
}
