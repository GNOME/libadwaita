desktop_file = i18n.merge_file(
  input: 'org.gnome.Adwaita1.Demo.desktop.in',
  output: 'org.gnome.Adwaita1.Demo.desktop',
  type: 'desktop',
  po_dir: '../../po',
  install: true,
  install_dir: datadir / 'applications'
)

desktop_utils = find_program('desktop-file-validate', required: false)
if desktop_utils.found()
  test('Validate desktop file', desktop_utils,
    args: [desktop_file]
  )
endif

today = 'unknown'
date = find_program('date',
         required: false)
if date.found()
  r = run_command(date, '-I')
  if r.returncode() == 0
    today = r.stdout().strip()
  endif
endif

appdata_config = configuration_data()
appdata_config.set('BUILD_VERSION', meson.project_version())
appdata_config.set('BUILD_DATE', today)

if target_system != 'windows'
  appstream_file = i18n.merge_file(
    input: configure_file(
      input: 'org.gnome.Adwaita1.Demo.metainfo.xml.in.in',
      output: 'org.gnome.Adwaita1.Demo.metainfo.xml.in',
      configuration: appdata_config
    ),
    output: 'org.gnome.Adwaita1.Demo.metainfo.xml',
    po_dir: '../../po',
    install: true,
    install_dir: datadir / 'metainfo'
  )
endif

appstream_util = find_program('appstream-util', required: false)
if appstream_util.found()
  test('Validate appstream file', appstream_util,
    args: ['validate', '--nonet', appstream_file]
  )
endif

install_data(
  'org.gnome.Adwaita1.Demo.svg',
  install_dir: datadir / 'icons' / 'hicolor' / 'scalable' / 'apps'
)

install_data(
  'org.gnome.Adwaita1.Demo-symbolic.svg',
  install_dir: datadir / 'icons' / 'hicolor' / 'symbolic' / 'apps'
)
