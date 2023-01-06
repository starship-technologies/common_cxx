#ifndef COMMON_SHARED_IMPL_HPP
#define COMMON_SHARED_IMPL_HPP

#include <type_traits>

namespace common
{
namespace impl
{

// Abstract std::result_of (C++11..C++17) and std::invoke_result (C++17..)
#if (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || (__cplusplus >= 201703L)
using std::invoke_result_t;
#else
template<class Fn, typename... Args>
using invoke_result_t = typename std::result_of<Fn(Args...)>::type;
#endif

}
}

#endif // COMMON_SHARED_IMPL_HPP
