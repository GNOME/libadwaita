if get_option('enable_tests')

test_env = [
  'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
  'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
  'G_DEBUG=gc-friendly,fatal-warnings',
  'GSETTINGS_BACKEND=memory',
  'PYTHONDONTWRITEBYTECODE=yes',
  'MALLOC_CHECK_=2',
]

test_cflags = [
  '-fPIE',
  '-DTEST_DATA_DIR="@0@/data"'.format(meson.current_source_dir()),
]

test_link_args = [
  '-fPIC',
]

test_dialer = executable('test-dialer', 'test-dialer.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libhandy_deps + [libhandy_dep],
)
test('test-dialer', test_dialer, env: test_env)

test_string_utf8 = executable('test-string-utf8', 'test-string-utf8.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libhandy_deps + [libhandy_dep],
)
test('test-string-utf8', test_string_utf8, env: test_env)


endif
