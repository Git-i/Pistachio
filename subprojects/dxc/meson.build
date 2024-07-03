project('dxc', 'cpp')
inc_dirs = ['include/']
search_dir = meson.current_source_dir() / 'lib'

cmp = meson.get_compiler('cpp')
dxc_lib = cmp.find_library('dxcompiler',dirs:search_dir)

dxc_dep = [declare_dependency(include_directories: include_directories(inc_dirs)), dxc_lib]