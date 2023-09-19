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

# Note that the builtin 'date' program in 'cmd.exe' (Visual Studio/
# clang-cl shells) cannot be found using find_program()
if date.found()
  sh = find_program('sh', required: target_system != 'windows')
  time_epoch = ''
  if sh.found()
    time_epoch = run_command(sh, '-c', 'echo "$SOURCE_DATE_EPOCH"', check: true).stdout().strip()
  endif
  if time_epoch != ''
    r = run_command(date, '-I', '-u', '-d', '@' + time_epoch, check: false)
  else
    r = run_command(date, '-I', check: false)
  endif
  if r.returncode() == 0
    today = r.stdout().strip()
  endif
endif

appdata_config = configuration_data()
appdata_config.set('BUILD_VERSION', meson.project_version())
appdata_config.set('BUILD_DATE', today)

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

appstreamcli = find_program('appstreamcli', required: false)
if appstreamcli.found()
  test('Validate appstream file', appstreamcli,
    args: ['validate', '--no-net', '--explain', appstream_file]
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
