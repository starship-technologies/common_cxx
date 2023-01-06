#ifndef COMMON_SHARED_DEFINES_HPP
#define COMMON_SHARED_DEFINES_HPP

#ifdef _MSC_VER
#  define COMMON_FULL_FUNCTION_NAME __FUNCSIG__
#else
#  define COMMON_FULL_FUNCTION_NAME __PRETTY_FUNCTION__
#endif

#ifdef _MSC_VER
#  define COMMON_FORMAT_VALIDATE(x, y)
#else
#  define COMMON_FORMAT_VALIDATE(x, y) __attribute__((format (printf, x, y)))
#endif

#endif // COMMON_SHARED_DEFINES_HPP
