project('3d-renderer', 'cpp',
        version : '0.1.0',
        license : 'MIT',
        default_options : [
                'cpp_std=c++17',
                'warning_level=3',
                'werror=false',
                'default_library=static',
                'buildtype=debugoptimized'
        ]
)

# Compiler
cpp = meson.get_compiler('cpp')

# Dependencies
glib_dep = dependency('glib-2.0')
gtk_dep = dependency('gtkmm-3.0')
opengl_dep = dependency('gl')
glfw_dep = dependency('glfw3')
glew_dep = dependency('glew')
glm_dep = dependency('glm')
epoxy_dep = dependency('epoxy')  # For GtkGLArea

# Include directories
incdir = include_directories('src')

# Subdirectories
subdir('src')
subdir('tests')

# Summary
summary({
                'Build type': get_option('buildtype'),
                'C++ standard': get_option('cpp_std'),
                'GTK': gtk_dep.found(),
                'OpenGL': opengl_dep.found(),
                'GLFW': glfw_dep.found(),
        }, section: 'Configuration')
