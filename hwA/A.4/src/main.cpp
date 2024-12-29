#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <cassert>
#include <fstream>
#include <sstream>
#include <windows.h>
#include "glad/gldebug.h"
#include "Shader/shader.h"
#include "Mesh/Mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

constexpr unsigned int SCR_WIDTH = 800;
constexpr unsigned int SCR_HEIGHT = 600;
constexpr float PI = 3.1415926f;
glm::vec3 lightPos(3.0f, 3.0f, 0.0f);

// 索引对结构体，表示两顶点的边
struct IndexPair2 {
    size_t v1, v2;
    IndexPair2() : v1(INVALID_INDEX), v2(INVALID_INDEX) {}
    IndexPair2(size_t vertex1, size_t vertex2) : v1(vertex1), v2(vertex2) {}

    bool operator==(const IndexPair2& other) const {
        return v1 == other.v1 && v2 == other.v2;
    }
};

// 用于计算索引对的哈希值
template <typename T>
inline void hash_combine(std::size_t& seed, const T& val) {
    seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

// 计算多个值的组合哈希值
template <typename... Types>
inline std::size_t hash_val(const Types&... args) {
    std::size_t seed = 0;
    (hash_combine(seed, args), ...);
    return seed;
}

// 索引对哈希函数
struct IndexPair2Hash {    
    std::size_t operator()(const IndexPair2& pair) const {        
        return hash_val(pair.v1, pair.v2);
    }	
};

// 索引三元组结构体，表示三顶点的关系
struct IndexPair3 {
    size_t vN, vOpp1, vOpp2;
    IndexPair3() : vN(INVALID_INDEX), vOpp1(INVALID_INDEX), vOpp2(INVALID_INDEX) {}
    IndexPair3(size_t vertexN, size_t vertexOpp1, size_t vertexOpp2 = INVALID_INDEX)
        : vN(vertexN), vOpp1(vertexOpp1), vOpp2(vertexOpp2) {}
};

// 添加边的顶点到边顶点映射中
int addEdgeVertex(std::unordered_map<IndexPair2, IndexPair3, IndexPair2Hash>& edgeVertices,
                 size_t v1Index, size_t v2Index, size_t v3Index, size_t& newIndex) {
    if (v1Index > v2Index) {
        std::swap(v1Index, v2Index);
    }
    IndexPair2 vertsPair(v1Index, v2Index);
    if (edgeVertices.find(vertsPair) == edgeVertices.end()) {
        edgeVertices[vertsPair] = IndexPair3(newIndex++, v3Index);
    } else {
        edgeVertices[vertsPair].vOpp2 = v3Index;
    }
    return edgeVertices[vertsPair].vN;
}

// Loop细分算法
Mesh LoopSubdivideNative(const Mesh& mesh) {
    Mesh newMesh;
    size_t numVerts = mesh.vertices.size();
    for (size_t i = 0; i < numVerts; i++) {
        newMesh.addVertex(mesh.vertices[i].position);
    }

    size_t numFaces = mesh.faceElements.size();
    size_t newIndexOfVertices = numVerts;
    std::unordered_map<IndexPair2, IndexPair3, IndexPair2Hash> edgeVertices;
    std::vector<size_t> newFaces;

    // 处理每个面
    for (size_t f_id = 0; f_id < numFaces; f_id++) {
        const FaceElement& face = mesh.faceElements[f_id];
        size_t startHalfEdgeId = face.startHalfEdgeId;
        size_t halfEdgeId = startHalfEdgeId;

        std::vector<size_t> vertexIds;
        do {
            vertexIds.push_back(mesh.halfEdges[halfEdgeId].fromVertexId);
            halfEdgeId = mesh.halfEdges[halfEdgeId].nextHalfEdgeId;
        } while (halfEdgeId != startHalfEdgeId);

        if (vertexIds.size() != 3) {
            std::cerr << "Error: Only triangular faces are supported!" << std::endl;
            continue;
        }

        // 为每条边添加顶点
        size_t vpIndex = addEdgeVertex(edgeVertices, vertexIds[0], vertexIds[1], vertexIds[2], newIndexOfVertices);
        if (vpIndex >= newMesh.vertices.size()) {
            newMesh.addVertex(glm::vec3(0.0f));
        }

        size_t vqIndex = addEdgeVertex(edgeVertices, vertexIds[1], vertexIds[2], vertexIds[0], newIndexOfVertices);
        if (vqIndex >= newMesh.vertices.size()) {
            newMesh.addVertex(glm::vec3(0.0f));
        }

        size_t vrIndex = addEdgeVertex(edgeVertices, vertexIds[2], vertexIds[0], vertexIds[1], newIndexOfVertices);
        if (vrIndex >= newMesh.vertices.size()) {
            newMesh.addVertex(glm::vec3(0.0f));
        }

        // 新增面
        newFaces.push_back(vertexIds[0]); newFaces.push_back(vpIndex); newFaces.push_back(vrIndex);
        newFaces.push_back(vpIndex); newFaces.push_back(vertexIds[1]); newFaces.push_back(vqIndex);
        newFaces.push_back(vrIndex); newFaces.push_back(vqIndex); newFaces.push_back(vertexIds[2]);
        newFaces.push_back(vrIndex); newFaces.push_back(vpIndex); newFaces.push_back(vqIndex);
    }

    // 计算边的中点并设置新顶点的位置
    for (auto& [edgePair, edgeInfo] : edgeVertices) {
        size_t v1 = edgePair.v1, v2 = edgePair.v2;
        size_t vN = edgeInfo.vN;
        size_t vOpp1 = edgeInfo.vOpp1, vOpp2 = edgeInfo.vOpp2;
        assert(vN != INVALID_INDEX);
        assert(v1 != INVALID_INDEX && v2 != INVALID_INDEX);
        assert(vOpp1 != INVALID_INDEX);

        glm::vec3 v1Pos = newMesh.vertices[v1].position;
        glm::vec3 v2Pos = newMesh.vertices[v2].position;

        if (vOpp2 == INVALID_INDEX) {
            newMesh.vertices[vN].position = 0.5f * (v1Pos + v2Pos);
        } else {
            glm::vec3 vNOpp1Pos = newMesh.vertices[vOpp1].position;
            glm::vec3 vNOpp2Pos = newMesh.vertices[vOpp2].position;
            glm::vec3 newPos = 0.375f * (v1Pos + v2Pos) + 0.125f * (vNOpp1Pos + vNOpp2Pos);
            newMesh.vertices[vN].position = newPos;
        }
    }

    // 更新顶点位置
    for (size_t v = 0; v < numVerts; v++) {
        glm::vec3 newPos(0.0f), adjBoundaryPos(0.0f);
        unsigned adjCount = 0, adjBoundaryCount = 0;
        const std::vector<size_t>& outgoingHalfEdges = mesh.vertexElements[v].outgoingHalfEdgeIds;

        for (size_t heId : outgoingHalfEdges) {
            size_t neighborV = mesh.halfEdges[heId].toVertexId;
            IndexPair2 edgePair(std::min(v, neighborV), std::max(v, neighborV));
            auto it = edgeVertices.find(edgePair);
            if (it == edgeVertices.end() || it->second.vOpp2 == INVALID_INDEX) {
                adjBoundaryCount++;
                adjBoundaryPos += mesh.vertices[neighborV].position;
            }
            newPos += mesh.vertices[neighborV].position;
            adjCount++;
        }

        if (adjBoundaryCount == 2) {
            newPos = 0.75f * mesh.vertices[v].position + 0.125f * adjBoundaryPos;
        } else {
            double val = 0.375 + 0.25 * std::cos(2.0 * PI / static_cast<double>(adjCount));
            double beta = (0.625 - val * val) / static_cast<double>(adjCount);
            newPos = static_cast<float>((1.0 - beta * adjCount)) * mesh.vertices[v].position + 
                     static_cast<float>(beta) * glm::vec3(newPos);
        }

        newMesh.vertices[v].position = newPos;
    }

    // 创建新面
    size_t newNumFaces = newFaces.size() / 3;
    for (size_t f = 0; f < newNumFaces; f++) {
        size_t loc = f * 3;
        std::vector<size_t> faceVertices = { newFaces[loc], newFaces[loc + 1], newFaces[loc + 2] };
        newMesh.addFace(faceVertices);
    }

    return newMesh;
}

int subdivisionLevel = 0;
float subdivisionLevelF = 0.0f;

void handleInputEvents(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        subdivisionLevelF = subdivisionLevelF + 0.01f; 
        int newSubdivisionLevel = static_cast<int>(subdivisionLevelF);
        if (newSubdivisionLevel > subdivisionLevel) {
            subdivisionLevel = newSubdivisionLevel;
            std::cout << "Subdivision level increased to: " << subdivisionLevel << std::endl;
        }
    } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        subdivisionLevelF = std::max(0.0f, subdivisionLevelF - 0.01f);
        int newSubdivisionLevel = static_cast<int>(subdivisionLevelF);
        if (newSubdivisionLevel < subdivisionLevel) {
            subdivisionLevel = newSubdivisionLevel;
            std::cout << "Subdivision level decreased to: " << subdivisionLevel << std::endl;
        }
    }
}

int main()
{
    // 初始化GLFW
    if (!glfwInit()) {
        std::cerr << "错误：GLFW初始化失败！" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); 

    // 创建GLFW窗口，OpenGL版本4.6
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "A-4", NULL, NULL);
    if (!window) {
        std::cerr << "错误：GLFW窗口创建失败！" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "初始化GLAD失败" << std::endl;
        return -1;
    }

    // 启用深度测试和背面剔除
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); // 剔除背面

    // 加载着色器程序
    Shader ourShader("./include/Shader/vs/A_4.vs", "./include/Shader/fs/A_4.fs");

    // 模型文件路径
    std::string filename = "./src/cow.obj"; 

    // 加载模型并添加到网格列表
    std::vector<Mesh> meshList;
    meshList.emplace_back(filename); // 添加模型

    // 相机设置
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 10.0f); // 相机位置
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // 相机方向（看向负Z轴）
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); // 相机上方向

    // 视图矩阵：相机朝向目标位置
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    // 设置模型矩阵
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f)); // 缩放模型

    // 主渲染循环
    while (!glfwWindowShouldClose(window)) { // 循环直到窗口关闭

        // 处理输入事件
        handleInputEvents(window);

        // 检查是否需要进行细分
        if (subdivisionLevel >= 0 && static_cast<size_t>(subdivisionLevel) >= meshList.size() - 1) {
            // 获取当前网格并进行细分
            Mesh newMesh = LoopSubdivideNative(meshList[subdivisionLevel]);
            std::cout << "细分级别 " << subdivisionLevel + 1 << " 应用。" << std::endl;
            newMesh.setupMesh(); // 设置新的网格
            meshList.push_back(newMesh); // 将细分后的网格添加到网格列表中
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清空缓冲区

        // 使用着色器程序
        ourShader.use();

        // 设置投影矩阵和视图矩阵
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
                                                static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 
                                                0.1f, 100.0f);
        ourShader.setMat4("projection", projection); // 设置投影矩阵
        ourShader.setMat4("view", view);             // 设置视图矩阵
        ourShader.setMat4("model", model);           // 设置模型矩阵

        // 设置颜色等其他参数
        ourShader.setBool("sgColor", true);
        ourShader.setVec3("backColor", glm::vec3(1.0f, 1.0f, 1.0f)); // 设置背景色为白色
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // 设置为线框模式
        meshList[subdivisionLevel].draw(ourShader); // 绘制当前网格

        glfwSwapBuffers(window); // 交换缓冲区
        glfwPollEvents(); // 处理事件
    }

    // 终止GLFW并清理资源
    glfwTerminate();
    return 0;
}