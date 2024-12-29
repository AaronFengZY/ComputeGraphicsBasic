#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

using std::cerr;
using std::endl;

// Settings
const int WIDTH = 800;
const int HEIGHT = 600;

// 顶点着色器
const GLchar* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 position;
    layout (location = 1) in vec3 color;
    out vec3 vertexColor; // 接收到的经过插值后的值
    uniform float angle;
    uniform vec2 translation;
    void main()
    {
       float cosAngle = cos(angle);
       float sinAngle = sin(angle);
       mat4 rotation = mat4(
           cosAngle, sinAngle, 0.0, 0.0,
          -sinAngle, cosAngle, 0.0, 0.0,
           0.0, 0.0, 1.0, 0.0,
           translation.x, translation.y, 0.0, 1.0);
       gl_Position = rotation * vec4(position, 1.0);
       vertexColor = color;
    }
)";

// 平面着色片段着色器
const GLchar* fragmentShaderSourceFlat = R"(
    #version 330 core
    in vec3 vertexColor; // 移除 flat 关键字
    out vec4 color;
    void main()
    {
       color = vec4(vertexColor, 1.0f);
    }
)";

// 全局变量
float rotationAngleTriangle = 0.0f;
float rotationAngleQuad = 0.0f;
float translationOffsetTriangleX = 0.0f;
float translationOffsetQuadX = 0.0f;
float translationSpeed = 0.0001f; // 控制平移速度
GLuint shaderProgramFlat;

GLFWwindow* initialize() {
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

  GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Rotating Triangle and Quad", nullptr, nullptr);
  if (!window) {
    cerr << "Unable to create GLFW window" << endl;
    glfwTerminate();
    return nullptr;
  }

  glfwMakeContextCurrent(window);

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

GLuint compileShaderProgram(const GLchar* fragmentShaderSource) {
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

  return shaderProgram;
}

int main(int argc, char* argv[]) {
  GLFWwindow* window = initialize();
  if (!window) {
    return -1;
  }

  shaderProgramFlat = compileShaderProgram(fragmentShaderSourceFlat);

  // 定义三角形和四边形的顶点
  GLfloat triangleVertices[] = {
      // Positions         // Colors
      -0.4f, -0.2f, 0.0f,  1.0f, 0.0f, 0.0f,
       0.4f, -0.2f, 0.0f,  0.0f, 1.0f, 0.0f,
       0.0f,  0.4f, 0.0f,  0.0f, 0.0f, 1.0f,
  };

  GLfloat quadVertices[] = {
      // Positions         // Colors
      -0.3f, -0.3f, 0.0f,  1.0f, 0.0f, 0.0f,
       0.3f, -0.3f, 0.0f,  0.0f, 1.0f, 0.0f,
       0.3f,  0.3f, 0.0f,  0.0f, 0.0f, 1.0f,
      -0.3f,  0.3f, 0.0f,  1.0f, 1.0f, 0.0f
  };

  GLuint VBOs[2], VAOs[2];
  glGenVertexArrays(2, VAOs);
  glGenBuffers(2, VBOs);

  // 三角形设置
  glBindVertexArray(VAOs[0]);
  glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);
  glBindVertexArray(0);

  // 四边形设置
  glBindVertexArray(VAOs[1]);
  glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);
  glBindVertexArray(0);

  while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();

      // 更新旋转和位移
      rotationAngleTriangle -= 0.002f;
      rotationAngleQuad += 0.002f;
      translationOffsetTriangleX += translationSpeed;
      translationOffsetQuadX -= translationSpeed;

      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      // 使用平面着色着色器程序
      glUseProgram(shaderProgramFlat);

      // 获取 uniform 变量位置
      GLint angleLocation = glGetUniformLocation(shaderProgramFlat, "angle");
      GLint translationLocation = glGetUniformLocation(shaderProgramFlat, "translation");

      // 绘制三角形
      glUniform1f(angleLocation, rotationAngleTriangle);
      glUniform2f(translationLocation, translationOffsetTriangleX, 0.2f);
      glBindVertexArray(VAOs[0]);
      glDrawArrays(GL_TRIANGLES, 0, 3);
      glBindVertexArray(0);

      // 绘制四边形
      glUniform1f(angleLocation, rotationAngleQuad);
      glUniform2f(translationLocation, translationOffsetQuadX, -0.2f);
      glBindVertexArray(VAOs[1]);
      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
      glBindVertexArray(0);

      glfwSwapBuffers(window);
  }

  glDeleteVertexArrays(2, VAOs);
  glDeleteBuffers(2, VBOs);
  glDeleteProgram(shaderProgramFlat);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
