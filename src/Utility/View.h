#pragma once

#include <ranges>
#include <string>
#include <type_traits>
#include <utility> // For std::forward.
#include <vector>

namespace detail {

template<class T>
struct is_initializer_list : std::false_type {};

template<class T>
struct is_initializer_list<std::initializer_list<T>> : std::true_type {};

template<class T>
inline constexpr bool is_initializer_list_v = is_initializer_list<T>::value;

template<class Range>
class ViewWrapper;

} // namespace detail

/**
 * CRTP mixin that provides a fluent range API on top of `std::ranges::view_interface`.
 *
 * Any class that inherits `ViewInterface<Derived>` and provides `begin()`/`end()` gets:
 * - `drop(n)`, `skipEmpty()`, `zip(other)` — chainable range transformations. We will be adding more as needed.
 * - `to(&container)` — populate an existing container.
 * - `operator Container()` — implicit conversion to a container.
 * - `empty()`, `size()`, `operator bool` — from `view_interface`.
 *
 * All methods are non-const, following the standard library convention for views. Standard view types like
 * `filter_view` and `drop_view` have non-const `begin()` (they cache the result of the first call), so const
 * iteration is generally not supported for views. The const/non-const distinction on the underlying range is
 * captured in the view's type parameter (e.g. `ref_view<T>` vs `ref_view<const T>`), not in method qualifiers.
 */
template<class Derived>
class ViewInterface : public std::ranges::view_interface<Derived> {
    [[nodiscard]] Derived &derived() { return static_cast<Derived &>(*this); }

 public:
    [[nodiscard]] auto drop(size_t n) {
        return detail::ViewWrapper(std::views::drop(derived(), n));
    }

    [[nodiscard]] auto skipEmpty()
        requires requires (std::ranges::range_reference_t<Derived> e) { { e.empty() } -> std::convertible_to<bool>; }
    {
        return detail::ViewWrapper(std::views::filter(derived(), [](const auto &e) { return !e.empty(); }));
    }

    template<class R>
    [[nodiscard]] auto zip(R &&other) {
        return detail::ViewWrapper(std::views::zip(derived(), std::forward<R>(other)));
    }

    template<class Container>
    void to(Container *container) {
        // We can't use assign_range / insert_range / std::ranges::to here because:
        // - assign_range / insert_range require implicit convertibility, and some conversions are explicit
        //   (e.g., string_view -> string).
        // - on top of that, std::ranges::to would enter infinite recursion through operator Container() below.
        // So we use a manual loop with emplace_back / emplace.
        container->clear();
        for (auto &&element : derived()) {
            if constexpr (requires { container->emplace_back(element); })
                container->emplace_back(element);
            else
                container->emplace(element);
        }
    }

    /**
     * Implicit conversion to a container type.
     *
     * The `initializer_list` exclusion in the requires clause is necessary to prevent ambiguous overload resolution
     * when assigning to containers. Without it, `v = range(...)` would be ambiguous because `std::vector::operator=`
     * has overloads for both `const vector&` and `initializer_list<value_type>`.
     */
    template<class Container> requires
        std::constructible_from<typename Container::value_type, std::ranges::range_reference_t<Derived>> &&
        (!detail::is_initializer_list_v<Container>)
    operator Container() {
        Container result;
        to(&result);
        return result;
    }
};

namespace detail {

template<class Range>
class ViewWrapper : public ViewInterface<ViewWrapper<Range>> {
 public:
    ViewWrapper() = default;

    explicit ViewWrapper(Range range) : _range(std::move(range)) {}

    auto begin() { return std::ranges::begin(_range); }
    auto end() { return std::ranges::end(_range); }

 private:
    Range _range;
};

// CTAD.
template<class Range>
ViewWrapper(Range) -> ViewWrapper<Range>;

} // namespace detail

#ifndef __DOXYGEN__ // Doxygen chokes here...
template<class Range>
inline constexpr bool std::ranges::enable_borrowed_range<detail::ViewWrapper<Range>> =
    std::ranges::enable_borrowed_range<Range>;
#endif

/**
 * Wraps a range into a `ViewWrapper` with fluent view methods (`drop`, `skipEmpty`, `zip`, `to`).
 *
 * @param range                          Range to wrap.
 */
template<class Range>
auto view(Range &&range) {
    return detail::ViewWrapper(std::views::all(std::forward<Range>(range)));
}
