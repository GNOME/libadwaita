if get_option('examples')

subdir('data')

demo_config_data = configuration_data()
demo_config_data.set_quoted('ADW_METAINFO', appstream_file.full_path())
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
   dependencies: appstream_file,
   source_dir: meson.current_build_dir(),
)

adwaita_demo_sources = [
  adwaita_demo_resources,

  'pages/about/adw-demo-page-about.c',
  'pages/alerts/adw-demo-page-alerts.c',
  'pages/animations/adw-demo-page-animations.c',
  'pages/avatar/adw-demo-page-avatar.c',
  'pages/banners/adw-demo-page-banners.c',
  'pages/bottom-sheets/adw-demo-page-bottom-sheets.c',
  'pages/buttons/adw-demo-page-buttons.c',
  'pages/carousel/adw-demo-page-carousel.c',
  'pages/clamp/adw-demo-page-clamp.c',
  'pages/lists/adw-demo-page-lists.c',
  'pages/multi-layout/adw-demo-page-multi-layout.c',
  'pages/multi-layout/adw-multi-layout-demo-dialog.c',
  'pages/navigation-view/adw-demo-page-navigation-view.c',
  'pages/navigation-view/adw-navigation-view-demo-dialog.c',
  'pages/spinner/adw-demo-page-spinner.c',
  'pages/split-views/adw-demo-page-split-views.c',
  'pages/split-views/adw-navigation-split-view-demo-dialog.c',
  'pages/split-views/adw-overlay-split-view-demo-dialog.c',
  'pages/styles/adw-demo-page-styles.c',
  'pages/styles/adw-style-demo-dialog.c',
  'pages/tab-view/adw-demo-page-tab-view.c',
  'pages/tab-view/adw-tab-view-demo-page.c',
  'pages/tab-view/adw-tab-view-demo-window.c',
  'pages/toasts/adw-demo-page-toasts.c',
  'pages/toggles/adw-demo-page-toggles.c',
  'pages/view-switcher/adw-demo-page-view-switcher.c',
  'pages/view-switcher/adw-view-switcher-demo-dialog.c',
  'pages/welcome/adw-demo-page-welcome.c',
  'pages/wrap-box/adw-demo-page-wrap-box.c',

  'adwaita-demo.c',
  'adw-demo-debug-info.c',
  'adw-demo-preferences-dialog.c',
  'adw-demo-sidebar-item.c',
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
