#version 330

uniform sampler2D tex;

in vec2 uv;
out vec4 col;

void main()
{
	vec2 flipped = uv;
	flipped.y = 1.0-uv.y;
	col = texture(tex, flipped);
}
