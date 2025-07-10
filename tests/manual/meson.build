test_cflags = [
  '-DADW_LOG_DOMAIN="Adwaita"',
  '-DTEST_DATA_DIR="@0@/data"'.format(meson.current_source_dir()),
]

test_names = [
  'test-alert-dialogs',
  'test-avatar-colors',
  'test-breakpoints',
  'test-button-states',
  'test-navigation',
  'test-shortcut-labels',
  'test-shortcuts-dialogs',
  'test-split-views',
  'test-toggle-groups',
  'test-toolbars',
  'test-view-switcher-bars',
  'test-view-switchers',
]

test_resources = gnome.compile_resources(
   'adwaita-manual-test-resources',
   'manual-tests.gresources.xml',

   c_name: 'manual_test',
)

foreach test_name : test_names
  test_sources = [
    test_name + '.c',
    test_resources,
    libadwaita_generated_headers
  ]

  executable(test_name,
             test_sources,
             c_args: test_cflags,
             dependencies: libadwaita_deps + [libadwaita_dep])
endforeach
