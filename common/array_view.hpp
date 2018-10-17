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

#ifndef COMMON_ARRAY_VIEW_HPP
#define COMMON_ARRAY_VIEW_HPP

#include <vector>
#include <array>
#include "common_optional.hpp"

namespace common
{

struct backindex
{
    constexpr explicit backindex(size_t ind) : count_back(ind) {}
    constexpr size_t to_index(size_t array_size) const { return array_size - (count_back + 1); }
private:
    size_t count_back;
};

template <class T>
struct array_view
{
    T* ptr       = nullptr;
    size_t count = 0;
    using const_type = typename std::add_const<T>::type;
    using mutable_type = typename std::remove_const<T>::type;
    
    constexpr array_view() = default;
    constexpr array_view(T* p, size_t c)
    : ptr(p), count(c)
    {}
    constexpr array_view(T* begin, T* end)
    : ptr(begin), count((end > begin) ? (end - begin) : 0)
    {}
    template <typename U>
    array_view(std::vector<T, U>& vec)
    : ptr(vec.data()), count(vec.size())
    {}
    template <typename U, typename = typename std::enable_if<std::is_same<const_type, T>::value>>
    array_view(const std::vector<mutable_type, U>& vec)
    : ptr(vec.data()), count(vec.size())
    {}
    template <size_t N>
    constexpr array_view(std::array<T, N>& array)
    : ptr(array.data()), count(N)
    {}
    template <size_t N, typename = typename std::enable_if<std::is_same<const_type, T>::value>>
    constexpr array_view(const std::array<mutable_type, N>& array)
    : ptr(array.data()), count(N)
    {}
    template <size_t N>
    constexpr array_view(T (&s)[N])
    : ptr(s), count(N)
    {}

    using iterator = T*;
    constexpr iterator begin() const { return ptr; }
    constexpr iterator end()   const { return ptr + count; }

    constexpr T& front() const { return *(ptr); }
    constexpr T& back()  const { return *(ptr + count - 1); }

    constexpr T*     data() const { return ptr; }
    constexpr size_t size() const { return count; }
    constexpr bool  empty() const { return size() == 0; }

    constexpr array_view<T> head(size_t num) const { return array_view<T>{ptr, std::min(count, num)}; }
    constexpr array_view<T> tail(size_t num) const { return array_view<T>{ptr + count-std::min(count, num), std::min(count, num)}; }

    constexpr array_view<T> head_without(size_t num) const { return array_view<T>{ptr, count - std::min(count, num)}; }
    constexpr array_view<T> tail_without(size_t num) const { return array_view<T>{ptr + std::min(count, num), count - std::min(count, num)}; }

    /// Contiguous elements from begin() while fn(elem&) returns true
    template <typename Fn>
    array_view<T> head_while(Fn&& fn) const {
        iterator it;
        for (it = begin(); it != end(); ++it)
            if (!fn(*it))
                break;
        return array_view{begin(), it};
    }
    /// Contiguous elements from end() while fn(elem&) returns true
    template <typename Fn>
    array_view<T> tail_while(Fn&& fn) const {
        iterator it;
        for (it = end(); it != begin(); --it)
            if (!fn(*(it - 1)))
                break;
        return array_view{it, end()};
    }

    constexpr T& operator[](size_t index) const { return ptr[index]; }
    constexpr T& operator[](backindex bi) const { return ptr[bi.to_index(this->count)]; }

    constexpr T at_or_default(size_t index, T def = T{}) const { return (index < count) ? ptr[index] : def; }
    constexpr T at_or_default(backindex bi, T def = T{}) const { return at_or_default(bi.to_index(this->count), std::move(def)); }

    constexpr common::optional<T&> at_opt(size_t index) const { return (index < count) ? common::optional<T&>(ptr[index]) : common::optional<T&>{}; }
    constexpr common::optional<T&> at_opt(backindex bi) const { return at_opt(bi.to_index(this->count)); }

    template <typename U, typename = typename std::enable_if<std::is_same<const_type, U>::value && !std::is_same<T, U>::value>::type>
    operator array_view<U>() { return array_view<const_type>(ptr, count); }

    array_view<const_type> to_const() const { return array_view<const_type>{ptr, count}; }

    array_view window(size_t index, size_t window_size) const
    {
        return advance(index * window_size).head(window_size);
    }
    void overwrite_with(const array_view<const_type> from)
    {
        std::copy(from.begin(), from.end(), begin());
    }
    void overwrite_with(const std::initializer_list<const_type> from)
    {
        std::copy(from.begin(), from.end(), begin());
    }
    std::vector<mutable_type> to_owned() const
    {
        return std::vector<mutable_type>(begin(), end());
    }
    constexpr bool operator==(const array_view<T>& other) const
    {
        return (count == other.count) && std::equal(begin(), end(), other.begin());
    }
    constexpr bool operator!=(const array_view<T>& other) const
    {
        return !(*this == other);
    }
    constexpr array_view advance(size_t elems) const
    {
        return array_view {data() + std::min(elems, size()), size() - std::min(elems, size())};
    }
    /// cast to same-sized child
    template <typename U, typename = typename std::enable_if<sizeof(T) == sizeof(U) && std::is_base_of<T, U>::value>::type>
    array_view<U> reinterpret_child() const
    {
        return array_view<U>{reinterpret_cast<U*>(data()), size()};
    }
    /// allow reintepreting array_view<char> as array_view<U>
    template <typename U, typename T1 = T, typename = typename std::enable_if<std::is_same<T1, char>::value>::type>
    constexpr array_view<U> reinterpret_as() const
    {
        return array_view<U>{reinterpret_cast<U*>(data()), size() * sizeof(T) / sizeof(U)};
    }
    /// allow reintepreting array_view<const char> as array_view<const U>
    template <typename U, typename T1 = T, typename = typename std::enable_if<std::is_same<T1, const char>::value>::type>
    constexpr array_view<const U> reinterpret_as() const
    {
        return array_view<const U>{reinterpret_cast<const U*>(data()), size() * sizeof(T) / sizeof(U)};
    }

    array_view& operator<<(const T& item)
    {
        auto& view = *this;
        view[0] = item;
        view = view.advance(1);
        return view;
    }
};

template <class T>
constexpr array_view<T> inline make_array_view(T* ptr, size_t count)
{
    return array_view<T>{ptr, count};
}

template <class T, typename U>
array_view<T> inline make_array_view(std::vector<T, U>& vec)
{
    return array_view<T>{vec.data(), vec.size()};
}

template <class T>
array_view<const T> inline make_array_view(const std::vector<T>& vec)
{
    return array_view<const T>{vec.data(), vec.size()};
}

template <class T, size_t N>
constexpr array_view<T> inline make_array_view(std::array<T, N>& array)
{
    return array_view<T>{array.data(), N};

}
template <class T, size_t N>
constexpr array_view<const T> inline make_array_view(const std::array<T, N>& array)
{
    return array_view<const T>{array.data(), N};
}

template <class T>
constexpr array_view<T> inline make_array_view_single(T& object)
{
    return array_view<T>{&object, 1};
}

template <class T>
constexpr array_view<const T> inline make_array_view_single(const T& object)
{
    return array_view<const T>{&object, 1};
}

template <class T, size_t N>
constexpr array_view<T> inline make_array_view(T (&a)[N])
{
    return array_view<T>{a, N};
}

template <class A>
auto constexpr inline make_array_view_generic(A& a) -> array_view<typename std::remove_reference<decltype(*std::declval<A>().data())>::type> 
{
    return array_view<typename std::remove_reference<decltype(*a.data())>::type>(a.data(), a.size());
}

} // namespace common

#endif // COMMON_ARRAY_VIEW_HPP
