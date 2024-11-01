#include "cat_desktop.h"
#include "cat_core.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// DEV MODE

CAT_simulator simulator;

void GLFW_error_callback(int error, const char* msg)
{
	printf("GLFW error %d: %s\n", error, msg);
}

void CAT_shader_init(char* vert_src, char* frag_src)
{
	int status;
	char log[512];

	int vert_id = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert_id, 1, (const char**)&vert_src, NULL);
	glCompileShader(vert_id);
	glGetShaderiv(vert_id, GL_COMPILE_STATUS, &status);
	if(status != GL_TRUE)
	{
		glGetShaderInfoLog(vert_id, 512, NULL, log);
		printf("While compiling vertex shader:\n%s\n", log);
	}
	
	int frag_id = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_id, 1, (const char**)&frag_src, NULL);
	glCompileShader(frag_id);
	glGetShaderiv(frag_id, GL_COMPILE_STATUS, &status);
	if(status != GL_TRUE)
	{
		glGetShaderInfoLog(frag_id, 512, NULL, log);
		printf("While compiling fragment shader:\n%s\n", log);
	}

	simulator.prog_id = glCreateProgram();
	glAttachShader(simulator.prog_id, vert_id);
	glAttachShader(simulator.prog_id, frag_id);
	glLinkProgram(simulator.prog_id);
	glGetProgramiv(simulator.prog_id, GL_LINK_STATUS, &status);
	if(status != GL_TRUE)
	{
		glGetShaderInfoLog(simulator.prog_id, 512, NULL, log);
		printf("While linking shader program:\n%s\n", log);
	}
	glDeleteShader(vert_id);
	glDeleteShader(frag_id);
}

void CAT_platform_init()
{
	glfwSetErrorCallback(GLFW_error_callback);
	
	if(!glfwInit())
	{
		printf("Failed to initialize GLFW\n");
	}
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	simulator.lcd = glfwCreateWindow(LCD_SCREEN_W, LCD_SCREEN_H, "CAT", NULL, NULL);
	if(simulator.lcd == NULL)
	{
		printf("Failed to create window\n");
	}

	glfwMakeContextCurrent(simulator.lcd);
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("GL version: %s\n", glGetString(GL_VERSION));
	printf("SL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	
	printf("Initializing GLEW\n");
	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK)
	{
		printf("Failed to initialize GLEW\n");
	}

	float geometry[12] =
	{
		-1.0f, -1.0f,
		1.0f, -1.0f,
		1.0f, 1.0f,
		1.0f, 1.0f,
		-1.0f, 1.0f,
		-1.0f, -1.0f
	};
	glGenVertexArrays(1, &simulator.vao_id);
	glBindVertexArray(simulator.vao_id);
	glEnableVertexAttribArray(0);
	glGenBuffers(1, &simulator.vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, simulator.vbo_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW); 
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0);

	glGenTextures(1, &simulator.tex_id);
	glBindTexture(GL_TEXTURE_2D, simulator.tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, LCD_SCREEN_W, LCD_SCREEN_H, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
	glGenerateMipmap(GL_TEXTURE_2D);

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
	CAT_shader_init(vert_src, frag_src);

	simulator.tex_loc = glGetUniformLocation(simulator.prog_id, "tex");

	ALCdevice* al_device = alcOpenDevice(NULL);
	if(al_device == NULL)
	{
		printf("Failed to open audio device\n");
	}

	ALCcontext* al_context = alcCreateContext(al_device, NULL);
	if(!alcMakeContextCurrent(al_context))
	{
		printf("Failed to activate OpenAL context\n");
	}

	alGenSources(1, &simulator.al_src_id);
	alSourcef(simulator.al_src_id, AL_PITCH, 1);
	alSourcef(simulator.al_src_id, AL_GAIN, 1);
	alSource3f(simulator.al_src_id, AL_POSITION, 0, 0, 0);
	alSource3f(simulator.al_src_id, AL_VELOCITY, 0, 0, 0);
	alSourcei(simulator.al_src_id, AL_LOOPING, AL_FALSE);

	simulator.time = glfwGetTime();
	simulator.delta_time = 0;
}

void CAT_platform_tick()
{
	float time = glfwGetTime();
	simulator.delta_time = time - simulator.time;
	simulator.time = time;

	glfwPollEvents();
}

void CAT_platform_cleanup()
{
	alDeleteSources(1, &simulator.al_src_id);

	glDeleteShader(simulator.prog_id);
	glDeleteTextures(1, &simulator.tex_id);
	glDeleteBuffers(1, &simulator.vbo_id);
	glDeleteVertexArrays(1, &simulator.vao_id);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// LCD SCREEN

void CAT_LCD_post(uint16_t* buffer)
{
	glBindTexture(GL_TEXTURE_2D, simulator.tex_id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, LCD_SCREEN_W, LCD_SCREEN_H, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, buffer);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(simulator.prog_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, simulator.tex_id);
	glProgramUniform1i(simulator.prog_id, simulator.tex_loc, 0);

	glBindVertexArray(simulator.vao_id);
	glDrawArrays(GL_TRIANGLES, 0, 6);	
	glfwSwapBuffers(simulator.lcd);
}

bool CAT_LCD_is_posted()
{
	return true;
}

void CAT_LCD_set_backlight(int percent)
{
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// EINK SCREEN

void CAT_ink_post(uint8_t* buffer)
{
	return;
}

bool CAT_ink_is_posted()
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SPEAKER

void CAT_play_tone(float pitch_hz, float time_s)
{
	ALsizei rate = 22050;
	ALsizei count = rate * time_s;
	ALsizei size = sizeof(int16_t) * count;
	int16_t* samples = malloc(size);
	for(int i = 0; i < count; i++)
	{
		float p = 2 * M_PI * pitch_hz;
		float t = (float) i / (float) rate;
		samples[i] = 32767 * sin(p * t);
	}

	ALuint buf;
	alGenBuffers(1, &buf);
	alBufferData(buf, AL_FORMAT_MONO16, samples, size, rate);
	alSourcei(simulator.al_src_id, AL_BUFFER, buf);
	alSourcePlay(simulator.al_src_id);
	alDeleteBuffers(1, &buf);
	free(samples);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// INPUT

int input_map[CAT_BUTTON_LAST] =
{
	GLFW_KEY_ESCAPE,
	GLFW_KEY_TAB,
	GLFW_KEY_W,
	GLFW_KEY_D,
	GLFW_KEY_S,
	GLFW_KEY_A,
	GLFW_KEY_P,
	GLFW_KEY_L
};

uint16_t CAT_get_buttons()
{
	uint16_t mask = 0;
	for(int i = 0; i < CAT_BUTTON_LAST; i++)
	{
		int key = input_map[i];
		if(glfwGetKey(simulator.lcd, key))
		{
			mask |= 1 << i;
		}
	}
	return mask;
}

void CAT_get_touch(CAT_touch* touch)
{
	double x, y;
	glfwGetCursorPos(simulator.lcd, &x, &y);
	touch->x = x;
	touch->y = y;
	touch->pressure = glfwGetMouseButton(simulator.lcd, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// TIME

uint64_t CAT_get_time_ms()
{
	return glfwGetTime();
}

float CAT_get_delta_time()
{
	return simulator.delta_time;
}

void CAT_get_datetime(CAT_datetime* datetime)
{
	time_t t = time(NULL);
	struct tm* lt = localtime(&t);
	datetime->year = lt->tm_year;
	datetime->month = lt->tm_mon;
	datetime->day = lt->tm_mday;
	datetime->minute = lt->tm_min;
	datetime->second = lt->tm_sec;
}

void CAT_set_datetime(CAT_datetime* datetime)
{
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// MEMORY

void* CAT_malloc(int bytes)
{
	return malloc(bytes);
}

void CAT_free(void* ptr)
{
	free(ptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// STORAGE

void CAT_write_save(uint8_t* in)
{
	int fd = open("cat.dat", O_WRONLY | O_CREAT | O_TRUNC);
	write(fd, in, PERSISTENCE_PAGE_SIZE);
	close(fd);
}

bool CAT_check_save()
{
	struct stat buf;
	return stat("cat.dat", &buf) == 0;
}

void CAT_read_save(uint8_t* out)
{
	int fd = open("cat.dat", O_RDONLY);
	read(fd, out, PERSISTENCE_PAGE_SIZE);
	close(fd);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// POWER

int CAT_get_battery_pct()
{
	if(glfwWindowShouldClose(simulator.lcd))
	{
		return 0;
	}
	return 100;
}
