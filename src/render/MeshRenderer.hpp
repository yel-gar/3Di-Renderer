#pragma once

#include "core/Mesh.hpp"

#include <epoxy/gl.h>

namespace di_renderer::graphics {

    void draw_mesh(const di_renderer::core::Mesh& mesh, GLuint shader_program);

} // namespace di_renderer::graphics
