# Declare subdirectories first
subdir('core')
subdir('math')
subdir('render')
subdir('io')
subdir('ui')

# Main executable
executable('3d-renderer',
           'main.cpp',
           include_directories: incdir,
           dependencies: [
               gtk_dep,
               opengl_dep,
               epoxy_dep,
               glm_dep
           ],
           link_with: [
               core_lib,
               math_lib,
               render_lib,
               io_lib,
               ui_lib
           ],
           install: true
)
