#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

const int WIDTH = 800;
const int HEIGHT = 600;

void getUserInput(const std::string& prompt, float& variable, float defaultValue) {
    std::cout << prompt << " (Press Enter to use default " << defaultValue << "): ";
    std::string input;
    std::getline(std::cin, input);
    if (input.empty()) {
        variable = defaultValue;
    } else {
        variable = std::stof(input);
    }
}

void getUserInput(const std::string& prompt, int& variable, int defaultValue) {
    std::cout << prompt << " (Press Enter to use default " << defaultValue << "): ";
    std::string input;
    std::getline(std::cin, input);
    if (input.empty()) {
        variable = defaultValue;
    } else {
        variable = std::stoi(input);
    }
}

// Vertex Shader
const GLchar* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 position;
    layout (location = 1) in vec3 color;
    layout (location = 2) in vec3 normal;
    out vec3 fragColor;
    out vec3 fragPos;
    out vec3 fragNormal;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    void main()
    {
        fragPos = vec3(model * vec4(position, 1.0));
        fragColor = color;
        fragNormal = mat3(transpose(inverse(model))) * normal;
        gl_Position = projection * view * vec4(fragPos, 1.0);
    }
)";

// Fragment Shader
const GLchar* fragmentShaderSource = R"(
    #version 330 core
    in vec3 fragColor;
    in vec3 fragPos;
    in vec3 fragNormal;
    out vec4 color;

    // Lighting parameters
    uniform vec3 lightPos;
    uniform vec3 viewPos;
    uniform vec3 lightColor;
    uniform float ambientStrength;
    uniform float diffuseStrength;
    uniform float specularStrength;
    uniform int shininess;

    void main()
    {
        // Ambient
        vec3 ambient = ambientStrength * lightColor;

        // Diffuse
        vec3 norm = normalize(fragNormal);
        vec3 lightDir = normalize(lightPos - fragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diffuseStrength * diff * lightColor;

        // Specular
        vec3 viewDir = normalize(viewPos - fragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = specularStrength * spec * lightColor;

        vec3 result = (ambient + diffuse + specular) * fragColor;
        color = vec4(result, 1.0);
    }
)";

GLuint createShaderProgram(const char* vShaderCode, const char* fShaderCode);
GLFWwindow* initialize();

int main() {
    GLFWwindow* window = initialize();
    if (!window) return -1;

    GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    // Vertex data: Expanded to include correct normals for each face
    GLfloat pyramidVertices[] = {
        // 底面顶点（位置，颜色，法线）
        // 顶点 A (索引 0)
        0.0f, 1.654f, 2.0f,   1.0f, 0.0f, 0.0f,    0.0f, -1.0f, 0.0f,
        // 顶点 B (索引 1)
        2.0f, 1.654f, 2.0f,   0.0f, 1.0f, 0.0f,    0.0f, -1.0f, 0.0f,
        // 顶点 C (索引 2)
        2.0f, 1.654f, 4.0f,   0.0f, 0.0f, 1.0f,    0.0f, -1.0f, 0.0f,
        // 顶点 D (索引 3)
        0.0f, 1.654f, 4.0f,   1.0f, 1.0f, 0.0f,    0.0f, -1.0f, 0.0f,

        // 侧面 1
        // 顶点 A
        0.0f, 1.654f, 2.0f,   1.0f, 0.0f, 0.0f,    -0.577f, 0.577f, -0.577f,
        // 顶点 B
        2.0f, 1.654f, 2.0f,   0.0f, 1.0f, 0.0f,    0.577f, 0.577f, -0.577f,
        // 顶点 E
        1.0f, 3.386f, 3.0f,   1.0f, 0.0f, 1.0f,    0.0f, 0.577f, -0.577f,

        // 侧面 2
        // 顶点 B
        2.0f, 1.654f, 2.0f,   0.0f, 1.0f, 0.0f,    0.577f, 0.577f, -0.577f,
        // 顶点 C
        2.0f, 1.654f, 4.0f,   0.0f, 0.0f, 1.0f,    0.577f, 0.577f, 0.577f,
        // 顶点 E
        1.0f, 3.386f, 3.0f,   1.0f, 0.0f, 1.0f,    0.0f, 0.577f, 0.577f,

        // 侧面 3
        // 顶点 C
        2.0f, 1.654f, 4.0f,   0.0f, 0.0f, 1.0f,    0.577f, 0.577f, 0.577f,
        // 顶点 D
        0.0f, 1.654f, 4.0f,   1.0f, 1.0f, 0.0f,    -0.577f, 0.577f, 0.577f,
        // 顶点 E
        1.0f, 3.386f, 3.0f,   1.0f, 0.0f, 1.0f,    0.0f, 0.577f, 0.577f,

        // 侧面 4
        // 顶点 D
        0.0f, 1.654f, 4.0f,   1.0f, 1.0f, 0.0f,    -0.577f, 0.577f, 0.577f,
        // 顶点 A
        0.0f, 1.654f, 2.0f,   1.0f, 0.0f, 0.0f,    -0.577f, 0.577f, -0.577f,
        // 顶点 E
        1.0f, 3.386f, 3.0f,   1.0f, 0.0f, 1.0f,    0.0f, 0.577f, -0.577f,
    };

    // Index data: Define base quadrilateral and four sides
    GLuint indices[] = {
        // Base
        0, 1, 2,
        0, 2, 3,

        // Side face 1
        4, 5, 6,
        // Side face 2
        7, 8, 9,
        // Side face 3
        10, 11, 12,
        // Side face 4
        13, 14, 15
    };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // Normal attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    glUseProgram(shaderProgram);

    // Set default lighting and material parameters
    float ambientStrength = 0.3f;
    float diffuseStrength = 0.5f;
    float specularStrength = 0.5f;
    int shininess = 32;
    glm::vec3 lightPos(1.0f, 6.0f, 3.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

    // Runtime user input for lighting parameters
    getUserInput("Enter ambient strength (ambientStrength)", ambientStrength, 0.4f);
    getUserInput("Enter diffuse strength (diffuseStrength)", diffuseStrength, 0.5f);
    getUserInput("Enter specular strength (specularStrength)", specularStrength, 1.0f);
    getUserInput("Enter shininess", shininess, 32);

    std::cout << "Enter light color R (Press Enter to use default " << lightColor.r << "): ";
    std::string input;
    std::getline(std::cin, input);
    if (!input.empty()) lightColor.r = std::stof(input);

    std::cout << "Enter light color G (Press Enter to use default " << lightColor.g << "): ";
    std::getline(std::cin, input);
    if (!input.empty()) lightColor.g = std::stof(input);

    std::cout << "Enter light color B (Press Enter to use default " << lightColor.b << "): ";
    std::getline(std::cin, input);
    if (!input.empty()) lightColor.b = std::stof(input);

    std::cout << "Enter light position x (Press Enter to use default " << lightPos.x << "): ";
    std::getline(std::cin, input);
    if (!input.empty()) lightPos.x = std::stof(input);

    std::cout << "Enter light position y (Press Enter to use default " << lightPos.y << "): ";
    std::getline(std::cin, input);
    if (!input.empty()) lightPos.y = std::stof(input);

    std::cout << "Enter light position z (Press Enter to use default " << lightPos.z << "): ";
    std::getline(std::cin, input);
    if (!input.empty()) lightPos.z = std::stof(input);

    // Pass lighting parameters to the shader
    glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), lightColor.r, lightColor.g, lightColor.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "ambientStrength"), ambientStrength);
    glUniform1f(glGetUniformLocation(shaderProgram, "diffuseStrength"), diffuseStrength);
    glUniform1f(glGetUniformLocation(shaderProgram, "specularStrength"), specularStrength);
    glUniform1i(glGetUniformLocation(shaderProgram, "shininess"), shininess);
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));

    glm::vec3 viewPos(3.0f, 6.0f, 10.0f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // Model matrix: translate pyramid to center at (1, 2, 3)
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 3.0f));
        // View matrix
        glm::mat4 view = glm::lookAt(
            viewPos,
            glm::vec3(1.0f, 2.0f, 3.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        // Projection matrix
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(viewPos));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

GLuint createShaderProgram(const char* vShaderCode, const char* fShaderCode) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, NULL);
    glCompileShader(vertexShader);

    // Check for vertex shader compile errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::VERTEX_SHADER_COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);

    // Check for fragment shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::FRAGMENT_SHADER_COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER_PROGRAM_LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLFWwindow* initialize() {
    if (!glfwInit()) return nullptr;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Colored Pyramid with Lighting", nullptr, nullptr);
    if (window) glfwMakeContextCurrent(window);
    if (!gladLoadGL()) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }
    glViewport(0, 0, WIDTH, HEIGHT);
    return window;
}
