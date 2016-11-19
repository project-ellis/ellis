# Summary.
project('ellis', 'cpp',
  version: '0.1',
  license: 'proprietary',
  default_options: [
    'c_std=c99',
    'cpp_std=c++11',
    'buildtype=debug',
    'warning_level=3',
    'werror=true'])

# Enable threads.
thread_deps = dependency('threads')

# Includes.
inc = include_directories('include')
install_headers(
 ['include/ellis/array_node.hpp',
  'include/ellis/binary_node.hpp',
  'include/ellis/ellis.h',
  'include/ellis/err.h',
  'include/ellis/err.hpp',
  'include/ellis/flow.h',
  'include/ellis/map_node.hpp',
  'include/ellis/node.hpp',
  'include/ellis/type.h',
  'include/ellis/type.hpp'],
  subdir: 'ellis')

# libellis.
src = [
  'src/array_node.cpp',
  'src/binary_node.cpp',
  'src/err.cpp',
  'src/map_node.cpp',
  'src/node.cpp']
lib = shared_library(
  'ellis',
  src,
  include_directories: inc,
  install: true,
  dependencies: thread_deps)

# Tests.
test_exe = executable(
  'ellis_test',
  'test/ellis_test.cpp',
  include_directories: inc,
  link_with: lib)
test('ellis_test', test_exe)
