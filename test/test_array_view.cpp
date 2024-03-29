#include <gtest/gtest.h>
#include "common/array_view.hpp"
#include "common/reverse_iterable.hpp"
#include "common/string_view.hpp"

using common::array_view;

TEST(reverse, simple_reverse) {
    int seq[] = {1, 2, 3, 4, 5};
    array_view<int> v{seq};
    int last = -1;
    for (auto it = v.rbegin(); it != v.rend(); ++it) {
        last = *it;
    }
    EXPECT_EQ(*v.rbegin(), 5);
    EXPECT_EQ(last, 1);
}

TEST(reverse, wrapper) {
    int seq[] = {1, 2, 3, 4, 5};
    array_view<int> v{seq};
    int last = -1;
    for (auto val : common::reverse_iterable(v)) {
        last = val;
    }
    EXPECT_EQ(last, 1);
}

TEST(reverse, independent) {
    int last = -1;
    for (const int& item : common::reverse_iterable(std::vector<int>{1, 2, 3})) {
        if (last == -1) {
            EXPECT_EQ(item, 3);
        }
        last = item;
    }
    EXPECT_EQ(last, 1);
}

#ifdef __cpp_lib_ranges
TEST(ranges, project) {
    struct puzzle_box {
        bool match;
        common::string_view key;
    };
    std::array boxes = {
        puzzle_box { .match = false, .key = "a pitter patter of tiny feet" },
        puzzle_box { .match = true,  .key = "a gentle wind" },
        puzzle_box { .match = false, .key = "five times fast" },
    };
    common::optional<puzzle_box&> found = common::array_view(boxes).find("a gentle wind", &puzzle_box::key);
    EXPECT_EQ(found.is_some(), 1);
    EXPECT_EQ(found->match, true);

    found = common::array_view(boxes).find_if([] (common::string_view s) { return s.find_opt("gentle").is_some(); }, &puzzle_box::key);
    EXPECT_EQ(found.is_some(), 1);
    EXPECT_EQ(found->match, true);
}
#endif

struct noncopyable {
    noncopyable() = default;
    noncopyable(noncopyable&&) = default;
    int i = 5;
};

auto opt() -> common::optional<std::string&>
{
    static std::string yo("yo");
    return yo;
}

TEST(opt, opt) {
    opt().map([] (const auto& s) { return s.c_str(); }).get_or("");
}


TEST(optional, noncopyable) {
    using common::optional;
    optional<float> f{5.0};
    optional<float&> fr{f.get()};
    optional<const float&> fcr{f.get()};

    optional<const float&> to = fr;
    optional<const float&> to2 = fcr;

    noncopyable nc;
    optional<noncopyable> oo = std::move(nc);
    optional<noncopyable> ooo = std::move(oo).map([] (noncopyable&& nc) { nc.i = 4; return std::move(nc); });
    EXPECT_EQ(ooo.get().i, 4);
}

