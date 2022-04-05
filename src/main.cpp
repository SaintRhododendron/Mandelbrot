#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>

#define MB_ITERS 50
#define MB_COORD_PIXELS 500
#define PROGRAM_NAME "Mandelbrot"
#define WINDOW_HEIGHT 1080
#define WINDOW_WIDTH 1920
//#define WINDOWED

glm::ivec2 g_windowSize(//pVideoMode->height, pVideoMode->width);
						WINDOW_WIDTH, WINDOW_HEIGHT);

void glfwKeyCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(pWindow, GL_TRUE);
    }

}

void setShaderProgram(const std::string& executablePath)
{
    size_t position = executablePath.find_last_of("/\\");
    std::string path = executablePath.substr(0, position);
    std::ifstream file;
    file.open(path + "/shaders/vertex.txt" , std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "[ERR] Failed to open shader code: " << path << "/shaders/" << "vertex.txt" << std::endl;
    }
    std::stringstream vertexBuffer;
    vertexBuffer << file.rdbuf();
    std::string vertexShaderCode = vertexBuffer.str();
    file.close();
    std::stringstream fragmentBuffer;
    file.open(path + "/shaders/fragment.txt", std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "[ERR] Failed to open shader code: " << path << "/shaders/" << "fragment.txt" << std::endl;
    }
    fragmentBuffer << file.rdbuf();
    std::string fragmentShaderCode = fragmentBuffer.str();
    file.close();

    GLuint vertexShaderID = 0;
    vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexCode = vertexShaderCode.c_str();
    glShaderSource(vertexShaderID, 1, &vertexCode, nullptr);
    glCompileShader(vertexShaderID);

    GLint success;
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[1024];
        glGetShaderInfoLog(vertexShaderID, 1024, nullptr, infoLog);
        std::cerr << "[ERR] vertex Shader-compiling error. " << infoLog << std::endl;
    }

    GLuint fragmentShaderID = 0;
    fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentCode = fragmentShaderCode.c_str();
    glShaderSource(fragmentShaderID, 1, &fragmentCode, nullptr);
    glCompileShader(fragmentShaderID);

    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[1024];
        glGetShaderInfoLog(fragmentShaderID, 1024, nullptr, infoLog);
        std::cerr << "[ERR] fragment Shader-compiling error. " << infoLog << std::endl;
    }
    GLuint shaderProgramID = glCreateProgram();
    glAttachShader(shaderProgramID, vertexShaderID);
    glAttachShader(shaderProgramID, fragmentShaderID);
    glLinkProgram(shaderProgramID);

    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &success);

    if (!success)
    {
        GLchar infoLog[1024];
        glGetShaderInfoLog(shaderProgramID, 1024, nullptr, infoLog);
        std::cerr << "[ERR] ShaderProgram-linking error. " << infoLog << std::endl;
    }

    glUseProgram(shaderProgramID);
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
}

void genArrays(const GLfloat* vertexCoords, const GLfloat* textureCoords)
{
    GLuint VAO = 0;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLuint vertexCoordsVBO = 0;
    glGenBuffers(1, &vertexCoordsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexCoordsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoords), &vertexCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    GLuint colorCoordsVBO = 0;
    glGenBuffers(1, &colorCoordsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorCoordsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), &textureCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
}

int main(int argc, char** argv)
{
   
    /* Initialize the library */
    if (!glfwInit())
    {
        std::cout << "[ERR] Fatal Error! GLFW failed to be initialized!" << std::endl;
        return -1;
    }

    GLFWmonitor* pMonitor = glfwGetPrimaryMonitor();
    if (!pMonitor)
    {
        std::cerr << "[ERR] Fatal Error! Failed to detect Primal Monitor!" << std::endl;
        return -1;
    }

    const GLFWvidmode* pVideoMode = glfwGetVideoMode(pMonitor);
    // g_windowSize.x = pVideoMode->width;
    // g_windowSize.y = pVideoMode->height;


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);



#ifdef WINDOWED
    GLFWwindow* pWindow = glfwCreateWindow(g_windowSize.x, g_windowSize.y, PROGRAM_NAME, nullptr, nullptr);
#else
    GLFWwindow* pWindow = glfwCreateWindow(g_windowSize.x, g_windowSize.y, PROGRAM_NAME, pMonitor, nullptr);
#endif
    if (!pWindow)
    {
        std::cout << "[ERR] Fatal Error! Window " << PROGRAM_NAME << " failed to be created!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(pWindow, glfwKeyCallback);
    /* Make the window's context current */
    glfwMakeContextCurrent(pWindow);

    if (!gladLoadGL())
    {
        std::cout << "[ERR] Fatal Error! GLAD failed to be loaded!" << std::endl;
        return -1;
    }

    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;

    glClearColor(1, 1, 1, 1);

    setShaderProgram(argv[0]);

    while (!glfwWindowShouldClose(pWindow))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);


        glDrawArrays(GL_POINTS, 0, 21);
        /* Swap front and back buffers */
        glfwSwapBuffers(pWindow);


        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();

	return 0;
}