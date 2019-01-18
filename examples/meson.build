if get_option('examples')

example_resources = gnome.compile_resources(
   'example-resources',
   'example.gresources.xml',

   c_name: 'hdy',
)

example_sources = [
  example_resources,
  'example-window.c',
  'example.c',
  libhandy_generated_headers,
]

example = executable('example', example_sources,
  dependencies: libhandy_dep,
  gui_app: true,
  install: installable,
)

endif
