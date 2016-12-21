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

common_flags = ['-Wshadow']
c_flags = common_flags
cpp_flags = common_flags + ['-fno-rtti']
add_project_arguments(c_flags, language: 'c')
add_project_arguments(cpp_flags, language: 'cpp')

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
  'include/ellis/core/emigration.hpp',
  'include/ellis/core/err.h',
  'include/ellis/core/err.hpp',
  'include/ellis/core/flow.h',
  'include/ellis/core/flow.hpp',
  'include/ellis/core/immigration.hpp',
  'include/ellis/core/map_node.hpp',
  'include/ellis/core/node.hpp',
  'include/ellis/core/stream_decoder.hpp',
  'include/ellis/core/stream_encoder.hpp',
  'include/ellis/core/sync_input_stream.hpp',
  'include/ellis/core/sync_output_stream.hpp',
  'include/ellis/core/system.hpp',
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
  'src/codec/json.cpp',
  'src/codec/obd/can.cpp',
  'src/codec/obd/elm327.cpp',
  'src/codec/obd/pid.cpp',
  'src/core/array_node.cpp',
  'src/core/binary_node.cpp',
  'src/core/emigration.cpp',
  'src/core/err.cpp',
  'src/core/immigration.cpp',
  'src/core/map_node.cpp',
  'src/core/node.cpp',
  'src/core/stream_decoder.cpp',
  'src/core/stream_encoder.cpp',
  'src/core/system.cpp',
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
tests = [
  ['ellistest', 'test/ellis_test.cpp'],
  ['codectest', 'test/codec_test.cpp'],
  ['jsontest', 'test/json_test.cpp'],
  ['obdtest', 'test/obd_test.cpp']]
foreach t : tests
  exe = executable(
    t.get(0),
    t.get(1),
    include_directories: inc,
    link_with: lib,
    dependencies: thread_deps)
  test(t.get(0), exe)
endforeach
