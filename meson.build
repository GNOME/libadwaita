project('libhandy', 'c',
          version: '0.90.0',
          license: 'LGPL-2.1+',
    meson_version: '>= 0.49.0',
  default_options: [ 'warning_level=1', 'buildtype=debugoptimized', 'c_std=gnu11' ],
)

version_arr = meson.project_version().split('.')
handy_version_major = version_arr[0].to_int()
handy_version_minor = version_arr[1].to_int()
handy_version_micro = version_arr[2].to_int()

# The major api version as encoded in the libraries name
apiversion = '1'
# The so major version of the library
soversion = 0
package_api_name = '@0@-@1@'.format(meson.project_name(), apiversion)

if handy_version_minor.is_odd()
  handy_interface_age = 0
else
  handy_interface_age = handy_version_micro
endif

# maintaining compatibility with libtool versioning
# current = minor * 100 + micro - interface
# revision = interface
current = handy_version_minor * 100 + handy_version_micro - handy_interface_age
revision = handy_interface_age
libversion = '@0@.@1@.@2@'.format(soversion, current, revision)

add_project_arguments([
  '-DHAVE_CONFIG_H',
  '-DHANDY_COMPILATION',
  '-I' + meson.build_root(),
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
datadir        = get_option('datadir')  / package_subdir
libdir         = get_option('libdir')  / package_subdir
girdir         = get_option('datadir') / package_subdir / 'gir-1.0'
typelibdir     = get_option('libdir')  / package_subdir / 'girepository-1.0'
if package_subdir != ''
  vapidir = get_option('datadir') / package_subdir / 'vapi'
else
  vapidir = get_option('datadir') / 'vala' / 'vapi'
endif

glade_catalog_feature = get_option('glade_catalog')
gladeui_dep = dependency('gladeui-2.0', required : glade_catalog_feature)
glade_catalog = not glade_catalog_feature.disabled() and gladeui_dep.found()

introspection_feature = get_option('introspection')
introspection = introspection_feature.enabled() or introspection_feature.auto()

gnome = import('gnome')

subdir('src')
subdir('po')
subdir('examples')
subdir('tests')
subdir('doc')
subdir('glade')

run_data = configuration_data()
run_data.set('ABS_BUILDDIR', meson.current_build_dir())
run_data.set('ABS_SRCDIR', meson.current_source_dir())
configure_file(
          input: 'run.in',
         output: 'run',
  configuration: run_data)

summary = [
  '',
  '------',
  'Handy @0@ (@1@)'.format(current, apiversion),
  '',
  '             Tests: @0@'.format(get_option('tests')),
  '          Examples: @0@'.format(get_option('examples')),
  '     Documentation: @0@'.format(get_option('gtk_doc')),
  '     Introspection: @0@'.format(introspection),
  '              Vapi: @0@'.format(get_option('vapi')),
  '     Glade Catalog: @0@'.format(glade_catalog),
 '------',
  ''
]

message('\n'.join(summary))

