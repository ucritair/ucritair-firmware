#version 330

uniform sampler2D tex;
uniform int brightness;

in vec2 uv;
out vec4 col;

void main()
{
	vec2 flipped = uv;
	flipped.y = 1.0-uv.y;
	col = smoothstep(-40, 75, brightness) * texture(tex, flipped);
}
