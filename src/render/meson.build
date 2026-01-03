render_lib = static_library(
    'render',
    'OpenGLArea.cpp',
    include_directories: incdir,
    dependencies: [gtk_dep, opengl_dep, glfw_dep, glew_dep, glm_dep, epoxy_dep],
    link_with: [core_lib, math_lib],
)
