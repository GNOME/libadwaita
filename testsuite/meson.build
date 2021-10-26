if get_option('tests')

testsuite_env = [
  'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
  'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
  'G_DEBUG=gc-friendly,fatal-warnings',
  'GSETTINGS_BACKEND=memory',
  'PYTHONDONTWRITEBYTECODE=yes',
  'MALLOC_CHECK_=2',
  'GTK_A11Y=none',
]

testsuite_cflags = [
  '-DADW_LOG_DOMAIN="Adwaita"',
  '-DTEST_DATA_DIR="@0@/data"'.format(meson.current_source_dir()),
]

testsuite_link_args = []
use_pie = false

if cc.get_argument_syntax() != 'msvc'
  testsuite_link_args += '-fPIC'
  use_pie = true
endif

testsuite_names = [
  'test-about-window',
  'test-action-row',
  'test-animation',
  'test-animation-target',
  'test-application-window',
  'test-avatar',
  'test-bin',
  'test-button-content',
  'test-carousel',
  'test-carousel-indicator-dots',
  'test-carousel-indicator-lines',
  'test-combo-row',
  'test-easing',
  'test-entry-row',
  'test-expander-row',
  'test-flap',
  'test-header-bar',
  'test-leaflet',
  'test-message-dialog',
  'test-password-entry-row',
  'test-preferences-group',
  'test-preferences-page',
  'test-preferences-row',
  'test-preferences-window',
  'test-split-button',
  'test-squeezer',
  'test-status-page',
  'test-style-manager',
  'test-tab-bar',
  'test-tab-button',
  'test-tab-overview',
  'test-tab-view',
  'test-timed-animation',
  'test-toast',
  'test-toast-overlay',
  'test-view-switcher',
  'test-view-switcher-bar',
  'test-window',
  'test-window-title',
]

foreach testsuite_name : testsuite_names
  t = executable(testsuite_name, [testsuite_name + '.c'] + libadwaita_generated_headers,
                       c_args: testsuite_cflags,
                    link_args: testsuite_link_args,
                 dependencies: libadwaita_deps + [libadwaita_dep],
                          pie: use_pie,
                )
  test(testsuite_name, t, env: testsuite_env)
endforeach

endif
