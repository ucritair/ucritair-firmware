#include "cat_menu.h"

#include "cat_math.h"
#include "mesh_assets.h"
#include "cat_render.h"
#include "cat_input.h"
#include "math.h"

CAT_mesh* mesh;

CAT_vec4 eye;
CAT_mat4 V;
CAT_mat4 P;
CAT_mat4 PV;
CAT_mat4 S;

float theta_h;

void CAT_MS_hedron(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_hedron);

			mesh = &eth_mesh;

			eye = (CAT_vec4) {0, 0, 6, 1.0f};
			CAT_vec4 up = (CAT_vec4) {0, 1, 0, 0};
			CAT_vec4 forward = CAT_vec4_sub(eye, (CAT_vec4){0, 0, 0, 1.0f});
			CAT_vec4 right = CAT_vec4_cross(up, forward);
			up = CAT_vec4_cross(forward, right);
			right = CAT_vec4_normalize(right);
			up = CAT_vec4_normalize(up);
			forward = CAT_vec4_normalize(forward);
			float tx = CAT_vec4_dot(eye, right);
			float ty = CAT_vec4_dot(eye, up);
			float tz = CAT_vec4_dot(eye, forward);
			V = (CAT_mat4)
			{{
				right.x, right.y, right.z, -tx,
				up.x, up.y, up.z, -ty,
				forward.x, forward.y, forward.z, -tz,
				0, 0, 0, 1
			}};

			float n = 0.01f;
			float f = 100.0f;
			float hfov = 1.57079632679 * 0.5f;
			float width = 2 * n * tan(hfov / 2);
			float asp = ((float) CAT_LCD_SCREEN_W / (float) CAT_LCD_SCREEN_H);
			float height = width / asp;
			P = (CAT_mat4)
			{{
				2 * n / width, 0, 0, 0,
				0, -2 * n / height, 0, 0,
				0, 0, -f/(f-n), -f*n/(f-n),
				0, 0, -1, 0
			}};

			PV = CAT_matmul(P, V);

			S = (CAT_mat4)
			{
				CAT_LCD_SCREEN_W / 2, 0, 0, CAT_LCD_SCREEN_W / 2,
				0, CAT_LCD_SCREEN_H / 2, 0, CAT_LCD_SCREEN_H / 2,
				0, 0, 1, 0,
				0, 0, 0, 1
			};

			theta_h = 0;
			break;
		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_back();

			theta_h += CAT_get_delta_time();
			if(theta_h >= 6.28318530718f)
				theta_h = 0;
			break;
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

CAT_vec4 fvtov4(int fidx, int vidx)
{
	return (CAT_vec4)
	{
		mesh->verts[mesh->faces[fidx*3+vidx]*3+0],
		mesh->verts[mesh->faces[fidx*3+vidx]*3+1],
		mesh->verts[mesh->faces[fidx*3+vidx]*3+2],
		1.0f
	};
}

void CAT_render_hedron()
{
	CAT_frameberry(0x0000);

	CAT_mat4 M = CAT_rotmat(0, theta_h, 0);
	CAT_mat4 MVP = CAT_matmul(PV, M);

	// The train arrives in clipspace
	for(int i = 0; i < mesh->n_faces; i++)
	{
		CAT_vec4 a = fvtov4(i, 0);
		CAT_vec4 b = fvtov4(i, 1);
		CAT_vec4 c = fvtov4(i, 2);

		a = CAT_matvec_mul(MVP, a);
		b = CAT_matvec_mul(MVP, b);
		c = CAT_matvec_mul(MVP, c);
	
		// W-culling
		bool clip_a = CAT_is_clipped(a);
		bool clip_b = CAT_is_clipped(b);
		bool clip_c = CAT_is_clipped(c);
		if(clip_a && clip_b && clip_c)
			continue;

		CAT_perspdiv(&a);
		CAT_perspdiv(&b);
		CAT_perspdiv(&c);

		a = CAT_matvec_mul(S, a);
		b = CAT_matvec_mul(S, b);
		c = CAT_matvec_mul(S, c);
		
		CAT_lineberry(a.x, a.y, b.x, b.y, 0xFFFF);
		CAT_lineberry(b.x, b.y, c.x, c.y, 0xFFFF);
		CAT_lineberry(c.x, c.y, a.x, a.y, 0xFFFF);
	}
}