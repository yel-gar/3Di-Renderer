render_lib = static_library('render',
                               'renderer.cpp',
                               'camera.cpp',
                               'shader.cpp',
                               'lighting.cpp',
                               'texture.cpp',
                               include_directories: incdir,
                               dependencies: [
                                   opengl_dep,
                                   glfw_dep,
                                   glew_dep,
                                   glm_dep,
                                   epoxy_dep
                               ],
                               link_with: [math_lib]
)