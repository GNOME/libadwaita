if get_option('examples')

adwaita_demo_resources = gnome.compile_resources(
  'adwaita-demo-resources',
  'adwaita-demo.gresources.xml',

  c_name: 'adw',
)

adwaita_demo_sources = [
  adwaita_demo_resources,

  'pages/controls/adw-demo-page-controls.c',
  'pages/lists/adw-demo-page-lists.c',
  'pages/stub/adw-demo-page-stub.c',

  'adwaita-demo.c',
  'adw-demo-page.c',
  'adw-demo-page-info.c',
  'adw-demo-window.c',

  libadwaita_generated_headers,
]

adwaita_demo = executable('adwaita-@0@-demo'.format(apiversion),
  adwaita_demo_sources,
  dependencies: libadwaita_dep,
  gui_app: true,
  install: true,
)

endif
