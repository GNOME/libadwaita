if get_option('examples')

subdir('data')

adwaita_demo_resources = gnome.compile_resources(
   'adwaita-demo-resources',
   'adwaita-demo.gresources.xml',

   c_name: 'adw',
)

adwaita_demo_sources = [
  adwaita_demo_resources,
  'adwaita-demo.c',
  'adw-demo-preferences-window.c',
  'adw-demo-window.c',
  'adw-flap-demo-window.c',
  'adw-tab-view-demo-window.c',
  'adw-view-switcher-demo-window.c',
  libadwaita_generated_headers,
]

adwaita_demo = executable('adwaita-@0@-demo'.format(apiversion),
  adwaita_demo_sources,
  dependencies: libadwaita_dep,
  gui_app: true,
  install: true,
)

endif
