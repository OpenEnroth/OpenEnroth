#pragma once

#include <cstddef>

struct UnsizedTag {};

struct PresizedTag {
    constexpr explicit PresizedTag(size_t size) : size(size) {}

    size_t size;
};

struct AppendTag {};

struct NullTerminatedTag {};

template<class T>
class ContextTag {
 public:
    constexpr explicit ContextTag(const T &ctx) : _ctx(ctx) {}

    constexpr const T *operator->() const {
        return &_ctx;
    }

    constexpr const T &operator*() const {
        return _ctx;
    }

 private:
    const T &_ctx;
};

class EachTag {};

namespace tags {
/**
 * Serialization tag that instructs the binary serialization framework to NOT write the vector size
 * into the stream. Note that this implies that you'll have to use the `tags::presized` tag when deserializing.
 *
 * @see presized
 */
constexpr UnsizedTag unsized;

/**
 * Deserialization tag that instructs the binary serialization to use the supplied vector size instead of
 * reading it from the stream. Note that this implies that the serialization must have been performed with the
 * `tags::unsized` tag.
 *
 * @param size                          Number of elements to deserialize.
 * @return                              Tag object to be passed into the `deserialize` call.
 * @see unsized
 */
constexpr PresizedTag presized(size_t size) {
    return PresizedTag(size);
}

/**
 * Deserialization tag that instructs the binary serialization framework to append deserialized elements
 * into the target vector instead of replacing its contents. This is useful when you want to deserialize several
 * sequences into the same `std::vector` - normally, subsequent `deserialize` calls would replace the vector
 * contents, but using this tag lets you accumulate the results of several `deserialize` calls instead.
 */
constexpr AppendTag append;

/**
 * Deserialization tag that instructs the binary serialization framework to read a null-terminated string from
 * the stream.
 */
constexpr NullTerminatedTag nullTerminated;

/**
 * This tag is mainly for the users who want to pass additional context into their serialization routines.
 * The way to do this is to accept a `ContextTag` as the last parameter in the serialization function, and
 * then pass the context into the call by invoking `tags::context` at the call site.
 *
 * @param ctx                           Context to pass to the serialization routine.
 * @return                              Tag object containing a reference to the passed context.
 */
template<class T>
constexpr ContextTag<T> context(const T &ctx) {
    return ContextTag<T>(ctx);
}

/**
 * Serialization tag that applies subsequent tags to each element of a container individually, rather than to the
 * container as a whole. Without this tag, there is no way to pass tags down to individual container elements.
 *
 * Example usage:
 * ```
 * std::vector<Monster> monsters;
 * deserialize(src, &monsters, tags::append, tags::each, tags::via<MonsterDesc_MM7>);
 * ```
 */
constexpr EachTag each;

} // namespace tags
