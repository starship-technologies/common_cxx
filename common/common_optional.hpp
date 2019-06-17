/*
 * Copyright (c) 2018 Starship Technologies, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef COMMON_OPTIONAL_HPP
#define COMMON_OPTIONAL_HPP

#include <cstdio>
#include <memory>
#include <type_traits>

#include "common/common_panic.hpp"

namespace common
{

struct none {};
struct def {};
struct in_place {};

template <typename T, typename E>
struct result;

namespace impl
{
template <typename T>
struct optional_storage { using type = T; };

template <typename R>
struct optional_storage<R&> { using type = std::reference_wrapper<R>; };

template <typename R>
struct optional_storage<const R&> { using type = std::reference_wrapper<const R>; };

template <typename T>
struct add_reference_const {
    using value_type = typename std::remove_reference<T>::type;
    using const_value_type = typename std::add_const<value_type>::type;
    using type = typename std::add_lvalue_reference<const_value_type>::type;
};

};

#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ <= 6
#  define common_bugless_constexpr
#else
#  define common_bugless_constexpr constexpr
#endif

/**
 * Represents an optionally present type,
 * default construction is empty, and the
 * contained type constructor will not be
 * called.
 */
template <typename T>
struct optional
{
    enum {
        EMPTY,
        PRESENT,
    } state = EMPTY;
    using value_type = T;
    using storage_type = typename impl::optional_storage<T>::type;
    using non_reference_type = typename std::remove_reference<T>::type;
    using reference_type = typename std::add_lvalue_reference<T>::type;
    using decayed_type = typename std::decay<T>::type;
    using const_type = typename std::conditional<std::is_reference<T>::value, typename impl::add_reference_const<T>::type, typename std::add_const<T>::type>::type;
    union {
        storage_type t;
    };
    common_bugless_constexpr optional() {};
    common_bugless_constexpr optional(none) {};
    template <typename U = T>
    optional(T&& t, typename std::enable_if<std::is_move_constructible<U>::value && !std::is_reference<U>::value>::type* = 0)
    : state(PRESENT) { new(&this->t) storage_type(std::move(t)); }
    template <typename U = T, typename = typename std::enable_if<std::is_copy_constructible<U>::value>::type>
    optional(const T& t) : state(PRESENT) { new(&this->t) storage_type(t); }
    template <typename... Args>
    optional(in_place, Args&&... args) : state(PRESENT) { new(&this->t) storage_type(std::forward<Args...>(args...)); }
    optional(def) : state(PRESENT) { new(&this->t) storage_type(); static_assert(std::is_default_constructible<T>::value, "type not default constructible"); }

    template <typename... Args>
    void reset(Args&&... args) {
        destroy();
        state = PRESENT;
        new(&this->t) storage_type(std::forward<Args>(args)...);
    }
    template <typename... Args>
    void emplace(Args&&... args) {
        destroy();
        state = PRESENT;
        new(&this->t) storage_type(std::forward<Args>(args)...);
    }
    void reset_default() {
        static_assert(std::is_default_constructible<T>::value, "type not default constructible");
        destroy();
        state = PRESENT;
        new(&this->t) storage_type();
    }
    optional& operator=(optional&& other)
    {
        if ((state == PRESENT) && (other.state == PRESENT)) {
            t = std::move(other.t);
        } else if (other.state == PRESENT) {
            new(&this->t) storage_type(std::move(other.t));
            state = PRESENT;
        } else {
            destroy();
        }
        return *this;
    }
    optional& operator=(const optional& other)
    {
        if ((state == PRESENT) && (other.state == PRESENT)) {
            t = other.t;
        } else if (other.state == PRESENT) {
            new(&this->t) storage_type(other.t);
            state = PRESENT;
        } else {
            destroy();
        }
        return *this;
    }
    // Allow assignment from anything T is constructible with
    template <typename U = T,
      typename = typename std::enable_if<
          !std::is_same<optional, typename std::decay<U>::type>::value
        && std::is_constructible<T, U>::value
        && std::is_assignable<T&, U>::value
        && (
              !std::is_scalar<T>::value
            || std::is_same<T, typename std::decay<U>::type>::value
           )
      >::type
    >
    optional& operator=(U&& v) {
        if (state == PRESENT) {
            t = std::forward<U>(v);
        } else {
            new(&this->t) storage_type(std::forward<U>(v));
            state = PRESENT;
        }
        return *this;
    }

    optional(optional&& other)
    : state{other.state}
    {
        if (state == PRESENT)
            new(&this->t) storage_type(std::move(other.t));
    }
    optional(const optional& other)
    : state{other.state}
    {
        if (state == PRESENT)
            new(&this->t) storage_type(other.t);
    }
    void clear()
    {
        destroy();
    }
    void destroy()
    {
        if (state == PRESENT) {
            t.~storage_type();
            state = EMPTY;
        }
    }
    ~optional()
    {
        if (state == PRESENT)
            t.~storage_type();
    }
    T& get_checked()
    {
        if (state == PRESENT)
            return t;
        COMMON_PANIC("get() called on empty optional");
    }
    const T& get_checked() const
    {
        if (state == PRESENT)
            return t;
        COMMON_PANIC("get() called on empty optional");
    }
    T& get() &
    {
        return get_checked();
    }
    T get() &&
    {
        return get_checked();
    }
    const T& get() const &
    {
        return get_checked();
    }
    T& value() &
    {
        return get_checked();
    }
    T value() &&
    {
        return get_checked();
    }
    const T& value() const &
    {
        return get_checked();
    }
    T& value_unchecked() &
    {
        return t;
    }
    T value_unchecked() &&
    {
        return t;
    }
    const T& value_unchecked() const &
    {
        return t;
    }
    constexpr T get_or(const T& def = T()) const
    {
        return (state == PRESENT) ? T(t) : def;
    }
    template <typename... Args>
    T& get_or_insert(Args&&... args) {
        if (state != PRESENT)
            reset(std::forward<Args>(args)...);
        return t;
    }
    template <typename Fn>
    T& get_or_insert_with(Fn&& fn) {
        if (state != PRESENT)
            reset(fn());
        return t;
    }
    constexpr const non_reference_type* operator->() const
    {
        return &(const T&)t;
    }
    non_reference_type* operator->()
    {
        return &(T&)t;
    }
    T& operator*() &
    {
        return t;
    }
    T operator*() &&
    {
        return t;
    }
    constexpr const T& operator*() const &
    {
        return t;
    }
    constexpr bool is_some() const
    {
        return state == PRESENT;
    }
    constexpr bool is_none() const
    {
        return !is_some();
    }
    constexpr bool some_equal(const T& other) const
    {
        return is_some() && (get() == other);
    }
    constexpr bool operator==(const T& other) const
    {
        return some_equal(other);
    }
    bool operator==(const optional& other) const
    {
        if (state != other.state)
            return false;
        if (state == PRESENT)
            return t == other.t;
        return true;
    }
    constexpr bool operator!=(const optional& other) const
    {
        return !(*this == other);
    }

    // F: (T) -> newtype
    template <typename Fn>
    optional<typename std::result_of<Fn(T)>::type> map(Fn&& fn) const
    {
        if (is_some())
            return fn(get());
        else
            return optional<typename std::result_of<Fn(T)>::type>(none{});
    }
    // F: (T) -> optional<newtype>
    template <typename Fn>
    typename std::result_of<Fn(T)>::type and_then(Fn&& fn) const
    {
        if (is_some())
            return fn(get());
        else
            return typename std::result_of<Fn(T)>::type(none{});
    }
    optional take()
    {
        optional ret{std::move(*this)};
        destroy();
        return ret;
    }
    // F: () -> T
    template <typename Fn>
    T unwrap_or_else(Fn&& fn) &&
    {
        if (is_some())
            return std::move(get());
        return fn();
    }
    // F: () -> optional<T>
    template <typename Fn>
    optional or_else(Fn&& fn) &&
    {
        if (is_some())
            return std::move(get());
        return fn();
    }
    optional or_other(const optional& other) const
    {
        if (is_some())
            return get();
        return other;
    }
    optional<const reference_type> as_ref() const
    {
        return map([] (const reference_type val) -> const reference_type { return val; });
    }
    optional<reference_type> as_ref()
    {
        if (is_some())
            return optional<reference_type>(get());
        return optional<reference_type>(none{});
    }
    optional<const_type> as_const() const
    {
        return map([] (const T val) -> const_type { return val; });
    }
    // optional<T> -> result<T, E>
    template <typename E>
    result<T, E> ok_or(const E& e) &&
    {
        if (is_some())
            return result<T, E>{std::move(t)};
        return result<T, E>{e};
    }
    template <typename E>
    result<T, E> ok_or(E&& e) &&
    {
        if (is_some())
            return result<T, E>{std::move(t)};
        return result<T, E>{std::move(e)};
    }
    template <typename E>
    result<T, E> ok_or(const E& e) const &
    {
        if (is_some())
            return result<T, E>{t};
        return result<T, E>{e};
    }
    template <typename E>
    result<T, E> ok_or(E&& e) const &
    {
        if (is_some())
            return result<T, E>{t};
        return result<T, E>{std::move(e)};
    }
    optional<decayed_type> cloned() const
    {
        return map([] (const reference_type val) -> decayed_type { return val; });
    }
    // F: (T&) -> void
    template <typename Fn>
    void with(Fn&& fn)
    {
        if (is_some())
            fn((T&) get());
    }
    // F: (const T&) -> bool
    template <typename Fn>
    optional& filter(Fn&& fn) &
    {
        if (is_some() && !fn((const T&) get()))
            destroy();
        return *this;
    }
    // F: (const T&) -> bool
    template <typename Fn>
    optional filtered(Fn&& fn) const &
    {
        if (!is_some() || !fn((const T&) get()))
            return none{};
        return *this;
    }
    // F: (const T&) -> bool
    template <typename Fn>
    optional filtered(Fn&& fn) &&
    {
        if (!is_some() || !fn((const T&) get()))
            return none{};
        return optional{std::move(get())};
    }

    explicit operator bool() const { return state == PRESENT; }
};

template <class T>
optional<T> make_optional(const T& t)
{
    return optional<T>{t};
}
template <class T>
optional<T> make_optional(T&& t)
{
    return optional<T>{std::move(t)};
}
template <class T, typename... Args>
optional<T> make_optional(Args&&... args)
{
    return optional<T>{in_place{}, std::forward<Args>(args)...};
}
template <class T>
optional<T> make_optional_if(bool flag, const T& t)
{
    return flag ? optional<T>{t} : optional<T>{none{}};
}
template <class T>
optional<T> make_optional_if(bool flag, T&& t)
{
    return flag ? optional<T>{std::forward<T>(t)} : optional<T>{none{}};
}
template <class T, typename... Args>
optional<T> make_optional_if(bool flag, Args&&... args)
{
    return flag ? optional<T>{in_place{}, std::forward<Args>(args)...} : optional<T>{none{}};
}

} // namespace common

#endif // COMMON_OPTIONAL_HPP

