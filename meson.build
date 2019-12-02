project('helia-app', 'c')

gnome = import('gnome')

res = gnome.compile_resources(
	'helia_resources',
	'build/data/gresource.xml',
	source_dir: 'build/data'
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

install_data('build/helia.desktop', install_dir: join_paths('share', 'applications'))

dependencies = [
	dependency('gtk+-3.0'),
	dependency('gstreamer-video-1.0'),
	dependency('gstreamer-mpegts-1.0')
]

executable('helia', src, dependencies: dependencies, include_directories: incdir, install: true)
