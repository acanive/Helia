project(
  'helia', 'c',
  version: '10.0'
)

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

common_flags = [
  '-DUSE_GETTEXT',
  '-DGETTEXT_PACKAGE=helia'
]

i18n = import('i18n')

po_dir = join_paths(meson.source_root(), 'po')

add_project_arguments(common_flags, language: 'c')

subdir('po')

src = [
	'src/helia.c',
	'src/helia-eqa.c',
	'src/helia-eqv.c',
	'src/helia-gst.c',
	'src/helia-info.c',
	'src/helia-keyboard.c',
	'src/helia-lang.c',
	'src/helia-level.c',
	'src/helia-location.c',
	'src/helia-list.c',
	'src/helia-mpegts.c',
	'src/helia-panel.c',
	'src/helia-pref.c',
	'src/helia-panel-treeview.c',
	'src/helia-scan.c',
	'src/helia-slider.c',
	'src/helia-treeview.c',
	'src/helia-video.c',
	'src/helia-window.c',
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
