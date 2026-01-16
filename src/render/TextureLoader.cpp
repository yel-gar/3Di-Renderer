#include "TextureLoader.hpp"

#include <filesystem>
#include <gdkmm/pixbuf.h>
#include <glibmm/error.h>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;
using namespace di_renderer::graphics;

void TextureLoader::resolve_path(std::string& texture_path, const std::string& base_path) {
    if (fs::exists(texture_path)) {
        return;
    }

    const fs::path base_dir = fs::path(base_path).parent_path();
    const fs::path relative_path = base_dir / texture_path;

    if (fs::exists(relative_path)) {
        texture_path = relative_path.string();
        return;
    }

    const std::vector<std::string> subdirs = {"textures", "texture", "tex", "images", "img"};
    for (const auto& subdir : subdirs) {
        const fs::path subdir_path = base_dir / subdir / fs::path(texture_path).filename();
        if (fs::exists(subdir_path)) {
            texture_path = subdir_path.string();
            return;
        }
    }

    const fs::path simple_path = base_dir / fs::path(texture_path).filename();
    if (fs::exists(simple_path)) {
        texture_path = simple_path.string();
    }
}

GLuint TextureLoader::load_texture(const std::string& filename, const std::string& base_path) {
    std::string texture_path = filename;
    resolve_path(texture_path, base_path);

    // Check cache first
    auto it = m_loaded_textures.find(texture_path);
    if (it != m_loaded_textures.end()) {
        return it->second;
    }

    try {
        if (!fs::exists(texture_path)) {
            texture_path = filename;
            if (!fs::exists(texture_path)) {
                std::cerr << "Texture file not found: " << texture_path << '\n';
                return 0;
            }
        }

        auto pixbuf = Gdk::Pixbuf::create_from_file(texture_path);
        const int width = pixbuf->get_width();
        const int height = pixbuf->get_height();
        const int channels = pixbuf->get_n_channels();
        const bool has_alpha = pixbuf->get_has_alpha();

        GLuint texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        guchar* pixels = pixbuf->get_pixels();
        const int rowstride = pixbuf->get_rowstride();

        if (has_alpha) {
            std::vector<guchar> rgba_buffer(static_cast<size_t>(width) * static_cast<size_t>(height) * 4);
            for (int y = 0; y < height; ++y) {
                // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                const guchar* src_row = pixels + static_cast<ptrdiff_t>(y * rowstride);
                guchar* dst_row = rgba_buffer.data() + static_cast<ptrdiff_t>(y * width * 4);
                for (int x = 0; x < width; ++x) {
                    dst_row[(x * 4) + 0] = src_row[(x * channels) + 0];
                    dst_row[(x * 4) + 1] = src_row[(x * channels) + 1];
                    dst_row[(x * 4) + 2] = src_row[(x * channels) + 2];
                    dst_row[(x * 4) + 3] = (channels > 3) ? src_row[(x * channels) + 3] : 255;
                }
                // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_buffer.data());
        } else {
            std::vector<guchar> rgb_buffer(static_cast<size_t>(width) * static_cast<size_t>(height) * 3);
            for (int y = 0; y < height; ++y) {
                // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                const guchar* src_row = pixels + static_cast<ptrdiff_t>(y * rowstride);
                guchar* dst_row = rgb_buffer.data() + static_cast<ptrdiff_t>(y * width * 3);
                for (int x = 0; x < width; ++x) {
                    dst_row[(x * 3) + 0] = src_row[(x * channels) + 0];
                    dst_row[(x * 3) + 1] = src_row[(x * channels) + 1];
                    dst_row[(x * 3) + 2] = src_row[(x * channels) + 2];
                }
                // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb_buffer.data());
        }

        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Cache the texture
        m_loaded_textures[texture_path] = texture;
        return texture;
    } catch (const Glib::Error& e) {
        std::cerr << "Failed to load texture from '" << texture_path << "': " << e.what() << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Texture loading error: " << e.what() << '\n';
    }
    return 0;
}

void TextureLoader::cleanup() {
    for (const auto& pair : m_loaded_textures) {
        if (pair.second != 0) {
            glDeleteTextures(1, &pair.second);
        }
    }
    m_loaded_textures.clear();
}
