project('Pistachio','cpp',
    default_options: ['cpp_std=c++17',' b_sanitize=address'])
engine = subproject('Pistachio-Engine',default_options: ['b_sanitize=address'])
pistachio_dep = engine.get_variable('pistachio_dep')
exe = executable('Sandbox-app','Sandbox/src/SandBoxApp.cpp',dependencies: pistachio_dep)

