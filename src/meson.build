libadwaita_header_subdir = package_subdir / package_api_name
libadwaita_header_dir = get_option('includedir') / libadwaita_header_subdir

subdir('stylesheet')

libadwaita_resources = gnome.compile_resources(
  'adw-resources',
  'adwaita.gresources.xml',
  source_dir: meson.current_source_dir(),
  c_name: 'adw',
)

adw_public_enum_headers = [
  'adw-animation.h',
  'adw-banner.h',
  'adw-breakpoint.h',
  'adw-flap.h',
  'adw-fold-threshold-policy.h',
  'adw-easing.h',
  'adw-header-bar.h',
  'adw-leaflet.h',
  'adw-length-unit.h',
  'adw-message-dialog.h',
  'adw-navigation-direction.h',
  'adw-style-manager.h',
  'adw-squeezer.h',
  'adw-tab-view.h',
  'adw-toast.h',
  'adw-toolbar-view.h',
  'adw-view-switcher.h',
]

adw_private_enum_headers = [
  'adw-settings-private.h',
]

version_data = configuration_data()
version_data.set('ADW_MAJOR_VERSION', adwaita_version_major)
version_data.set('ADW_MINOR_VERSION', adwaita_version_minor)
version_data.set('ADW_MICRO_VERSION', adwaita_version_micro)
version_data.set('ADW_VERSION', meson.project_version())

adw_version_h = configure_file(
           input: 'adw-version.h.in',
          output: 'adw-version.h',
     install_dir: libadwaita_header_dir,
   configuration: version_data)

libadwaita_generated_headers = [
  adw_version_h,
]

install_headers(['adwaita.h'],
  subdir: libadwaita_header_subdir)

# Filled out in the subdirs
libadwaita_public_headers = []
libadwaita_public_sources = []
libadwaita_private_sources = []

adw_public_enums = gnome.mkenums_simple('adw-enums',
         sources: adw_public_enum_headers,
     body_prefix: '#include "config.h"',
   header_prefix: '#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)\n' +
                  '#error "Only <adwaita.h> can be included directly."\n' +
                  '#endif\n' +
                  '\n' +
                  '#include "adw-version.h"\n',
       decorator: 'ADW_AVAILABLE_IN_ALL',
  install_header: true,
     install_dir: libadwaita_header_dir,
)

adw_private_enums = gnome.mkenums_simple('adw-enums-private',
         sources: adw_private_enum_headers,
     body_prefix: '#include "config.h"',
   header_prefix: '#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)\n' +
                  '#error "Only <adwaita.h> can be included directly."\n' +
                  '#endif\n' +
                  '\n' +
                  '#include "adw-enums.h"\n',
  install_header: false,
)

libadwaita_public_sources += [adw_public_enums[0]]
libadwaita_private_sources += adw_private_enums
libadwaita_generated_headers += [adw_public_enums[1]]

adw_marshalers = gnome.genmarshal('adw-marshalers',
  sources: 'adw-marshalers.list',
  prefix: 'adw_marshal',
  valist_marshallers: true,
)

libadwaita_private_sources += adw_marshalers

src_headers = [
  'adw-about-window.h',
  'adw-action-row.h',
  'adw-animation.h',
  'adw-animation-target.h',
  'adw-animation-util.h',
  'adw-application.h',
  'adw-application-window.h',
  'adw-avatar.h',
  'adw-banner.h',
  'adw-bin.h',
  'adw-breakpoint.h',
  'adw-breakpoint-bin.h',
  'adw-button-content.h',
  'adw-carousel.h',
  'adw-carousel-indicator-dots.h',
  'adw-carousel-indicator-lines.h',
  'adw-clamp.h',
  'adw-clamp-layout.h',
  'adw-clamp-scrollable.h',
  'adw-combo-row.h',
  'adw-easing.h',
  'adw-entry-row.h',
  'adw-enum-list-model.h',
  'adw-expander-row.h',
  'adw-flap.h',
  'adw-fold-threshold-policy.h',
  'adw-header-bar.h',
  'adw-leaflet.h',
  'adw-length-unit.h',
  'adw-main.h',
  'adw-message-dialog.h',
  'adw-navigation-direction.h',
  'adw-navigation-split-view.h',
  'adw-navigation-view.h',
  'adw-overlay-split-view.h',
  'adw-password-entry-row.h',
  'adw-preferences-group.h',
  'adw-preferences-page.h',
  'adw-preferences-row.h',
  'adw-preferences-window.h',
  'adw-spin-row.h',
  'adw-split-button.h',
  'adw-spring-animation.h',
  'adw-spring-params.h',
  'adw-squeezer.h',
  'adw-status-page.h',
  'adw-style-manager.h',
  'adw-swipe-tracker.h',
  'adw-swipeable.h',
  'adw-switch-row.h',
  'adw-tab-bar.h',
  'adw-tab-button.h',
  'adw-tab-overview.h',
  'adw-tab-view.h',
  'adw-timed-animation.h',
  'adw-toast.h',
  'adw-toast-overlay.h',
  'adw-toolbar-view.h',
  'adw-view-stack.h',
  'adw-view-switcher.h',
  'adw-view-switcher-bar.h',
  'adw-view-switcher-title.h',
  'adw-window.h',
  'adw-window-title.h',
]

gen_public_types = find_program('gen-public-types.py', required: true)

libadwaita_init_public_types = custom_target('adw-public-types.c',
   output: 'adw-public-types.c',
    input: [src_headers, libadwaita_generated_headers],
  command: [gen_public_types, '@INPUT@'],
  capture: true,
)

src_sources = [
  'adw-about-window.c',
  'adw-action-row.c',
  'adw-animation.c',
  'adw-animation-target.c',
  'adw-animation-util.c',
  'adw-application.c',
  'adw-application-window.c',
  'adw-avatar.c',
  'adw-banner.c',
  'adw-bin.c',
  'adw-breakpoint.c',
  'adw-breakpoint-bin.c',
  'adw-button-content.c',
  'adw-carousel.c',
  'adw-carousel-indicator-dots.c',
  'adw-carousel-indicator-lines.c',
  'adw-clamp.c',
  'adw-clamp-layout.c',
  'adw-clamp-scrollable.c',
  'adw-combo-row.c',
  'adw-easing.c',
  'adw-entry-row.c',
  'adw-enum-list-model.c',
  'adw-expander-row.c',
  'adw-flap.c',
  'adw-fold-threshold-policy.c',
  'adw-header-bar.c',
  'adw-leaflet.c',
  'adw-length-unit.c',
  'adw-main.c',
  'adw-message-dialog.c',
  'adw-navigation-direction.c',
  'adw-navigation-split-view.c',
  'adw-navigation-view.c',
  'adw-overlay-split-view.c',
  'adw-password-entry-row.c',
  'adw-preferences-group.c',
  'adw-preferences-page.c',
  'adw-preferences-row.c',
  'adw-preferences-window.c',
  'adw-spin-row.c',
  'adw-split-button.c',
  'adw-spring-animation.c',
  'adw-spring-params.c',
  'adw-squeezer.c',
  'adw-style-manager.c',
  'adw-status-page.c',
  'adw-swipe-tracker.c',
  'adw-swipeable.c',
  'adw-switch-row.c',
  'adw-tab-bar.c',
  'adw-tab-button.c',
  'adw-tab-overview.c',
  'adw-tab-view.c',
  'adw-timed-animation.c',
  'adw-toast.c',
  'adw-toast-overlay.c',
  'adw-toolbar-view.c',
  'adw-version.c',
  'adw-view-stack.c',
  'adw-view-switcher.c',
  'adw-view-switcher-bar.c',
  'adw-view-switcher-title.c',
  'adw-window.c',
  'adw-window-title.c',
]

# Files that should not be introspected
libadwaita_private_sources += files([
  'adw-back-button.c',
  'adw-bidi.c',
  'adw-fading-label.c',
  'adw-gizmo.c',
  'adw-gtkbuilder-utils.c',
  'adw-indicator-bin.c',
  'adw-inspector-page.c',
  'adw-settings.c',
  'adw-settings-impl.c',
  'adw-settings-impl-gsettings.c',
  'adw-settings-impl-legacy.c',
  'adw-shadow-helper.c',
  'adw-tab.c',
  'adw-tab-box.c',
  'adw-tab-grid.c',
  'adw-tab-thumbnail.c',
  'adw-toast-widget.c',
  'adw-view-switcher-button.c',
  'adw-widget-utils.c',
])

libadwaita_public_headers += files(src_headers)
libadwaita_public_sources += files(src_sources)

install_headers(src_headers, subdir: libadwaita_header_subdir)

glib_min_version = '>= 2.72.0'
gtk_min_version = '>= 4.11.3'

gio_dep = dependency('gio-2.0', version: glib_min_version)
gtk_dep = dependency('gtk4', version: gtk_min_version)
appstream_dep = dependency('appstream',
  fallback : ['appstream', 'appstream_dep'],
  default_options : ['systemd=false', 'apidocs=false', 'install-docs=false'],
)

libadwaita_deps = [
  dependency('glib-2.0', version: glib_min_version),
  dependency('fribidi'),
  gio_dep,
  gtk_dep,
  appstream_dep,
  cc.find_library('m', required: false),
]

libadwaita_public_deps = [
  gio_dep,
  gtk_dep,
]

libadwaita_c_args = [
  '-DG_LOG_DOMAIN="Adwaita"',
]

config_h = configuration_data()
config_h.set_quoted('GETTEXT_PACKAGE', 'libadwaita')
config_h.set_quoted('LOCALEDIR', get_option('prefix') / get_option('localedir'))

# Symbol visibility
if target_system == 'windows'
  config_h.set('DLL_EXPORT', true)
  config_h.set('_ADW_EXTERN', '__declspec(dllexport) extern')
else
  config_h.set('_ADW_EXTERN', '__attribute__((visibility("default"))) extern')
endif

if target_system == 'darwin'
  appleframework_modules = [
    'AppKit',
    'Foundation',
  ]
  libadwaita_deps += [
    dependency('appleframeworks', modules: appleframework_modules),
  ]
  libadwaita_private_sources += [
    'adw-settings-impl-macos.c',
  ]
  libadwaita_c_args += ['-xobjective-c']
elif target_system == 'windows'
  libadwaita_deps += [
    dependency('gtk4-win32', version: gtk_min_version),
  ]
  libadwaita_private_sources += [
    'adw-settings-impl-win32.c',
  ]
else
  libadwaita_private_sources += [
    'adw-settings-impl-portal.c',
  ]
endif

libadwaita_sources = [
  libadwaita_generated_headers,
  libadwaita_public_sources,
  libadwaita_private_sources,
  libadwaita_resources,
  libadwaita_stylesheet_resources,
  libadwaita_init_public_types,
]

configure_file(
         output: 'config.h',
  configuration: config_h,
)

libadwaita_link_args = []
libadwaita_symbols_file = 'libadwaita.syms'

# Check linker flags
ld_version_script_arg = '-Wl,--version-script,@0@/@1@'.format(meson.project_source_root(),
                                                              libadwaita_symbols_file)
if cc.links('int main() { return 0; }', args : ld_version_script_arg, name : 'ld_supports_version_script')
  libadwaita_link_args += [ld_version_script_arg]
endif

# set default libdir on win32 for libadwaita target to keep MinGW compatibility
if target_system == 'windows'
  adwaita_libdir = [true]
else
  adwaita_libdir = libdir
endif

libadwaita = library(
  'adwaita-' + apiversion,
  libadwaita_sources,

              soversion: soversion,
                 c_args: libadwaita_c_args,
           dependencies: libadwaita_deps,
    include_directories: [ root_inc, src_inc ],
                install: true,
              link_args: libadwaita_link_args,
            install_dir: adwaita_libdir,
  gnu_symbol_visibility: 'hidden',
)

libadwaita_dep = declare_dependency(
              sources: libadwaita_generated_headers,
         dependencies: libadwaita_public_deps,
            link_with: libadwaita,
  include_directories: include_directories('.'),
)

meson.override_dependency(package_api_name, libadwaita_dep)

if introspection

   libadwaita_gir_extra_args = [
     '--c-include=adwaita.h',
     '--quiet',
     '-DADWAITA_COMPILATION',
   ]

   libadwaita_gir = gnome.generate_gir(libadwaita,
                 sources: libadwaita_generated_headers + libadwaita_public_headers + libadwaita_public_sources,
               nsversion: apiversion,
               namespace: 'Adw',
         export_packages: package_api_name,
           symbol_prefix: 'adw',
       identifier_prefix: 'Adw',
               link_with: libadwaita,
                includes: ['Gio-2.0', 'Gtk-4.0'],
                 install: true,
         install_dir_gir: girdir,
     install_dir_typelib: typelibdir,
              extra_args: libadwaita_gir_extra_args,
   )

   if get_option('vapi')

     libadwaita_vapi = gnome.generate_vapi(package_api_name,
             sources: libadwaita_gir[0],
            packages: [ 'gio-2.0', 'gtk4' ],
             install: true,
         install_dir: vapidir,
       metadata_dirs: [ meson.current_source_dir() ],
     )

   endif
endif

pkgg = import('pkgconfig')

pkgg.generate(
    libraries: [libadwaita],
      subdirs: libadwaita_header_subdir,
      version: meson.project_version(),
         name: 'Adwaita',
     filebase: package_api_name,
  description: 'Adwaita Mobile widgets',
     requires: 'gtk4',
  install_dir: libdir / 'pkgconfig',
)
