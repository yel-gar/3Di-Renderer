#pragma once
#include "math/Matrix4x4.hpp"
#include "math/Vector3.hpp"

namespace di_renderer::render {
    class Camera {
      public:
        Camera();

        Camera(const math::Vector3& position, const math::Vector3& target, float fov, float aspect_ratio,
               float near_plane, float far_plane);

        void set_position(const math::Vector3& position);
        void set_target(const math::Vector3& target);
        void set_up_vector(const math::Vector3& up);

        void set_aspect_ratio(float aspect_ratio);
        void set_fov(float fov);
        void set_planes(float near_plane, float far_plane);

        const math::Vector3& get_position() const;
        const math::Vector3& get_target() const;

        void move_position(const math::Vector3& position);
        void move_target(const math::Vector3& target);
        void move(const math::Vector3& direction);

        math::Matrix4x4 get_view_matrix() const;
        math::Matrix4x4 get_projection_matrix() const;

      private:
        math::Vector3 m_position;
        math::Vector3 m_target;
        math::Vector3 m_up{0.0F, 1.0F, 0.0F};

        float m_fov;
        float m_aspect_ratio;
        float m_near_plane;
        float m_far_plane;
    };

} // namespace di_renderer::render
