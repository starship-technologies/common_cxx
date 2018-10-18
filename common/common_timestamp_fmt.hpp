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

#ifndef COMMON_TIMESTAMP_FMT_HPP
#define COMMON_TIMESTAMP_FMT_HPP

#include <ctime>
#include "common/string_view.hpp"

namespace common
{

inline size_t timestamp_fmt(array_view<char> to, const char* strformat, common::timestamp stamp)
{
    struct tm tm;
    std::time_t unixtime = stamp.secs;
    ::gmtime_r(&unixtime, &tm);
    return ::strftime(to.data(), to.size(), strformat, &tm);
}

inline common::string_view timestamp_fmt_str(array_view<char> to, const char* strformat, common::timestamp stamp)
{
    const size_t bytes = timestamp_fmt(to, strformat, stamp);
    return common::string_view{to}.head(bytes);
}

} // namespace common

#endif // COMMON_TIMESTAMP_FMT_HPP
