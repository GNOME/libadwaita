libhandy_header_subdir = join_paths(package_subdir, 'libhandy-' + apiversion)
libhandy_header_dir = join_paths(get_option('includedir'), libhandy_header_subdir)
libhandy_resources = gnome.compile_resources(
   'hdy-resources',
   'handy.gresources.xml',

   c_name: 'hdy',
)

hdy_enum_headers = []

version_data = configuration_data()
version_data.set('HDY_MAJOR_VERSION', handy_version_major)
version_data.set('HDY_MINOR_VERSION', handy_version_minor)
version_data.set('HDY_MICRO_VERSION', handy_version_micro)
version_data.set('HDY_VERSION', meson.project_version())

hdy_version_h = configure_file(
           input: 'hdy-version.h.in',
          output: 'hdy-version.h',
     install_dir: libhandy_header_dir,
         install: true,
   configuration: version_data)

libhandy_generated_headers = [
]

install_headers(['handy.h'],
  subdir: libhandy_header_subdir)

# Filled out in the subdirs
libhandy_public_headers = []
libhandy_public_sources = []
libhandy_private_sources = []

src_headers = [
  'hdy-dialer-button.h',
  'hdy-dialer-cycle-button.h',
  'hdy-dialer.h',
  'hdy-string-utf8.h',
]

src_sources = [
  'gtkprogresstracker.c',
  'hdy-dialer-button.c',
  'hdy-dialer-cycle-button.c',
  'hdy-dialer.c',
  'hdy-string-utf8.c',
]

libhandy_public_headers += files(src_headers)
libhandy_public_sources += files(src_sources)

install_headers(src_headers, subdir: libhandy_header_subdir)


libhandy_sources = [
  libhandy_generated_headers,
  libhandy_public_sources,
  libhandy_private_sources,
  libhandy_resources,
]

libhandy_deps = [
  dependency('gio-2.0'),
  dependency('gmodule-2.0'),
  dependency('gtk+-3.0'),
  cc.find_library('m', required: false),
  cc.find_library('rt', required: false),
]

libhandy_args = []
libhandy = shared_library(
  'handy-' + apiversion,
  libhandy_sources,

            soversion: 0,
               c_args: libhandy_args,
         dependencies: libhandy_deps,
  include_directories: [ root_inc, src_inc ],
              install: true,
)

libhandy_dep = declare_dependency(
              sources: libhandy_public_headers,
         dependencies: libhandy_deps,
            link_with: libhandy,
  include_directories: include_directories('.'),
)

if get_option('introspection')

   libhandy_gir = gnome.generate_gir(libhandy,
                 sources: libhandy_generated_headers + libhandy_public_headers + libhandy_public_sources,
               nsversion: apiversion,
               namespace: 'Handy',
           symbol_prefix: 'hdy',
       identifier_prefix: 'Hdy',
               link_with: libhandy,
                includes: ['Gio-2.0', 'Gtk-3.0'],
                 install: true,
         install_dir_gir: girdir,
     install_dir_typelib: typelibdir,
              extra_args: [ '--c-include=handy.h', '--quiet' ],
   )

   if get_option('vapi')

     libhandy_vapi = gnome.generate_vapi('libhandy-' + apiversion,
             sources: libhandy_gir[0],
            packages: [ 'gio-2.0', 'gtk+-3.0' ],
             install: true,
         install_dir: vapidir,
       metadata_dirs: [ meson.current_source_dir() ],
     )

   endif
endif

pkgg = import('pkgconfig')

pkgg.generate(
    libraries: [libhandy],
      subdirs: libhandy_header_subdir,
      version: meson.project_version(),
         name: 'Handy',
     filebase: 'libhandy-@0@'.format(apiversion),
  description: 'Handy Mobile widgets',
     requires: 'gtk+-3.0',
  install_dir: join_paths(libdir, 'pkgconfig'),
)
