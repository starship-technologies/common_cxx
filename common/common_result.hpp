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

#ifndef COMMON_RESULT_HPP
#define COMMON_RESULT_HPP

#include <cstdlib>
#include <cstdio>
#include <memory>

#include "common/common_panic.hpp"

namespace common
{

struct ok
{
};
template <typename T>
struct ok_t
{
    T&& t;
    ok_t(T&& arg) : t(std::forward<T>(arg)) {};
};
template <typename T>
ok_t<T> make_ok(T&& t)
{
    return ok_t<T>(std::forward<T>(t));
}

/**
 * Represents a (result | error) type,
 * containing one or the other based on
 * the constructor.
 *
 * To use,
 *   auto res = some_operation(some_data);
 *   if (res)
 *      happy_path(res.res());
 *   else
 *      sad_path(res.err());
 *
 * Can be used for binary success | error
 * as:
 *   result<ok, E>
 *
 * To return success, simply:
 *   return ok{};
 */
template <typename T, typename E>
struct result
{
    using success_type = T;
    using error_type = E;

    enum {
        MOVED_OUT,
        IS_T,
        IS_E,
    } state = MOVED_OUT;
    union {
        T t;
        E e;
    };
    result(T&& t) : state(IS_T) { new(&this->t) T(std::move(t)); }
    result(E&& e) : state(IS_E) { new(&this->e) E(std::move(e)); }
    result(const T& t) : state(IS_T) { new(&this->t) T(t); }
    result(const E& e) : state(IS_E) { new(&this->e) E(e); }
    template <typename C>
    result(ok_t<C>&& ok) : state(IS_T) { new(&this->t) T(std::forward<C>(ok.t)); }

    result(result&& other)
    {
        *this = std::move(other);
    }
    result(const result& other)
    {
        *this = other;
    }
    result& operator=(result&& other)
    {
        destruct();
        if (other.state == IS_T)
            new(&t) T(std::move(other.t));
        else if (other.state == IS_E)
            new(&e) E(std::move(other.e));
        state = other.state;
        other.destruct();
        return *this;
    }
    result& operator=(const result& other)
    {
        destruct();
        if (other.state == IS_T)
            new(&t) T(other.t);
        else if (other.state == IS_E)
            new(&e) E(other.e);
        state = other.state;
        return *this;
    }
    void destruct()
    {
        if (state == IS_T)
            t.~T();
        else if (state == IS_E)
            e.~E();
        state = MOVED_OUT;
    }
    ~result()
    {
        destruct();
    }
    T& res_inner()
    {
        if (state == IS_T)
            return t;
        COMMON_PANIC("res() called in error state");
    }
    const T& res_inner() const
    {
        if (state == IS_T)
            return t;
        COMMON_PANIC("res() called in error state");
    }
    T& res() &
    {
        return res_inner();
    }
    T res() &&
    {
        return res_inner();
    }
    const T& res() const &
    {
        return res_inner();
    }
    T result_or(const T& def) const
    {
        return (state == IS_T) ? t : def;
    }

    E& err_inner()
    {
        if (state == IS_E)
            return e;
        fprintf(stderr, "%s: object not in error state, aborting.\n", __func__);
        abort();
    }
    const E& err_inner() const
    {
        if (state == IS_E)
            return e;
        fprintf(stderr, "%s: object not in error state, aborting.\n", __func__);
        abort();
    }
    E& err() &
    {
        return err_inner();
    }
    E err() const &&
    {
        return err_inner();
    }
    const E& err() const &
    {
        return err_inner();
    }
    template <typename Fn>
    result or_else(Fn&& fn) &&
    {
        if (is_ok())
            return std::move(*this);
        else
            return fn(err());
    }
    // Fn: (T) -> new_type
    template <typename Fn>
    result<typename std::result_of<Fn(T)>::type, E> map(Fn&& fn) &&
    {
        if (is_ok())
            return fn(std::move(res()));
        else
            return result<typename std::result_of<Fn(T)>::type, E>(std::move(err()));
    }
    template <typename Fn>
    result<typename std::result_of<Fn(T)>::type, E> map(Fn&& fn) const &
    {
        if (is_ok())
            return fn(res());
        else
            return result<typename std::result_of<Fn(T)>::type, E>(err());
    }
    result<ok, E> as_ok()
    {
        if (is_ok())
            return result<ok, E>{ok{}};
        else
            return result<ok, E>(err());
    }
    // Fn: (E&&) -> new_type
    template <typename Fn>
    result<T, typename std::result_of<Fn(E)>::type> map_err(Fn&& fn) &&
    {
        if (is_ok())
            return result<T, typename std::result_of<Fn(E)>::type>(std::move(res()));
        else
            return fn(std::move(err()));
    }
    // Fn: (T&&) -> result<new_type, E>
    template <typename Fn>
    typename std::result_of<Fn(T&&)>::type and_then(Fn&& fn) &&
    {
        using result_t = typename std::result_of<Fn(T&&)>::type;
        static_assert(std::is_same<typename result_t::error_type, E>::value, "error types don't match");
        if (is_ok())
            return fn(std::move(res_inner()));
        else
            return result_t(std::move(err()));
    }
    // Fn: (E) -> T
    template <typename Fn>
    T unwrap_or_else(Fn&& fn) &&
    {
        if (is_ok())
            return std::move(res_inner());
        return fn(std::move(err()));
    }

    constexpr bool is_ok() const { return state == IS_T; }
    constexpr bool is_err() const { return !is_ok(); }
    constexpr explicit operator bool() const { return is_ok(); }
};

namespace impl
{

template <typename Res>
struct wrap_try_result {
    using type = typename Res::success_type;
};
template <typename Res>
struct wrap_try_result<Res&> {
    using type = std::reference_wrapper<typename Res::success_type>;
};

template <typename T>
struct unwrap_reference_wrapper_t {
    using ret = T;
    static T unwrap(T&& t) { return std::forward<T>(t); }
};
template <typename T>
struct unwrap_reference_wrapper_t<std::reference_wrapper<T>> {
    using ret = T&;
    static T& unwrap(std::reference_wrapper<T> t) { return t; }
};
template <typename T>
typename unwrap_reference_wrapper_t<T>::ret unwrap_reference_wrapper(T t) { return unwrap_reference_wrapper_t<T>::unwrap(std::forward<T>(t)); }

} // namespace impl

} // namespace common

/**
 * Expands to the result or immediately
 * returns from the current function with error.
 *
 *     result<bar, error_type> do_all_the_frob()
 *     {
 *         result<foo, error_type> res = frob();
 *         foo& f = result_try(res);
 *         return foo_to_bar(f);
 *     }
 *
 */
#ifndef COMMON_RESULT_NO_MACROS
#define result_try(x) common::impl::unwrap_reference_wrapper( ({          \
    decltype((x)) ref = (x);                                              \
    if (!ref)                                                             \
        return ref.err();                                                 \
    using wrap_type = common::impl::wrap_try_result<decltype(ref)>::type; \
    std::forward<wrap_type>(ref.res());                                   \
}) )
#endif

#endif // COMMON_RESULT_HPP
