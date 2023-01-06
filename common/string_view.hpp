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

#ifndef COMMON_STRING_VIEW_HPP
#define COMMON_STRING_VIEW_HPP

#include "common/array_view.hpp"
#include "common/common_optional.hpp"
#include "common/shared_defines.hpp"

#include <string>
#include <algorithm>
#include <cstring>

#include <stdarg.h>

#ifdef __cpp_impl_three_way_comparison
#include <compare>
#endif

namespace common
{

namespace impl
{
template <typename T>
size_t strlen(const T* null_terminated)
{
    return std::char_traits<T>::length(null_terminated);
}
#ifdef __has_builtin
#define COMMON_HAS_BUILTIN(x) __has_builtin(x)
#else
#define COMMON_HAS_BUILTIN(x) false
#endif
#if COMMON_HAS_BUILTIN(__builtin_strlen) \
 || (defined(__GNUC__) && !defined(__clang__))
template <>
constexpr size_t strlen<char>(const char* null_terminated)
{
    return __builtin_strlen(null_terminated);
}
#endif
}

template <class T>
struct basic_string_view : public array_view<T>
{
    constexpr basic_string_view() = default;
    constexpr basic_string_view(T* p, size_t c)
    : array_view<T>(p, c)
    {}
    constexpr basic_string_view(T* p, T* e)
    : array_view<T>(p, e)
    {}
    basic_string_view(const std::string& str)
    : array_view<T>(str.data(), str.size())
    {}
    basic_string_view(std::string& str)
    : array_view<T>(&str.front(), str.size())
    {}
    template <size_t N>
    constexpr basic_string_view(T (&s)[N])
    : array_view<T>(s, N-1)
    {}
    template <size_t N>
    constexpr basic_string_view(std::array<T, N>& array)
    : array_view<T>(array.data(), array.size())
    {}
    constexpr basic_string_view(T* null_terminated)
    : array_view<T>(null_terminated, impl::strlen(null_terminated))
    {
    }
#if !defined(__clang) && __GNUC__ == 4
    // Pointless template parameter to prevent ambiguous
    // constructor errors on G++ 4.8. This pushes the
    // array view constructor to the same tier of resolution
    // as the std::array constructor.
    template <typename = void>
#endif
    explicit basic_string_view(array_view<T> av)
    : array_view<T>(av)
    {}

    using array_view<T>::data;
    using array_view<T>::size;

    using const_type = typename std::add_const<T>::type;
    using mutable_type = typename std::remove_const<T>::type;

    template <typename U, typename = typename std::enable_if<std::is_same<const_type, U>::value && !std::is_same<T, U>::value>::type>
    constexpr operator basic_string_view<U>() const { return basic_string_view<const_type>(data(), size()); }

    constexpr bool operator==(const basic_string_view<T>& other) const
    {
        return array_view<T>::operator==(other);
    }
    constexpr bool operator!=(const basic_string_view<T>& other) const
    {
        return !(*this == other);
    }
    constexpr bool operator<(const basic_string_view& other) const
    {
        const int cmp = ::strncmp(data(), other.data(), std::min(size(), other.size()));
        if (cmp != 0)
            return (cmp < 0);
        return size() < other.size();
    }
    constexpr int compare(const basic_string_view& other) const
    {
        const int cmp = ::strncmp(data(), other.data(), std::min(size(), other.size()));
        if (cmp != 0)
            return cmp;
        return (size() < other.size()) ? -1 : (size() > other.size()) ? 1 : 0;
    }
#ifdef __cpp_impl_three_way_comparison
    friend constexpr std::strong_ordering operator<=>(const basic_string_view& a, const basic_string_view& b)
    {
        const int cmp = a.compare(b);
        return (cmp == 0) ? std::strong_ordering::equal
             : (cmp < 0)  ? std::strong_ordering::less
             :              std::strong_ordering::greater;
    }
#endif

    std::string to_string() const
    {
        return std::string{data(), size()};
    }
    // to support printf("%.*s", view.sizei(), view.data()) sans cast
    constexpr int sizei() const
    {
        return static_cast<int>(this->size());
    }

    bool begins_with(basic_string_view<const_type> prefix) const
    {
        if (size() < prefix.size())
            return false;
        return this->head(prefix.size()) == prefix;
    }
    bool ends_with(basic_string_view<const_type> suffix) const
    {
        if (size() < suffix.size())
            return false;
        return this->tail(suffix.size()) == suffix;
    }

    constexpr basic_string_view head(size_t num) const { return basic_string_view{array_view<T>::head(num)}; }
    constexpr basic_string_view tail(size_t num) const { return basic_string_view{array_view<T>::tail(num)}; }
    constexpr basic_string_view head_without(size_t num) const { return basic_string_view{array_view<T>::head_without(num)}; }
    constexpr basic_string_view tail_without(size_t num) const { return basic_string_view{array_view<T>::tail_without(num)}; }

    basic_string_view clip_head(basic_string_view<const_type> substr) const { return begins_with(substr) ? tail_without(substr.size()) : *this; }
    basic_string_view clip_tail(basic_string_view<const_type> substr) const { return ends_with(substr) ? head_without(substr.size()) : *this; }

    template <typename S, class Fn>
    size_t split_fn_any(S c, Fn&& fn) const
    {
        basic_string_view<T> remaining = *this;
        size_t count = 0;

        while (remaining.size()) {
            const T* pos = remaining.find_in(c);
            if (!pos)
                pos = remaining.end();
            if (pos != remaining.begin()) {
                fn(basic_string_view<T>{remaining.begin(), pos});
                count += 1;
            }

            if (pos == remaining.end())
                break;
            remaining = {pos + pattern_length(c), remaining.end()};
        }
        return count;
    }

    enum split_flag
    {
        none              = 0x00,
        last_captures_all = 0x01,
        skip_empty        = 0x02,
    };
    struct split_def
    {
        T c;
        split_flag flags;
        split_def(T c, split_flag flags = none) : c{c}, flags{flags} {}
    };
    enum class pattern_type { str, any_char };
    template <pattern_type P>
    struct split_def_long_base
    {
        basic_string_view pattern;
        split_flag flags;
        split_def_long_base(basic_string_view p, split_flag flags = none) : pattern{p}, flags{flags} {}
    };
    template <typename Fn>
    size_t split_fn(split_def def, Fn&& fn) const
    {
        return split_fn_any(def, std::move(fn));
    }
    template <typename Fn, pattern_type P>
    size_t split_fn(split_def_long_base<P> def, Fn&& fn) const
    {
        return split_fn_any(def, std::move(fn));
    }
    static size_t pattern_length(split_def)
    {
        return 1;
    }
    using split_def_string = split_def_long_base<pattern_type::str>;
    using split_def_any_char = split_def_long_base<pattern_type::any_char>;
    struct splitter {
        using flag = split_flag;
        static split_def_string string(basic_string_view pattern, split_flag flags = split_flag::none)
        {
            return split_def_string{pattern, flags};
        }
        static split_def_any_char any_char(basic_string_view chars, split_flag flags = split_flag::none)
        {
            return split_def_any_char{chars, flags};
        }
    };
    static size_t pattern_length(const split_def_string& def)
    {
        return def.pattern.size();
    }
    static size_t pattern_length(const split_def_any_char&)
    {
        return 1;
    }
    const T* find_in(const split_def_string& def) const
    {
#ifndef _WIN32
        const T* res = (const T*) ::memmem(data(), size() * sizeof(T), def.pattern.data(), def.pattern.size() * sizeof(T));
#else
        const T* res = std::search(this->begin(), this->end(), def.pattern.begin(), def.pattern.end());
        if (res == this->end())
            return nullptr;
#endif
        return res;
    }
    const T* find_in(const split_def_any_char& def) const
    {
        auto it = std::find_if(this->begin(), this->end(), [&def] (T chr) {
            return ::memchr(def.pattern.data(), chr, def.pattern.size()) != nullptr;
        });
        if (it != this->end())
            return &*it;
        return nullptr;
    }
    const T* find_in(const split_def& def) const
    {
        return (const T*) ::memchr(data(), def.c, size());
    }
    template <typename S>
    common::optional<size_t> split_arg_single(const S& c, basic_string_view& arg) const
    {
        const T* found = find_in(c);
        if (!found) {
            arg = *this;
            return common::none{};
        }
        arg = basic_string_view{data(), found};
        return arg.size();
    }
    template <class S>
    common::optional<size_t> split_arg(S splitter, basic_string_view& arg, basic_string_view& remaining) const
    {
        auto skip_count = split_arg_single(splitter, arg);
        if (!skip_count)
            return skip_count;
        remaining = advance(*skip_count + pattern_length(splitter));
        while (!skip_count.get() && (splitter.flags & skip_empty) && remaining.size()) {
            skip_count = remaining.split_arg_single(splitter, arg);
            if (!skip_count)
                break;
            remaining = remaining.advance(*skip_count + pattern_length(splitter));
        }
        return skip_count;
    }
    template <class S>
    size_t split_args_any(S splitter, basic_string_view& arg) const
    {
        auto remaining = *this;
        split_arg(splitter, arg, remaining);
        if (splitter.flags & last_captures_all)
            arg = {arg.begin(), remaining.end()};
        return 1;
    }
    template <class S, typename... Args>
    size_t split_args_any(S splitter, basic_string_view& arg, Args&... args) const
    {
        auto remaining = *this;
        common::optional<size_t> read_count = split_arg(splitter, arg, remaining);
        if (!read_count)
            return 1;
        return 1 + remaining.split_args_any(splitter, args...);
    }
    template <typename... Args>
    size_t split_args(split_def splitter, basic_string_view& arg, Args&... args) const
    {
        return split_args_any<split_def>(splitter, arg, args...);
    }
    template <pattern_type P, typename... Args>
    size_t split_args(split_def_long_base<P> splitter, basic_string_view& arg, Args&... args) const
    {
        return split_args_any<decltype(splitter)>(splitter, arg, args...);
    }

    common::optional<size_t> rsplit_arg_single(T c, basic_string_view& arg) const
    {
#ifdef _GNU_SOURCE
        const T* pos = (const T*) ::memrchr(data(), c, size());
#else
        auto it = std::find(this->rbegin(), this->rend(), c);
        const T* pos = (it == this->rend()) ? nullptr : &*it;
#endif
        if (!pos) {
            arg = *this;
            return common::none{};
        }
        arg = basic_string_view<T>{pos + 1, this->end()};
        return arg.size();
    }
    common::optional<size_t> rsplit_arg(split_def c, basic_string_view& arg, basic_string_view& remaining) const
    {
        auto read_count = rsplit_arg_single(c.c, arg);
        if (!read_count)
            return read_count;
        remaining = head_without(*read_count + 1);
        while (!read_count.get() && (c.flags & skip_empty) && remaining.size()) {
            read_count = remaining.rsplit_arg_single(c.c, arg);
            if (!read_count)
                break;
            remaining = remaining.head_without(*read_count + 1);
        }
        return read_count;
    }
    size_t rsplit_args(split_def c, basic_string_view& arg) const
    {
        auto remaining = *this;
        common::optional<size_t> read_count = rsplit_arg(c, arg, remaining);
        if (c.flags & last_captures_all)
            arg = {remaining.begin(), arg.end()};
        return 1;
    }
    template <typename... Args>
    size_t rsplit_args(split_def c, basic_string_view& arg, Args&... args) const
    {
        auto remaining = *this;
        common::optional<size_t> read_count = rsplit_arg(c, arg, remaining);
        if (!read_count)
            return 1;
        return 1 + remaining.rsplit_args(c, args...);
    }
    common::optional<basic_string_view> find_opt_str(basic_string_view<const_type> needle) const
    {
#ifndef _WIN32
        void* found = ::memmem(static_cast<const void*>(data()),        size()        * sizeof(T),
                               static_cast<const void*>(needle.data()), needle.size() * sizeof(T));
        if (!found)
            return common::none{};
#else
        T* found = std::search(this->begin(), this->end(), needle.begin(), needle.end());
        if (found == this->end())
            return common::none{};
#endif
        return basic_string_view{static_cast<T*>(found), this->end()};
    }
    common::optional<size_t> find_opt(basic_string_view<const_type> needle) const
    {
        auto found = find_opt_str(needle);
        if (!found)
            return common::none{};
        return found->data() - data();
    }
    enum : size_t { npos = size_t(-1) };
    size_t find(basic_string_view<const_type> needle) const
    {
        return find_opt(needle).get_or(npos);
    }
    common::optional<size_t> find_char(T c) const
    {
        const T* pos = (const T*) memchr(data(), c, size());
        if (!pos)
            return common::none{};
        return pos - data();
    }
    common::optional<size_t> rfind_char(T c) const
    {
        const T* pos = (const T*) memrchr(data(), c, size());
        if (!pos)
            return common::none{};
        return pos - data();
    }
    basic_string_view dirname() const
    {
        basic_string_view file, dir;
        rsplit_args({'/', last_captures_all}, file, dir);
        return dir;
    }
    basic_string_view basename() const
    {
        basic_string_view file, dir;
        rsplit_args({'/', last_captures_all}, file, dir);
        return file;
    }

    template <typename = typename std::enable_if<!std::is_const<T>::value>>
    int formatv(const char* format_string, va_list args) const
    {
        if (!size())
            return 0;
        return vsnprintf(data(), size(), format_string, args);
    }
    template <typename = typename std::enable_if<!std::is_const<T>::value>>
    basic_string_view formatv_advance(const char* format_string, va_list args) const
    {
        if (!size())
            return *this;
        int ret = formatv(format_string, args);
        if (ret <= 0)
            return *this;
        return advance(ret);
    }
    template <typename = typename std::enable_if<!std::is_const<T>::value>>
    int format(const char* format_string, ...) const COMMON_FORMAT_VALIDATE(2, 3);
    template <typename = typename std::enable_if<!std::is_const<T>::value>>
    basic_string_view format_advance(const char* format_string, ...) const COMMON_FORMAT_VALIDATE(2, 3);

    constexpr basic_string_view advanced(size_t chars) const
    {
        return basic_string_view {data() + std::min(chars, size()), size() - std::min(chars, size())};
    }
    constexpr basic_string_view advance(size_t chars) const { return advanced(chars); }
    constexpr basic_string_view advanced_by_str(basic_string_view<const_type> str) const
    {
        return advanced(str.size());
    }
};

template <typename T> template <typename>
int basic_string_view<T>::format(const char* format_string, ...) const
{
    va_list args;
    va_start(args, format_string);
    int ret = formatv(format_string, args);
    va_end(args);
    return ret;
}
template <typename T> template <typename>
basic_string_view<T> basic_string_view<T>::format_advance(const char* format_string, ...) const
{
    va_list args;
    va_start(args, format_string);
    basic_string_view<T> ret = formatv_advance(format_string, args);
    va_end(args);
    return ret;
}

using string_view = basic_string_view<const char>;
using string_view_writeable = basic_string_view<char>;

} // namespace common

#ifndef COMMON_STRING_NO_STRING_OPERATORS
template <typename T>
std::basic_string<T> operator+(std::basic_string<T>&& string, common::basic_string_view<const T> view)
{
    string.append(view.data(), view.size());
    return std::move(string);
}
template <typename T>
std::basic_string<T>& operator+=(std::basic_string<T>& string, common::basic_string_view<const T> view)
{
    string.append(view.data(), view.size());
    return string;
}
template <typename T>
std::basic_string<T> operator+(const std::basic_string<T>& string_ref, common::basic_string_view<const T> view)
{
    auto string = string_ref;
    string.append(view.data(), view.size());
    return string;
}
#endif // !COMMON_STRING_NO_STRING_OPERATORS

#endif // COMMON_STRING_VIEW_HPP
