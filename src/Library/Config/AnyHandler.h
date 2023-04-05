#pragma once

#include <string>
#include <string_view>
#include <any>

#include "Library/Serialization/Serialization.h"

class AnyHandler {
 public:
    virtual std::string serialize(const std::any &value) const = 0;
    virtual std::any deserialize(std::string_view value) const = 0;
    virtual bool equals(const std::any &l, const std::any &r) const = 0;
    virtual const std::type_info &type() const = 0;

    template<class T>
    static AnyHandler *forType();
};

namespace detail {
template<class T>
class TypedHandler : public AnyHandler {
 public:
    virtual std::string serialize(const std::any &value) const override {
        return toString(std::any_cast<const T &>(value));
    }

    virtual std::any deserialize(std::string_view value) const override {
        return fromString<T>(value);
    }

    virtual bool equals(const std::any &l, const std::any &r) const override {
        return std::any_cast<const T &>(l) == std::any_cast<const T &>(r);
    }

    virtual const std::type_info &type() const override {
        return typeid(T);
    }
};

template<class T>
constinit TypedHandler<T> globalTypedSerializer;
} // namespace detail

template<class T>
AnyHandler *AnyHandler::forType() {
    return &detail::globalTypedSerializer<T>;
}
