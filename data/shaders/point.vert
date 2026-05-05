// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core


/**
typedef enum rs2_distortion
{
RS2_DISTORTION_NONE
RS2_DISTORTION_MODIFIED_BROWN_CONRADY, 1
RS2_DISTORTION_INVERSE_BROWN_CONRADY , 2
RS2_DISTORTION_FTHETA                , 3
RS2_DISTORTION_BROWN_CONRADY         , 4
RS2_DISTORTION_KANNALA_BRANDT4       , 5
RS2_DISTORTION_COUNT
} rs2_distortion;
*/

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

uniform UBO
{
	uniform vec3 		camera_world_position;
	uniform float		realsense_depth_scale;
	uniform float		point_size_scale;
	uniform float 		max_distance;
} ubo;

uniform cam_intrinsics
{
	uniform float         width;     /**< Width of the image in pixels */
	uniform float         height;    /**< Height of the image in pixels */
	uniform float         ppx;       /**< Horizontal coordinate of the principal point of the image, as a pixel offset from the left edge */
	uniform float         ppy;       /**< Vertical coordinate of the principal point of the image, as a pixel offset from the top edge */
	uniform float         fx;        /**< Focal length of the image plane, as a multiple of pixel width */
	uniform float         fy;        /**< Focal length of the image plane, as a multiple of pixel height */
	uniform int 		  model;     /**< Distortion model of the image */
	uniform float         coeffs[5]; /**< Distortion coefficients. Order for Brown-Conrady: [k1, k2, p1, p2, k3]. Order for F-Theta Fish-eye: [k1, k2, k3, k4, 0]. Other models are subject to their own interpretations */
} intrinsics;

uniform sampler2D depth_texture;
uniform sampler2D color_texture;

in vec3	in_Position;
in vec4	in_Color0;
in vec3	in_UV0;

out vec4 pass_Color;
out vec3 pass_Uvs;

vec3 deproject_pixel_to_point(vec2 pixel, float depth)
{
	depth = depth * ubo.realsense_depth_scale * 65535;
	float x = pixel.x * intrinsics.width;
	float y = pixel.y * intrinsics.height;

	x = (x - intrinsics.ppx) / intrinsics.fx;
	y = (y - intrinsics.ppy) / intrinsics.fy;

	float xo = x;
	float yo = y;

	if (intrinsics.model == 2)
	{
		// need to loop until convergence
		// 10 iterations determined empirically
		for (int i = 0; i < 10; i++)
		{
			float r2 = x * x + y * y;
			float icdist = 1.0 / (1.0 + ((intrinsics.coeffs[4] * r2 + intrinsics.coeffs[1]) * r2 + intrinsics.coeffs[0]) * r2);
			float xq = x / icdist;
			float yq = y / icdist;
			float delta_x = 2 * intrinsics.coeffs[2] * xq * yq + intrinsics.coeffs[3] * (r2 + 2 * xq * xq);
			float delta_y = 2 * intrinsics.coeffs[3] * xq * yq + intrinsics.coeffs[2] * (r2 + 2 * yq * yq);
			x = (xo - delta_x) * icdist;
			y = (yo - delta_y) * icdist;
		}
	}else if (intrinsics.model == 4)
	{
		// need to loop until convergence
		// 10 iterations determined empirically
		for (int i = 0; i < 10; i++)
		{
			float r2 = x * x + y * y;
			float icdist = 1.0 / (1.0 + ((intrinsics.coeffs[4] * r2 + intrinsics.coeffs[1]) * r2 + intrinsics.coeffs[0]) * r2);
			float delta_x = 2 * intrinsics.coeffs[2] * x * y + intrinsics.coeffs[3] * (r2 + 2 * x * x);
			float delta_y = 2 * intrinsics.coeffs[3] * x * y + intrinsics.coeffs[2] * (r2 + 2 * y * y);
			x = (xo - delta_x) * icdist;
			y = (yo - delta_y) * icdist;
		}

	}else if (intrinsics.model == 5)
	{
		float rd = sqrt(x * x + y * y);
		if (rd < 0.00001)
		{
			rd = 0.00001;
		}

		float theta = rd;
		float theta2 = rd * rd;
		for (int i = 0; i < 4; i++)
		{
			float f = theta * (1 + theta2 * (intrinsics.coeffs[0] + theta2 * (intrinsics.coeffs[1] + theta2 * (intrinsics.coeffs[2] + theta2 * intrinsics.coeffs[3])))) - rd;
			if (abs(f) < 0.00001)
			{
				break;
			}
			float df = 1 + theta2 * (3 * intrinsics.coeffs[0] + theta2 * (5 * intrinsics.coeffs[1] + theta2 * (7 * intrinsics.coeffs[2] + 9 * theta2 * intrinsics.coeffs[3])));
			theta -= f / df;
			theta2 = theta * theta;
		}
		float r = tan(theta);
		x *= r / rd;
		y *= r / rd;
	}
	if (intrinsics.model == 3)
	{
		float rd = sqrt(x * x + y * y);
		if (rd < 0.00001)
		{
			rd = 0.00001;
		}
		float r = (tan(intrinsics.coeffs[0] * rd) / atan(2 * tan(intrinsics.coeffs[0] / 2.0f)));
		x *= r / rd;
		y *= r / rd;
	}

	return vec3(depth * x, depth * y, depth);
}

void main(void)
{
	float r = texture(depth_texture, in_UV0.xy).r;
	vec4 c = texture(color_texture, in_UV0.xy).rgba;
	if(r>0.0)
	{
		vec3 p = deproject_pixel_to_point(in_UV0.xy, r);
		p.y *= -1.0;

		gl_Position =
			mvp.projectionMatrix *
			mvp.viewMatrix *
			mvp.modelMatrix * vec4(p, 1);

		pass_Color = c;

		gl_PointSize = ubo.point_size_scale;
	}else
	{
		pass_Color = vec4(0,0,0,0);
	}
}