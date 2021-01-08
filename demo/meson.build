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
  'pages/spring/adw-demo-adjustment-row.c',
  'pages/spring/adw-demo-page-spring.c',
  'pages/spring/adw-demo-spring-basic.c',
  'pages/spring/adw-demo-spring-interactive.c',
  'pages/spring/adw-demo-spring-preset.c',
  'pages/spring/adw-demo-transform-layout.c',
  'pages/stub/adw-demo-page-stub.c',

  'adwaita-demo.c',
  'adw-demo-page.c',
  'adw-demo-page-info.c',
  'adw-demo-window.c',
  'adw-spring-animation.c',

  libadwaita_generated_headers,
]

adwaita_demo = executable('adwaita-@0@-demo'.format(apiversion),
  adwaita_demo_sources,
  dependencies: [
    libadwaita_dep,
    cc.find_library('m', required: false),
  ],
  gui_app: true,
  install: true,
)

endif
