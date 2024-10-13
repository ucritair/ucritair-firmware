#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include "png.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

float lerp(float a, float b, float t)
{
	return a * (1-t) + b * t;
}

typedef struct CAT_colour
{
	float r;
	float g;
	float b;
	float a;
} CAT_colour;

CAT_colour CAT_colour_lerp(CAT_colour a, CAT_colour b, float t)
{
	CAT_colour c;
	c.r = lerp(a.r, b.r, t);
	c.g = lerp(a.g, b.g, t);
	c.b = lerp(a.b, b.b, t);
	c.a = lerp(a.a, b.a, t);
	return c;
}

typedef struct CAT_texture
{
	int width;
	int height;
	float* data;
} CAT_texture;

void CAT_texture_init(CAT_texture* texture, int width, int height)
{
	texture->width = width;
	texture->height = height;
	texture->data = (float*) malloc(width * height * 4 * sizeof(float));
}

CAT_colour CAT_texture_read(CAT_texture* texture, int x, int y)
{
	int idx = y * texture->width * 4 + x * 4;
	CAT_colour c;
	c.r = texture->data[idx+0];
	c.g = texture->data[idx+1];
	c.b = texture->data[idx+2];
	c.a = texture->data[idx+3];
	return c;
}

void CAT_texture_write(CAT_texture* texture, int x, int y, CAT_colour c)
{
	CAT_colour base = CAT_texture_read(texture, x, y);
	CAT_colour blend = CAT_colour_lerp(base, c, c.a);
	int idx = y * texture->width * 4 + x * 4;
	texture->data[idx+0] = blend.r;
	texture->data[idx+1] = blend.g;
	texture->data[idx+2] = blend.b;
	texture->data[idx+3] = blend.a;
}

void CAT_load_PNG(CAT_texture* texture, const char* path)
{
	FILE* file = fopen(path, "rb");

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info = png_create_info_struct(png);
	png_init_io(png, file);
	png_set_sig_bytes(png, 0);
	int png_transforms =
		PNG_TRANSFORM_STRIP_16 |
		PNG_TRANSFORM_PACKING |
		PNG_TRANSFORM_EXPAND;
	png_read_png(png, info, png_transforms, NULL);

	png_uint_32 width;
	png_uint_32 height;
	png_get_IHDR(png, info, &width, &height, NULL, NULL, NULL, NULL, NULL);\
	
	size_t row_size = png_get_rowbytes(png, info);
	size_t size = row_size * height;
	uint8_t bytes[size];
	
	png_bytepp rows = png_get_rows(png, info);
	for(int i = 0; i < height; i++)
	{
		memcpy(bytes + row_size * i, rows[i], row_size);
	}

	CAT_texture_init(texture, width, height);
	for(int y = 0; y < height*4; y++)
	{
		for(int x = 0; x < width*4; x++)
		{
			int idx = y * width + x;
			uint8_t c_b = bytes[idx];
			float c_f = c_b / 255.0f;
			texture->data[idx] = c_f;
		}
	}

	png_destroy_read_struct(&png, &info, NULL);
}

typedef struct CAT_shader
{
	GLuint vert_id;
	GLuint frag_id;
	GLuint prog_id;
} CAT_shader;

void CAT_shader_init(CAT_shader* shader, char* vert_src, char* frag_src)
{
	int status;
	char log[512];

	shader->vert_id = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shader->vert_id, 1, &vert_src, NULL);
	glCompileShader(shader->vert_id);
	glGetShaderiv(shader->vert_id, GL_COMPILE_STATUS, &status);
	if(status != GL_TRUE)
	{
		glGetShaderInfoLog(shader->vert_id, 512, NULL, log);
		printf("While compiling vertex shader:\n%s\n", log);
	}
	
	shader->frag_id = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shader->frag_id, 1, &frag_src, NULL);
	glCompileShader(shader->frag_id);
	glGetShaderiv(shader->frag_id, GL_COMPILE_STATUS, &status);
	if(status != GL_TRUE)
	{
		glGetShaderInfoLog(shader->frag_id, 512, NULL, log);
		printf("While compiling fragment shader:\n%s\n", log);
	}

	shader->prog_id = glCreateProgram();
	glAttachShader(shader->prog_id, shader->vert_id);
	glAttachShader(shader->prog_id, shader->frag_id);
	glLinkProgram(shader->prog_id);
	glGetProgramiv(shader->prog_id, GL_LINK_STATUS, &status);
	if(status != GL_TRUE)
	{
		glGetShaderInfoLog(shader->prog_id, 512, NULL, log);
		printf("While linking shader program:\n%s\n", log);
	}
}

typedef struct CAT_display
{
	GLFWwindow* window;
	
	GLuint vao_id;
	GLuint vbo_id;
	CAT_texture frame;
	GLuint tex_id;
	
	CAT_shader shader;
	int tex_loc;	
} CAT_display;

void glfw_error_callback(int error, const char* description)
{
	printf("[%d] %s\n", error, description);
}

void CAT_display_init(CAT_display* display)
{
	glfwSetErrorCallback(glfw_error_callback);
	
	printf("Initializing GLFW\n");
	if(!glfwInit())
	{
		printf("Failed to initialize GLFW\n");
	}
	
	printf("Creating window\n");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    display->window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CAT", NULL, NULL);
	if(display->window == NULL)
	{
		printf("Failed to create window\n");
	}

	glfwMakeContextCurrent(display->window);
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("GL version: %s\n", glGetString(GL_VERSION));
	printf("SL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	
	printf("Initializing GLEW\n");
	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK)
	{
		printf("Failed to initialize GLEW\n");
	}
	
	printf("Initializing frame geometry\n");
	float geometry[12] =
	{
		-1.0f, -1.0f,
		1.0f, -1.0f,
		1.0f, 1.0f,
		1.0f, 1.0f,
		-1.0f, 1.0f,
		-1.0f, -1.0f
	};
    glGenVertexArrays(1, &display->vao_id);
    glBindVertexArray(display->vao_id);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &display->vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, display->vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW); 
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0);

	printf("Initializing frame texture\n");
    CAT_texture_init(&display->frame, SCREEN_WIDTH, SCREEN_HEIGHT);
    glGenTextures(1, &display->tex_id);
    glBindTexture(GL_TEXTURE_2D, display->tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 240, 320, 0, GL_RGBA, GL_FLOAT, display->frame.data);
    glGenerateMipmap(GL_TEXTURE_2D);	

	printf("Building shaders\n");
	FILE* vert_file = fopen("shaders/cat.vert", "r");
	fseek(vert_file, SEEK_SET, SEEK_END);
	size_t vert_len = ftell(vert_file);
	rewind(vert_file);
	char vert_src[vert_len+1];
	vert_src[vert_len] = '\0';
	fread(vert_src, 1, vert_len, vert_file);
	fclose(vert_file);
	FILE* frag_file = fopen("shaders/cat.frag", "r");
	fseek(frag_file, SEEK_SET, SEEK_END);
	size_t frag_len = ftell(frag_file);
	rewind(frag_file);
	char frag_src[frag_len+1];
	frag_src[frag_len] = '\0';
	fread(frag_src, 1, frag_len, frag_file);
	fclose(frag_file);
	CAT_shader_init(&display->shader, vert_src, frag_src);

	printf("Locating uniforms\n");
	display->tex_loc = glGetUniformLocation(display->shader.prog_id, "tex");
}

void CAT_display_refresh(CAT_display* display)
{
    glBindTexture(GL_TEXTURE_2D, display->tex_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA, GL_FLOAT, display->frame.data);
}

void CAT_debug_GUI_init(CAT_display* display)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void) io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.IniFilename = NULL;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(display->window, true);
	ImGui_ImplOpenGL3_Init("#version 150");
}

void CAT_debug_GUI_begin()
{
	int flags =
	ImGuiWindowFlags_NoSavedSettings |
	ImGuiWindowFlags_NoMove |
	ImGuiWindowFlags_NoResize |
	ImGuiWindowFlags_NoSavedSettings |
	ImGuiWindowFlags_NoCollapse |
	ImGuiWindowFlags_NoTitleBar |
	ImGuiWindowFlags_NoBackground;

	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(SCREEN_WIDTH, SCREEN_HEIGHT));
	ImGui::Begin("CAT", nullptr, flags);
}

void CAT_debug_GUI_end()
{
	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void CAT_debug_GUI_cleanup()
{
	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

typedef struct CAT_spriter
{
	CAT_texture* atlas;
	CAT_texture* frame;
	int sprites[64][4];
} CAT_spriter;

void CAT_spriter_init(CAT_spriter* spriter, CAT_texture* atlas, CAT_texture* frame)
{
	spriter->atlas = atlas;
	spriter->frame = frame;
}

void CAT_spriter_register(CAT_spriter* spriter, int sprite, int x, int y, int w, int h)
{
	spriter->sprites[sprite][0] = x;
	spriter->sprites[sprite][1] = y;
	spriter->sprites[sprite][2] = w;
	spriter->sprites[sprite][3] = h;
}

void CAT_spriter_draw(CAT_spriter* spriter, int x_w, int y_w, int sprite)
{
	int x_r = spriter->sprites[sprite][0];
	int y_r = spriter->sprites[sprite][1];
	int w = spriter->sprites[sprite][2];
	int h = spriter->sprites[sprite][3];

	for(int dy = 0; dy < h; dy++)
	{
		for(int dx = 0; dx < w; dx++)
		{
			CAT_colour c = CAT_texture_read(spriter->atlas, x_r+dx, y_r+dy);
			CAT_texture_write(spriter->frame, x_w+dx, y_w+dy, c);
		}
	}
}

int main(int argc, char** argv)
{
	printf("Initializing display\n");
	CAT_display display;
	CAT_display_init(&display);

	printf("Loading atlas\n");
	CAT_texture atlas;
	CAT_load_PNG(&atlas, "sprites/atlas.png");
	printf("Initializing spriter\n");
	CAT_spriter spriter;
	CAT_spriter_init(&spriter, &atlas, &display.frame);
	CAT_spriter_register(&spriter, 0, 0, 0, 16, 16);
	CAT_spriter_register(&spriter, 1, 16, 0, 16, 16);
	CAT_spriter_register(&spriter, 2, 32, 0, 16, 32);
	CAT_spriter_register(&spriter, 3, 48, 0, 16, 16);
	CAT_spriter_register(&spriter, 4, 64, 0, 16, 16);

	CAT_debug_GUI_init(&display);

	float last_time = 0;
	float time = 0;
	float delta_time = 0;

	printf("Entering main loop\n");
	while(!glfwWindowShouldClose(display.window))
	{
		time = glfwGetTime();
		delta_time = time - last_time;
		last_time = time;

		glfwPollEvents();

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		
		int tile_size = 16;
		int wall_rows = 6 * tile_size;
		for(int y = 0; y < wall_rows; y += tile_size)
		{
			for(int x = 0; x < SCREEN_WIDTH; x += tile_size)
			{
				CAT_spriter_draw(&spriter, x, y, 1);
			}
		}
		for(int y = wall_rows; y < SCREEN_HEIGHT; y += tile_size)
		{
			for(int x = 0; x < SCREEN_WIDTH; x += tile_size)
			{
				CAT_spriter_draw(&spriter, x, y, 0);
			}
		}

		CAT_display_refresh(&display);
		glUseProgram(display.shader.prog_id);
		glBindVertexArray(display.vao_id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, display.tex_id);
		glProgramUniform1i(display.shader.prog_id, display.tex_loc, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		CAT_debug_GUI_begin();
		ImGui::Text("%.0f FPS", 1.0f/delta_time);
		CAT_debug_GUI_end();
		
		glfwSwapBuffers(display.window);
	}

	CAT_debug_GUI_cleanup();

	return 0;
}
