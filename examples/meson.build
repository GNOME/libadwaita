if get_option('examples')

handy_demo_resources = gnome.compile_resources(
   'handy-demo-resources',
   'handy-demo.gresources.xml',

   c_name: 'hdy',
)

handy_demo_sources = [
  handy_demo_resources,
  'handy-demo.c',
  'hdy-demo-window.c',
  libhandy_generated_headers,
]

handy_demo = executable('handy-demo', handy_demo_sources,
  dependencies: libhandy_dep,
  gui_app: true,
  install: installable,
)

endif
