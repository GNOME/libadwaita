if get_option('examples')

subdir('data')

adwaita_demo_resources = gnome.compile_resources(
   'adwaita-demo-resources',
   'adwaita-demo.gresources.xml',

   c_name: 'adw',
)

adwaita_demo_sources = [
  adwaita_demo_resources,

  'pages/avatar/adw-demo-page-avatar.c',
  'pages/buttons/adw-demo-page-buttons.c',
  'pages/carousel/adw-demo-page-carousel.c',
  'pages/clamp/adw-demo-page-clamp.c',
  'pages/flap/adw-demo-page-flap.c',
  'pages/flap/adw-flap-demo-window.c',
  'pages/leaflet/adw-demo-page-leaflet.c',
  'pages/lists/adw-demo-page-lists.c',
  'pages/tab-view/adw-demo-page-tab-view.c',
  'pages/tab-view/adw-tab-view-demo-window.c',
  'pages/view-switcher/adw-demo-page-view-switcher.c',
  'pages/view-switcher/adw-view-switcher-demo-window.c',
  'pages/welcome/adw-demo-page-welcome.c',

  'adwaita-demo.c',
  'adw-demo-preferences-window.c',
  'adw-demo-window.c',
  'adw-style-demo-window.c',
  libadwaita_generated_headers,
]

adwaita_demo = executable('adwaita-@0@-demo'.format(apiversion),
  adwaita_demo_sources,
  dependencies: libadwaita_dep,
  install: true,
)

endif
