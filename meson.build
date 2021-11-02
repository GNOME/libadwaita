project('libadwaita', 'c',
          version: '1.0.0.alpha.4',
          license: 'LGPL-2.1-or-later',
    meson_version: '>= 0.59.0',
  default_options: [ 'warning_level=1', 'buildtype=debugoptimized', 'c_std=gnu11' ],
)

version_arr = meson.project_version().split('-')[0].split('.')
adwaita_version_major = version_arr[0].to_int()
adwaita_version_minor = version_arr[1].to_int()
adwaita_version_micro = version_arr[2].to_int()

# The major api version as encoded in the libraries name
apiversion = adwaita_version_major.to_string()
# The so major version of the library
soversion = 0
package_api_name = '@0@-@1@'.format(meson.project_name(), apiversion)

adwaita_interface_age = adwaita_version_micro

# maintaining compatibility with libtool versioning
# current = minor * 100 + micro - interface
# revision = interface
current = adwaita_version_minor * 100 + adwaita_version_micro - adwaita_interface_age
revision = adwaita_interface_age
libversion = '@0@.@1@.@2@'.format(soversion, current, revision)

add_project_arguments([
  '-DHAVE_CONFIG_H',
  '-DADWAITA_COMPILATION',
  '-I' + meson.project_build_root(),
], language: 'c')

root_inc = include_directories('.')
src_inc = include_directories('src')

cc = meson.get_compiler('c')

global_c_args = []
test_c_args = [
  '-Wcast-align',
  '-Wdate-time',
  '-Wdeclaration-after-statement',
  ['-Werror=format-security', '-Werror=format=2'],
  '-Wendif-labels',
  '-Werror=incompatible-pointer-types',
  '-Werror=missing-declarations',
  '-Werror=overflow',
  '-Werror=return-type',
  '-Werror=shift-count-overflow',
  '-Werror=shift-overflow=2',
  '-Werror=implicit-fallthrough=3',
  '-Wformat-nonliteral',
  '-Wformat-security',
  '-Winit-self',
  '-Wmaybe-uninitialized',
  '-Wmissing-field-initializers',
  '-Wmissing-include-dirs',
  '-Wmissing-noreturn',
  '-Wnested-externs',
  '-Wno-missing-field-initializers',
  '-Wno-sign-compare',
  '-Wno-strict-aliasing',
  '-Wno-unused-parameter',
  '-Wold-style-definition',
  '-Wpointer-arith',
  '-Wredundant-decls',
  '-Wshadow',
  '-Wstrict-prototypes',
  '-Wswitch-default',
  '-Wswitch-enum',
  '-Wtype-limits',
  '-Wundef',
  '-Wunused-function',
]

target_system = target_machine.system()

if get_option('buildtype') != 'plain'
  if target_system == 'windows'
    test_c_args += '-fstack-protector'
  else
    test_c_args += '-fstack-protector-strong'
  endif
endif
if get_option('profiling')
  test_c_args += '-pg'
endif

foreach arg: test_c_args
  if cc.has_multi_arguments(arg)
    global_c_args += arg
  endif
endforeach
add_project_arguments(
  global_c_args,
  language: 'c'
)

# Setup various paths that subdirectory meson.build files need
package_subdir = get_option('package_subdir') # When used as subproject
if package_subdir != ''
  datadir    = get_option('datadir') / package_subdir
  libdir     = get_option('libdir')  / package_subdir
  girdir     = get_option('datadir') / package_subdir / 'gir-1.0'
  typelibdir = get_option('libdir')  / package_subdir / 'girepository-1.0'
  vapidir    = get_option('datadir') / package_subdir / 'vapi'
else
  datadir    = get_option('datadir')
  libdir     = get_option('libdir')
  girdir     = get_option('datadir') / 'gir-1.0'
  typelibdir = get_option('libdir')  / 'girepository-1.0'
  vapidir    = get_option('datadir') / 'vala' / 'vapi'
endif

introspection_feature = get_option('introspection')
introspection = introspection_feature.enabled() or introspection_feature.auto()

gnome = import('gnome')

subdir('src')
subdir('po')
subdir('demo')
subdir('tests')
subdir('doc')

run_data = configuration_data()
run_data.set('ABS_BUILDDIR', meson.current_build_dir())
run_data.set('ABS_SRCDIR', meson.current_source_dir())
configure_file(
          input: 'run.in',
         output: 'run',
  configuration: run_data)

summary(
  {
    'Tests': get_option('tests'),
    'Examples': get_option('examples'),
    'Documentation': get_option('gtk_doc'),
    'Introspection': introspection,
    'Vapi': get_option('vapi'),
  }, section: 'Options')
