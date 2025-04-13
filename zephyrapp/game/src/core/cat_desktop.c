#include "cat_core.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "cat_math.h"
#include <string.h>
#include "cat_version.h"
#include <stdarg.h>
#include "cat_main.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// PLATFORM

struct
{
    GLFWwindow* window;
    GLuint vao_id;
    GLuint vbo_id;
    GLuint tex_id;
    GLuint prog_id;
    GLuint tex_loc;
	GLuint brightness_loc;

	uint64_t slept_s;
    float uptime_s;
    float delta_time_s;
} simulator =
{
	.window = NULL,
	.vao_id = 0,
	.vbo_id = 0,
	.tex_id = 0,
	.prog_id = 0,
	.tex_loc = 0,
	.brightness_loc = 0,

	.slept_s = 0,
	.uptime_s = 0,
	.delta_time_s = 0
};

void GLFW_error_callback(int error, const char* msg)
{
	CAT_printf("GLFW error %d: %s\n", error, msg);
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
		CAT_printf("While compiling vertex shader:\n%s\n", log);
	}
	
	int frag_id = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_id, 1, (const char**)&frag_src, NULL);
	glCompileShader(frag_id);
	glGetShaderiv(frag_id, GL_COMPILE_STATUS, &status);
	if(status != GL_TRUE)
	{
		glGetShaderInfoLog(frag_id, 512, NULL, log);
		CAT_printf("While compiling fragment shader:\n%s\n", log);
	}

	simulator.prog_id = glCreateProgram();
	glAttachShader(simulator.prog_id, vert_id);
	glAttachShader(simulator.prog_id, frag_id);
	glLinkProgram(simulator.prog_id);
	glGetProgramiv(simulator.prog_id, GL_LINK_STATUS, &status);
	if(status != GL_TRUE)
	{
		glGetShaderInfoLog(simulator.prog_id, 512, NULL, log);
		CAT_printf("While linking shader program:\n%s\n", log);
	}
	glDeleteShader(vert_id);
	glDeleteShader(frag_id);
}

void CAT_platform_init()
{
	CAT_printf
	(
		"Starting CAT v%d.%d.%d.%d...\n",
		CAT_VERSION_MAJOR, CAT_VERSION_MINOR,
		CAT_VERSION_PATCH, CAT_VERSION_PUSH
	);

	glfwSetErrorCallback(GLFW_error_callback);
	
	if(!glfwInit())
	{
		CAT_printf("Failed to initialize GLFW\n");
	}
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	simulator.window = glfwCreateWindow(CAT_LCD_SCREEN_W, CAT_LCD_SCREEN_H + 16, "μCritAir", NULL, NULL);
	if(simulator.window == NULL)
	{
		CAT_printf("Failed to create window\n");
	}

	glfwMakeContextCurrent(simulator.window);
	CAT_printf("Renderer: %s\n", glGetString(GL_RENDERER));
	CAT_printf("GL version: %s\n", glGetString(GL_VERSION));
	CAT_printf("SL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	CAT_printf("Initializing GLEW\n");
	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK)
	{
		CAT_printf("Failed to initialize GLEW\n");
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, CAT_LCD_SCREEN_W, CAT_LCD_SCREEN_H + 16, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
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
	simulator.brightness_loc = glGetUniformLocation(simulator.prog_id, "brightness");
	
	time_t sleep_time;
	int fd = open("sleep.dat", O_RDONLY);
	if(fd != -1)
	{
		read(fd, &sleep_time, sizeof(sleep_time));
		close(fd);
		time_t now;
		time(&now);
		simulator.slept_s = difftime(now, sleep_time);
	}
	simulator.uptime_s = glfwGetTime();
	simulator.delta_time_s = 0;	
}

void CAT_platform_tick()
{
	glfwPollEvents();

	simulator.delta_time_s = glfwGetTime() - simulator.uptime_s;
	simulator.uptime_s = glfwGetTime();
}

void CAT_platform_cleanup()
{
	glDeleteShader(simulator.prog_id);
	glDeleteTextures(1, &simulator.tex_id);
	glDeleteBuffers(1, &simulator.vbo_id);
	glDeleteVertexArrays(1, &simulator.vao_id);

	time_t now;
	time(&now);
	int fd = open("sleep.dat", O_WRONLY | O_CREAT | O_TRUNC,  S_IRUSR | S_IWUSR);
	write(fd, &now, sizeof(now));
	close(fd);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// LCD SCREEN

static uint16_t lcd_framebuffer[CAT_LCD_FRAMEBUFFER_PIXELS];
static int render_cycle = 0;

uint16_t* CAT_LCD_get_framebuffer()
{
	return lcd_framebuffer;
}

void CAT_LCD_post()
{
	int frame_offset = CAT_LCD_FRAMEBUFFER_H * CAT_get_render_cycle();
	
	if(CAT_get_screen_orientation() == CAT_SCREEN_ORIENTATION_DOWN)
	{
		frame_offset = CAT_LCD_FRAMEBUFFER_H * (CAT_LCD_FRAMEBUFFER_SEGMENTS - CAT_get_render_cycle() - 1);

		for(int y = 0; y < CAT_LCD_FRAMEBUFFER_H/2; y++)
		{
			for(int x = 0; x < CAT_LCD_FRAMEBUFFER_W; x++)
			{
				int y_flip = CAT_LCD_FRAMEBUFFER_H - y - 1;
				int x_flip = CAT_LCD_FRAMEBUFFER_W - x - 1;
				int temp = lcd_framebuffer[y_flip * CAT_LCD_FRAMEBUFFER_W + x_flip];
				lcd_framebuffer[y_flip * CAT_LCD_FRAMEBUFFER_W + x_flip] = lcd_framebuffer[y * CAT_LCD_FRAMEBUFFER_W + x];
				lcd_framebuffer[y * CAT_LCD_FRAMEBUFFER_W + x] = temp;
			}
		}
	}

	glBindTexture(GL_TEXTURE_2D, simulator.tex_id);
	glTexSubImage2D
	(
		GL_TEXTURE_2D, 0,
		0, frame_offset,
		CAT_LCD_SCREEN_W, CAT_LCD_FRAMEBUFFER_H,
		GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
		lcd_framebuffer
	);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(simulator.prog_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, simulator.tex_id);
	glProgramUniform1i(simulator.prog_id, simulator.tex_loc, 0);
	glProgramUniform1i(simulator.prog_id, simulator.brightness_loc, CAT_LCD_get_brightness());

	glBindVertexArray(simulator.vao_id);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void CAT_LCD_flip()
{
	glfwSwapBuffers(simulator.window);
}

bool CAT_LCD_is_posted()
{
	return true;
}

void CAT_set_render_cycle(int cycle)
{
	render_cycle = cycle;
}

int CAT_get_render_cycle()
{
	return render_cycle;
}

bool CAT_is_first_render_cycle()
{
	return render_cycle == 0;
}

bool CAT_is_last_render_cycle()
{
	return render_cycle == CAT_LCD_FRAMEBUFFER_SEGMENTS-1;
}

static uint8_t lcd_brightness = CAT_LCD_MAX_BRIGHTNESS;

uint8_t CAT_LCD_get_brightness()
{
	return lcd_brightness;
}

void CAT_LCD_set_brightness(uint8_t percent)
{
	lcd_brightness = clamp(percent, CAT_LCD_MIN_BRIGHTNESS, CAT_LCD_MAX_BRIGHTNESS);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// EINK SCREEN

void CAT_eink_post(uint8_t* buffer)
{
	return;
}

bool CAT_eink_is_posted()
{
	return true;
}

void CAT_eink_update()
{
	CAT_printf("[CALL] CAT_eink_update\n");
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// LEDs

uint16_t led_buffer[CAT_LCD_SCREEN_W * 16];

void CAT_set_LEDs(uint8_t r, uint8_t g, uint8_t b)
{
	float power = CAT_LED_get_brightness() / 100.0f;
	uint16_t scaled_rgb565 = 
	RGB8882565
	(
		SCALEBYTE(r, power),
		SCALEBYTE(g, power),
		SCALEBYTE(b, power)
	);

	for(int y = 0; y < 16; y++)
	{
		for(int x = 0; x < CAT_LCD_SCREEN_W; x++)
		{
			led_buffer[y * CAT_LCD_SCREEN_W + x] = scaled_rgb565;
		}
	}
	
	glBindTexture(GL_TEXTURE_2D, simulator.tex_id);
	glTexSubImage2D
	(
		GL_TEXTURE_2D, 0,
		0, CAT_LCD_SCREEN_H,
		CAT_LCD_SCREEN_W, 16,
		GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
		led_buffer
	);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// SPEAKER

void CAT_sound_power(bool value)
{
	CAT_printf("[CALL] CAT_sound_power\n");
}

void CAT_play_sound(CAT_sound* sound)
{
	CAT_printf("[CALL] CAT_play_sound\n");
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// INPUT

int input_map[] =
{
	GLFW_KEY_ESCAPE,
	GLFW_KEY_TAB,
	GLFW_KEY_P,
	GLFW_KEY_L,
	GLFW_KEY_S,
	GLFW_KEY_D,
	GLFW_KEY_A,
	GLFW_KEY_W
};
#define NUM_BUTTONS (sizeof(input_map) / sizeof(input_map[0]))

uint16_t CAT_get_buttons()
{
	uint16_t mask = 0;
	for(int i = 0; i < NUM_BUTTONS; i++)
	{
		int key = input_map[i];
		if(glfwGetKey(simulator.window, key))
		{
			mask |= 1 << i;
		}
	}
	return mask;
}

void CAT_get_touch(CAT_touch* touch)
{
	double x, y;
	glfwGetCursorPos(simulator.window, &x, &y);
	touch->x = x;
	touch->y = y;
	touch->pressure = glfwGetMouseButton(simulator.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// TIME

uint64_t CAT_get_slept_s()
{
	return simulator.slept_s;
}

uint64_t CAT_get_uptime_ms()
{
	return simulator.uptime_s * 1000;
}

float CAT_get_delta_time_s()
{
	return simulator.delta_time_s;
}

void CAT_get_datetime(CAT_datetime* datetime)
{
	time_t t = time(NULL);
	struct tm* lt = localtime(&t);
	
	datetime->year = lt->tm_year;
	datetime->month = lt->tm_mon;
	datetime->day = lt->tm_mday;
	datetime->hour = lt->tm_hour;
	datetime->minute = lt->tm_min;
	datetime->second = lt->tm_sec;
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
// SAVE

CAT_save the_save;

CAT_save* CAT_start_save()
{
	CAT_printf("Saving...\n");

	return &the_save;
}

void CAT_finish_save(CAT_save* save)
{
	CAT_printf("Save done!\n");

	save->magic_number = CAT_SAVE_MAGIC;
	int fd = open("save.dat", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	uint8_t* buffer = CAT_malloc(sizeof(the_save));
	memcpy(buffer, &the_save, sizeof(the_save));
	write(fd, buffer, sizeof(the_save));
	CAT_free(buffer);
	close(fd);
}

CAT_save* CAT_start_load()
{
	CAT_printf("Loading...\n");

	int fd = open("save.dat", O_RDONLY);
	read(fd, &the_save, sizeof(the_save));
	close(fd);

	CAT_printf
	(
		"Loaded save from version v%d.%d.%d.%d\n",
		the_save.version_major, the_save.version_minor,
		the_save.version_patch, the_save.version_push
	);
		
	return &the_save;
}

void CAT_finish_load()
{
	CAT_printf("Load done!\n");
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// POWER

int CAT_get_battery_pct()
{
	if(glfwWindowShouldClose(simulator.window))
	{
		return 0;
	}
	return 100;
}

bool CAT_is_charging()
{
	return true;
}

void CAT_sleep()
{
	glfwSetWindowShouldClose(simulator.window, true);
}

void CAT_shutdown()
{
	glfwSetWindowShouldClose(simulator.window, true);
}

void CAT_factory_reset()
{
	CAT_set_load_flag(CAT_LOAD_FLAG_DIRTY);
	CAT_set_load_flag(CAT_LOAD_FLAG_RESET);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// AIR QUALITY

CAT_AQ_readings readings;

void CAT_get_AQ_readings()
{
	readings.lps22hh.uptime_last_updated = 0;
	readings.lps22hh.temp = 20;
	readings.lps22hh.pressure = 1013;

	readings.sunrise.uptime_last_updated = 0;
	readings.sunrise.ppm_filtered_compensated = 400;
	readings.sunrise.temp = 20;

	readings.sen5x.uptime_last_updated = 0;
	readings.sen5x.pm2_5 = 9;
	readings.sen5x.pm10_0 = 15;
	readings.sen5x.humidity_rhpct = 40;

	readings.sen5x.temp_degC = 20;
	readings.sen5x.voc_index = 1;
	readings.sen5x.nox_index = 100;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// IMU

void CAT_IMU_tick()
{
	return;
}

bool CAT_IMU_is_upside_down()
{
	return false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// DEBUG

void CAT_printf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

void CAT_print_buffer(void* buf, int width, int height, size_t size)
{
	uint8_t* ptr = buf;
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			uint64_t value = 0;
			for(int b = 0; b < size; b++)
				value |= *(ptr++);
			CAT_printf("%d ", value);
		}
		CAT_printf("\n");
	}
}