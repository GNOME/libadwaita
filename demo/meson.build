if get_option('examples')

subdir('data')

demo_config_data = configuration_data()
demo_config_data.set_quoted('ADW_DEMO_VCS_TAG', '@VCS_TAG@')

demo_config_h = vcs_tag(
  input: configure_file(
    output: 'config.h.in',
    configuration: demo_config_data
  ),
  output: 'config.h'
)

adwaita_demo_resources = gnome.compile_resources(
   'adwaita-demo-resources',
   'adwaita-demo.gresources.xml',

   c_name: 'adw',
)

adwaita_demo_sources = [
  adwaita_demo_resources,

  'pages/about/adw-demo-page-about.c',
  'pages/animations/adw-demo-page-animations.c',
  'pages/avatar/adw-demo-page-avatar.c',
  'pages/buttons/adw-demo-page-buttons.c',
  'pages/carousel/adw-demo-page-carousel.c',
  'pages/clamp/adw-demo-page-clamp.c',
  'pages/dialogs/adw-demo-page-dialogs.c',
  'pages/flap/adw-demo-page-flap.c',
  'pages/flap/adw-flap-demo-window.c',
  'pages/leaflet/adw-demo-page-leaflet.c',
  'pages/lists/adw-demo-page-lists.c',
  'pages/styles/adw-demo-page-styles.c',
  'pages/styles/adw-style-demo-window.c',
  'pages/tab-view/adw-demo-page-tab-view.c',
  'pages/tab-view/adw-tab-view-demo-window.c',
  'pages/toasts/adw-demo-page-toasts.c',
  'pages/view-switcher/adw-demo-page-view-switcher.c',
  'pages/view-switcher/adw-view-switcher-demo-window.c',
  'pages/welcome/adw-demo-page-welcome.c',

  'adwaita-demo.c',
  'adw-demo-debug-info.c',
  'adw-demo-preferences-window.c',
  'adw-demo-window.c',
  demo_config_h,
  libadwaita_generated_headers,
]

adwaita_demo = executable('adwaita-@0@-demo'.format(apiversion),
  adwaita_demo_sources,
  dependencies: libadwaita_dep,
  install: true,
)

endif
