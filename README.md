common_cxx: A header-only micro-library for C++11
-------------------------------------------------

### What is it?

This a tiny set of C++11 utility headers that bring minimal
implementations of useful concepts useful for general C++
development, developed as needed at Starship. Some of these
have analogues in later C++ standards, but most are modelled on
the more functional Rust equivalents, like `common::result` and
`common::optional`.

These are designed to be low to zero dependency, and minimal
in implementation size, while enabling sane error handling
sans exceptions.

### Dependencies

Requires a C++11 conformant compiler and STL implementation.
Currently uses some posix APIs directly, and is primarily
tested on linux.

Tests (in the `tests/` directory) require
(Google Test)[https://github.com/google/googletest].

### Trivial example

```
result<size_t, unix_err> extract_key_values(const char* filename, std::function<void (string_view key, string_view value)>&& dispatch_key_value)
{
    file_handle file;
    std::string content;
    return file
        .open(filename)
        .and_then([&] (ok) {
            return file.read_all(content);
        })
        .map([&] (ok) {
            size_t count{0};
            string_view{content}.split_fn('\n', [&] (string_view line)
            {
                string_view key, value;
                line.split_args('=', key, value);
                dispatch_key_value(key, value);
                count += 1;
            });
            return count;
        });
}
```

### Current content

* optional<T>: a `T` or `none`, include reference support and `map`/`and_then`
* result<T, Error>: a `T` or `Error` including `map`/`and_then`
* array_view<T>: a non-owning view to a contiguous block of 0..N `T`
* string_view<T>: a non-owning view with string helper methods for splitting and in-place formatting
* unix_err: a trivial wrapper around `errno`
* file_handle: a very-trivial RAII wrapper around `FILE*` with a few convenience functions
* timestamp: a {seconds, nanoseconds} timestamp

