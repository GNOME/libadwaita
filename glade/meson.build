if glade_catalog

glade_xml          = 'libhandy.xml'
module_dir         = gladeui_dep.get_pkgconfig_variable('moduledir',
                                                        define_variable: ['libdir', get_option('libdir')])
dtd                = meson.current_source_dir() / 'glade-catalog.dtd'
glade_catalogdir   = gladeui_dep.get_pkgconfig_variable('catalogdir',
                                                        define_variable: ['datadir', get_option('datadir')])

libglade_hdy_sources = [
  'glade-hdy-carousel.c',
  'glade-hdy-expander-row.c',
  'glade-hdy-header-bar.c',
  'glade-hdy-header-group.c',
  'glade-hdy-leaflet.c',
  'glade-hdy-preferences-page.c',
  'glade-hdy-preferences-window.c',
  'glade-hdy-search-bar.c',
  'glade-hdy-swipe-group.c',
  'glade-hdy-window.c',
  'glade-hdy-utils.c',
]

libglade_hdy_deps = [
  libhandy_dep,
  gladeui_dep,
]

libglade_hdy_args = []
# Our custom glade module
libglade_hdy = shared_library(
  'glade-handy-' + apiversion,
  libglade_hdy_sources,
               c_args: libglade_hdy_args,
         dependencies: libglade_hdy_deps,
  include_directories: [ root_inc, src_inc ],
              install: true,
          install_dir: module_dir,
)

# Validate glade catalog
xmllint = find_program('xmllint', required: true)
if xmllint.found()
  custom_target(
    'xmllint',
    build_by_default: true,
               input: glade_xml,
              output: 'doesnotexist',
             command: [xmllint, '--dtdvalid', dtd, '--noout', '@INPUT@'],
  )
endif

# Install glade catalog
install_data(
  glade_xml,
       rename: 'libhandy-@0@.xml'.format(apiversion),
  install_dir: glade_catalogdir)

endif
