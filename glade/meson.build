if get_option('glade_catalog')

  gladeui_dep = dependency('gladeui-2.0')
  module_dir = gladeui_dep.get_pkgconfig_variable('moduledir')

  libglade_hdy_sources = [
    'glade-hdy-header-group.c',
  ]

  libglade_hdy_deps = [
    libhandy_dep,
    gladeui_dep,
  ]

  libglade_hdy_args = []
  libglade_hdy = shared_library(
    'glade-handy',
    libglade_hdy_sources,
                 c_args: libglade_hdy_args,
           dependencies: libglade_hdy_deps,
    include_directories: [ root_inc, src_inc ],
                install: true,
            install_dir: module_dir,
  )

endif
