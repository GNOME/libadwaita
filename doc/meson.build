if get_option('gtk_doc')

subdir('xml')

private_headers = [
    'config.h',
    'gtkprogresstrackerprivate.h',
    'gtk-window-private.h',
    'hdy-main-private.h',
    'hdy-preferences-group-private.h',
    'hdy-preferences-page-private.h',
    'hdy-style-private.h',
    'hdy-view-switcher-button-private.h',
]

images = [
  'images/arrows.png',
  'images/dialer.png',
  'images/dialog-desktop.png',
  'images/dialog-mobile.png',
  'images/header-bar.png',
  'images/list.png',
  'images/preferences-window.png',
  'images/search.png',
  'images/view-switcher.png',
  'images/view-switcher-bar.png',
]

content_files = [
  'build-howto.xml',
  'visual-index.xml',
]

glib_prefix = dependency('glib-2.0').get_pkgconfig_variable('prefix')
glib_docpath = join_paths(glib_prefix, 'share', 'gtk-doc', 'html')
docpath = join_paths(get_option('datadir'), 'gtk-doc', 'html')

gnome.gtkdoc('libhandy',
           main_xml: 'handy-docs.xml',
            src_dir: [
                       join_paths(meson.source_root(), 'src'),
                       join_paths(meson.build_root(), 'src'),
                     ],
       dependencies: libhandy_dep,
  gobject_typesfile: 'libhandy.types',
          scan_args: [
                       '--rebuild-types',
                       '--ignore-headers=' + ' '.join(private_headers),
                     ],
       fixxref_args: [
                       '--html-dir=@0@'.format(docpath),
                       '--extra-dir=@0@'.format(join_paths(glib_docpath, 'glib')),
                       '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gobject')),
                       '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gio')),
                       '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gi')),
                       '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gtk3')),
                     ],
        install_dir: 'libhandy',
      content_files: content_files,
        html_assets: images,
            install: installable)

endif
