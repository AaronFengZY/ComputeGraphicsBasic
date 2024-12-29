// main.cpp

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// Mouse control variables
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;
bool leftButtonPressed = false;

// Camera control variables
float yaw = -90.0f;  // Yaw is initialized to -90.0 degrees to point towards negative Z
float pitch = 0.0f;
float fov = 45.0f;

glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

// Model rotation variables
float rotationX = 0.0f;
float rotationY = 0.0f;

// Half-Edge Data Structures
struct Vertex;
struct HalfEdge;
struct Edge;
struct Face;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal; // Added normal for lighting calculations
    HalfEdge* halfEdge = nullptr;
    int id;
};

struct HalfEdge {
    Vertex* origin = nullptr;
    HalfEdge* twin = nullptr;
    HalfEdge* next = nullptr;
    Face* face = nullptr;
    Edge* edge = nullptr;
};

struct Edge {
    HalfEdge* halfEdge = nullptr;
};

struct Face {
    HalfEdge* halfEdge = nullptr;
    glm::vec3 normal; // Added normal for flat shading
};

// Global variables
std::vector<Vertex*> vertices;
std::vector<HalfEdge*> halfEdges;
std::vector<Edge*> edges;
std::vector<Face*> faces;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
bool loadOBJ(const std::string& path);
void buildHalfEdgeStructure(const std::vector<glm::ivec3>& faceIndices);
void computeVertexNormals();
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource);
GLFWwindow* initialize();
void getUserInput(const std::string& prompt, float& variable, float defaultValue);
void getUserColorSelection(glm::vec3& color);

// Mouse and scroll callbacks
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int main() {
    GLFWwindow* window = initialize();
    if (!window) return -1;


    // Adjust the OBJ file path
    std::string objFilePath = "./src/eight.uniform.obj";

    // Load the OBJ file and build the half-edge structure
    if (!loadOBJ(objFilePath)) {
        std::cout << "Failed to load OBJ file." << std::endl;
        return -1;
    }

    // Compute vertex normals for lighting
    computeVertexNormals();

    // Vertex Shader
    const GLchar* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 position;
    layout (location = 1) in vec3 normal;
    out vec3 FragPos;
    out vec3 Normal;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    void main()
    {
        FragPos = vec3(model * vec4(position, 1.0));
        Normal = mat3(transpose(inverse(model))) * normal;
        gl_Position = projection * view * vec4(FragPos, 1.0);
    }
    )";

    // Fragment Shader
    const GLchar* fragmentShaderSource = R"(
    #version 330 core
    struct Material {
        vec3 color;
        float ambientStrength;
        float diffuseStrength;
        float specularStrength;
        float shininess;
    };

    struct Light {
        vec3 position;
        vec3 color;
    };

    in vec3 FragPos;
    in vec3 Normal;
    out vec4 color;

    uniform Material material;
    uniform Light light;
    uniform vec3 viewPos;

    void main()
    {
        // Ambient
        vec3 ambient = material.ambientStrength * light.color;

        // Diffuse
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(light.position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = material.diffuseStrength * diff * light.color;

        // Specular
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = material.specularStrength * spec * light.color;

        vec3 result = (ambient + diffuse + specular) * material.color;
        color = vec4(result, 1.0);
    }
    )";

    // Create shader program
    GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    // Prepare data for faces
    std::vector<GLfloat> faceVertices;
    for (Face* face : faces) {
        HalfEdge* he = face->halfEdge;
        do {
            Vertex* v = he->origin;
            // Position
            faceVertices.push_back(v->position.x);
            faceVertices.push_back(v->position.y);
            faceVertices.push_back(v->position.z);
            // Normal
            faceVertices.push_back(v->normal.x);
            faceVertices.push_back(v->normal.y);
            faceVertices.push_back(v->normal.z);
            he = he->next;
        } while (he != face->halfEdge);
    }

    GLuint VAO_faces, VBO_faces;
    glGenVertexArrays(1, &VAO_faces);
    glGenBuffers(1, &VBO_faces);

    glBindVertexArray(VAO_faces);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_faces);
    glBufferData(GL_ARRAY_BUFFER, faceVertices.size() * sizeof(GLfloat), faceVertices.data(), GL_STATIC_DRAW);
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Transformation matrices
    glm::mat4 model = glm::mat4(1.0f);

    // User input for lighting parameters
    float ambientStrength = 0.1f;
    float diffuseStrength = 0.8f;
    float specularStrength = 0.5f;
    float shininess = 32.0f;

    std::cout << "Enter ambient strength (default 0.1): ";
    std::string input;
    std::getline(std::cin, input);
    if (!input.empty()) ambientStrength = std::stof(input);

    std::cout << "Enter diffuse strength (default 0.8): ";
    std::getline(std::cin, input);
    if (!input.empty()) diffuseStrength = std::stof(input);

    std::cout << "Enter specular strength (default 0.5): ";
    std::getline(std::cin, input);
    if (!input.empty()) specularStrength = std::stof(input);

    std::cout << "Enter shininess (default 32): ";
    std::getline(std::cin, input);
    if (!input.empty()) shininess = std::stof(input);

    // Get user color selection
    glm::vec3 modelColor;
    getUserColorSelection(modelColor);

    // Light properties
    glm::vec3 lightPos(1.0f, 1.0f, 2.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

    // Set the mouse callbacks
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Capture the mouse cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Clear buffers
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f); // White background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // View matrix (camera remains fixed)
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f), cameraUp);

        // Projection matrix
        glm::mat4 projection = glm::perspective(glm::radians(fov), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

        // Model matrix with rotations
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));

        // Use shader program
        glUseProgram(shaderProgram);

        // Pass transformation matrices to shader
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Pass material properties to shader
        glUniform3fv(glGetUniformLocation(shaderProgram, "material.color"), 1, glm::value_ptr(modelColor));
        glUniform1f(glGetUniformLocation(shaderProgram, "material.ambientStrength"), ambientStrength);
        glUniform1f(glGetUniformLocation(shaderProgram, "material.diffuseStrength"), diffuseStrength);
        glUniform1f(glGetUniformLocation(shaderProgram, "material.specularStrength"), specularStrength);
        glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), shininess);

        // Pass light properties to shader
        glUniform3fv(glGetUniformLocation(shaderProgram, "light.position"), 1, glm::value_ptr(lightPos));
        glUniform3fv(glGetUniformLocation(shaderProgram, "light.color"), 1, glm::value_ptr(lightColor));

        // Pass view position to shader
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));

        // Render faces
        glBindVertexArray(VAO_faces);
        glDrawArrays(GL_TRIANGLES, 0, faceVertices.size() / 6);
        glBindVertexArray(0);

        // Swap buffers
        glfwSwapBuffers(window);
    }

    // Clean up
    glDeleteVertexArrays(1, &VAO_faces);
    glDeleteBuffers(1, &VBO_faces);
    glDeleteProgram(shaderProgram);

    // Delete dynamically allocated data
    for (Vertex* v : vertices) delete v;
    for (HalfEdge* he : halfEdges) delete he;
    for (Edge* e : edges) delete e;
    for (Face* f : faces) delete f;

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

GLFWwindow* initialize() {
    if (!glfwInit()) return nullptr;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "3D Model Display with Lighting", nullptr, nullptr);
    if (window) glfwMakeContextCurrent(window);
    if (!gladLoadGL()) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }
    glViewport(0, 0, WIDTH, HEIGHT);
    glfwSetKeyCallback(window, key_callback);
    return window;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    // Close window on ESC key
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

// Load OBJ file and build half-edge structure
bool loadOBJ(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Cannot open file: " << path << std::endl;
        return false;
    }

    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::ivec3> faceIndices;

    std::string line;
    while (std::getline(file, line)) {
        if (line.substr(0, 2) == "v ") {
            std::istringstream s(line.substr(2));
            glm::vec3 v;
            s >> v.x; s >> v.y; s >> v.z;
            temp_vertices.push_back(v);
        }
        else if (line.substr(0, 2) == "f ") {
            std::istringstream s(line.substr(2));
            glm::ivec3 f;
            s >> f.x; s >> f.y; s >> f.z;
            // OBJ indices start at 1
            f -= glm::ivec3(1);
            faceIndices.push_back(f);
        }
    }
    file.close();

    // Create vertices
    for (size_t i = 0; i < temp_vertices.size(); ++i) {
        Vertex* v = new Vertex();
        v->position = temp_vertices[i];
        v->id = i;
        vertices.push_back(v);
    }

    // Build half-edge structure
    buildHalfEdgeStructure(faceIndices);

    return true;
}

void buildHalfEdgeStructure(const std::vector<glm::ivec3>& faceIndices) {
    std::unordered_map<std::string, HalfEdge*> edgeMap;

    for (const auto& indices : faceIndices) {
        Face* face = new Face();
        faces.push_back(face);

        HalfEdge* he1 = new HalfEdge();
        HalfEdge* he2 = new HalfEdge();
        HalfEdge* he3 = new HalfEdge();

        he1->origin = vertices[indices.x];
        he2->origin = vertices[indices.y];
        he3->origin = vertices[indices.z];

        he1->next = he2;
        he2->next = he3;
        he3->next = he1;

        he1->face = face;
        he2->face = face;
        he3->face = face;

        face->halfEdge = he1;

        halfEdges.push_back(he1);
        halfEdges.push_back(he2);
        halfEdges.push_back(he3);

        // Create edges and assign twins
        Edge* edge1 = new Edge();
        Edge* edge2 = new Edge();
        Edge* edge3 = new Edge();

        edges.push_back(edge1);
        edges.push_back(edge2);
        edges.push_back(edge3);

        he1->edge = edge1;
        he2->edge = edge2;
        he3->edge = edge3;

        edge1->halfEdge = he1;
        edge2->halfEdge = he2;
        edge3->halfEdge = he3;

        // Store half-edges in map for twin finding
        std::string key1 = std::to_string(he1->origin->id) + "_" + std::to_string(he1->next->origin->id);
        std::string key2 = std::to_string(he2->origin->id) + "_" + std::to_string(he2->next->origin->id);
        std::string key3 = std::to_string(he3->origin->id) + "_" + std::to_string(he3->next->origin->id);

        edgeMap[key1] = he1;
        edgeMap[key2] = he2;
        edgeMap[key3] = he3;
    }

    // Assign twins
    for (HalfEdge* he : halfEdges) {
        std::string twinKey = std::to_string(he->next->origin->id) + "_" + std::to_string(he->origin->id);
        if (edgeMap.find(twinKey) != edgeMap.end()) {
            he->twin = edgeMap[twinKey];
            he->twin->twin = he;
        }
    }
}

// Compute vertex normals for lighting calculations
void computeVertexNormals() {
    // Initialize normals
    for (Vertex* v : vertices) {
        v->normal = glm::vec3(0.0f);
    }

    // Compute face normals and accumulate
    for (Face* face : faces) {
        HalfEdge* he = face->halfEdge;
        glm::vec3 v0 = he->origin->position;
        glm::vec3 v1 = he->next->origin->position;
        glm::vec3 v2 = he->next->next->origin->position;
        glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        face->normal = normal;

        // Accumulate normals to vertices
        he->origin->normal += normal;
        he->next->origin->normal += normal;
        he->next->next->origin->normal += normal;
    }

    // Normalize vertex normals
    for (Vertex* v : vertices) {
        v->normal = glm::normalize(v->normal);
    }
}

GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    // Check for vertex shader compile errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::VERTEX_SHADER_COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // Check for fragment shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::FRAGMENT_SHADER_COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Link shaders into a program
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

    // Delete shaders as they're linked into our program now and no longer necessery
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Get user input for a float variable with a default value
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

// Provide a selection of colors for the user to choose
void getUserColorSelection(glm::vec3& color) {
    std::cout << "Select model color:\n";
    std::cout << "1 - Red\n";
    std::cout << "2 - Green\n";
    std::cout << "3 - Blue\n";
    std::cout << "4 - Yellow\n";
    std::cout << "5 - Purple\n";
    std::cout << "6 - Cyan\n";
    std::cout << "7 - White\n";
    std::cout << "Enter color number (default 7): ";
    std::string input;
    int colorChoice = 7; // Default to white
    std::getline(std::cin, input);
    if (!input.empty()) colorChoice = std::stoi(input);

    switch (colorChoice) {
        case 1:
            color = glm::vec3(1.0f, 0.0f, 0.0f); // Red
            break;
        case 2:
            color = glm::vec3(0.0f, 1.0f, 0.0f); // Green
            break;
        case 3:
            color = glm::vec3(0.0f, 0.0f, 1.0f); // Blue
            break;
        case 4:
            color = glm::vec3(1.0f, 1.0f, 0.0f); // Yellow
            break;
        case 5:
            color = glm::vec3(0.5f, 0.0f, 0.5f); // Purple
            break;
        case 6:
            color = glm::vec3(0.0f, 1.0f, 1.0f); // Cyan
            break;
        case 7:
        default:
            color = glm::vec3(1.0f, 1.0f, 1.0f); // White
            break;
    }
}

// Mouse movement callback
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!leftButtonPressed) {
        firstMouse = true;
        return;
    }

    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos; // Reversed since y-coordinates go from bottom to top

    lastX = (float)xpos;
    lastY = (float)ypos;

    float sensitivity = 0.2f; // Adjust this value for sensitivity
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    // Update rotation angles
    rotationY += xoffset;
    rotationX += yoffset;

    // Constrain the rotation angles if desired
    if(rotationX > 89.0f)
        rotationX = 89.0f;
    if(rotationX < -89.0f)
        rotationX = -89.0f;
}

// Mouse button callback
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            leftButtonPressed = true;
            firstMouse = true;
        } else if (action == GLFW_RELEASE) {
            leftButtonPressed = false;
        }
    }
}

// Scroll callback for zooming
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 90.0f)
        fov = 90.0f;
}
