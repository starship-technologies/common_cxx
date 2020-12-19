project('common_cxx', 'cpp', default_options: ['cpp_std=c++14'], version: '0.1.1')
includes = include_directories('.')
common_cxx_dep = declare_dependency(include_directories: includes)
meson.override_dependency('common_cxx', common_cxx_dep)
