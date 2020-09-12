if get_option('gtk_doc')

subdir('xml')

private_headers = [
    'config.h',
    'gtkprogresstrackerprivate.h',
    'gtk-window-private.h',
    'adw-animation-private.h',
    'adw-bidi-private.h',
    'adw-enums.h',
    'adw-enums-private.h',
    'adw-enum-value-object-private.h',
    'adw-fading-label-private.h',
    'adw-focus-private.h',
    'adw-gizmo-private.h',
    'adw-indicator-bin-private.h',
    'adw-main-private.h',
    'adw-preferences-group-private.h',
    'adw-preferences-page-private.h',
    'adw-shadow-helper-private.h',
    'adw-swipe-tracker-private.h',
    'adw-tab-private.h',
    'adw-tab-bar-private.h',
    'adw-tab-box-private.h',
    'adw-tab-view-private.h',
    'adw-view-switcher-button-private.h',
    'adw-window-mixin-private.h',
]

images = [
  'images/avatar.png',
  'images/header-bar.png',
  'images/list.png',
  'images/preferences-window.png',
  'images/search.png',
  'images/view-switcher.png',
  'images/view-switcher-bar.png',
]

content_files = [
  'build-howto.xml',
  'hdy-migrating-0-0-to-1.xml',
  'visual-index.xml',
]

glib_prefix  = dependency('glib-2.0').get_pkgconfig_variable('prefix')
glib_docpath = glib_prefix / 'share' / 'gtk-doc' / 'html'
docpath      = get_option('datadir') / 'gtk-doc' / 'html'

gnome.gtkdoc('libadwaita',
     module_version: apiversion,
           main_xml: 'adwaita-docs.xml',
            src_dir: [
                       meson.source_root() / 'src',
                       meson.build_root()  / 'src',
                     ],
       dependencies: libadwaita_dep,
  gobject_typesfile: 'libadwaita.types',
          scan_args: [
                       '--rebuild-types',
                       '--ignore-headers=' + ' '.join(private_headers),
                     ],
       fixxref_args: [
                       '--html-dir=@0@'.format(docpath),
                       '--extra-dir=@0@'.format(glib_docpath / 'glib'),
                       '--extra-dir=@0@'.format(glib_docpath / 'gobject'),
                       '--extra-dir=@0@'.format(glib_docpath / 'gio'),
                       '--extra-dir=@0@'.format(glib_docpath / 'gi'),
                       '--extra-dir=@0@'.format(glib_docpath / 'gtk3'),
                       '--extra-dir=@0@'.format(glib_docpath / 'gdk-pixbuf'),
                     ],
      content_files: content_files,
        html_assets: images,
            install: true)

endif
