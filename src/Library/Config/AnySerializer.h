#pragma once

#include <string>
#include <string_view>
#include <any>

#include "Library/Serialization/Serialization.h"

class AnySerializer {
 public:
    virtual std::string serialize(const std::any &value) const = 0;
    virtual std::any deserialize(std::string_view value) const = 0;
    virtual const std::type_info &type() const = 0;

    template<class T>
    static AnySerializer *forType();
};

namespace detail {
template<class T>
class TypedSerializer : public AnySerializer {
 public:
    virtual std::string serialize(const std::any &value) const override {
        return toString(std::any_cast<const T &>(value));
    }

    virtual std::any deserialize(std::string_view value) const override {
        return fromString<T>(value);
    }

    virtual const std::type_info &type() const override {
        return typeid(T);
    }
};

template<class T>
constinit TypedSerializer<T> globalTypedSerializer;
} // namespace detail

template<class T>
AnySerializer *AnySerializer::forType() {
    return &detail::globalTypedSerializer<T>;
}
