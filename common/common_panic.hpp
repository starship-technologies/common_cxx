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

#ifndef COMMON_PANIC_HPP
#define COMMON_PANIC_HPP

#ifndef COMMON_PANIC_HANDLER
#include <cstdio>
#include <cstdlib>

namespace common
{
namespace impl
{
inline void default_panic [[noreturn]] (const char* full_function_name, const char* msg, const char* file, size_t line)
{
    fprintf(stderr, "** panic: %s(%s:%zu): %s [aborting]\n", full_function_name, file, line, msg);
    ::abort();
}
} // namespace impl
} // namespace common

#define COMMON_PANIC_HANDLER impl::default_panic
#endif

#define COMMON_PANIC(msg) COMMON_PANIC_HANDLER(__PRETTY_FUNCTION__, msg, __FILE__, __LINE__)

#endif // COMMON_PANIC_HPP
