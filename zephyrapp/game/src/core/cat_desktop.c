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
#include "cat_math.h"
#include "cat_aqi.h"
#include <sys/mman.h>
#include "cat_pet.h"
#include "cat_crisis.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// PLATFORM

#define WINDOW_SCALE 2
#ifdef CAT_MACOS
#define RETINA_SCALE 2
#else
#define RETINA_SCALE 1
#endif

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
} simulator = {0};

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
	char cwd_buf[128];
	getcwd(cwd_buf, sizeof(cwd_buf));
	CAT_printf("[%s]\n", cwd_buf);
	
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
	simulator.window = glfwCreateWindow(CAT_LCD_SCREEN_W*WINDOW_SCALE, CAT_LCD_SCREEN_H*WINDOW_SCALE, "μCritAir", NULL, NULL);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, CAT_LCD_SCREEN_W, CAT_LCD_SCREEN_H, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
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

#define SCREEN_CAPTURE_ROWS (CAT_LCD_SCREEN_H*RETINA_SCALE*WINDOW_SCALE)
#define SCREEN_CAPTURE_COLS (CAT_LCD_SCREEN_W*RETINA_SCALE*WINDOW_SCALE)
#define SCREEN_CAPTURE_ROW_SIZE (SCREEN_CAPTURE_COLS*3)
#define SCREEN_CAPTURE_SIZE (SCREEN_CAPTURE_ROWS * SCREEN_CAPTURE_ROW_SIZE)
uint8_t screen_capture[2][SCREEN_CAPTURE_SIZE];
int screen_capture_buffer_idx = 0;

void flip_screen_capture()
{
	int a_idx = screen_capture_buffer_idx;
	int b_idx = (screen_capture_buffer_idx+1)%2;

	for(int y = 0; y < SCREEN_CAPTURE_ROWS; y++)
	{
		int y_inv = SCREEN_CAPTURE_ROWS-y-1;
		memcpy
		(
			&screen_capture[b_idx][y*SCREEN_CAPTURE_ROW_SIZE],
			&screen_capture[a_idx][y_inv*SCREEN_CAPTURE_ROW_SIZE],
			SCREEN_CAPTURE_ROW_SIZE
		);
	}

	screen_capture_buffer_idx = b_idx;
}

void write_screen_capture(const char* path)
{
	FILE* file = fopen(path, "w");
	fprintf(file, "%s\n%u %u\n%u\n", "P6", SCREEN_CAPTURE_COLS, SCREEN_CAPTURE_ROWS, 255);
	fwrite(screen_capture[screen_capture_buffer_idx], sizeof(uint8_t), SCREEN_CAPTURE_SIZE, file);
	fclose(file);
}

void CAT_platform_tick()
{
	glfwPollEvents();

	simulator.delta_time_s = glfwGetTime() - simulator.uptime_s;
	simulator.uptime_s = glfwGetTime();

	if(glfwGetKey(simulator.window, GLFW_KEY_ENTER))
	{
		glReadPixels
		(
			0, 0,
			SCREEN_CAPTURE_COLS, SCREEN_CAPTURE_ROWS,
			GL_RGB, GL_UNSIGNED_BYTE,
			screen_capture[screen_capture_buffer_idx]
		);
		flip_screen_capture();
		
		CAT_datetime t;
		CAT_get_datetime(&t);
		char path[strlen("capture/####_##_##_##_##_##.ppm")+1];
		snprintf
		(
			path, sizeof(path),
			"capture/%.4d_%.2d_%.2d_%.2d_%.2d_%.2d.ppm",
			t.year, t.month, t.day, t.hour, t.minute, t.second
		);
		write_screen_capture(path);
	}
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
	sleep(1);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// LEDs

uint8_t LEDs[6][3];

void CAT_set_LEDs(uint8_t r, uint8_t g, uint8_t b)
{
	float power = CAT_LED_get_brightness() / 100.0f;

	for(int i = 0; i < 6; i++)
	{
		LEDs[i][0] = (uint8_t) (power * r);
		LEDs[i][1] = (uint8_t) (power * g);
		LEDs[i][2] = (uint8_t) (power * b);
	}
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
	x /= WINDOW_SCALE;
	y /= WINDOW_SCALE;
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

uint64_t CAT_get_RTC_offset()
{
	return 0;
}

uint64_t CAT_get_RTC_now()
{
	time_t t = time(NULL);
	struct tm tm;
	localtime_r(&t, &tm);
	tm.tm_year += 1900;
	return timegm(&tm);
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

uint8_t save_backing[0x0e000];

CAT_save* CAT_start_save()
{
	return (CAT_save*) save_backing;
}

void CAT_finish_save(CAT_save* save)
{
	save->magic_number = CAT_SAVE_MAGIC;
	int fd = open("save.dat", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	write(fd, &save_backing, sizeof(save_backing));
	close(fd);
}

CAT_save* CAT_start_load()
{
	int fd = open("save.dat", O_RDONLY);
	read(fd, &save_backing, sizeof(save_backing));
	close(fd);
	return (CAT_save*) &save_backing;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// LOGS

CAT_log_cell log_cell =
{
	.flags = CAT_LOG_CELL_FLAG_HAS_CO2 | CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES,

	.timestamp = 61686489600,
	.temp_Cx1000 = 22700,
	.pressure_hPax10 = 9760,

	.rh_pctx100 = 4800,
	.co2_ppmx1 = 644,
	.pm_ugmx100 = {100, 110, 110, 110},
	.pn_ugmx100 = {650, 720, 720, 720, 720},

	.voc_index = 105,
	.nox_index = 1,

	.co2_uncomp_ppmx1 = 644
};

void CAT_read_log_cell_at_idx(int idx, CAT_log_cell* out)
{
	memcpy(out, &log_cell, sizeof(CAT_log_cell));
}

int CAT_read_log_cell_before_time(int base_idx, uint64_t time, CAT_log_cell* out)
{
	CAT_read_log_cell_at_idx(1, out);
	return 1;
}

int CAT_read_log_cell_after_time(int base_idx, uint64_t time, CAT_log_cell* out)
{
	CAT_read_log_cell_at_idx(1, out);
	return 1;
}

int CAT_read_first_calendar_cell(CAT_log_cell* cell)
{
	CAT_read_log_cell_at_idx(1, cell);
	return 1;
}

int CAT_get_log_cell_count()
{
	return 1;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// POWER

int CAT_get_battery_pct()
{
	if(glfwWindowShouldClose(simulator.window))
		return 0;
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
	CAT_set_load_flags(CAT_LOAD_FLAG_DIRTY);
	CAT_set_load_flags(CAT_LOAD_FLAG_DEFAULT);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// AIR QUALITY

static CAT_AQ_score_block aq_moving_scores =
{
	.CO2 = 450,
	.VOC = 1,
	.NOX = 100,
	.PM2_5 = 9 * 100,
	.temp = 23 * 1000,
	.rh = 40 * 100,
	.aggregate = 75,
	.sample_count = 0
};

static CAT_AQ_score_block aq_score_buffer[7];
static int aq_score_head = 0;

CAT_AQ_score_block* CAT_AQ_get_moving_scores()
{
	return &aq_moving_scores;
}

void CAT_AQ_score_buffer_reset()
{
	for(int i = 0; i < 7; i++)
	{
		memcpy(&aq_score_buffer[i], &aq_moving_scores, sizeof(CAT_AQ_score_block));
	}
	aq_score_head = 0;
}

void CAT_AQ_score_buffer_push(CAT_AQ_score_block* in)
{
	CAT_AQ_score_block* block = &aq_score_buffer[aq_score_head];
	memcpy(block, in, sizeof(CAT_AQ_score_block));
	aq_score_head = (aq_score_head+1) % 7;
}

CAT_AQ_score_block* CAT_AQ_score_buffer_get(int idx)
{
	idx = (aq_score_head+idx) % 7;
	return &aq_score_buffer[idx];
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// IMU

CAT_IMU_values imu_values =
{
	.x = 0,
	.y = -1,
	.z = 0
};

void CAT_IMU_export_raw(CAT_IMU_values* out)
{
	memcpy(out, &imu_values, sizeof(imu_values));
}

void CAT_IMU_export_normalized(CAT_IMU_values* out)
{
	memcpy(out, &imu_values, sizeof(imu_values));
}

void CAT_IMU_tick()
{
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// DEBUG

void CAT_printf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vdprintf(STDOUT_FILENO, fmt, args);
	va_end(args);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// PERSISTENCE

static int AQ_crisis_state_fd = -1;
static uint8_t* AQ_crisis_state_mmap = NULL;
static int pet_timing_state_fd = -1;
static uint8_t* pet_timing_state_mmap = NULL;
static int persist_flags_fd = -1;
static uint64_t* persist_flags_mmap = NULL;

uint8_t* generate_persist(const char* path, size_t size, int* fd)
{
	*fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	size_t file_size = lseek(*fd, 0, SEEK_END);
	if(file_size < size)
	{
		lseek(*fd, 0, SEEK_SET);
		uint8_t* zeroes = malloc(size);
		write(*fd, zeroes, size);
	}
	uint8_t* mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, 0);
	close(*fd);
	return mem;
}

uint8_t* CAT_AQ_crisis_state_persist()
{
	if(AQ_crisis_state_fd == -1)
		AQ_crisis_state_mmap = generate_persist("persist/AQ_crisis_state.dat", sizeof(CAT_AQ_crisis_state), &AQ_crisis_state_fd);
	return AQ_crisis_state_mmap;
}

uint8_t* CAT_pet_timing_state_persist()
{
	if(pet_timing_state_fd == -1)
		pet_timing_state_mmap = generate_persist("persist/pet_timing_state.dat", sizeof(CAT_pet_timing_state), &pet_timing_state_fd);
	return pet_timing_state_mmap;
}

bool CAT_was_persist_wiped()
{
	return false;
}

void load_persist_flags()
{
	if(persist_flags_fd == -1)
		persist_flags_mmap = (uint64_t*) generate_persist("persist/persist_flags.dat", sizeof(uint64_t), &persist_flags_fd);
}

uint64_t CAT_get_persist_flags()
{
	load_persist_flags();
	return *persist_flags_mmap;
}

void CAT_set_persist_flags(uint64_t flags)
{
	load_persist_flags();
	*persist_flags_mmap = flags;
}

bool CAT_get_persist_flag(uint64_t flags)
{
	load_persist_flags();
	return *persist_flags_mmap & flags;
}

void CAT_raise_persist_flag(uint64_t flags)
{
	load_persist_flags();
	*persist_flags_mmap |= flags;
}

void CAT_lower_persist_flag(uint64_t flags)
{
	load_persist_flags();
	*persist_flags_mmap &= ~flags;
}

