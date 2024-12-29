#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const int WIDTH = 800;
const int HEIGHT = 600;

GLuint loadTexture(const char* path);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// 顶点着色器代码
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    out vec2 TexCoord;
    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
)";

// 片段着色器代码
const char* fragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
    uniform sampler2D texture1;
    uniform vec3 color;
    void main() {
        float brightness = 10; // 增亮因子
        FragColor = texture(texture1, TexCoord) * vec4(color * brightness, 1.0);
    }
)";

GLuint createShaderProgram();

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Rotating Stars", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 创建着色器程序
    GLuint shaderProgram = createShaderProgram();

    // 加载纹理
    GLuint starTexture = loadTexture("src/Star.bmp");

    // 设置星星的初始参数
    const float maxRadius = 1.5f; // 星星最大可视半径
    std::vector<float> starLifetimes; // 每个星星的生命周期，用于计算位置
    std::vector<glm::vec3> starColors; // 每颗星星的颜色

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float vertices[] = {
        // positions       // texture coords
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
    };
    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glUseProgram(shaderProgram);

    // 设置视图矩阵（相机位置）
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.5f));
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // 设置投影矩阵（透视投影）
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    float newStarInterval = 1.0f; // 每1秒产生一个新星星
    float elapsedTime = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float deltaTime = 0.005f; // 每次循环的时间步长，用于模拟时间
        elapsedTime += deltaTime;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glBindTexture(GL_TEXTURE_2D, starTexture);

        GLuint colorLoc = glGetUniformLocation(shaderProgram, "color");
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");

        // 定时产生新星星
        if (elapsedTime >= newStarInterval) {
            elapsedTime = 0.0f;

            // 添加一个新的星星在中心
            starLifetimes.push_back(0.0f); // 初始生命周期为0（从中心开始）
            float r = static_cast<float>(rand()) / RAND_MAX; // 随机颜色
            float g = static_cast<float>(rand()) / RAND_MAX;
            float b = static_cast<float>(rand()) / RAND_MAX;
            starColors.push_back(glm::vec3(r, g, b));
        }

        // 逐个绘制星星
        for (size_t i = 0; i < starLifetimes.size(); ++i) {
            float theta = starLifetimes[i];
            float radius = 0.07f * theta; // 阿基米德螺线公式 r = b * theta

            // 检查是否超过最大可视半径，如果超过则不再绘制该星星
            if (radius > maxRadius) {
                continue;
            }

            // 计算星星的位置
            float x = radius * cos(theta);
            float y = radius * sin(theta);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, y, 0.0f));
            model = glm::scale(model, glm::vec3(0.08f, 0.08f, 1.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            // 为每个星星设置颜色
            glm::vec3 color = starColors[i];
            glUniform3fv(colorLoc, 1, glm::value_ptr(color));

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // 更新生命周期，使星星缓慢向外扩散
            starLifetimes[i] += deltaTime; // 使用小步长控制扩散速度
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "Texture loaded successfully: " << path << std::endl;
    } else {
        std::cerr << "Failed to load texture: " << path << std::endl;
    }
    stbi_image_free(data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

void checkShaderCompileStatus(GLuint shader, const std::string& type) {
    GLint success;
    GLchar infoLog[512];
    if (type == "PROGRAM") {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 512, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
    } else {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }
}

GLuint createShaderProgram() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkShaderCompileStatus(vertexShader, "VERTEX");

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkShaderCompileStatus(fragmentShader, "FRAGMENT");

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkShaderCompileStatus(shaderProgram, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
