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
public_headers = run_command('meson/find_includes').stdout().strip().split('\n')
install_headers(public_headers, subdir: 'ellis')

# libellis.
src = run_command('meson/find_srcs').stdout().strip().split('\n')
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
run_target('cppcheck', command: 'meson/cppcheck')

# Tests.
test_exe = executable(
  'ellis_test',
  'test/ellis_test.cpp',
  include_directories: inc,
  link_with: lib,
  dependencies: thread_deps)
test('ellis_test', test_exe)
