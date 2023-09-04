cpp_files += files(
	'boot.cpp',
	'debug.cpp',
	# The PMM and VMM are arch-specific as there may be optimizations that are only possible on an architecture-level basis.
	'mem/pmm.cpp',
	'mem/vmm.cpp'
)

tenessine_link_args += ['-nostdlib', '-Wl,-T,' + meson.current_source_dir() + '/linker.ld', '-lgcc']
tenessine_link_depends += files('linker.ld')
tenessine_includes += include_directories('inc')
