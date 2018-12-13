if get_option('tests')

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

test_names = [
  'test-action-row',
  'test-arrows',
  'test-dialer',
  'test-dialer-cycle-button',
  'test-header-group',
  'test-search-bar',
  'test-string-utf8',
]

foreach test_name : test_names
  t = executable(test_name, [test_name + '.c'] + libhandy_generated_headers,
		 c_args: test_cflags,
		 link_args: test_link_args,
		 dependencies: libhandy_deps + [libhandy_dep],
		)
  test(test_name, t, env: test_env)
endforeach

endif
