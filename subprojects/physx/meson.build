project('physx','cpp')

cmp = meson.get_compiler('cpp')
look_dirs = []
id = cmp.get_id()
if(build_machine.system() == 'linux')
    root = 'linux.' + id
    root = meson.current_source_dir() / root
    look_dirs += root / 'checked'
endif
px_cooking_dep = cmp.find_library('PhysXCooking_static_64',dirs:look_dirs)
px_vehicle_dep = cmp.find_library('PhysXVehicle_static_64',dirs:look_dirs)
px_extensions_dep = cmp.find_library('PhysXExtensions_static_64',dirs:look_dirs)
px_common_dep = cmp.find_library('PhysXCommon_static_64',dirs:look_dirs)
px_dep = cmp.find_library('PhysX_static_64',dirs:look_dirs)
px_pvd_dep = cmp.find_library('PhysXPvdSDK_static_64',dirs:look_dirs)
px_char_kinematic_dep = cmp.find_library('PhysXCharacterKinematic_static_64',dirs:look_dirs)
px_foundation_dep = cmp.find_library('PhysXFoundation_static_64',dirs:look_dirs)
px_vehicle_2_dep = cmp.find_library('PhysXVehicle2_static_64',dirs:look_dirs)

px_all_deps = [
    px_cooking_dep,
    px_vehicle_dep,
    px_extensions_dep,
    px_common_dep,
    px_dep,
    px_pvd_dep,
    px_char_kinematic_dep,
    px_foundation_dep,
    px_vehicle_2_dep,
    declare_dependency(include_directories: include_directories('include/'))
]

