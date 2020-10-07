libhandy_header_subdir = package_subdir / package_api_name
libhandy_header_dir = get_option('includedir') / libhandy_header_subdir
libhandy_resources = gnome.compile_resources(
   'hdy-resources',
   'handy.gresources.xml',

   c_name: 'hdy',
)

hdy_public_enum_headers = [
  'hdy-deck.h',
  'hdy-header-bar.h',
  'hdy-header-group.h',
  'hdy-leaflet.h',
  'hdy-navigation-direction.h',
  'hdy-squeezer.h',
  'hdy-view-switcher.h',
]

hdy_private_enum_headers = [
  'hdy-stackable-box-private.h',
]

version_data = configuration_data()
version_data.set('HDY_MAJOR_VERSION', handy_version_major)
version_data.set('HDY_MINOR_VERSION', handy_version_minor)
version_data.set('HDY_MICRO_VERSION', handy_version_micro)
version_data.set('HDY_VERSION', meson.project_version())

hdy_version_h = configure_file(
           input: 'hdy-version.h.in',
          output: 'hdy-version.h',
     install_dir: libhandy_header_dir,
   configuration: version_data)

libhandy_generated_headers = [
]

install_headers(['handy.h'],
  subdir: libhandy_header_subdir)

# Filled out in the subdirs
libhandy_public_headers = []
libhandy_public_sources = []
libhandy_private_sources = []

hdy_public_enums = gnome.mkenums('hdy-enums',
      h_template: 'hdy-enums.h.in',
      c_template: 'hdy-enums.c.in',
         sources: hdy_public_enum_headers,
  install_header: true,
     install_dir: libhandy_header_dir,
)

hdy_private_enums = gnome.mkenums('hdy-enums-private',
      h_template: 'hdy-enums-private.h.in',
      c_template: 'hdy-enums-private.c.in',
         sources: hdy_private_enum_headers,
  install_header: false,
)

libhandy_public_sources += [hdy_public_enums[0]]
libhandy_private_sources += [hdy_private_enums[0]]
libhandy_generated_headers += [hdy_public_enums[1]]

src_headers = [
  'hdy-action-row.h',
  'hdy-animation.h',
  'hdy-application-window.h',
  'hdy-avatar.h',
  'hdy-carousel.h',
  'hdy-carousel-indicator-dots.h',
  'hdy-carousel-indicator-lines.h',
  'hdy-clamp.h',
  'hdy-combo-row.h',
  'hdy-deck.h',
  'hdy-deprecation-macros.h',
  'hdy-enum-value-object.h',
  'hdy-expander-row.h',
  'hdy-header-bar.h',
  'hdy-header-group.h',
  'hdy-keypad.h',
  'hdy-leaflet.h',
  'hdy-main.h',
  'hdy-navigation-direction.h',
  'hdy-preferences-group.h',
  'hdy-preferences-page.h',
  'hdy-preferences-row.h',
  'hdy-preferences-window.h',
  'hdy-search-bar.h',
  'hdy-squeezer.h',
  'hdy-swipe-group.h',
  'hdy-swipe-tracker.h',
  'hdy-swipeable.h',
  'hdy-title-bar.h',
  'hdy-types.h',
  'hdy-value-object.h',
  'hdy-view-switcher.h',
  'hdy-view-switcher-bar.h',
  'hdy-view-switcher-title.h',
  'hdy-window.h',
  'hdy-window-handle.h',
]

sed = find_program('sed', required: true)
gen_public_types = find_program('gen-public-types.sh', required: true)

libhandy_init_public_types = custom_target('hdy-public-types.c',
   output: 'hdy-public-types.c',
    input: [src_headers, libhandy_generated_headers],
  command: [gen_public_types, '@INPUT@'],
  capture: true,
)

src_sources = [
  'gtkprogresstracker.c',
  'gtk-window.c',
  'hdy-action-row.c',
  'hdy-animation.c',
  'hdy-application-window.c',
  'hdy-avatar.c',
  'hdy-carousel.c',
  'hdy-carousel-box.c',
  'hdy-carousel-indicator-dots.c',
  'hdy-carousel-indicator-lines.c',
  'hdy-clamp.c',
  'hdy-combo-row.c',
  'hdy-css.c',
  'hdy-deck.c',
  'hdy-enum-value-object.c',
  'hdy-expander-row.c',
  'hdy-header-bar.c',
  'hdy-header-group.c',
  'hdy-keypad-button.c',
  'hdy-keypad.c',
  'hdy-leaflet.c',
  'hdy-main.c',
  'hdy-navigation-direction.c',
  'hdy-nothing.c',
  'hdy-preferences-group.c',
  'hdy-preferences-page.c',
  'hdy-preferences-row.c',
  'hdy-preferences-window.c',
  'hdy-search-bar.c',
  'hdy-shadow-helper.c',
  'hdy-squeezer.c',
  'hdy-stackable-box.c',
  'hdy-swipe-group.c',
  'hdy-swipe-tracker.c',
  'hdy-swipeable.c',
  'hdy-title-bar.c',
  'hdy-value-object.c',
  'hdy-view-switcher.c',
  'hdy-view-switcher-bar.c',
  'hdy-view-switcher-button.c',
  'hdy-view-switcher-title.c',
  'hdy-window.c',
  'hdy-window-handle.c',
  'hdy-window-handle-controller.c',
  'hdy-window-mixin.c',
]

libhandy_public_headers += files(src_headers)
libhandy_public_sources += files(src_sources)

install_headers(src_headers, subdir: libhandy_header_subdir)


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

libhandy_c_args = [
  '-DG_LOG_DOMAIN="Handy"',
]

config_h = configuration_data()
config_h.set_quoted('GETTEXT_PACKAGE', 'libhandy')
config_h.set_quoted('LOCALEDIR', get_option('prefix') / get_option('localedir'))

# Symbol visibility
if target_system == 'windows'
  config_h.set('DLL_EXPORT', true)
  config_h.set('_HDY_EXTERN', '__declspec(dllexport) extern')
  if cc.get_id() != 'msvc'
    libhandy_c_args += ['-fvisibility=hidden']
  endif
else
  config_h.set('_HDY_EXTERN', '__attribute__((visibility("default"))) extern')
  libhandy_c_args += ['-fvisibility=hidden']
endif

configure_file(
         output: 'config.h',
  configuration: config_h,
)

libhandy_link_args = []
libhandy_symbols_file = 'libhandy.syms'

# Check linker flags
ld_version_script_arg = '-Wl,--version-script,@0@/@1@'.format(meson.source_root(),
                                                              libhandy_symbols_file)
if cc.links('int main() { return 0; }', args : ld_version_script_arg, name : 'ld_supports_version_script')
  libhandy_link_args += [ld_version_script_arg]
endif

# set default libdir on win32 for libhandy target to keep MinGW compatibility
if target_system == 'windows'
  handy_libdir = [true]
else
  handy_libdir = libdir
endif

libhandy = shared_library(
  'handy-' + apiversion,
  libhandy_sources,

            soversion: soversion,
               c_args: libhandy_c_args,
         dependencies: libhandy_deps,
  include_directories: [ root_inc, src_inc ],
              install: true,
            link_args: libhandy_link_args,
          install_dir: handy_libdir,
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
         export_packages: package_api_name,
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

     libhandy_vapi = gnome.generate_vapi(package_api_name,
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
     filebase: package_api_name,
  description: 'Handy Mobile widgets',
     requires: 'gtk+-3.0',
  install_dir: libdir / 'pkgconfig',
)
