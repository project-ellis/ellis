# Summary.
project('ellis', 'cpp',
  version: '0.1',
  license: 'proprietary',
  default_options: [
    'c_std=c99',
    'cpp_std=c++14',
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

# Dependencies that are specific to Ellis Ex.
asio_dep = dependency('boost', modules : ['asio', 'system'])
nghttp2_dep = dependency('libnghttp2_asio', version : '>=1.18.0')
ssl_dep = dependency('libssl')

# Includes.
inc = include_directories('include')
public_headers = [
  'include/ellis/core/array_node.hpp',
  'include/ellis/core/binary_node.hpp',
  'include/ellis/core/codec/delimited_text.hpp',
  'include/ellis/core/codec/json.hpp',
  'include/ellis/core/decoder.hpp',
  'include/ellis/core/defs.hpp',
  'include/ellis/core/disposition.hpp',
  'include/ellis/core/emigration.hpp',
  'include/ellis/core/encoder.hpp',
  'include/ellis/core/err.hpp',
  'include/ellis/core/immigration.hpp',
  'include/ellis/core/map_node.hpp',
  'include/ellis/core/codec/msgpack.hpp',
  'include/ellis/core/node.hpp',
  'include/ellis/core/sync_input_stream.hpp',
  'include/ellis/core/sync_output_stream.hpp',
  'include/ellis/core/system.hpp',
  'include/ellis/core/type.hpp',
  'include/ellis/core/u8str_node.hpp',
  'include/ellis/stream/cpp_input_stream.hpp',
  'include/ellis/stream/cpp_output_stream.hpp',
  'include/ellis/stream/fd_input_stream.hpp',
  'include/ellis/stream/fd_output_stream.hpp',
  'include/ellis/stream/file_input_stream.hpp',
  'include/ellis/stream/file_output_stream.hpp',
  'include/ellis/stream/mem_input_stream.hpp',
  'include/ellis/stream/mem_output_stream.hpp',
  'include/ellis/stream/tcp_client_stream.hpp']
install_headers(public_headers, subdir: 'ellis')

# libellis.
src = [
  'src/codec/delimited_text.cpp',
  'src/codec/json.cpp',
  'src/codec/msgpack.cpp',
  'src/codec/obd/can.cpp',
  'src/codec/obd/elm327.cpp',
  'src/codec/obd/pid.cpp',
  'src/core/array_node.cpp',
  'src/core/binary_node.cpp',
  'src/core/decoder.cpp',
  'src/core/emigration.cpp',
  'src/core/encoder.cpp',
  'src/core/err.cpp',
  'src/core/immigration.cpp',
  'src/core/map_node.cpp',
  'src/core/node.cpp',
  'src/core/system.cpp',
  'src/core/type.cpp',
  'src/core/u8str_node.cpp',
  'src/stream/cpp_input_stream.cpp',
  'src/stream/cpp_output_stream.cpp',
  'src/stream/fd_input_stream.cpp',
  'src/stream/fd_output_stream.cpp',
  'src/stream/file_input_stream.cpp',
  'src/stream/file_output_stream.cpp',
  'src/stream/mem_input_stream.cpp',
  'src/stream/mem_output_stream.cpp',
  'src/stream/tcp_client_stream.cpp']
# Library
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

# Command line utilities.
utils = [
  ['ellisconv', 'util/ellisconv.cpp']]
foreach u : utils
  executable(
    u.get(0),
    u.get(1),
    include_directories: inc,
    link_with: lib,
    dependencies: [ thread_deps, asio_dep, nghttp2_dep, ssl_dep ])
endforeach

# Documentation.
run_target('doc', command: 'meson/makedocs')

# Static analysis.
run_target('check', command: 'meson/check')
run_target('clangcheck', command: 'meson/clangcheck')
run_target('clangtidy', command: 'meson/clangtidy')
run_target('cppcheck', command: 'meson/cppcheck')

# Tests.
tests = [
  ['core_node_test', 'test/core/node_test.cpp'],
  ['op_op_test', 'test/op/op_test.cpp'],
  ['codec_delimited_text_test', 'test/codec/delimited_text_test.cpp'],
  ['codec_json_test', 'test/codec/json_test.cpp'],
  ['codec_msgpack_test', 'test/codec/msgpack_test.cpp'],
  ['codec_obd_test', 'test/codec/obd_test.cpp'],
  ['stream_fd_test', 'test/stream/fd_test.cpp'],
  ['stream_file_test', 'test/stream/file_test.cpp']]
foreach t : tests
  exe = executable(
    t.get(0),
    t.get(1),
    include_directories: inc,
    link_with: lib,
    dependencies: [ thread_deps, asio_dep, nghttp2_dep, ssl_dep ])
  test(t.get(0), exe)
endforeach
