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
  'include/ellis/array_node.hpp',
  'include/ellis/binary_node.hpp',
  'include/ellis/codec/delimited_text.hpp',
  'include/ellis/codec/json.hpp',
  'include/ellis/defs.hpp',
  'include/ellis/ellis.h',
  'include/ellis/err.h',
  'include/ellis/err.hpp',
  'include/ellis/flow.h',
  'include/ellis/flow.hpp',
  'include/ellis/map_node.hpp',
  'include/ellis/node.hpp',
  'include/ellis/private',
  'include/ellis/stream/cpp_input_stream.hpp',
  'include/ellis/stream/cpp_output_stream.hpp',
  'include/ellis/stream/fd_input_stream.hpp',
  'include/ellis/stream/fd_output_stream.hpp',
  'include/ellis/stream/file_input_stream.hpp',
  'include/ellis/stream/file_output_stream.hpp',
  'include/ellis/stream/tcp_client_stream.hpp',
  'include/ellis/stream_decoder.hpp',
  'include/ellis/stream_encoder.hpp',
  'include/ellis/sync_input_stream.hpp',
  'include/ellis/sync_output_stream.hpp',
  'include/ellis/type.h',
  'include/ellis/type.hpp']
install_headers(public_headers, subdir: 'ellis')

# libellis.
src = [
  'src/array_node.cpp',
  'src/binary_node.cpp',
  'src/codec/delimited_text.cpp',
  'src/err.cpp',
  'src/map_node.cpp',
  'src/node.cpp',
  'src/stream/cpp_input_stream.cpp',
  'src/stream/cpp_output_stream.cpp',
  'src/stream/file_input_stream.cpp',
  'src/stream/file_output_stream.cpp',
  'src/stream/tcp_client_stream.cpp',
  'src/type.cpp']
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
  'codec_test',
  'test/codec_test.cpp',
  include_directories: inc,
  link_with: lib,
  dependencies: thread_deps)
test('codec_test', codec_test_exe)
