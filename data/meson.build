glade_xml = 'glade/libhandy.xml'

if get_option('glade_catalog')
  gladeui = dependency('gladeui-2.0')
  dtd = join_paths(gladeui.get_pkgconfig_variable('catalogdir'), 'glade-catalog.dtd')

  glade_catalogdir = join_paths(get_option('prefix'), get_option('datadir'), 'glade', 'catalogs')
  install_data(glade_xml, install_dir: glade_catalogdir)

  xmllint = find_program('xmllint', required: true)
  if xmllint.found()
    custom_target(
      'xmllint',
        build_by_default: true,
        input: glade_xml,
        output: 'doesnotexist',
        command: [xmllint, '--dtdvalid', dtd, '--noout', '@INPUT@'],
    )
  endif
endif
