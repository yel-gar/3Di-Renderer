#pragma once

#include <epoxy/gl.h>
#include <gdkmm/pixbuf.h>
#include <glibmm/error.h>
#include <string>
#include <unordered_map>

namespace di_renderer::graphics {

    class TextureLoader {
      public:
        GLuint load_texture(const std::string& filename, const std::string& base_path);
        void cleanup();

      private:
        // Make this static since it doesn't use member variables
        static void resolve_path(std::string& texture_path, const std::string& base_path);

        std::unordered_map<std::string, GLuint> m_loaded_textures;
    };

} // namespace di_renderer::graphics
