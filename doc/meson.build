if get_option('documentation')

subdir('tools')

expand_content_md_files = [
  'adaptive-layouts.md',
  'boxed-lists.md',
  'build-howto.md',
  'css-variables.md',
  'initialization.md',
  'migrating-libhandy-1-4-to-libadwaita.md',
  'migrating-to-adaptive-dialogs.md',
  'migrating-to-breakpoints.md',
  'styles-and-appearance.md',
  'style-classes.md',
  'widget-gallery.md',
]

toml_data = configuration_data()
toml_data.set('VERSION', meson.project_version())

libadwaita_toml = configure_file(
  input: 'libadwaita.toml.in',
  output: 'libadwaita.toml',
  configuration: toml_data
)

dependency('gi-docgen', version: '>= 2021.1',
           fallback: ['gi-docgen', 'dummy_dep'],
           native: true,
           required: get_option('documentation'))

gidocgen = find_program('gi-docgen')

docs_dir = datadir / 'doc'

custom_target('libadwaita-doc',
  input: [ libadwaita_toml, libadwaita_gir[0] ],
  output: 'libadwaita-@0@'.format(apiversion),
  command: [
    gidocgen,
    'generate',
    '--quiet',
    '--config=@INPUT0@',
    '--output-dir=@OUTPUT@',
    '--no-namespace-dir',
    '--content-dir=@0@'.format(meson.current_source_dir()),
    '@INPUT1@',
  ],
  depend_files: [ expand_content_md_files ],
  build_by_default: true,
  install: true,
  install_dir: docs_dir,
)

test('Validate docs',
  gidocgen,
  args: [
    'check',
    '--config', libadwaita_toml,
    '--add-include-path=@0@'.format(meson.current_build_dir() / '../src'),
    libadwaita_gir[0],
  ],
  depends: libadwaita_gir[0],
  suite: ['doc'],
)

endif
