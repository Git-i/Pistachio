project('Pistachio','cpp',
    default_options: ['cpp_std=c++17'])

engine = subproject('Pistachio-Engine')
pistachio_dep = engine.get_variable('pistachio_dep')

exe = executable('test app', 'test.cpp',dependencies: pistachio_dep)

