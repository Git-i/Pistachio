project('Pistachio','cpp',
    default_options: ['cpp_std=c++17'])
engine = subproject('Pistachio-Engine')
pistachio_dep = engine.get_variable('pistachio_dep')
exe = executable('Sandbox-app','Sandbox/src/SandBoxApp.cpp',dependencies: pistachio_dep)

