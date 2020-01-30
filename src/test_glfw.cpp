#include <stdio.h>
#include <math.h>
//#define GLAD_DEBUG
#include <glad/glad.h>
#include <GLFW/glfw3.h>

constexpr float PI = 3.14159265359f;

template<class T> T min(T a, T b) { return a < b ? a : b; }
template<class T> T max(T a, T b) { return a > b ? a : b; }

void glErrorCallback(const char* name, void* funcptr, int len_args, ...) {
    GLenum error_code;
    error_code = glad_glGetError();

    if (error_code != GL_NO_ERROR) {
        fprintf(stderr, "ERROR %d in %s\n", error_code, name);
    }
}

const char* verShadSrc =
R"GLSL(
#version 330
layout(location = 0) in vec2 a_pos;
uniform vec2 u_scale;
uniform vec2 u_translation;
void main()
{
    gl_Position = vec4(vec3(u_scale * a_pos + u_translation, 0.0), 1.0);
}
)GLSL";

const char* fragShadSrc = R"GLSL(
#version 330
layout(location = 0) out vec4 o_color;
void main()
{
    o_color = vec4(1.0, 0.0, 0.0, 1.0);
}
)GLSL";

const float quad[] = {
    -1, -1,  +1, -1,  +1, +1,
    -1, -1,  +1, +1,  -1, +1
};

GLFWwindow* window;
unsigned prog;
unsigned vbo;
unsigned vao;

struct vec2{ float x, y; };
vec2 p = { 0, 0 };
float speed = 1.5f;
void updatePos(float dt)
{
    static float d = 0;
    d += speed * dt;
    int mode = (int)(d / (2*PI)) % 4;
    const float phase = (float)sin(d);
    switch (mode)
    {
    case 0:
        p.x = phase;
        p.y = 0;
        break;
    case 1:
        p.x = 0;
        p.y = phase;
        break;
    case 2:
        p.x = phase;
        p.y = phase;
        break;
    case 3:
        p.x = phase;
        p.y = -phase;
        break;
    }
}

static float getAspectRatio()
{
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    return (float)w / h;
}

static void cycleWindowMode()
{
    static int savedWindowDims[4];
    static int windowMode = 0;
    windowMode = (windowMode + 1) % 3;
    GLFWmonitor* monitor = glfwGetWindowMonitor(window);
    if (!monitor)
        monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* vidMode = glfwGetVideoMode(monitor);
    switch (windowMode)
    {
    case 0:
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
        glfwSetWindowMonitor(window, nullptr, savedWindowDims[0], savedWindowDims[1], savedWindowDims[2], savedWindowDims[3], -1);
        break;
    case 1:
        glfwGetWindowPos(window, &savedWindowDims[0], &savedWindowDims[1]);
        glfwGetWindowSize(window, &savedWindowDims[2], &savedWindowDims[3]);
        glfwSetWindowMonitor(window, monitor, 0, 0, vidMode->width, vidMode->height, -1);
    case 2:
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        break;
    }
}

int main()
{
    glfwSetErrorCallback([](int error, const char* description) -> void {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    });

    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    window = glfwCreateWindow(720, 720, "test", NULL, NULL);
    if (window == NULL)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // Enable vsync

    if (gladLoadGL() == 0) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }
    glad_set_post_callback(glErrorCallback);

    {
        auto vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &verShadSrc, nullptr);
        glCompileShader(vs);
        auto fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fragShadSrc, nullptr);
        glCompileShader(fs);
        prog = glCreateProgram();
        glAttachShader(prog, vs);
        glAttachShader(prog, fs);
        glLinkProgram(prog);
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glUseProgram(prog);

    glClearColor(0, 0.1f, 0.05f, 1);
    float t0 = glfwGetTime();
    float t1 = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        const float dt = t1 - t0;
        glfwPollEvents();
        {
            static int prevState = GLFW_RELEASE;
            int state = glfwGetKey(window, GLFW_KEY_F);
            if (state == GLFW_PRESS && prevState == GLFW_RELEASE)
                cycleWindowMode();
            prevState = state;
        }
        {
            static int prevState = GLFW_RELEASE;
            int state = glfwGetKey(window, GLFW_KEY_LEFT);
            if (state == GLFW_PRESS && prevState == GLFW_RELEASE)
                speed = max(0.f, speed-0.1f);
            prevState = state;
        }
        {
            static int prevState = GLFW_RELEASE;
            int state = glfwGetKey(window, GLFW_KEY_RIGHT);
            if (state == GLFW_PRESS && prevState == GLFW_RELEASE)
                speed = min(20.f, speed + 0.1f);
            prevState = state;
        }

        updatePos(dt);

        int w, h;
        glfwGetWindowSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(prog);
        glUniform2f(1, p.x, p.y);
        const float ar = getAspectRatio();
        if (ar < 0)
            glUniform2f(0, 0.05f, 0.05f*ar);
        else
            glUniform2f(0, 0.05f / ar, 0.05f);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        t0 = t1;
        t1 = glfwGetTime();
    }
}