rendered_icon_variants = [
	{ 'output_suffix': '.png', 'target_suffix': '-from-svg', 'width': 256, 'height': 256, },
	{ 'output_suffix': '_48.png', 'target_suffix': '_48-from-svg', 'width': 48, 'height': 48, },
	{ 'output_suffix': '_32.png', 'target_suffix': '_32-from-svg', 'width': 32, 'height': 32, },
	{ 'output_suffix': '_16.png', 'target_suffix': '_16-from-svg', 'width': 16, 'height': 16, },
]

rendered_icon_defs = [
	{ 'name': 'icon_exe', 'input': 'icon_exe.svg', },
	{ 'name': 'icon_cps', 'input': 'icon_cps.svg', },
]

rendered_icons = {}

foreach def : rendered_icon_defs
	rendered_icon = []
	foreach variant : rendered_icon_variants
		if render_icons_with_inkscape.allowed()
			rendered_icon += custom_target(
				def['name'] + variant['target_suffix'],
				output: def['name'] + variant['output_suffix'],
				command: [
					inkscape,
					'--export-area-page',
					'--export-type', 'png',
					'--export-filename', '@OUTPUT@',
					'--export-width', variant['width'],
					'--export-height', variant['height'],
					files(def['input']),
				],
			)
		else
			rendered_icon += files('generated_icons' / def['name'] + variant['output_suffix'])
		endif
	endforeach
	rendered_icons += { def['name']: rendered_icon }
endforeach

if host_platform == 'windows'
	rc_conf_data = configuration_data()
	rc_conf_data.merge_from(conf_data)
	rc_conf_depends = []
	rc_conf_depend_files = []

	windows_icons = get_option('windows_icons')
	windows_utf8cp = get_option('windows_utf8cp')

	rc_conf_data.set10('HAVE_ICONS', windows_icons)
	rc_conf_data.set10('HAVE_UTF8CP', windows_utf8cp)
	rc_conf_data.set('ACTIVE_CODEPAGE_MANIFEST_ENTRY',
		windows_utf8cp
		? '<activeCodePage xmlns="http://schemas.microsoft.com/SMI/2019/WindowsSettings">UTF-8</activeCodePage>'
		: ''
	)

	rc_conf_depend_files += [ 'resource.h' ]

	if windows_icons
		foreach name, rendered_icon : rendered_icons
			rc_conf_depends += custom_target(
				name + '-ico',
				output: name + '.ico',
				command: [ python3_prog, files('make-ico.py'), '@OUTPUT@', rendered_icon ],
			)
		endforeach
	endif
	rc_conf_depend_files += configure_file(
		input: 'manifest.template.xml',
		output: 'manifest.xml',
		configuration: rc_conf_data,
	)
	powder_files += windows_mod.compile_resources(
		configure_file(
			input: 'powder-res.template.rc',
			output: 'powder-res.rc',
			configuration: rc_conf_data,
		),
		depends: rc_conf_depends,
		depend_files: rc_conf_depend_files,
		include_directories: include_directories('.'),
	)
elif host_platform == 'darwin'
	configure_file(
		input: 'Info.template.plist',
		output: 'Info.plist',
		configuration: conf_data,
	)
elif host_platform == 'linux'
	data_files += to_array.process(rendered_icons['icon_exe'], extra_args: 'icon_exe_png')
	data_files += to_array.process(rendered_icons['icon_cps'], extra_args: 'icon_cps_png')
	data_files += to_array.process('save.xml', extra_args: 'save_xml')
	data_files += to_array.process(configure_file(
		input: 'powder.template.desktop',
		output: 'powder.desktop',
		configuration: conf_data,
	), extra_args: 'powder_desktop')

	configure_file(
		input: 'appdata.template.xml',
		output: 'appdata.xml',
		configuration: conf_data,
	)
endif

data_files += to_array.process('save_local.png', extra_args: 'save_local_png')
data_files += to_array.process('save_online.png', extra_args: 'save_online_png')
data_files += to_array.process('font.bz2', extra_args: 'compressed_font_data')
