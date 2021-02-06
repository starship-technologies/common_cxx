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

#ifndef COMMON_REVERSE_ITERABLE_HPP
#define COMMON_REVERSE_ITERABLE_HPP

#include <iterator>

namespace common
{

template <typename T>
struct reverse_wrapped
{
    T wrapped;
    auto begin() { return std::rbegin(wrapped); }
    auto end()   { return std::rend(wrapped);   }
};

/**
 * Reverse any iterable that supports std::rbegin()/rend()
 *
 * for (const int& item : common::reverse_iterable(std::vector{1, 2, 3}))
 *  printf("item: %d\n", item);
 */
template <typename T>
reverse_wrapped<const T&> reverse_iterable(const T& iterable) { return {iterable}; }
template <typename T>
reverse_wrapped<T&> reverse_iterable(T& iterable) { return {iterable}; }
template <typename T>
reverse_wrapped<T> reverse_iterable(T&& iterable) { return {std::move(iterable)}; }

} // namspace common

#endif // COMMON_REVERSE_ITERABLE_HPP
