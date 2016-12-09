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

extra_gcc_flags = ['-Wshadow']
add_project_arguments(extra_gcc_flags, language: 'cpp')
add_project_arguments(extra_gcc_flags, language: 'c')

pkg = import('pkgconfig')

# Enable threads.
thread_deps = dependency('threads')

# Includes.
inc = include_directories('include')
public_headers = [
  'include/ellis/core/array_node.hpp',
  'include/ellis/core/binary_node.hpp',
  'include/ellis/core/codec/delimited_text.hpp',
  'include/ellis/core/codec/json.hpp',
  'include/ellis/core/defs.hpp',
  'include/ellis/core/ellis.h',
  'include/ellis/core/err.h',
  'include/ellis/core/err.hpp',
  'include/ellis/core/flow.h',
  'include/ellis/core/flow.hpp',
  'include/ellis/core/map_node.hpp',
  'include/ellis/core/node.hpp',
  'include/ellis/core/stream_decoder.hpp',
  'include/ellis/core/stream_encoder.hpp',
  'include/ellis/core/sync_input_stream.hpp',
  'include/ellis/core/sync_output_stream.hpp',
  'include/ellis/core/type.h',
  'include/ellis/core/type.hpp',
  'include/ellis/stream/cpp_input_stream.hpp',
  'include/ellis/stream/cpp_output_stream.hpp',
  'include/ellis/stream/fd_input_stream.hpp',
  'include/ellis/stream/fd_output_stream.hpp',
  'include/ellis/stream/file_input_stream.hpp',
  'include/ellis/stream/file_output_stream.hpp',
  'include/ellis/stream/tcp_client_stream.hpp']
install_headers(public_headers, subdir: 'ellis')

# libellis.
src = [
  'src/codec/delimited_text.cpp',
  'src/core/array_node.cpp',
  'src/core/binary_node.cpp',
  'src/core/err.cpp',
  'src/core/map_node.cpp',
  'src/core/node.cpp',
  'src/core/type.cpp',
  'src/stream/cpp_input_stream.cpp',
  'src/stream/cpp_output_stream.cpp',
  'src/stream/file_input_stream.cpp',
  'src/stream/file_output_stream.cpp',
  'src/stream/tcp_client_stream.cpp']
lib = shared_library(
  'ellis',
  src,
  include_directories: inc,
  install: true,
  dependencies: thread_deps)
pkg.generate(
  name: 'ellis',
  description: 'A library implementing a common, interoperable data framework',
  libraries: [lib],
  subdirs: 'ellis',
  version: '0.1')

# Static analysis.
run_target('check', command: 'meson/check')
run_target('clangcheck', command: 'meson/clangcheck')
run_target('clangtidy', command: 'meson/clangtidy')
run_target('cppcheck', command: 'meson/cppcheck')

# Tests.
test_exe = executable(
  'ellis_test',
  'test/ellis_test.cpp',
  include_directories: inc,
  link_with: lib,
  dependencies: thread_deps)
test('ellis_test', test_exe)

codec_test_exe = executable(
  'codectest',
  'test/codec_test.cpp',
  include_directories: inc,
  link_with: lib,
  dependencies: thread_deps)
test('codectest', codec_test_exe)
