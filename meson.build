project(
  'helia', 'c',
  version: '9.9'
)


gettext_option = get_option('enable-gettext')
if gettext_option

common_flags = [
  '-DUSE_GETTEXT',
  '-DGETTEXT_PACKAGE=helia'
]

  i18n = import('i18n')
  subdir('po')

  po_dir = join_paths(meson.source_root(), 'po')

  add_project_arguments(common_flags, language: 'c')
  message('** Using the Gettext')

endif


helia_prefix = get_option('prefix')
helia_bindir = join_paths(helia_prefix, get_option('bindir'))

desktop_sh  = find_program('data/desktop.sh')
gen_gres_sh = find_program('data/gen-gres.sh')

desktop_out    = 'build/helia.desktop'
helia_gres_xml = 'build/gresource.xml'

run_command(desktop_sh,  desktop_out, helia_bindir)
run_command(gen_gres_sh, helia_gres_xml)

gnome = import('gnome')

res = gnome.compile_resources(
	'helia_resources',
	helia_gres_xml,
	source_dir: 'build'
)

src = [
	'src/base.c',
	'src/dtv-gst.c',
	'src/dtv-level.c',
	'src/dtv-panel.c',
	'src/dtv-win.c',
	'src/helia-eqa.c',
	'src/helia-eqv.c',
	'src/info.c',
	'src/lang.c',
	'src/mpegts.c',
	'src/player-gst.c',
	'src/player-panel.c',
	'src/player-slider.c',
	'src/player-win.c',
	'src/pref.c',
	'src/scan.c',
	'src/tree-view.c',
	res
]

incdir = include_directories('include')

install_data(desktop_out, install_dir: join_paths('share', 'applications'))

dependencies = [
	dependency('gtk+-3.0'),
	dependency('gstreamer-video-1.0'),
	dependency('gstreamer-mpegts-1.0')
]

executable('helia', src, dependencies: dependencies, include_directories: incdir, install: true)
