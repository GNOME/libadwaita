if glade_catalog

# This is needed to work around this issue in Glade:
# https://gitlab.gnome.org/GNOME/glade/issues/379
is_flatpak_app = get_option('prefix') == '/app'

glade_xml          = 'libhandy.xml'
if (is_flatpak_app)
  module_dir       = libdir / 'glade' / 'modules'
else
  module_dir       = gladeui_dep.get_pkgconfig_variable('moduledir')
endif
dtd                = meson.current_source_dir() / 'glade-catalog.dtd'
if (is_flatpak_app)
  glade_catalogdir = datadir / 'glade' / 'catalogs'
else
  glade_catalogdir = gladeui_dep.get_pkgconfig_variable('catalogdir')
endif

libglade_hdy_sources = [
  'glade-hdy-carousel.c',
  'glade-hdy-header-group.c',
  'glade-hdy-swipe-group.c',
]

libglade_hdy_deps = [
  libhandy_dep,
  gladeui_dep,
]

libglade_hdy_args = []
# Our custom glade module
libglade_hdy = shared_library(
  'glade-handy',
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
install_data(glade_xml, install_dir: glade_catalogdir)

endif
