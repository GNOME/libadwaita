test_cflags = [
  '-DADW_LOG_DOMAIN="Adwaita"',
  '-DTEST_DATA_DIR="@0@/data"'.format(meson.current_source_dir()),
]

test_names = [
  'test-avatar-colors',
  'test-breakpoints',
  'test-button-states',
  'test-message-dialogs',
  'test-navigation',
  'test-split-views',
  'test-toolbars',
  'test-view-switcher-bars',
]

foreach test_name : test_names
  executable(test_name,
             [test_name + '.c'] + libadwaita_generated_headers,
             c_args: test_cflags,
             dependencies: libadwaita_deps + [libadwaita_dep])
endforeach
