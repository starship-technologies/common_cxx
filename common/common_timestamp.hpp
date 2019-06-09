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

#ifndef COMMON_TIMESTAMP_HPP
#define COMMON_TIMESTAMP_HPP

#include <cstdint>
#include <time.h>
#include <chrono>

namespace common
{

struct timestamp
{
    enum { SEC_NS = 1 * 1000 * 1000 * 1000 };

    uint32_t secs  = 0;
    uint32_t nsecs = 0;
    constexpr timestamp() = default;
    constexpr explicit timestamp(double s) : secs(s), nsecs((s - uint32_t(s)) * 1e9) {}
    constexpr double to_double() const { return secs + nsecs * 1e-9; }
    constexpr timestamp(uint32_t s, uint32_t ns) : secs(s), nsecs(ns) {}
    constexpr timestamp(const timestamp& other) = default;
    static constexpr timestamp from_nanos(uint64_t nanos) { return timestamp{uint32_t(nanos / SEC_NS), uint32_t(nanos % SEC_NS)}; }
    template <typename T>
    static constexpr timestamp from_chrono_duration(T dur) { return from_nanos(std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count()); }

    constexpr static timestamp zero() { return timestamp(); }
    constexpr explicit operator bool() const { return secs || nsecs; }

    constexpr bool operator>(const timestamp& other) const { return (secs > other.secs) || ( (secs == other.secs) && (nsecs > other.nsecs) ); }
    constexpr bool operator>=(const timestamp& other) const { return (secs > other.secs) || ( (secs == other.secs) && (nsecs >= other.nsecs) ); }
    constexpr bool operator<(const timestamp& other) const { return (secs < other.secs) || ( (secs == other.secs) && (nsecs < other.nsecs) ); }
    constexpr bool operator<=(const timestamp& other) const { return (secs < other.secs) || ( (secs == other.secs) && (nsecs <= other.nsecs) ); }
    constexpr bool operator==(const timestamp& other) const { return (secs == other.secs) && (nsecs == other.nsecs); }
    constexpr bool operator!=(const timestamp& other) const { return (secs != other.secs) || (nsecs != other.nsecs); }

    constexpr double operator-(const timestamp& other) const { return to_double() - other.to_double(); }
    timestamp operator+(const timestamp& other) const
    {
        common::timestamp ret{secs + other.secs, nsecs + other.nsecs};
        if (ret.nsecs >= SEC_NS) {
            ret.secs += ret.nsecs / SEC_NS;
            ret.nsecs = ret.nsecs % SEC_NS;
        }
        return ret;
    }
    constexpr timestamp minus_seconds(double secs) const
    {
        return timestamp{*this - timestamp{secs}};
    }

    static timestamp now()
    {
        struct timespec ts;
        ::clock_gettime(CLOCK_REALTIME, &ts);
        return common::timestamp(ts.tv_sec, ts.tv_nsec);
    }
    template <typename T>
    constexpr static timestamp cvt(const T& t)
    {
        return timestamp{t.sec, t.nsec};
    }
    static constexpr timestamp min_value()
    {
        return timestamp{0, 0};
    }
    static constexpr timestamp max_value()
    {
        return timestamp{std::numeric_limits<decltype(secs)>::max(), 999999999};
    }
};

} // namespace common

#endif // COMMON_TIMESTAMP_HPP
