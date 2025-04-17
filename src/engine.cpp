#include "engine.h"
#include <vector>
#include <glm/gtc/matrix_transform.hpp>  // for glm::ortho

Engine::Engine(bool** a_grid, int a_width, int a_height, const char* a_windowName)
{
    this->grid = a_grid;
    this->screenWidth = a_width;
    this->screenHeight = a_height;
    this->windowName = a_windowName;
}

int Engine::Initialize()
{
    // Initialize GLFW.
    glfwInit();

    // Tell GLFW that we want to use OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Tell GLFW that we want to use the OpenGL's core profile.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Do this for mac compatability.
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create Window.

    // Instantiate the window object.
    this->window = glfwCreateWindow(this->screenWidth, this->screenHeight, this->windowName, NULL, NULL);

    // Make sure that the window is created.
    if(window == NULL)
    {
        std::cout << "Failed to create GLFW window." << std::endl;
        glfwTerminate();

        std::cin.get();
        return 0;
    }

    glfwMakeContextCurrent(window);

    // Initialize GLAD.

    // Make sure that glad has been initialized successfully.
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD." << std::endl;
        
        std::cin.get();
        return 0;
    }

    // Set the viewport

    glViewport(0, 0, this->screenWidth, this->screenHeight);

    // Setup callbacks.

    // Binds the 'framebuffer_size_callback' method to the window resize event.
    glfwSetFramebufferSizeCallback(window, WindowResize);

    this->SetupOpenGlRendering();

    // Start game loop.
    while(!glfwWindowShouldClose(this->window))
    {
        // Calculate the elapsed time between the current and previous frame.
        float m_frameTime = (float)glfwGetTime();
        float m_deltaTime = m_frameTime - this->lastFrameTime;
        this->lastFrameTime = m_frameTime;

        glfwPollEvents();
        this->ProcessInput(this->window);

        glClearColor(this->clearColor.x, this->clearColor.y, this->clearColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Application logic
        this->Update(m_deltaTime);
        this->Draw();

        glfwSwapBuffers(this->window);
    }

    glfwTerminate();

    return 1;
}

void WindowResize(GLFWwindow* a_window, int a_width, int a_height)
{
    glViewport(0, 0, a_width, a_height);

    // TODO: Do your resize logic here...
}

void Engine::ProcessInput(GLFWwindow* a_window)
{
    // TODO: Process your input here...

    // If the escape key gets pressed, close the window.
    if(glfwGetKey(a_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(a_window, true);
}

void Engine::SetupOpenGlRendering()
{
    // Enable blending for transparency if needed later (optional)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable face culling and depth testing if doing 3D (disabled here)
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Set the default clear color (can be changed in draw loop)
    glClearColor(this->clearColor.x, this->clearColor.y, this->clearColor.z, 1.0f);

    // Enable scissor test if you plan on using it (disabled by default)
    glDisable(GL_SCISSOR_TEST);

    // Set viewport again to be sure (also done in Initialize)
    glViewport(0, 0, screenWidth, screenHeight);

    // No matrix mode (no fixed-function pipeline in core profile)
    // All transformation must be handled in shaders
}

void Engine::Update(float a_deltaTime)
{
    // TODO: Update your logic here...
}

void Engine::Draw()
{
    static bool initialized = false;
    static GLuint vao = 0, vbo = 0, shaderProgram = 0;
    static GLint projLoc = -1;

    // 1) On first call, compile a minimal shader + set up a VAO/VBO for 2D points
    if (!initialized) {
        // — Vertex Shader —
        const char* vertexSrc = R"(
            #version 330 core
            layout(location = 0) in vec2 aPos;
            uniform mat4 projection;
            void main() {
                gl_PointSize = 1.0;
                gl_Position = projection * vec4(aPos, 0.0, 1.0);
            }
        )";

        // — Fragment Shader —
        const char* fragmentSrc = R"(
            #version 330 core
            out vec4 FragColor;
            void main() {
                FragColor = vec4(1.0); // pure white
            }
        )";

        auto compileShader = [&](GLenum type, const char* src) {
            GLuint s = glCreateShader(type);
            glShaderSource(s, 1, &src, nullptr);
            glCompileShader(s);
            return s;
        };

        GLuint vs = compileShader(GL_VERTEX_SHADER,   vertexSrc);
        GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vs);
        glAttachShader(shaderProgram, fs);
        glLinkProgram(shaderProgram);
        glDeleteShader(vs);
        glDeleteShader(fs);

        projLoc = glGetUniformLocation(shaderProgram, "projection");

        // Setup VAO/VBO (we'll stream in (x,y) pairs each frame)
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
          glBindBuffer(GL_ARRAY_BUFFER, vbo);
          glEnableVertexAttribArray(0);
          glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glBindVertexArray(0);

        initialized = true;
    }

    // 2) Gather all true cells into a flat float array
    std::vector<float> pts;
    pts.reserve(this->screenHeight * this->screenWidth * 2);
    for (int y = 0; y < this->screenHeight; ++y) {
        for (int x = 0; x < this->screenWidth; ++x) {
            if (this->grid[y][x]) {
                pts.push_back((float)x);
                pts.push_back((float)y);
            }
        }
    }

    // 3) Upload to GPU
    glBindVertexArray(vao);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER,
                   pts.size() * sizeof(float),
                   pts.data(),
                   GL_DYNAMIC_DRAW);

      // 4) Set projection matrix so that (0,0)-(this->screenWidth,this->screenHeight) maps to NDC
      glm::mat4 proj = glm::ortho(
        0.0f, (float)this->screenWidth,
        0.0f, (float)this->screenHeight
      );
      glUseProgram(shaderProgram);
      glUniformMatrix4fv(projLoc, 1, GL_FALSE, &proj[0][0]);

      // 5) Draw all points as white pixels
      glDrawArrays(GL_POINTS, 0, (GLsizei)(pts.size() / 2));

    // 6) Clean up state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}