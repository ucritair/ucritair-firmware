#version 330

layout(location=0) in vec2 coords;

out vec2 uv;

void main()
{
	gl_Position = vec4(coords, 0.0, 1.0);
	uv = coords;
}
