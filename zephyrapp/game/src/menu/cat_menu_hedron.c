#include "cat_menu.h"

#include "cat_math.h"
#include "mesh_assets.h"
#include "cat_render.h"
#include "cat_input.h"
#include "math.h"

typedef struct CAT_mat4
{
	float data[16];
} CAT_mat4;

typedef struct CAT_vec4
{
	float x;
	float y;
	float z;
	float w;
} CAT_vec4;

CAT_vec4 CAT_matvec_mul(CAT_mat4 M, CAT_vec4 v);
void CAT_perspdiv(CAT_vec4* v);
CAT_vec4 CAT_vec4_cross(CAT_vec4 u, CAT_vec4 v);
float CAT_vec4_dot(CAT_vec4 u, CAT_vec4 v);
CAT_vec4 CAT_vec4_sub(CAT_vec4 u, CAT_vec4 v);
CAT_vec4 CAT_vec4_normalize(CAT_vec4 v);
CAT_mat4 CAT_matmul(CAT_mat4 A, CAT_mat4 B);
CAT_mat4 CAT_rotmat(float x, float y, float z);
bool CAT_is_clipped(CAT_vec4 v);

CAT_vec4 CAT_matvec_mul(CAT_mat4 M, CAT_vec4 v)
{
	return (CAT_vec4)
	{
		M.data[0] * v.x + M.data[1] * v.y + M.data[2] * v.z + M.data[3] * v.w,
		M.data[4] * v.x + M.data[5] * v.y + M.data[6] * v.z + M.data[7] * v.w,
		M.data[8] * v.x + M.data[9] * v.y + M.data[10] * v.z + M.data[11] * v.w,
		M.data[12] * v.x + M.data[13] * v.y + M.data[14] * v.z + M.data[15] * v.w
	};
}

void CAT_perspdiv(CAT_vec4* v)
{
	v->x /= v->w;
	v->y /= v->w;
	v->z /= v->w;
	v->w /= v->w;
}

CAT_vec4 CAT_vec4_cross(CAT_vec4 u, CAT_vec4 v)
{
	return (CAT_vec4) {u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x, 0.0f};
}

float CAT_vec4_dot(CAT_vec4 u, CAT_vec4 v)
{
	return u.x * v.x + u.y * v.y + u.z * v.z + u.w * v.w;
}

CAT_vec4 CAT_vec4_sub(CAT_vec4 u, CAT_vec4 v)
{
	return (CAT_vec4) {u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w};
}

CAT_vec4 CAT_vec4_normalize(CAT_vec4 v)
{
	float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
	float s = 1.0f / len;
	return (CAT_vec4) {v.x * s, v.y * s, v.z * s, v.w * s};
}

CAT_mat4 CAT_matmul(CAT_mat4 A, CAT_mat4 B)
{
	CAT_mat4 C;
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			C.data[i * 4 + j] =
			A.data[i * 4 + 0] * B.data[0 * 4 + j] +
			A.data[i * 4 + 1] * B.data[1 * 4 + j] +
			A.data[i * 4 + 2] * B.data[2 * 4 + j] +
			A.data[i * 4 + 3] * B.data[3 * 4 + j];
		}
	}
	return C;
}

CAT_mat4 CAT_rotmat(float x, float y, float z)
{
	CAT_mat4 X =
	{
		1, 0, 0, 0,
		0, cosf(x), -sinf(x), 0,
		0, sinf(x), cosf(x), 0,
		0, 0, 0, 1
	};

	CAT_mat4 Y =
	{
		cosf(y), 0, sinf(y), 0,
		0, 1, 0, 0,
		-sinf(y), 0, cosf(y), 0,
		0, 0, 0, 1
	};

	CAT_mat4 Z =
	{
		cosf(z), -sinf(z), 0, 0,
		sinf(z), cosf(z), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	return CAT_matmul(Z, CAT_matmul(Y, X));
}

bool CAT_is_clipped(CAT_vec4 v)
{
	if(v.x < -v.w || v.x > v.w)
		return true;
	if(v.y < -v.w || v.y > v.w)
		return true;
	if(v.z < 0 || v.z > v.w)
		return true;
	return false;
}

CAT_mesh* mesh;

CAT_vec4 eye;
CAT_mat4 V;
CAT_mat4 P;
CAT_mat4 PV;
CAT_mat4 S;

float theta_h;

void CAT_MS_hedron(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_hedron);

			mesh = &hedron_mesh;

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
			float width = 2 * n * tanf(hfov / 2);
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
		case CAT_FSM_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_pushdown_pop();

			theta_h += CAT_get_delta_time_s();
			if(theta_h >= 6.28318530718f)
				theta_h = 0;
			break;
		case CAT_FSM_SIGNAL_EXIT:
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