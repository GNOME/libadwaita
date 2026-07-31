if get_option('tests')

subdir('manual')

test_resources = gnome.compile_resources(
   'adwaita-test-resources',
   'tests.gresources.xml',

   c_name: 'test',
)

test_env = {
  'G_DEBUG': 'gc-friendly,fatal-warnings',
  'G_ENABLE_DIAGNOSTIC': '1',
  'G_TEST_BUILDDIR': meson.current_build_dir(),
  'G_TEST_SRCDIR': meson.current_source_dir(),
  'GDK_DEBUG': 'no-portals',
  'GDK_DISABLE': 'vulkan',
  'GIO_USE_VFS': 'local',
  'GSETTINGS_BACKEND': 'memory',
  'GTK_A11Y': 'test',
  'GTK_EXE_PREFIX': '',
  'MALLOC_CHECK_': '2',
  'NO_AT_BRIDGE': '1',
  'PYTHONDEVMODE': '1',
  'PYTHONDONTWRITEBYTECODE': 'yes',
}

test_cflags = [
  '-DADW_LOG_DOMAIN="Adwaita"',
]

test_link_args = []
use_pie = false

if cc.get_argument_syntax() != 'msvc'
  test_link_args += '-fPIC'
  use_pie = true
endif

test_names = [
  'test-about-dialog',
  'test-about-window',
  'test-accent-color',
  'test-action-row',
  'test-alert-dialog',
  'test-animation',
  'test-animation-target',
  'test-application-window',
  'test-avatar',
  'test-back-button',
  'test-banner',
  'test-bin',
  'test-bottom-sheet',
  'test-breakpoint',
  'test-breakpoint-bin',
  'test-button-content',
  'test-button-row',
  'test-carousel',
  'test-carousel-indicator-dots',
  'test-carousel-indicator-lines',
  'test-combo-row',
  'test-dialog',
  'test-easing',
  'test-entry-row',
  'test-expander-row',
  'test-flap',
  'test-header-bar',
  'test-inline-view-switcher',
  'test-leaflet',
  'test-message-dialog',
  'test-multi-layout-view',
  'test-navigation-split-view',
  'test-navigation-view',
  'test-overlay-split-view',
  'test-password-entry-row',
  'test-preferences-dialog',
  'test-preferences-group',
  'test-preferences-page',
  'test-preferences-row',
  'test-preferences-window',
  'test-sidebar',
  'test-sidebar-item',
  'test-sidebar-section',
  'test-shortcut-label',
  'test-spinner',
  'test-spinner-paintable',
  'test-spin-row',
  'test-split-button',
  'test-squeezer',
  'test-status-page',
  'test-style-manager',
  'test-switch-row',
  'test-tab-bar',
  'test-tab-button',
  'test-tab-overview',
  'test-tab-view',
  'test-timed-animation',
  'test-toast',
  'test-toast-overlay',
  'test-toggle-group',
  'test-toolbar-view',
  'test-view-switcher',
  'test-view-switcher-bar',
  'test-view-switcher-sidebar',
  'test-window',
  'test-window-title',
  'test-wrap-layout',
]

foreach test_name : test_names
  test_sources = [
    test_name + '.c',
    test_resources,
    libadwaita_generated_headers,
    'ignore-deprecations.h',
  ]

  t = executable(test_name, test_sources,
                       c_args: test_cflags,
                    link_args: test_link_args,
                 dependencies: libadwaita_deps + [libadwaita_internal_dep],
                          pie: use_pie,
                )
  test(test_name, t, protocol: 'tap', env: test_env)
endforeach

endif
