#include "Triangle.hpp"

#include <cstddef>
#include <cstring>
#include <iostream>

namespace di_renderer::graphics {

    static const char* vertex_src = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec2 aUV;

out vec3 vColor;
out vec3 vNormal;
out vec2 vUV;
out vec3 vPosition;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;

void main() {
    vColor = aColor;
    vNormal = uNormalMatrix * aNormal;
    vUV = aUV;
    vPosition = vec3(uModel * vec4(aPos, 1.0));
    
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}
)";

    static const char* fragment_src = R"(
#version 330 core
in vec3 vColor;
in vec3 vNormal;
in vec2 vUV;
in vec3 vPosition;

out vec4 FragColor;

uniform bool uUseTexture = false;
uniform sampler2D uTexture;
uniform vec3 uLightPos = vec3(2.0, 2.0, 2.0);
uniform vec3 uCameraPos = vec3(0.0, 0.0, 0.0);
uniform vec3 uLightColor = vec3(1.0, 1.0, 1.0);
uniform vec3 uAmbientColor = vec3(0.2, 0.2, 0.3);
uniform float uAmbientStrength = 0.5;
uniform float uDiffuseStrength = 0.7;

void main() {
    vec3 normalizedNormal = normalize(vNormal);
    vec3 lightDir = normalize(uLightPos - vPosition);
    
    // Diffuse lighting with controlled intensity
    float diff = max(dot(normalizedNormal, lightDir), 0.0);
    vec3 diffuse = uDiffuseStrength * diff * uLightColor;
    
    // Stronger ambient lighting
    vec3 ambient = uAmbientStrength * uAmbientColor;
    
    // Softer specular highlights
    vec3 viewDir = normalize(uCameraPos - vPosition);
    vec3 reflectDir = reflect(-lightDir, normalizedNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
    vec3 specular = 0.3 * spec * uLightColor;
    
    // Combine lighting with better balance
    vec3 lighting = (ambient + diffuse + specular) * vColor;
    
    // Gamma correction
    lighting = pow(lighting, vec3(1.0/2.2));
    
    if (uUseTexture) {
        vec4 texColor = texture(uTexture, vUV);
        if (texColor.a < 0.1) discard;
        FragColor = vec4(lighting * texColor.rgb, texColor.a);
    } else {
        FragColor = vec4(lighting, 1.0);
    }
}
)";

    static GLuint compile_shader(GLenum type, const char* src) {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        GLint ok = 0;
        glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (ok == 0) {
            char buf[1024];
            glGetShaderInfoLog(s, sizeof(buf), nullptr, buf);
            std::cerr << "Shader compile error: " << buf << '\n';
            glDeleteShader(s);
            return 0;
        }
        return s;
    }

    GLuint create_shader_program() {
        GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_src);
        if (vs == 0u) {
            return 0;
        }
        GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_src);
        if (fs == 0u) {
            glDeleteShader(vs);
            return 0;
        }
        GLuint prog = glCreateProgram();
        glAttachShader(prog, vs);
        glAttachShader(prog, fs);
        glLinkProgram(prog);
        GLint ok = 0;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (ok == 0) {
            char buf[1024];
            glGetProgramInfoLog(prog, sizeof(buf), nullptr, buf);
            std::cerr << "Program link error: " << buf << '\n';
            glDeleteProgram(prog);
            prog = 0;
        }
        glDeleteShader(vs);
        glDeleteShader(fs);
        return prog;
    }

    void destroy_shader_program(GLuint program) {
        if (program != 0u) {
            glDeleteProgram(program);
        }
    }

    static GLuint g_vao = 0;
    static GLuint g_vbo = 0;
    static GLuint g_ebo = 0;

    void init_mesh_batch() {
        if (g_vao != 0) {
            return;
        }

        glGenVertexArrays(1, &g_vao);
        glGenBuffers(1, &g_vbo);
        glGenBuffers(1, &g_ebo);

        glBindVertexArray(g_vao);

        glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) nullptr);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, color));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, uv));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void destroy_mesh_batch() {
        if (g_ebo != 0u) {
            glDeleteBuffers(1, &g_ebo);
            g_ebo = 0;
        }
        if (g_vbo != 0u) {
            glDeleteBuffers(1, &g_vbo);
            g_vbo = 0;
        }
        if (g_vao != 0u) {
            glDeleteVertexArrays(1, &g_vao);
            g_vao = 0;
        }
    }

    void draw_indexed_mesh(const Vertex* vertices, size_t vertex_count, const unsigned int* indices, size_t index_count,
                           GLuint shader_program) {
        if ((vertices == nullptr) || (indices == nullptr) || vertex_count == 0 || index_count == 0) {
            return;
        }
        if (g_vao == 0 || g_vbo == 0 || g_ebo == 0) {
            init_mesh_batch();
        }

        glBindVertexArray(g_vao);

        glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(Vertex), vertices, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(unsigned int), indices, GL_DYNAMIC_DRAW);

        glUseProgram(shader_program);
        glDrawElements(GL_TRIANGLES, (GLsizei) index_count, GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);
        glUseProgram(0);
    }

} // namespace di_renderer::graphics
