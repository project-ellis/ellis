# Summary.
project('ellis', 'cpp',
  version: '0.1',
  license: 'mit',
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

# Includes.
inc = include_directories('include')
install_subdir('include/ellis/codec', install_dir : 'include/ellis')
install_subdir('include/ellis/core', install_dir : 'include/ellis')
install_subdir('include/ellis/stream', install_dir : 'include/ellis')

# libellis.
src = [
  'src/codec/delimited_text.cpp',
  'src/codec/json.cpp',
  'src/codec/msgpack.cpp',
  'src/codec/obd/can.cpp',
  'src/codec/obd/elm327.cpp',
  'src/codec/obd/pid.cpp',
  'src/convenience/file.cpp',
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
    install: true,
    dependencies: [ thread_deps ])
endforeach

# Documentation.
run_target('docs', command: 'meson/makedoc')

# Static analysis.
run_target('check', command: 'meson/check')
run_target('clangcheck', command: 'meson/clangcheck')
run_target('clangtidy', command: 'meson/clangtidy')
run_target('cppcheck', command: 'meson/cppcheck')

# Tests.
tests = [
  ['core_node_test', 'test/core/node_test.cpp'],
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
    dependencies: [ thread_deps ])
  test(t.get(0), exe)
endforeach
