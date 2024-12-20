project('Pistachio','cpp',
    default_options: ['cpp_std=c++20'])#,'b_sanitize=address'])
pistachio = subproject('Pistachio-Engine')
pistachio_dep = pistachio.get_variable('pistachio_dep')
exe = executable('Sandbox-app','Sandbox/src/SandBoxApp.cpp',dependencies: pistachio_dep,override_options: ['cpp_std=c++20'])

