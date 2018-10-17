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

#ifndef COMMON_UNIX_ERR
#define COMMON_UNIX_ERR

#include <cstring>
#include <errno.h>

namespace common
{

struct unix_err
{
    constexpr unix_err(int unix_errno) : unix_errno(unix_errno) {}
    static unix_err current() { return unix_err{errno}; }

    const char* c_str() const { return ::strerror(unix_errno); }

    constexpr bool operator==(const unix_err& other) const { return unix_errno == other.unix_errno; }
    constexpr bool operator!=(const unix_err& other) const { return !(*this == other); }
    constexpr bool operator==(int other_errno) const { return unix_errno == other_errno; }

    int unix_errno = 0;
};

inline std::string to_string(unix_err e) { return e.c_str(); }
inline const char* to_cstr(unix_err e) { return e.c_str(); }

} // namespace common

#endif // COMMON_UNIX_ERR
