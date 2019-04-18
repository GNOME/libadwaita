libhandy_header_subdir = join_paths(package_subdir, 'libhandy-' + apiversion)
libhandy_header_dir = join_paths(get_option('includedir'), libhandy_header_subdir)
libhandy_resources = gnome.compile_resources(
   'hdy-resources',
   'handy.gresources.xml',

   c_name: 'hdy',
)

hdy_enum_headers = ['hdy-arrows.h',
                    'hdy-fold.h',
                    'hdy-header-bar.h',
                    'hdy-leaflet.h',
                    'hdy-squeezer.h',
                    'hdy-view-switcher.h',
                   ]

version_data = configuration_data()
version_data.set('HDY_MAJOR_VERSION', handy_version_major)
version_data.set('HDY_MINOR_VERSION', handy_version_minor)
version_data.set('HDY_MICRO_VERSION', handy_version_micro)
version_data.set('HDY_VERSION', meson.project_version())

# TODO: Once Meson 0.50.0 is required, use this to prevent installation:
#       install: installable,
if installable
  hdy_version_h = configure_file(
             input: 'hdy-version.h.in',
            output: 'hdy-version.h',
       install_dir: libhandy_header_dir,
     configuration: version_data)
else
  hdy_version_h = configure_file(
             input: 'hdy-version.h.in',
            output: 'hdy-version.h',
     configuration: version_data)
endif

libhandy_generated_headers = [
]
if installable
  install_headers(['handy.h'],
    subdir: libhandy_header_subdir)
endif

# Filled out in the subdirs
libhandy_public_headers = []
libhandy_public_sources = []
libhandy_private_sources = []

hdy_enums = gnome.mkenums('hdy-enums',
      h_template: 'hdy-enums.h.in',
      c_template: 'hdy-enums.c.in',
         sources: hdy_enum_headers,
  install_header: installable,
     install_dir: libhandy_header_dir,
)

libhandy_public_sources += [hdy_enums[0]]
libhandy_generated_headers += [hdy_enums[1]]

src_headers = [
  'hdy-action-row.h',
  'hdy-arrows.h',
  'hdy-column.h',
  'hdy-combo-row.h',
  'hdy-dialer-button.h',
  'hdy-dialer-cycle-button.h',
  'hdy-dialer.h',
  'hdy-dialog.h',
  'hdy-enum-value-object.h',
  'hdy-expander-row.h',
  'hdy-fold.h',
  'hdy-header-bar.h',
  'hdy-header-group.h',
  'hdy-leaflet.h',
  'hdy-list-box.h',
  'hdy-main.h',
  'hdy-preferences-group.h',
  'hdy-preferences-page.h',
  'hdy-preferences-row.h',
  'hdy-preferences-window.h',
  'hdy-search-bar.h',
  'hdy-squeezer.h',
  'hdy-string-utf8.h',
  'hdy-title-bar.h',
  'hdy-value-object.h',
  'hdy-view-switcher.h',
  'hdy-view-switcher-bar.h',
]

sed = find_program('sed', required: true)
gen_public_types = find_program('gen-public-types.sh', required: true)

libhandy_init_public_types = custom_target('hdy-public-types.c',
  output : 'hdy-public-types.c',
  input : [src_headers, libhandy_generated_headers],
  command : [gen_public_types, '@INPUT@'],
  capture: true,
)

src_sources = [
  'gtkprogresstracker.c',
  'gtk-window.c',
  'hdy-action-row.c',
  'hdy-arrows.c',
  'hdy-column.c',
  'hdy-combo-row.c',
  'hdy-dialer-button.c',
  'hdy-dialer-cycle-button.c',
  'hdy-dialer.c',
  'hdy-dialog.c',
  'hdy-enum-value-object.c',
  'hdy-expander-row.c',
  'hdy-fold.c',
  'hdy-header-bar.c',
  'hdy-header-group.c',
  'hdy-leaflet.c',
  'hdy-list-box.c',
  'hdy-main.c',
  'hdy-preferences-group.c',
  'hdy-preferences-page.c',
  'hdy-preferences-row.c',
  'hdy-preferences-window.c',
  'hdy-search-bar.c',
  'hdy-squeezer.c',
  'hdy-string-utf8.c',
  'hdy-title-bar.c',
  'hdy-value-object.c',
  'hdy-view-switcher.c',
  'hdy-view-switcher-bar.c',
  'hdy-view-switcher-button.c',
]

libhandy_public_headers += files(src_headers)
libhandy_public_sources += files(src_sources)

if installable
  install_headers(src_headers, subdir: libhandy_header_subdir)
endif


libhandy_sources = [
  libhandy_generated_headers,
  libhandy_public_sources,
  libhandy_private_sources,
  libhandy_resources,
  libhandy_init_public_types,
]

glib_min_version = '>= 2.44'

libhandy_deps = [
  dependency('glib-2.0', version: glib_min_version),
  dependency('gio-2.0', version: glib_min_version),
  dependency('gmodule-2.0', version: glib_min_version),
  dependency('gtk+-3.0', version: '>= 3.24.1'),
  cc.find_library('m', required: false),
  cc.find_library('rt', required: false),
]

libhandy_c_args = []
libhandy_link_args = []
libhandy_symbols_file = 'libhandy.syms'

# Check linker flags
ld_version_script_arg = '-Wl,--version-script,@0@/@1@'.format(meson.source_root(),
                                                              libhandy_symbols_file)
if cc.links('int main() { return 0; }', args : ld_version_script_arg, name : 'ld_supports_version_script')
  libhandy_link_args += [ld_version_script_arg]
endif

static = get_option('static')
if static
  libtype = 'static_library'
else
  libtype = 'shared_library'
endif

# set default libdir on win32 for libhandy target to keep MinGW compatibility
if target_system == 'windows'
  handy_libdir = [true]
else
  handy_libdir = libdir
endif

libhandy = build_target(
  'handy-' + apiversion,
  libhandy_sources,

            soversion: 0,
               c_args: libhandy_c_args,
         dependencies: libhandy_deps,
  include_directories: [ root_inc, src_inc ],
              install: installable,
            link_args: libhandy_link_args,
          install_dir: handy_libdir,
          target_type: libtype,
)

libhandy_dep = declare_dependency(
              sources: libhandy_generated_headers,
         dependencies: libhandy_deps,
            link_with: libhandy,
  include_directories: include_directories('.'),
)

if introspection

   libhandy_gir_extra_args = [
     '--c-include=handy.h',
     '--quiet',
     '-DHANDY_COMPILATION',
   ]

   libhandy_gir = gnome.generate_gir(libhandy,
                 sources: libhandy_generated_headers + libhandy_public_headers + libhandy_public_sources,
               nsversion: apiversion,
               namespace: 'Handy',
               export_packages: 'libhandy-@0@'.format(apiversion),
           symbol_prefix: 'hdy',
       identifier_prefix: 'Hdy',
               link_with: libhandy,
                includes: ['Gio-2.0', 'Gtk-3.0'],
                 install: true,
         install_dir_gir: girdir,
     install_dir_typelib: typelibdir,
              extra_args: libhandy_gir_extra_args,
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

if installable
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
endif
