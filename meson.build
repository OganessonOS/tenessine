project('tenessine', ['c', 'cpp'],
	license: 'MIT',
	meson_version: '>=0.57.0',
	default_options: [
		'cpp_std=gnu++20',
		'c_std=gnu11',
		'warning_level=2'
	])

add_project_arguments('-Wimplicit-fallthrough', '-Werror=misleading-indentation', '-Werror=switch', language: ['c', 'cpp'])

# declare constants that subdirs are going to use
c = meson.get_compiler('cpp')
cxx = meson.get_compiler('cpp')
libgcc = cxx.find_library('gcc')
arch = host_machine.cpu_family()

supported_archs = [
	'x86_64'
]

if not supported_archs.contains(arch)
	error('unknown architecture ' + arch)
endif

cpp_files = files()
c_files = files()

tenessine_link_args = []
tenessine_c_args = []
tenessine_cpp_args = []

tenessine_includes = []

tenessine_link_depends = files()

subdir('arch/' + arch)
subdir('generic')

executable('tennesine', c_files, cpp_files,
	include_directories: tenessine_includes,
	c_args: tenessine_c_args,
	cpp_args: tenessine_cpp_args,
	link_args: tenessine_link_args,
	link_depends: tenessine_link_depends,
	install: true
)
