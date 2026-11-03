project(
        'fever',
        'cpp',
        version : '0.1',
        meson_version : '>= 1.3.0',
        default_options : ['warning_level=3', 'cpp_std=c++20'],
)

if host_machine.system() == 'windows'
    add_project_arguments(
        '-DPLATFORM_WINDOWS',
        language : 'cpp',
    )
elif host_machine.system() == 'linux'
    add_project_arguments(
        '-DPLATFORM_UNIX',
        '-DPLATFORM_LINUX',
        language : 'cpp',
    )
else
    add_project_arguments(
        '-DPLATFORM_UNIX',
        language : 'cpp',
    )
endif

cc = meson.get_compiler('cpp')

src_inc = include_directories('src')


# engine/core
# ---------------------------------------------
core_src = files(
    'src/engine/core/net.cc'
)

system_net_dep = dependency('', required : false)
if host_machine.system() == 'windows'
    system_net_dep = cc.find_library('ws2_32')
endif

core_lib = static_library(
    'core',
    sources : core_src,
    include_directories : src_inc,
    dependencies : system_net_dep,
)

core_dep = declare_dependency(
    link_with : core_lib,
    include_directories : src_inc,
    dependencies : system_net_dep,
)


# engine
# ---------------------------------------------
executable(
    'engine',
    'src/engine/main.cc',
    dependencies : core_dep,
)
