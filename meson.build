project(
	'the-powder-toy',
	[ 'c', 'cpp' ],
	version: 'the.cake.is.a.lie',
	default_options: [
		'c_std=c99',
		'cpp_std=c++latest,c++2a',
		'cpp_rtti=false',
	],
	meson_version: '>=0.64.0',
)

if get_option('prepare')
	# we're being run by prepare.py in a ghactions workflow only to determine the values of options; exit early
	subdir_done()
endif

python = import('python')
python3_prog = python.find_installation('python3')

fs = import('fs')
cmake = import('cmake')
to_array = generator(
	python3_prog,
	output: [ '@PLAINNAME@.cpp', '@PLAINNAME@.h' ],
	arguments: [ join_paths(meson.current_source_dir(), 'resources/to-array.py'), '@OUTPUT0@', '@OUTPUT1@', '@INPUT@', '@EXTRA_ARGS@' ]
)

render_icons_with_inkscape = get_option('render_icons_with_inkscape')
inkscape = find_program('inkscape', required: render_icons_with_inkscape)

c_compiler = meson.get_compiler('c')

is_x86 = host_machine.cpu_family() in [ 'x86', 'x86_64' ]
is_64bit = host_machine.cpu_family() in [ 'aarch64', 'x86_64' ]

host_arch = host_machine.cpu_family()
host_platform = host_machine.system()
# educated guesses follow, PRs welcome
if c_compiler.get_id() in [ 'msvc', 'clang-cl' ]
	if host_platform != 'windows'
		error('this seems fishy')
	endif
	host_libc = 'msvc'
elif c_compiler.get_id() in [ 'gcc' ] and host_platform == 'windows'
	host_libc = 'mingw'
elif host_platform in [ 'darwin' ]
	host_libc = 'macos'
elif host_platform in [ 'emscripten' ]
	host_platform = 'emscripten'
	host_libc = 'emscripten'
elif host_platform in [ 'android' ]
	host_platform = 'android'
	host_libc = 'bionic'
else
	if host_platform != 'linux'
		# TODO: maybe use 'default' in place of 'linux', or use something other than host_platform where details such as desktop integration are concerned
		warning('host platform is not linux but we will pretend that it is')
		host_platform = 'linux'
	endif
	host_libc = 'gnu'
endif

static_variant = get_option('static')
if static_variant != 'prebuilt' and host_platform == 'android'
	warning('only prebuilt libs are supported for android')
	static_variant = 'prebuilt'
endif
if static_variant == 'system' and host_platform == 'windows' and host_libc == 'msvc'
	warning('no way to find system libs for msvc on windows')
	static_variant = 'prebuilt'
endif

is_static = static_variant != 'none'

is_debug = get_option('buildtype') in [ 'debug', 'debugoptimized' ]
debug_info = get_option('debug_info')
is_optimized = get_option('optimization') not in [ '0', 'g' ]

app_exe = get_option('app_exe')

tpt_libs_static = 'none'
if static_variant == 'prebuilt'
	tpt_libs_static = 'static'
endif
if static_variant == 'none' and host_platform == 'windows' and host_libc == 'msvc'
	tpt_libs_static = 'dynamic'
endif
tpt_libs_debug = is_debug ? 'debug' : 'release'
tpt_libs_variant = '@0@-@1@-@2@-@3@'.format(host_arch, host_platform, host_libc, tpt_libs_static)
tpt_libs_vtag = get_option('tpt_libs_vtag')
if tpt_libs_vtag == ''
	tpt_libs_vtag = 'v20240112165024'
endif
if tpt_libs_static != 'none'
	if tpt_libs_variant not in [
		'x86_64-linux-gnu-static',
		'x86_64-windows-mingw-static',
		'x86_64-windows-msvc-static',
		'x86_64-windows-msvc-dynamic',
		'x86-windows-msvc-static',
		'x86-windows-msvc-dynamic',
		'x86_64-darwin-macos-static',
		'aarch64-darwin-macos-static',
		'x86-android-bionic-static',
		'x86_64-android-bionic-static',
		'arm-android-bionic-static',
		'aarch64-android-bionic-static',
		'wasm32-emscripten-emscripten-static',
	]
		error('no prebuilt @0@ libraries are currently provided'.format(tpt_libs_variant))
	endif
	tpt_libs = subproject('tpt-libs-prebuilt-@0@-@1@-@2@'.format(tpt_libs_variant, tpt_libs_debug, tpt_libs_vtag))
else
	if get_option('workaround_elusive_bzip2')
		bzip2_lib_name = get_option('workaround_elusive_bzip2_lib_name')
		bzip2_include_name = get_option('workaround_elusive_bzip2_include_name')
		bzip2_lib_dir = get_option('workaround_elusive_bzip2_lib_dir')
		bzip2_include_dir = include_directories(get_option('workaround_elusive_bzip2_include_dir'))
		bzip2_static = get_option('workaround_elusive_bzip2_static')
		meson.override_dependency('bzip2', declare_dependency(
			dependencies: c_compiler.find_library(
				bzip2_lib_name,
				has_headers: bzip2_include_name,
				dirs: bzip2_lib_dir,
				header_include_directories: bzip2_include_dir,
				static: bzip2_static,
			),
			include_directories: bzip2_include_dir,
		))
	endif
endif

x86_sse_level_str = get_option('x86_sse')
if x86_sse_level_str == 'auto'
	if host_machine.cpu_family() == 'x86_64' and host_platform != 'darwin'
		x86_sse_level = 20
	else
		x86_sse_level = 0
	endif
elif x86_sse_level_str == 'avx512'
	x86_sse_level = 500
elif x86_sse_level_str == 'avx2'
	x86_sse_level = 200
elif x86_sse_level_str == 'avx'
	x86_sse_level = 100
elif x86_sse_level_str == 'sse4.2'
	x86_sse_level = 42
elif x86_sse_level_str == 'sse4.1'
	x86_sse_level = 41
elif x86_sse_level_str == 'sse3'
	x86_sse_level = 30
elif x86_sse_level_str == 'sse2'
	x86_sse_level = 20
elif x86_sse_level_str == 'sse'
	x86_sse_level = 10
elif x86_sse_level_str == 'none'
	x86_sse_level = 0
endif

lua_variant = get_option('lua')
if lua_variant == 'auto'
	if host_platform == 'emscripten'
		lua_variant = 'lua5.2'
	else
		lua_variant = 'luajit'
	endif
endif
if lua_variant == 'luajit' and host_platform == 'emscripten'
	error('luajit does not work with emscripten')
endif

enable_http = get_option('http')
if host_platform == 'android'
	android_ndk_toolchain_prefix = meson.get_external_property('android_ndk_toolchain_prefix')
	android_platform = meson.get_external_property('android_platform')
	tpt_libs_android_toolchain_prefix = tpt_libs.get_variable('android_toolchain_prefix')
	tpt_libs_android_system_version = tpt_libs.get_variable('android_system_version')
	tpt_libs_android_platform = tpt_libs.get_variable('android_platform')
	if '@0@@1@-'.format(tpt_libs_android_toolchain_prefix, tpt_libs_android_system_version) != android_ndk_toolchain_prefix
		error('tpt-libs android toolchain mismatch')
	endif
	if tpt_libs_android_platform != android_platform
		error('tpt-libs android platform mismatch')
	endif
endif

project_link_args = []
project_c_args = []
project_cpp_args = []

if is_optimized
	args_ccomp_opt = []
	if c_compiler.get_argument_syntax() == 'msvc'
		args_ccomp_opt += [
			'/O2',
			'/Oy-',
			'/fp:fast',
		]
		project_link_args += [
			'/OPT:REF',
			'/OPT:ICF',
		]
	else
		args_ccomp_opt += [
			'-O3',
			'-ftree-vectorize',
			'-funsafe-math-optimizations',
			'-ffast-math',
			'-fomit-frame-pointer',
		]
	endif
	project_c_args += args_ccomp_opt
	project_cpp_args += args_ccomp_opt
endif

if debug_info
	args_ccomp_debuginfo = []
	if c_compiler.get_argument_syntax() == 'msvc'
		args_ccomp_debuginfo += [ '/Zi' ]
		project_link_args += [ '/debug:full' ]
	elif emcc_args
		project_link_args += [ '--source-map-base=./' ]
		emcc_args += [ '-gsource-map' ]
	else
		args_ccomp_debuginfo += [ '-g' ]
	endif
	project_c_args += args_ccomp_debuginfo
	project_cpp_args += args_ccomp_debuginfo
endif

if is_optimized
	args_ccomp_lto = []
	if c_compiler.get_id() in [ 'clang', 'clang-cl' ]
		# use ThinLTO for Clang/LLVM
		args_ccomp_lto += [ '-flto=thin' ]
	elif c_compiler.get_argument_syntax() == 'msvc'
		args_ccomp_lto += [ '/GL' ]
		project_link_args += [ '/LTCG' ]
	endif
	project_c_args += args_ccomp_lto
	project_cpp_args += args_ccomp_lto
endif

if c_compiler.get_argument_syntax() == 'msvc'
	args_msvc = [
		'/permissive-',
		'/Zc:__cplusplus',
		'/GS',
		'/D_SCL_SECURE_NO_WARNINGS',
	]
	project_c_args += args_msvc
	project_cpp_args += args_msvc
endif

# clang-cl supports both syntaxes. use the GCC one.
if c_compiler.get_argument_syntax() == 'msvc' and c_compiler.get_id() not in [ 'clang-cl' ]
	args_ccomp_sse = []
	if x86_sse_level == 30
		warning('SSE3 configured to be enabled but unavailable in msvc')
	endif
	if x86_sse_level >= 500
		args_ccomp_sse += [ '/arch:AVX512' ]
	elif x86_sse_level >= 200
		args_ccomp_sse += [ '/arch:AVX2' ]
	elif x86_sse_level >= 100
		args_ccomp_sse += [ '/arch:AVX' ]
	elif x86_sse_level >= 20 and host_machine.cpu_family() == 'x86'
		args_ccomp_sse += [ '/arch:SSE2' ]
	elif x86_sse_level >= 10 and host_machine.cpu_family() == 'x86'
		args_ccomp_sse += [ '/arch:SSE' ]
	endif
	project_c_args += args_ccomp_sse
	project_cpp_args += args_ccomp_sse
else
	args_ccomp_sse = []
	if host_platform == 'darwin' and x86_sse_level > 0
		message('SSE level explicitly configured but unavailable on macosx')
		x86_sse_level = 0
	endif
	if x86_sse_level >= 500
		args_ccomp_sse += [ '-mavx512f', '-mavx512cd', '-mavx512vl', '-mavx512dq', '-mavx512bw' ]
	elif x86_sse_level >= 200
		args_ccomp_sse += [ '-mavx2' ]
	elif x86_sse_level >= 100
		args_ccomp_sse += [ '-mavx' ]
	elif x86_sse_level >= 42
		args_ccomp_sse += [ '-msse4.2' ]
	elif x86_sse_level >= 41
		args_ccomp_sse += [ '-msse4.1' ]
	elif x86_sse_level >= 30
		args_ccomp_sse += [ '-msse3' ]
	elif x86_sse_level >= 20
		args_ccomp_sse += [ '-msse2' ]
	elif x86_sse_level >= 10
		args_ccomp_sse += [ '-msse' ]
	endif
	project_c_args += args_ccomp_sse
	project_cpp_args += args_ccomp_sse
endif

if host_platform == 'windows'
	args_ccomp_win = []
	defs_ccomp_win = []
	defs_ccomp_win += [
		'_WIN32_WINNT=0x0602',
		'NOMINMAX',
		'UNICODE',
		'_UNICODE',
	]
	windows_mod = import('windows')
	if is_static
		defs_ccomp_win += [ 'CURL_STATICLIB' ]
		if host_arch == 'x86_64'
			defs_ccomp_win += [ 'ZLIB_WINAPI' ]
		endif
	endif
	if tpt_libs_static == 'dynamic'
		foreach input_output_condition : tpt_libs.get_variable('config_dlls')
			dll_input = input_output_condition[0]
			dll_output = input_output_condition[1]
			dll_condition = input_output_condition[2]
			do_copy = false
			if dll_condition == 'all'
				do_copy = true
			elif dll_condition == 'lua=' + lua_variant
				do_copy = true
			endif
			if do_copy
				fs.copyfile(dll_input, dll_output)
			endif
		endforeach
	endif
	foreach def : defs_ccomp_win
		if c_compiler.get_argument_syntax() == 'msvc'
			args_ccomp_win += [ '/D' + def ]
		else
			args_ccomp_win += [ '-D' + def ]
		endif
	endforeach
	project_c_args += args_ccomp_win
	project_cpp_args += args_ccomp_win
elif host_platform == 'android'
	args_ccomp_android = []
	if not is_64bit
		args_ccomp_android += [ '-U_FILE_OFFSET_BITS' ]
	endif
	# android doesn't ship libc++_shared.so, so we might as well link it statically;
	# the alternative would be to grab libc++_shared.so from the NDK and ship it with
	# the app alongside libpowder.so, and possibly add it to SDL's list of libraries to load
	project_link_args += [ '-static-libstdc++' ]
	project_c_args += args_ccomp_android
	project_cpp_args += args_ccomp_android
elif host_platform == 'emscripten'
	args_ccomp_emscripten = [
		'-s', 'DISABLE_EXCEPTION_CATCHING=0',
	]
	project_link_args += [
		'-o', app_exe + '.js', # so we get a .wasm, and a .js
		'--no-heap-copy',
		'-s', 'WASM=1',
		'-s', 'ALLOW_MEMORY_GROWTH=1',
		'-s', 'FORCE_FILESYSTEM=1',
		'-s', 'EXIT_RUNTIME=0',
		'-s', 'EXPORTED_RUNTIME_METHODS=ccall,cwrap',
		'-s', 'FS_DEBUG',
		'-s', 'MODULARIZE',
		'-s', 'EXPORT_NAME=create_' + app_exe,
		'-Wl,-u,_emscripten_run_callback_on_thread',
		'-lidbfs.js',
	]
	project_link_args += args_ccomp_emscripten
	project_c_args += args_ccomp_emscripten
	project_cpp_args += args_ccomp_emscripten
endif

if c_compiler.get_argument_syntax() == 'msvc' and c_compiler.get_id() not in [ 'clang-cl' ]
	project_c_args += [
		'/wd5262',
		'/wd4834',
		'/wd4100',
	]
	project_cpp_args += [
		'/wd4834',
		'/wd4100',
	]
else
	project_c_args += [
		'-Wno-implicit-fallthrough',
		'-Wno-missing-field-initializers',
		'-Wno-unused-result',
		'-Wno-unused-parameter',
	]
	project_cpp_args += [
		'-Wno-invalid-offsetof',
		'-Wno-unused-result',
		'-Wno-missing-field-initializers',
		'-Wno-unused-parameter',
	]
endif

link_args_str = ''
c_args_str = ''
cpp_args_str = ''
foreach arg : project_link_args
	link_args_str += arg + ' '
endforeach
foreach arg : project_c_args
	c_args_str += arg + ' '
endforeach
foreach arg : project_cpp_args
	cpp_args_str += arg + ' '
endforeach

project_inc = include_directories([ 'src', 'resources' ])

if host_platform == 'windows'
	ident_platform = is_64bit ? 'WIN64' : 'WIN32'
elif host_platform == 'linux'
	ident_platform = is_64bit ? 'LIN64' : 'LIN32'
elif host_platform == 'darwin'
	ident_platform = host_arch == 'aarch64' ? 'MACOSARM' : 'MACOSX'
else
	ident_platform = 'UNKNOWN'
endif

common_files = []
common_inc_files = []
common_deps = []
powder_files = []
powder_inc_files = []
powder_deps = []
render_files = []
render_inc_files = []
render_deps = []
font_files = []
font_inc_files = []
font_deps = []

data_files = []

project_export_dynamic = false

subdir('src')
subdir('resources')

powder_files += data_files
render_files += data_files
font_files += data_files

if get_option('export_lua_symbols')
	if is_static and lua_variant != 'none' and not project_export_dynamic
		if host_platform == 'windows'
			error('Lua symbols are currently impossible to export correctly on Windows')
		elif c_compiler.has_link_argument('-Wl,--export-dynamic-symbol')
			project_link_args += [
				'-Wl,--export-dynamic-symbol=lua_*',
				'-Wl,--export-dynamic-symbol=luaL_*',
				'-Wl,--export-dynamic-symbol=luaopen_*',
			]
		else
			warning('your linker does not support -Wl,--export-dynamic-symbol so Meson will be instructed to export all symbols in order to enable loading Lua shared modules, which may blow up the size of the resulting binary')
			project_export_dynamic = true
		endif
	endif
endif

common_deps += dependency('threads')

mimalloc_options = cmake.subproject_options()
if is_optimized
	if debug_info
		mimalloc_options.add_cmake_defines({ 'CMAKE_BUILD_TYPE': 'RelWithDebInfo' })
	else
		mimalloc_options.add_cmake_defines({ 'CMAKE_BUILD_TYPE': 'Release' })
	endif
else
	mimalloc_options.add_cmake_defines({ 'CMAKE_BUILD_TYPE': 'Debug' })
endif
mimalloc_options.add_cmake_defines(
	{ 'MI_SECURE': 1 },
	{ 'MI_DEBUG_FULL': 1 },
	{ 'MI_SHOW_ERRORS': 1 },
	{ 'MI_USE_CXX': 1 },
	{ 'MI_WIN_REDIRECT': 0 },
)
mimalloc_options.add_cmake_defines({
	'CMAKE_C_FLAGS': c_args_str,
	'CMAKE_CXX_FLAGS': cpp_args_str,
	'CMAKE_EXE_LINKER_FLAGS': link_args_str,
	'CMAKE_SHARED_LINKER_FLAGS': link_args_str,
})
mimalloc_proj = cmake.subproject('mimalloc', options: mimalloc_options)
common_deps += mimalloc_proj.dependency('mimalloc-static')

if lua_variant == 'lua5.1' or lua_variant == 'lua5.2'
	lua_dep = dependency(lua_variant + '-c++', static: is_static, required: false)
	if not lua_dep.found()
		if not get_option('workaround_noncpp_lua')
			error('your system @0@ is not compatible with C++, configure with -Dworkaround_noncpp_lua=true to disable this error'.format(lua_variant))
		endif
		lua_dep = dependency(lua_variant, static: is_static)
	endif
elif lua_variant == 'luajit'
	lua_proj = subproject('luajit')
	lua_dep = lua_proj.get_variable('luajit_dep')
endif
powder_deps += lua_dep

if host_platform == 'emscripten'
	args_emcc_deps = [
		'-s', 'USE_ZLIB=1',
		'-s', 'USE_LIBPNG',
		'-s', 'USE_BZIP2=1',
		'-s', 'USE_SDL=2',
	]
	project_link_args += args_emcc_deps
	project_c_args += args_emcc_deps
	project_cpp_args += args_emcc_deps
else
	common_deps += dependency('zlib', static: is_static)
	common_deps += dependency('libpng16', static: is_static)
	common_deps += dependency('bzip2', static: is_static)
	powder_deps += dependency('sdl2', static: is_static)
	font_deps += dependency('sdl2', static: is_static)
	if enable_http
		powder_deps += dependency('libcurl', static: is_static)
	endif
endif

common_deps += dependency('jsoncpp', static: is_static)

powder_deps += dependency('fftw3f', static: is_static)

if get_option('build_powder')
	if host_platform == 'android'
		powder_sha = shared_library(
			app_exe,
			sources: [ powder_files, common_files ],
			include_directories: project_inc,
			c_args: project_c_args,
			cpp_args: project_cpp_args,
			link_args: project_link_args,
			dependencies: common_deps + powder_deps,
		)
		subdir('android')
	else
		executable(
			app_exe,
			sources: [ powder_files, common_files ],
			include_directories: project_inc,
			c_args: project_c_args,
			cpp_args: project_cpp_args,
			win_subsystem: is_debug ? 'console' : 'windows',
			link_args: project_link_args,
			dependencies: common_deps + powder_deps,
			export_dynamic: project_export_dynamic,
			install: true,
		)
	endif
endif

if get_option('build_render')
	if host_platform == 'emscripten'
		error('render does not target emscripten')
	endif
	render_link_args = project_link_args
	if host_platform == 'linux' and is_static
		render_link_args += [ '-static' ]
	endif
	executable(
		'render',
		sources: [ render_files, common_files ],
		include_directories: project_inc,
		c_args: project_c_args,
		cpp_args: project_cpp_args,
		link_args: render_link_args,
		dependencies: common_deps + render_deps,
		export_dynamic: project_export_dynamic,
	)
endif

if get_option('build_font')
	if host_platform == 'emscripten'
		error('font does not target emscripten')
	endif
	executable(
		'font',
		sources: [ font_files, common_files ],
		include_directories: project_inc,
		c_args: project_c_args,
		cpp_args: project_cpp_args,
		link_args: project_link_args,
		dependencies: common_deps + font_deps,
		export_dynamic: project_export_dynamic,
	)
endif
