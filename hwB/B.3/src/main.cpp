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
#include <filesystem> // Add at the top with other includes

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// Half-Edge Data Structures
struct Vertex;
struct HalfEdge;
struct Edge;
struct Face;

struct Vertex {
    glm::vec3 position;
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
    glm::vec3 color;
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
void renderModel(GLuint shaderProgram, int displayMode);
GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath);
GLFWwindow* initialize();

// Display modes
enum DisplayMode {
    DISPLAY_WIREFRAME = 1,
    DISPLAY_VERTICES,
    DISPLAY_FACES,
    DISPLAY_FACES_EDGES
};

int main() {
    GLFWwindow* window = initialize();
    if (!window) return -1;

    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
    // Rest of your code...

    // Load the OBJ file and build the half-edge structure
    if (!loadOBJ("./src/eight.uniform.obj")) {
        std::cout << "Failed to load OBJ file." << std::endl;
        return -1;
    }

    // Create shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 position;
    uniform mat4 MVP;
    void main()
    {
        gl_Position = MVP * vec4(position, 1.0);
    }
    )";
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShaderFaces = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* fragmentShaderSourceFaces = R"(
    #version 330 core
    out vec4 color;
    uniform vec3 faceColor;
    void main()
    {
        color = vec4(faceColor, 1.0);
    }
    )";
    glShaderSource(fragmentShaderFaces, 1, &fragmentShaderSourceFaces, NULL);
    glCompileShader(fragmentShaderFaces);

    GLuint fragmentShaderEdges = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* fragmentShaderSourceEdges = R"(
    #version 330 core
    out vec4 color;
    void main()
    {
        color = vec4(0.0, 0.0, 0.0, 1.0); // Black color for edges
    }
    )";
    glShaderSource(fragmentShaderEdges, 1, &fragmentShaderSourceEdges, NULL);
    glCompileShader(fragmentShaderEdges);

    GLuint fragmentShaderVertices = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* fragmentShaderSourceVertices = R"(
    #version 330 core
    out vec4 color;
    uniform vec3 vertexColor;
    void main()
    {
        color = vec4(vertexColor, 1.0);
    }
    )";
    glShaderSource(fragmentShaderVertices, 1, &fragmentShaderSourceVertices, NULL);
    glCompileShader(fragmentShaderVertices);

    // Create shader programs
    GLuint shaderProgramFaces = glCreateProgram();
    glAttachShader(shaderProgramFaces, vertexShader);
    glAttachShader(shaderProgramFaces, fragmentShaderFaces);
    glLinkProgram(shaderProgramFaces);

    GLuint shaderProgramEdges = glCreateProgram();
    glAttachShader(shaderProgramEdges, vertexShader);
    glAttachShader(shaderProgramEdges, fragmentShaderEdges);
    glLinkProgram(shaderProgramEdges);

    GLuint shaderProgramVertices = glCreateProgram();
    glAttachShader(shaderProgramVertices, vertexShader);
    glAttachShader(shaderProgramVertices, fragmentShaderVertices);
    glLinkProgram(shaderProgramVertices);

    // Setup VAOs and VBOs
    GLuint VAO_faces, VBO_faces;
    glGenVertexArrays(1, &VAO_faces);
    glGenBuffers(1, &VBO_faces);

    GLuint VAO_edges, VBO_edges;
    glGenVertexArrays(1, &VAO_edges);
    glGenBuffers(1, &VBO_edges);

    GLuint VAO_vertices, VBO_vertices;
    glGenVertexArrays(1, &VAO_vertices);
    glGenBuffers(1, &VBO_vertices);

    // Prepare data for faces
    std::vector<GLfloat> faceVertices;
    for (Face* face : faces) {
        HalfEdge* he = face->halfEdge;
        do {
            glm::vec3 pos = he->origin->position;
            faceVertices.push_back(pos.x);
            faceVertices.push_back(pos.y);
            faceVertices.push_back(pos.z);
            he = he->next;
        } while (he != face->halfEdge);
    }

    glBindVertexArray(VAO_faces);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_faces);
    glBufferData(GL_ARRAY_BUFFER, faceVertices.size() * sizeof(GLfloat), faceVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Prepare data for edges
    std::vector<GLfloat> edgeVertices;
    for (Edge* edge : edges) {
        HalfEdge* he = edge->halfEdge;
        glm::vec3 pos1 = he->origin->position;
        glm::vec3 pos2 = he->twin->origin->position;
        edgeVertices.push_back(pos1.x);
        edgeVertices.push_back(pos1.y);
        edgeVertices.push_back(pos1.z);
        edgeVertices.push_back(pos2.x);
        edgeVertices.push_back(pos2.y);
        edgeVertices.push_back(pos2.z);
    }

    glBindVertexArray(VAO_edges);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_edges);
    glBufferData(GL_ARRAY_BUFFER, edgeVertices.size() * sizeof(GLfloat), edgeVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Prepare data for vertices
    std::vector<GLfloat> vertexPositions;
    for (Vertex* vertex : vertices) {
        glm::vec3 pos = vertex->position;
        vertexPositions.push_back(pos.x);
        vertexPositions.push_back(pos.y);
        vertexPositions.push_back(pos.z);
    }

    glBindVertexArray(VAO_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_vertices);
    glBufferData(GL_ARRAY_BUFFER, vertexPositions.size() * sizeof(GLfloat), vertexPositions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // Transformation matrices
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 3.0f, 0.0f), // 将相机位置移到模型上方
        glm::vec3(0.0f, 0.0f, 0.0f), // 看向模型中心
        glm::vec3(0.0f, 0.0f, -1.0f) // 负z轴方向为“上”
    );
    glm::mat4 projection;

    // Display mode selection
    int displayMode = DISPLAY_FACES_EDGES;
    std::cout << "Select display mode:\n";
    std::cout << "1 - Wireframe (edges only)\n";
    std::cout << "2 - Vertices only\n";
    std::cout << "3 - Faces only\n";
    std::cout << "4 - Faces and edges\n";
    std::cout << "Enter mode number (default 4): ";
    std::string input;
    std::getline(std::cin, input);
    if (!input.empty()) displayMode = std::stoi(input);

    // Set face and vertex colors
    glm::vec3 faceColor(0.8f, 0.8f, 0.8f);     // Light gray
    glm::vec3 vertexColor(1.0f, 0.0f, 0.0f);   // Red

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Clear buffers
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f); // White background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // Compute MVP matrix
        projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
        glm::mat4 MVP = projection * view * model;

        // Render based on display mode
        if (displayMode == DISPLAY_WIREFRAME) {
            // Render edges
            glUseProgram(shaderProgramEdges);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgramEdges, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));

            glBindVertexArray(VAO_edges);
            glDrawArrays(GL_LINES, 0, edgeVertices.size() / 3);
            glBindVertexArray(0);
        }
        else if (displayMode == DISPLAY_VERTICES) {
            // Render vertices
            glUseProgram(shaderProgramVertices);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgramVertices, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
            glUniform3fv(glGetUniformLocation(shaderProgramVertices, "vertexColor"), 1, glm::value_ptr(vertexColor));

            glPointSize(5.0f);
            glBindVertexArray(VAO_vertices);
            glDrawArrays(GL_POINTS, 0, vertexPositions.size() / 3);
            glBindVertexArray(0);
        }
        else if (displayMode == DISPLAY_FACES) {
            // Render faces
            glUseProgram(shaderProgramFaces);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgramFaces, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
            glUniform3fv(glGetUniformLocation(shaderProgramFaces, "faceColor"), 1, glm::value_ptr(faceColor));

            glBindVertexArray(VAO_faces);
            glDrawArrays(GL_TRIANGLES, 0, faceVertices.size() / 3);
            glBindVertexArray(0);
        }
        else if (displayMode == DISPLAY_FACES_EDGES) {
            // Render faces
            glUseProgram(shaderProgramFaces);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgramFaces, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
            glUniform3fv(glGetUniformLocation(shaderProgramFaces, "faceColor"), 1, glm::value_ptr(faceColor));

            glBindVertexArray(VAO_faces);
            glDrawArrays(GL_TRIANGLES, 0, faceVertices.size() / 3);
            glBindVertexArray(0);

            // Render edges on top
            glUseProgram(shaderProgramEdges);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgramEdges, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));

            glBindVertexArray(VAO_edges);
            glDrawArrays(GL_LINES, 0, edgeVertices.size() / 3);
            glBindVertexArray(0);
        }

        // Swap buffers
        glfwSwapBuffers(window);
    }

    // Clean up
    glDeleteVertexArrays(1, &VAO_faces);
    glDeleteBuffers(1, &VBO_faces);
    glDeleteVertexArrays(1, &VAO_edges);
    glDeleteBuffers(1, &VBO_edges);
    glDeleteVertexArrays(1, &VAO_vertices);
    glDeleteBuffers(1, &VBO_vertices);
    glDeleteProgram(shaderProgramFaces);
    glDeleteProgram(shaderProgramEdges);
    glDeleteProgram(shaderProgramVertices);

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
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "3D Model Display", nullptr, nullptr);
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
        face->color = glm::vec3(0.8f, 0.8f, 0.8f); // Default face color
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

