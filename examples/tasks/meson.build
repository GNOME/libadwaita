# Uncomment everything to build separately
# project('tasks', 'c')

gnome = import('gnome')

executable('tasks',
  'main.c',

  'tasks-list.c',
  'tasks-manager.c',
  'tasks-task.c',

  'tasks-preferences-window.c',
  'tasks-utils.c',
  'tasks-view.c',
  'tasks-window.c',
  gnome.compile_resources('tasks-resources',
    'tasks.gresource.xml',
    c_name: 'tasks'
  ),
  gnome.compile_schemas(),
  dependencies: [
    dependency('gtk4'),
    dependency('libadwaita-1'),
  ],
#  install: true,
)

#install_data('org.example.Tasks.gschema.xml',
#  install_dir: join_paths(get_option('datadir'), 'glib-2.0' / 'schemas')
#)

#gnome.post_install(glib_compile_schemas: true)
