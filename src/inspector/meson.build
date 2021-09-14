if get_option('inspector')
  module_libdir = libdir / 'gtk-4.0' / 'inspector'

  libadwaita_inspector_resources = gnome.compile_resources(
    'adw-inspector-resources',
    'adw-inspector-module.gresources.xml',
    c_name: 'adw',
  )

  libadwaita_inspector_sources = [
    libadwaita_inspector_resources,
    'adw-inspector-module.c',
    'adw-inspector-page.c',
  ]

  shared_library(
    'adwaita-inspector-module' + apiversion,
    libadwaita_inspector_sources,
              soversion: soversion,
           dependencies: libadwaita_dep,
                install: true,
            install_dir: module_libdir,
  )
endif
