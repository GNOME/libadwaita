screenshot_resources = gnome.compile_resources(
   'screenshot-resources',
   'screenshot.gresources.xml',

   c_name: 'adw',
)

screenshot_sources = [
  screenshot_resources,
  'screenshot.c',
  libadwaita_generated_headers,
]

screenshot = executable('screenshot'.format(apiversion),
  screenshot_sources,
  dependencies: libadwaita_deps + [libadwaita_dep],
)
