import OpenGL;
OpenGL.FULL_LOGGING = True;
from OpenGL.GL import *;

import glfw;
import imgui;
from imgui.integrations.glfw import GlfwRenderer;

import imageio.v3 as iio;
import numpy as np;

from cowtools import *;

class CAT_shader:
    def __init__(self, vert_src, frag_src):
        vert_id = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vert_id, vert_src);
        glCompileShader(vert_id);
        if glGetShaderiv(vert_id, GL_COMPILE_STATUS) != GL_TRUE:
            print("While compiling vertex shader:");
            print(glGetShaderInfoLog(vert_id).decode("utf-8"));

        frag_id = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag_id, frag_src);
        glCompileShader(frag_id);
        if glGetShaderiv(frag_id, GL_COMPILE_STATUS) != GL_TRUE:
            print("While compiling fragment shader:");
            print(glGetShaderInfoLog(frag_id).decode("utf-8"));

        prog_id = glCreateProgram();
        glAttachShader(prog_id, vert_id);
        glAttachShader(prog_id, frag_id);
        glLinkProgram(prog_id);
        if glGetProgramiv(prog_id, GL_LINK_STATUS) != GL_TRUE:
            print("While linking shader program:");
            print(glGetProgramInfoLog(prog_id).decode("utf-8"));

        self.vert_id = vert_id;
        self.frag_id = frag_id;
        self.prog_id = prog_id;

class CAT_texture:
    def __init__(self, width, height):
        self.width = width;
        self.height = height;
        self.data = np.zeros((height * height * 4), dtype=np.float32);

    def read(self, x, y):
        idx = y * self.width * 4 + x * 4;
        return self.data[idx:idx+4];

    def write(self, x, y, c):
        idx = y * self.width * 4 + x * 4;
        base = self.data[idx:idx+4];
        blend = base * (1-c[3]) + c * c[3];
        self.data[idx:idx+4] = blend;

    def from_image(path):
        img = iio.imread(path);
        tex = CAT_texture(img.shape[1], img.shape[0]);
        buf = img.flatten();
        buf = (buf - np.min(buf))/(np.max(buf)-np.min(buf));
        tex.data = buf;

        return tex;

class CAT_display:
    def __init__(self):
        if not glfw.init():
            print("Failed to initialize GLFW. Exiting");
            exit();

        glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 3);
        glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 3);
        glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE);
        glfw.window_hint(glfw.OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfw.window_hint(glfw.RESIZABLE, GL_FALSE)
        handle = glfw.create_window(240, 320, "CAT", None, None);
        if not handle:
            print("Failed to create window. Exiting");
            glfw.terminate();
            exit();

        glfw.make_context_current(handle);
        print("Renderer:", glGetString(GL_RENDERER));
        print("GL Version:", glGetString(GL_VERSION));
        print("SL Version:", glGetString(GL_SHADING_LANGUAGE_VERSION));
        
        with open("shaders/cat.vert", "r") as file:
            vert_src = file.read();
        with open("shaders/cat.frag", "r") as file:
            frag_src = file.read();
        shader = CAT_shader(vert_src, frag_src);
        uniforms = {
            "tex" : glGetUniformLocation(shader.prog_id, "tex")
        };

        coords = np.array([
            -1.0, -1.0,
            1.0, -1.0,
            1.0, 1.0,
            1.0, 1.0,
            -1.0, 1.0,
            -1.0, -1.0
        ], dtype=np.float32);
        vao_id = glGenVertexArrays(1);
        glBindVertexArray(vao_id);
        glEnableVertexAttribArray(0);
        vbo_id = glGenBuffers(1);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferData(GL_ARRAY_BUFFER, coords, GL_STATIC_DRAW); 
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, ctypes.c_void_p(0));

        framebuffer = CAT_texture(240, 320);
        tex_id = glGenTextures(1);
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 240, 320, 0, GL_RGBA, GL_FLOAT, framebuffer.data);
        glGenerateMipmap(GL_TEXTURE_2D);

        imgui.create_context();
        imgui_io = imgui.get_io();
        imgui_io.display_size = (240, 320);
        imgui_io.fonts.get_tex_data_as_rgba32();
        imgui_renderer = GlfwRenderer(handle);

        self.handle = handle;
        self.shader = shader;
        self.uniforms = uniforms;
        self.vao_id = vao_id;
        self.vbo_id = vbo_id;
        self.framebuffer = framebuffer;
        self.tex_id = tex_id;
        
        self.time = 0;
        self.delta_time = 0;
        
        self.imgui = imgui;
        self.imgui_renderer = imgui_renderer;

        self.capture_requests = [];

    def refresh(self):
        glBindTexture(GL_TEXTURE_2D, self.tex_id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 240, 320, GL_RGBA, GL_FLOAT, self.framebuffer.data);

    def capture(path):
        self.capture_requests.append(path);

    def loop(self, input_callback, logic_callback, render_callback):
        self.input_callback = input_callback;
        self.logic_callback = logic_callback;
        self.render_callback = render_callback;
        
        glClearColor(0, 0, 0, 1);

        while not glfw.window_should_close(self.handle):
            time = glfw.get_time();
            self.delta_time = time - self.time;
            self.time = time;

            glfw.poll_events();
            self.imgui_renderer.process_inputs();

            self.input_callback(self);
            self.logic_callback(self);

            glClear(GL_COLOR_BUFFER_BIT);
            self.imgui.new_frame();

            self.render_callback(self);
            self.refresh();
    
            glUseProgram(self.shader.prog_id);
            glBindVertexArray(self.vao_id);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, self.tex_id);
            glProgramUniform1i(self.shader.prog_id, self.uniforms["tex"], 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            
            for request in self.capture_requests:
                buffer = glReadPixels(0, 0, 240*2, 320*2, GL_RGB, GL_UNSIGNED_BYTE);
                pixels = np.frombuffer(buffer, dtype=np.uint8);
                pixels = np.reshape(pixels, (240*2, 320*2, 3));
                pixels = np.flipud(pixels);
                iio.imwrite(request, pixels);
            self.capture_requests = [];

            self.imgui.render();
            self.imgui_renderer.render(imgui.get_draw_data());
            self.imgui.end_frame();

            glfw.swap_buffers(self.handle);

class CAT_atlas:
    def __init__(self, texture):
        self.texture = texture;
        self.sprites = {};

    def register(self, name, x, y, w, h):
        self.sprites[name] = (x, y, w, h);

class CAT_spriter:
    def __init__(self, atlas, screen):
        self.atlas = atlas;
        self.screen = screen;

    def draw(self, name, x_w, y_w):
        sprite = self.atlas.sprites[name];
        x_r = sprite[0];
        y_r = sprite[1];
        w = sprite[2];
        h = sprite[3];

        for dy in range(h):
            for dx in range(w):
                c = self.atlas.texture.read(x_r+dx, y_r+dy);
                self.screen.write(x_w+dx, y_w+dy, c);

display = CAT_display();

atlas_tex = CAT_texture.from_image("sprites/atlas.png");
atlas = CAT_atlas(atlas_tex);
atlas.register("floor", 0, 0, 16, 16);
atlas.register("wall", 16, 0, 16, 16);
atlas.register("vending machine", 32, 0, 16, 32);
atlas.register("tile_yes", 48, 0, 16, 16);
atlas.register("tile_no", 64, 0, 16, 16);

spriter = CAT_spriter(atlas, display.framebuffer);

def input_callback(display):
    return;

def logic_callback(display):
    return;

def render_callback(display):
    #top_border = 8;
    top_border = 0;
    #tile_size = 24;
    tile_size = 16;
    
    width_t = 240 // tile_size;
    height_t = (320-top_border) // tile_size;

    wall_rows = 4;
    
    for y_t in range(wall_rows):
        for x_t in range(width_t):
            x_p = x_t * tile_size;
            y_p = y_t * tile_size;
            spriter.draw("wall", x_p, y_p);
            spriter.draw("tile_yes", x_p, y_p);
    for y_t in range(wall_rows, height_t):
        for x_t in range(width_t):
            x_p = x_t * tile_size;
            y_p = y_t * tile_size;
            spriter.draw("floor", x_p, y_p);
    return;

display.loop(input_callback, logic_callback, render_callback);

