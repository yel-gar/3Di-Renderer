#pragma once
#include "math/Matrix4x4.hpp"
#include "math/Vector3.hpp"

namespace di_renderer::render {
    class Camera {
      public:
        Camera(math::Vector3 position, math::Vector3 target, float fov, float aspectRatio, float nearPlane,
               float farPlane);
        void set_position(math::Vector3 position);
        void set_target(math::Vector3 target);
        void set_aspect_ratio(float aspectRatio);
        void set_up_vector(math::Vector3 up);
        void set_fov(float fov);
        void set_planes(float nearPlane, float farPlane);

        math::Vector3 get_position() const;
        math::Vector3 get_target() const;

        void move_position(math::Vector3 position);
        void move_target(math::Vector3 target);
        void move(math::Vector3 direction);

        math::Matrix4x4& get_view_matrix() const;
        math::Matrix4x4& get_projection_matrix() const;

      private:
        math::Vector3 m_position;
        math::Vector3 m_target;
        math::Vector3 m_up;

        float m_fov;
        float m_aspect_ratio;
        float m_near_plane;
        float m_far_plane;

        mutable math::Matrix4x4 m_view_matrix;
        mutable bool m_view_changed{true};

        mutable math::Matrix4x4 m_projection_matrix;
        mutable bool m_projection_changed{true};
    };

} // namespace di_renderer::render
