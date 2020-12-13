#include <gtest/gtest.h>
#include "common/string_view.hpp"

using common::string_view;

TEST(split_args, value_empty) {
    string_view key, value;
    string_view source{"key="};
    size_t count = source.split_args('=', key, value);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value, "");
}

TEST(split_args, normal) {
    string_view key, value;
    string_view source{"key=value"};
    size_t count = source.split_args('=', key, value);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value, "value");
}

TEST(split_args, no_split_character) {
    string_view key, value;
    string_view source{"key"};
    size_t count = source.split_args('=', key, value);
    EXPECT_EQ(count, 1);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value, "");
}

TEST(split_args, empty) {
    string_view key, value;
    string_view source{""};
    size_t count = source.split_args('=', key, value);
    EXPECT_EQ(count, 1);
    EXPECT_EQ(key, "");
    EXPECT_EQ(value, "");
}

TEST(split_args, last_captures) {
    string_view key, value;
    string_view source{"key=value=true"};
    size_t count = source.split_args({'=', string_view::last_captures_all}, key, value);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value, "value=true");
}

TEST(split_args, last_no_captures) {
    string_view key, value;
    string_view source{"key=value=true"};
    size_t count = source.split_args('=', key, value);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value, "value");
}

TEST(split_args, middle_empty) {
    string_view key, empty, value;
    string_view source{"key==value"};
    size_t count = source.split_args('=', key, empty, value);
    EXPECT_EQ(count, 3);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(empty, "");
    EXPECT_EQ(value, "value");
}

TEST(split_args, skip_empty) {
    string_view key, value;
    string_view source{"key==value"};
    size_t count = source.split_args({'=', string_view::skip_empty}, key, value);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value.to_string(), std::string("value"));
}

TEST(split_args, skip_empty_extra) {
    string_view key, value;
    string_view source{"key==value="};
    size_t count = source.split_args({'=', string_view::skip_empty}, key, value);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value.to_string(), std::string("value"));
}

TEST(split_args, skip_empty_multi_trailing) {
    string_view key, value;
    string_view source{"key==value=last"};
    size_t count = source.split_args({'=', string_view::skip_empty}, key, value);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value.to_string(), std::string("value"));
}

TEST(split_args, skip_empty_multi) {
    string_view key, value, last;
    string_view source{"key==value=last"};
    size_t count = source.split_args({'=', string_view::skip_empty}, key, value, last);
    EXPECT_EQ(count, 3);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value.to_string(), std::string("value"));
    EXPECT_EQ(last.to_string(), std::string("last"));
}

TEST(split_args, skip_empty_take_all) {
    string_view key, value;
    string_view source{"key====value=last"};
    string_view::split_flag flags{string_view::split_flag(string_view::skip_empty | string_view::last_captures_all)};
    size_t count = source.split_args({'=', flags}, key, value);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value.to_string(), std::string("value=last"));
}

TEST(split_args, normal_trailing) {
    string_view key, value;
    string_view source{"key=value=trailing"};
    size_t count = source.split_args('=', key, value);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value, "value");
}

TEST(split_advanced, string_pattern) {
    string_view key, value;
    string_view source{"key===value"};
    size_t count = source.split_args(string_view::splitter::string("==="), key, value);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value, "value");
}

TEST(split_advanced, string_pattern_longer) {
    string_view key, value;
    string_view source{"and you are the wind beneath my wings my dear"};
    size_t count = source.split_args(string_view::splitter::string(" are the wind beneath my wings "), key, value);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key, "and you");
    EXPECT_EQ(value, "my dear");
}

TEST(split_advanced, string_pattern_many) {
    string_view one, two, three;
    string_view source{"BEGIN SOURCE;\nNEXT LINE;\nFINAL INSTANTIATION;\n"};
    size_t count = source.split_args(string_view::splitter::string(";\n"), one, two, three);
    EXPECT_EQ(count, 3);
    EXPECT_EQ(one, "BEGIN SOURCE");
    EXPECT_EQ(two, "NEXT LINE");
    EXPECT_EQ(three, "FINAL INSTANTIATION");
}

TEST(split_advanced, split_any) {
    string_view key, value, final;
    string_view source{"key?value&final"};
    size_t count = source.split_args(string_view::splitter::any_char("&?"), key, value, final);
    EXPECT_EQ(count, 3);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value, "value");
    EXPECT_EQ(final, "final");
}

TEST(basename, normal) {
    string_view source{"/some/path"};
    string_view basename = source.basename();
    EXPECT_EQ(basename, "path");
}

TEST(dirname, normal) {
    string_view source{"/some/path"};
    string_view dirname = source.dirname();
    EXPECT_EQ(dirname.to_string(), std::string("/some"));
}

TEST(basename, trailing_slash) {
    string_view source{"/some/path/"};
    string_view basename = source.basename();
    EXPECT_EQ(basename.to_string(), std::string(""));
}

TEST(dirname, trailing_slash) {
    string_view source{"/some/path/"};
    string_view dirname = source.dirname();
    EXPECT_EQ(dirname.to_string(), std::string("/some/path"));
}

TEST(rsplit_args, value_empty) {
    string_view key, value;
    string_view source{"=key"};
    size_t count = source.rsplit_args('=', key, value);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key.to_string(), "key");
    EXPECT_EQ(value.to_string(), "");
}

TEST(rsplit_args, normal) {
    string_view key, value;
    string_view source{"key=value"};
    size_t count = source.rsplit_args('=', value, key);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value, "value");
}

TEST(rsplit_args, no_split_character) {
    string_view key, value;
    string_view source{"key"};
    size_t count = source.rsplit_args('=', value, key);
    EXPECT_EQ(count, 1);
    EXPECT_EQ(key, "");
    EXPECT_EQ(value, "key");
}

TEST(rsplit_args, empty) {
    string_view key, value;
    string_view source{""};
    size_t count = source.rsplit_args('=', key, value);
    EXPECT_EQ(count, 1);
    EXPECT_EQ(key, "");
    EXPECT_EQ(value, "");
}

TEST(rsplit_args, last_captures) {
    string_view key, value;
    string_view source{"key=value=true"};
    size_t count = source.rsplit_args({'=', string_view::last_captures_all}, key, value);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key, "true");
    EXPECT_EQ(value, "key=value");
}

TEST(rsplit_args, last_no_captures) {
    string_view key, value;
    string_view source{"key=value=true"};
    size_t count = source.rsplit_args('=', key, value);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key, "true");
    EXPECT_EQ(value, "value");
}

TEST(rsplit_args, middle_empty) {
    string_view key, empty, value;
    string_view source{"key==value"};
    size_t count = source.rsplit_args('=', value, empty, key);
    EXPECT_EQ(count, 3);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(empty, "");
    EXPECT_EQ(value, "value");
}

TEST(rsplit_args, skip_empty) {
    string_view key, value;
    string_view source{"key==value"};
    size_t count = source.rsplit_args({'=', string_view::skip_empty}, value, key);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value.to_string(), std::string("value"));
}

TEST(rsplit_args, skip_empty_extra) {
    string_view key, value;
    string_view source{"=key==value"};
    size_t count = source.rsplit_args({'=', string_view::skip_empty}, value, key);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value.to_string(), std::string("value"));
}

TEST(rsplit_args, skip_empty_multi_trailing) {
    string_view key, value;
    string_view source{"last=value==key"};
    size_t count = source.rsplit_args({'=', string_view::skip_empty}, key, value);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value.to_string(), std::string("value"));
}

TEST(rsplit_args, skip_empty_multi) {
    string_view key, value, last;
    string_view source{"last=value==key"};
    size_t count = source.rsplit_args({'=', string_view::skip_empty}, key, value, last);
    EXPECT_EQ(count, 3);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value.to_string(), std::string("value"));
    EXPECT_EQ(last.to_string(), std::string("last"));
}

TEST(rsplit_args, skip_empty_take_all) {
    string_view key, value;
    string_view source{"last=value====key"};
    string_view::split_flag flags{string_view::split_flag(string_view::skip_empty | string_view::last_captures_all)};
    size_t count = source.rsplit_args({'=', flags}, key, value);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value.to_string(), std::string("last=value"));
}

TEST(rsplit_args, normal_trailing) {
    string_view key, value;
    string_view source{"trailing=value=key"};
    size_t count = source.rsplit_args('=', key, value);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(key, "key");
    EXPECT_EQ(value, "value");
}

TEST(split_fn, normal) {
    std::vector<string_view> res;
    string_view source{"key=value=trailing"};
    size_t count = source.split_fn('=', [&res] (common::string_view str) {
        res.push_back(str);
    });
    EXPECT_EQ(count, 3);
    std::vector<string_view> expected = {"key", "value", "trailing"};
    EXPECT_EQ(res, expected);
}

TEST(compare, less_than) {
    string_view a{"a"}, b{"b"};
    EXPECT_EQ(a < b, true);
}

TEST(compare, equal) {
    string_view a{"a"}, b{"b"}, a2{"a"};
    EXPECT_EQ(a == b, false);
    EXPECT_EQ(a == a2, true);
}
