CFLAGS = -g -Wall

INCS = `pkg-config --cflags glew`
INCS += `pkg-config --cflags glfw3`
INCS += `pkg-config --cflags libpng`
INCS += `pkg-config --cflags openal`

LIBS = `pkg-config --static --libs glew`
LIBS += `pkg-config --static --libs glfw3`
LIBS += `pkg-config --static --libs libpng`
LIBS += `pkg-config --static --libs openal`
LIBS += -framework OpenGL

MAIN_SRCS = src/cat_main.c
MODULE_SRCS = $(filter-out $(MAIN_SRCS), $(wildcard src/*.c));

OBJS = $(addprefix bin/, $(addsuffix .o, $(basename $(notdir $(MODULE_SRCS)))))

bin/%.o: src/%.c
	cc -c $(CFLAGS) $(INCS) $(LIBS) $< -o $@

bin/cat_main: $(MAIN_SRCS) $(OBJS)
	cc $(CFLAGS) $(INCS) $(LIBS) $(MAIN_SRCS) $(OBJS) -o $@

clean:
	trash bin/*
