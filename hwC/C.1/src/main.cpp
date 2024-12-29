#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

using std::cerr;
using std::endl;

// Function prototypes
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Settings
const int WIDTH = 800;
const int HEIGHT = 600;

// Shaders
const GLchar* vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 position;\n"
    "uniform float angle;\n"
    "void main()\n"
    "{\n"
    "   float cosAngle = cos(angle);\n"
    "   float sinAngle = sin(angle);\n"
    "   mat4 rotation = mat4(\n"
    "       cosAngle, sinAngle, 0.0, 0.0,\n"
    "      -sinAngle, cosAngle, 0.0, 0.0,\n"
    "       0.0, 0.0, 1.0, 0.0,\n"
    "       0.0, 0.0, 0.0, 1.0);\n"
    "   gl_Position = rotation * vec4(position, 1.0);\n"
    "}\0";

const GLchar* fragmentShaderSource =
    "#version 330 core\n"
    "out vec4 color;\n"
    "uniform vec4 triangleColor;\n"
    "void main()\n"
    "{\n"
    "   color = triangleColor;\n"
    "}\n\0";

// Global variables for rotation angle and color
float rotationAngle = 0.0f;
GLfloat triangleColor[4] = {1.0f, 0.5f, 0.2f, 1.0f}; // Initial color

GLFWwindow* initialize() {
  // Init GLFW
  if (!glfwInit()) {
    cerr << "Unable to initialize GLFW" << endl;
    return nullptr;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Rotating Triangle", nullptr, nullptr);
  if (!window) {
    cerr << "Unable to create GLFW window" << endl;
    glfwTerminate();
    return nullptr;
  }

  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, keyCallback);

  if (!gladLoadGL()) {
    cerr << "Unable to initialize GLAD" << endl;
    glfwDestroyWindow(window);
    glfwTerminate();
    return nullptr;
  }

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  return window;
}

int main(int argc, char* argv[]) {
  GLFWwindow* window = initialize();
  if (!window) {
    return -1;
  }

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  GLint success;
  GLchar infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
  }

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
  }

  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  GLfloat vertices[] = {
      -0.5f, -0.5f, 0.0f,
       0.5f, -0.5f, 0.0f,
       0.0f,  0.5f, 0.0f
  };

  GLuint VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glUseProgram(shaderProgram);
  GLint angleLocation = glGetUniformLocation(shaderProgram, "angle");
  GLint colorLocation = glGetUniformLocation(shaderProgram, "triangleColor");

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    rotationAngle -= 0.002f;  // Increment angle for clockwise rotation

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glUniform1f(angleLocation, rotationAngle);
    glUniform4fv(colorLocation, 1, triangleColor);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3); // »æÖÆÈý½ÇÐÎ
    glBindVertexArray(0);

    glfwSwapBuffers(window);
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteProgram(shaderProgram);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_1:
        triangleColor[0] = 1.0f; triangleColor[1] = 0.0f; triangleColor[2] = 0.0f; // Red
        break;
      case GLFW_KEY_2:
        triangleColor[0] = 0.0f; triangleColor[1] = 1.0f; triangleColor[2] = 0.0f; // Green
        break;
      case GLFW_KEY_3:
        triangleColor[0] = 0.0f; triangleColor[1] = 0.0f; triangleColor[2] = 1.0f; // Blue
        break;
    }
  }
}
