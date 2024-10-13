CFLAGS = -g -Wall -std=c++11

INCS = `pkg-config --cflags glew`
INCS += `pkg-config --cflags glfw3`
INCS += `pkg-config --cflags libpng`
INCS += -Iimgui

LIBS = `pkg-config --static --libs glew`
LIBS += `pkg-config --static --libs glfw3`
LIBS += `pkg-config --static --libs libpng`
LIBS += -framework OpenGL

OBJS = $(wildcard imgui/bin/*.o)

SRCS = cat_sim.cpp

cat_sim: $(SRCS)
	c++ $(CFLAGS) $(INCS) $(LIBS) $(OBJS) $(SRCS) -o cat_sim
