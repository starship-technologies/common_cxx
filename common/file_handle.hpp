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

#ifndef COMMON_FILE_HANDLE_HPP
#define COMMON_FILE_HANDLE_HPP

#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <string>

#include "common_result.hpp"
#include "unix_err.hpp"
#include "array_view.hpp"

namespace common
{

/**
 * A trivial RAII FILE* wrapper,
 * with a few convenience functions
 * for reading, writing and size.
 */
struct file_handle
{
    FILE* file{nullptr};
    constexpr file_handle() = default;
    file_handle(const char* filename, const char* mode = "r")
    {
        open(filename, mode);
    }
    result<ok, unix_err> open(const char* filename, const char* mode = "r")
    {
        close();
        file = ::fopen(filename, mode);
        int err = errno;
        if (!file)
            return unix_err(err);
        return ok{};
    }
    file_handle(file_handle&& other)
    {
        std::swap(file, other.file);
    }
    file_handle& operator=(file_handle&& other)
    {
        std::swap(file, other.file);
        return *this;
    }
    constexpr bool good() const
    {
        return is_open();
    }
    constexpr bool is_open() const
    {
        return (file != nullptr);
    }
    size_t size() const
    {
        if (!good())
            return 0;
        struct stat stat_buf;
        int err = ::fstat(fileno(file), &stat_buf);
        if (err == 0)
            return stat_buf.st_size;
        return 0;
    }
    result<ok, unix_err> write(array_view<const char> bytes)
    {
        if (!good())
            return unix_err{ENOENT};
        int ret = ::fwrite(bytes.data(), bytes.size(), 1, file);
        if (ret == 1)
            return ok{};
        return unix_err::current();
    }
    result<ok, unix_err> read(array_view<char> into)
    {
        if (!good())
            return unix_err{ENOENT};
        int ret = ::fread(into.data(), into.size(), 1, file);
        if (ret == 1)
            return ok{};
        return unix_err::current();
    }
    result<ok, unix_err> read_all(std::string& into)
    {
        if (!good())
            return unix_err{ENOENT};
        into.resize(size());
        return read(array_view<char>{&*into.begin(), into.size()});
    }
    void close()
    {
        if (file) {
            fclose(file);
            file = nullptr;
        }
    }
    ~file_handle()
    {
        if (file)
            fclose(file);
    }
    int fd()
    {
        return good() ? fileno(file) : -1;
    }
};


} // namespace common

#endif // COMMON_FILE_HANDLE_HPP
