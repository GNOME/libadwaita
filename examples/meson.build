if get_option('examples')

handy_demo_resources = gnome.compile_resources(
   'handy-demo-resources',
   'handy-demo.gresources.xml',

   c_name: 'hdy',
)

handy_demo_sources = [
  handy_demo_resources,
  'handy-demo.c',
  'hdy-demo-preferences-window.c',
  'hdy-demo-window.c',
  'hdy-view-switcher-demo-window.c',
  libhandy_generated_headers,
]

handy_demo = executable('handy-@0@-demo'.format(apiversion),
  handy_demo_sources,
  dependencies: libhandy_dep,
  gui_app: true,
  install: true,
)

endif
