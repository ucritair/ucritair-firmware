#version 330

uniform sampler2D tex;
uniform int brightness;

in vec2 uv;
out vec4 col;

void main()
{
	vec2 flipped = uv;
	flipped.y = 1.0-uv.y;
	col = texture(tex, flipped);
	col *= smoothstep(-40, 75, brightness);
	vec4 gamma = vec4(1.2, 1.2, 1.6, 1.0);
	col = pow(col, vec4(1.0)/gamma);
}
