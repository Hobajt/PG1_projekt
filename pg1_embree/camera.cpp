#include "stdafx.h"
#include "camera.h"

Camera::Camera(const int width, const int height, const float fov_y, const vec3f view_from, const vec3f view_at) {
	width_ = width;
	height_ = height;
	fov_y_ = fov_y;

	view_from_ = view_from;
	view_at_ = view_at;

	f_y_ = height / (2 * tanf(fov_y * 0.5f));

	Update();
}

RTCRay Camera::GenerateRay(const float x_i, const float y_i) const {
	RTCRay ray = RTCRay();

	vec3f dir = vec3f( x_i - width_ * 0.5f, height_ * 0.5f - y_i, -f_y_ ).normalize();
	dir = M_c_w_ * dir;
	dir.normalize();

	ray.org_x = view_from_.x;
	ray.org_y = view_from_.y;
	ray.org_z = view_from_.z;

	ray.dir_x = dir.x;
	ray.dir_y = dir.y;
	ray.dir_z = dir.z;

	ray.tnear = FLT_MIN;
	ray.tfar = FLT_MAX;

	ray.mask = 0;
	ray.id = 0;
	ray.flags = 0;

	return ray;
}

void Camera::Update() {
	vec3f z_c = (view_from_ - view_at_).normalize();
	vec3f x_c = vec3f{ 0.f, 0.f, 1.f }.cross(z_c).normalize();
	vec3f y_c = z_c.cross(x_c).normalize();
	M_c_w_ = Matrix3x3(x_c, y_c, z_c);
}