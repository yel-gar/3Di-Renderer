#pragma once
#include "math/Matrix4x4.hpp"
#include "math/Vector3.hpp"

namespace di_renderer::math {
    class Camera {
      public:
        Camera();

        Camera(const Vector3& position, const Vector3& target, float fov, float aspect_ratio, float near_plane,
               float far_plane);

        void set_position(const Vector3& position);
        void set_target(const Vector3& target);
        void set_up_vector(const Vector3& up);

        void set_aspect_ratio(float aspect_ratio);
        void set_fov(float fov);
        void set_planes(float near_plane, float far_plane);

        const Vector3& get_position() const;
        const Vector3& get_target() const;

        Vector3 get_front() const;

        void move_position(const Vector3& position);
        void move_target(const Vector3& target);
        void move(const Vector3& direction);

        void rotate_view(float dx, float dy);

        void orbit_around_target(float dx, float dy);

        void zoom(float offset);

        Matrix4x4 get_view_matrix() const;
        Matrix4x4 get_projection_matrix() const;

      private:
        void update_euler_from_vectors();
        void update_vectors_from_euler(bool is_orbiting);

        Vector3 m_position;
        Vector3 m_target;
        Vector3 m_up{0.0f, 1.0f, 0.0f};

        float m_yaw;
        float m_pitch;

        // Hardcoded settings (needs to be debugged)
        float m_sensetivity = 0.1f;
        float m_zoom_speed = 0.5f;

        float m_fov;
        float m_aspect_ratio;
        float m_near_plane;
        float m_far_plane;
    };

} // namespace di_renderer::math
