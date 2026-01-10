#include "Triangle.hpp"

#include <cstddef>
#include <cstring>
#include <iostream>

namespace di_renderer {
    namespace graphics {

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

void main() {
    vColor = aColor;
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
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
uniform vec3 uLightPos = vec3(0.0, 0.0, 2.0);
uniform vec3 uCameraPos = vec3(0.0, 0.0, 0.0);
uniform vec3 uLightColor = vec3(1.0, 1.0, 1.0);

void main() {
    vec3 normalizedNormal = normalize(vNormal);
    vec3 lightDir = normalize(uLightPos - vPosition);
    float diff = max(dot(normalizedNormal, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;
    
    vec3 viewDir = normalize(uCameraPos - vPosition);
    vec3 reflectDir = reflect(-lightDir, normalizedNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * uLightColor;
    
    vec3 lighting = (diffuse + specular) * vColor;
    
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
            if (!ok) {
                char buf[1024];
                glGetShaderInfoLog(s, sizeof(buf), nullptr, buf);
                std::cerr << "Shader compile error: " << buf << std::endl;
                glDeleteShader(s);
                return 0;
            }
            return s;
        }

        GLuint create_shader_program() {
            GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_src);
            if (!vs)
                return 0;
            GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_src);
            if (!fs) {
                glDeleteShader(vs);
                return 0;
            }
            GLuint prog = glCreateProgram();
            glAttachShader(prog, vs);
            glAttachShader(prog, fs);
            glLinkProgram(prog);
            GLint ok = 0;
            glGetProgramiv(prog, GL_LINK_STATUS, &ok);
            if (!ok) {
                char buf[1024];
                glGetProgramInfoLog(prog, sizeof(buf), nullptr, buf);
                std::cerr << "Program link error: " << buf << std::endl;
                glDeleteProgram(prog);
                prog = 0;
            }
            glDeleteShader(vs);
            glDeleteShader(fs);
            return prog;
        }

        void destroy_shader_program(GLuint program) {
            if (program)
                glDeleteProgram(program);
        }

        static GLuint g_vao = 0;
        static GLuint g_vbo = 0;
        static constexpr std::size_t TRI_VERTEX_COUNT = 3;

        void init_triangle_batch() {
            if (g_vao != 0)
                return;

            glGenVertexArrays(1, &g_vao);
            glGenBuffers(1, &g_vbo);
            glBindVertexArray(g_vao);
            glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
            glBufferData(GL_ARRAY_BUFFER, TRI_VERTEX_COUNT * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

            // Position attribute (location 0)
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 0);

            // Color attribute (location 1)
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, color));

            // Normal attribute (location 2)
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));

            // UV attribute (location 3)
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, uv));

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }

        void destroy_triangle_batch() {
            if (g_vbo) {
                glDeleteBuffers(1, &g_vbo);
                g_vbo = 0;
            }
            if (g_vao) {
                glDeleteVertexArrays(1, &g_vao);
                g_vao = 0;
            }
        }

        void draw_single_triangle(const Vertex* vertices, GLuint shader_program) {
            if (!vertices)
                return;
            if (g_vao == 0 || g_vbo == 0)
                init_triangle_batch();

            glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, TRI_VERTEX_COUNT * sizeof(Vertex), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glUseProgram(shader_program);
            glBindVertexArray(g_vao);
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei) TRI_VERTEX_COUNT);
            glBindVertexArray(0);
            glUseProgram(0);
        }

        void draw_triangle_batch(const Vertex* vertices, size_t count, GLuint shader_program) {
            if (!vertices || count == 0)
                return;
            if (g_vao == 0 || g_vbo == 0)
                init_triangle_batch();

            glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
            glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), vertices, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glUseProgram(shader_program);
            glBindVertexArray(g_vao);
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei) count);
            glBindVertexArray(0);
            glUseProgram(0);
        }

    } // namespace graphics
} // namespace di_renderer
