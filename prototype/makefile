CFLAGS = -g -Wall

INCS = `pkg-config --cflags glew`
INCS += `pkg-config --cflags glfw3`
INCS += `pkg-config --cflags libpng`

LIBS = `pkg-config --static --libs glew`
LIBS += `pkg-config --static --libs glfw3`
LIBS += `pkg-config --static --libs libpng`
LIBS += -framework OpenGL

MAIN_SRCS = cat_sim.c
MODULE_SRCS = $(filter-out $(MAIN_SRCS), $(wildcard *.c));

OBJS = $(addsuffix .o, $(basename $(notdir $(MODULE_SRCS))))

%.o: %.c
	cc -c $(CFLAGS) $(INCS) $(LIBS) $< -o $@

cat_sim: $(MAIN_SRCS) $(OBJS)
	cc $(CFLAGS) $(INCS) $(LIBS) $(MAIN_SRCS) $(OBJS) -o cat_sim

clean:
	trash *.o
	trash cat_sim
