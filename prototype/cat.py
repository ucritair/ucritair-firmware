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

class CAT_display:
    def __init__(self):
        if not glfw.init():
            print("Failed to initialize GLFW. Exiting");
            exit();

        glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 3);
        glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 3);
        glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE);
        glfw.window_hint(glfw.OPENGL_FORWARD_COMPAT, GL_TRUE);
        handle = glfw.create_window(240, 320, "Ledger", None, None);
        if not handle:
            print("Failed to create window. Exiting");
            glfw.terminate();
            exit();

        glfw.make_context_current(handle);
        print("Renderer:", glGetString(GL_RENDERER));
        print("GL Version:", glGetString(GL_VERSION));
        print("SL Version:", glGetString(GL_SHADING_LANGUAGE_VERSION));
        
        with open("standard.vert", "r") as file:
            vert_src = file.read();
        with open("standard.frag", "r") as file:
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

        pixels = np.array([0 for i in range(240*320*3)], dtype=np.float32);
        tex_id = glGenTextures(1);
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 240, 320, 0, GL_RGB, GL_FLOAT, pixels);
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
        self.pixels = pixels;
        self.tex_id = tex_id;
        
        self.time = 0;
        self.delta_time = 0;
        
        self.imgui = imgui;
        self.imgui_renderer = imgui_renderer;

        self.capture_requests = [];

    def write(self, x, y, c):
        idx = y * 240 * 3 + x * 3;
        self.pixels[idx + 0] = c[0];
        self.pixels[idx + 1] = c[1];
        self.pixels[idx + 2] = c[2];

    def refresh(self):
        glBindTexture(GL_TEXTURE_2D, self.tex_id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 240, 320, GL_RGB, GL_FLOAT, self.pixels);

    def capture(path):
        self.capture_requests.append(path);

    def loop(self, input_callback, logic_callback, render_callback):
        self.input_callback = input_callback;
        self.logic_callback = logic_callback;
        self.render_callback = render_callback;
        
        while not glfw.window_should_close(self.handle):
            time = glfw.get_time();
            self.delta_time = time - self.time;
            self.time = time;

            glfw.poll_events();
            self.imgui_renderer.process_inputs();

            self.input_callback(self);
            self.logic_callback(self);

            glClearColor(0, 0, 0, 1);
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

display = CAT_display();
display.loop();

