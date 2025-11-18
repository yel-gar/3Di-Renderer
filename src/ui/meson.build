ui_lib = static_library(
    'ui',
    'OpenGLArea.cpp',
    'MainWindowHandler.cpp',
    'DiRendererApp.cpp',
    include_directories: incdir,
    dependencies: [
        gtkdep,
        opengl_dep,
        epoxy_dep
    ],
    link_with: [
#        core_lib,
#        math_lib,
#        render_lib,
#        io_lib
    ]
)

install_data('res/di_renderer.ui', install_tag: 'ui', install_dir: join_paths(get_option('datadir'), 'direnderer/ui'))
