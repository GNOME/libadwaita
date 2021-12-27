# Uncomment to build separately
# project('hello', 'c')

gnome = import('gnome')

executable('hello',
  'hello.c',
  dependencies: [
    dependency('gtk4'),
    dependency('libadwaita-1'),
  ],
  install: false,
)
