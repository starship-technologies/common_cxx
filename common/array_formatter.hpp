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

#ifndef COMMON_ARRAY_FORMATTER_HPP
#define COMMON_ARRAY_FORMATTER_HPP

#include "string_view.hpp"
#include <array>

namespace common
{

template <size_t Size>
struct array_formatter
{
    std::array<char, Size> buf {{'\0'}};
    size_t bytes_remaining = Size;

    array_formatter() = default;
    array_formatter(const char* format_string, ...) __attribute__ ((format (printf, 2, 3)))
    {
        va_list args;
        va_start(args, format_string);
        formatv(format_string, args);
        va_end(args);
    }
    void format(const char* format_string, ...) __attribute__ ((format (printf, 2, 3)))
    {
        va_list args;
        va_start(args, format_string);
        formatv(format_string, args);
        va_end(args);
    }
    void formatv(const char* const format, va_list args)
    {
        update_remaining(remaining().formatv_advance(format, args));
    }
    void update_remaining(common::string_view_writeable rem)
    {
        bytes_remaining = rem.size() + 1;
    }
    common::string_view_writeable remaining()
    {
        return common::string_view_writeable{buf}.tail(bytes_remaining).head_without(1);
    }

    common::string_view str() const
    {
        return common::string_view{buf.data(), size_t(Size - bytes_remaining)};
    }
    std::string to_string() const
    {
        return str().to_string();
    }
    const char* c_str() const
    {
        return buf.data();
    }
    void clear()
    {
        bytes_remaining = Size;
    }

    operator common::string_view() const
    {
        return str();
    }
};

} // namespace common


#endif // COMMON_ARRAY_FORMATTER_HPP
