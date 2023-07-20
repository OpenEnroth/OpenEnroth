#pragma once

#include <cstddef>

struct UnsizedTag {};

struct PresizedTag {
    explicit PresizedTag(size_t size) : size(size) {}

    size_t size;
};

struct AppendTag {};

template<class T>
class ContextTag {
 public:
    explicit ContextTag(const T &ctx) : _ctx(ctx) {}

    const T *operator->() const {
        return &_ctx;
    }

    const T &operator*() const {
        return _ctx;
    }

 private:
    const T &_ctx;
};

/**
 * Creates a serialization tag that instructs the binary serialization framework to NOT write the vector size
 * into the stream. Note that this implies that you'll have to use the `presized` tag when deserializing.
 *
 * @return                              Tag object to be passed into the `serialize` call.
 * @see presized
 */
inline UnsizedTag unsized() {
    return {};
}

/**
 * Creates a deserialization tag that instructs the binary serialization to use the supplied vector size instead of
 * reading it from the stream. Note that this implies that the serialization must have been performed with the
 * `unsized` tag.
 *
 * @param size                          Number of elements to deserialize.
 * @return                              Tag object to be passed into the `deserialize` call.
 * @see unsized
 */
inline PresizedTag presized(size_t size) {
    return PresizedTag(size);
}

/**
 * Creates a deserialization tag that instructs the binary serialization framework to append deserialized elements
 * into the target vector instead of replacing its contents. This is useful when you want to deserialize several
 * sequences into the same `std::vector` object - normally subsequent `deserialize` calls would replace the vector
 * contents, but using this wrapper lets you accumulate the results of several `deserialize` calls instead.
 *
 * @return                              Tag object to be passed into `deserialize` call.
 */
inline AppendTag append() {
    return {};
}

/**
 * This tag factory function is mainly for the users who want to pass additional context into their serialization
 * routines. The way to do this is to accept a `ContextTag` as the last parameter in the serialization function, and
 * then pass the context itself by calling `context` at call site.
 *
 * @param ctx                           Context to pass to the serialization routine.
 * @return                              Tag object containing a reference to the passed context.
 */
template<class T>
inline ContextTag<T> context(const T &ctx) {
    return ContextTag<T>(ctx);
}
