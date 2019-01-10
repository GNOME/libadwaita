if glade_catalog

glade_xml = 'libhandy.xml'
module_dir = gladeui_dep.get_pkgconfig_variable('moduledir')
dtd = join_paths(meson.current_source_dir(), 'glade-catalog.dtd')
glade_catalogdir = gladeui_dep.get_pkgconfig_variable('catalogdir')

libglade_hdy_sources = [
  'glade-hdy-header-group.c',
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
