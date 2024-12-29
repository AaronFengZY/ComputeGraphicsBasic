#ifndef MESH_H
#define MESH_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>
#include <unordered_set>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shader/shader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

typedef glm::mat4x4 Mat4;
typedef glm::vec3 Vec3;
typedef glm::vec4 Vec4;
typedef glm::vec2 Vec2;

struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec2 texCoords;
    Mat4 quadric;
    
    Vertex() : position(0.0f), normal(0.0f), texCoords(0.0f), quadric(0.0f) {}
};

struct Face {
    Vec4 normal;
    
    Face() : normal(0.0f) {}
};

struct Edge {
    float cost;
    Mat4 QBar;
    Vec4 vBar;
    
    Edge() : cost(0.0f), QBar(0.0f), vBar(0.0f) {}
};

constexpr size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

class VertexElement {
public:
    size_t id;
    size_t smpfId;
    std::vector<size_t> outgoingHalfEdgeIds;
    std::vector<size_t> incomingHalfEdgeIds;
    bool deleted;

    VertexElement() : id(0), smpfId(INVALID_INDEX), deleted(false) {}
    VertexElement(size_t vertexId) : id(vertexId), smpfId(INVALID_INDEX), deleted(false) {}
};

class HalfEdge {
public:
    size_t id;
    size_t edgeId;
    size_t prevHalfEdgeId;
    size_t nextHalfEdgeId;
    size_t oppositeHalfEdgeId;
    size_t toVertexId;
    size_t fromVertexId;
    size_t faceId;
    bool deleted;

    HalfEdge() 
        : id(INVALID_INDEX), edgeId(INVALID_INDEX), prevHalfEdgeId(INVALID_INDEX),
          nextHalfEdgeId(INVALID_INDEX), oppositeHalfEdgeId(INVALID_INDEX),
          toVertexId(INVALID_INDEX), fromVertexId(INVALID_INDEX),
          faceId(INVALID_INDEX), deleted(false) {}
    
    bool operator==(const HalfEdge& other) const;
    bool operator!=(const HalfEdge& other) const { return !(*this == other); }
};

class EdgeElement {
public:
    size_t halfEdge1Id;
    size_t halfEdge2Id;
    size_t halfEdge1ToVertexId;
    size_t halfEdge1FromVertexId;
    bool deleted;

    EdgeElement() 
        : halfEdge1Id(INVALID_INDEX), halfEdge2Id(INVALID_INDEX),
          halfEdge1ToVertexId(INVALID_INDEX), halfEdge1FromVertexId(INVALID_INDEX),
          deleted(false) {}
    
    EdgeElement(size_t he1Id) 
        : halfEdge1Id(he1Id), halfEdge2Id(INVALID_INDEX),
          halfEdge1ToVertexId(INVALID_INDEX), halfEdge1FromVertexId(INVALID_INDEX),
          deleted(false) {}
};

class FaceElement {
public:
    size_t startHalfEdgeId;
    std::vector<size_t> vertexIds;
    bool deleted;

    FaceElement() : startHalfEdgeId(INVALID_INDEX), deleted(false) {}
    FaceElement(size_t startHeId, const std::vector<size_t>& vids) 
        : startHalfEdgeId(startHeId), vertexIds(vids), deleted(false) {}
};

class Mesh {
public:
    // 数据结构
    std::vector<Vertex> vertices;
    std::vector<VertexElement> vertexElements;
    std::vector<Edge> edges;
    std::vector<EdgeElement> edgeElements;
    std::vector<Face> faces;
    std::vector<FaceElement> faceElements;
    std::vector<HalfEdge> halfEdges;
    std::vector<unsigned int> indices;

    // OpenGL相关
    unsigned int VAO;
    unsigned int VBO, EBO;

    // 构造函数
    Mesh() {
        clearData();
    }

    Mesh(const std::string& filename) {
        clearData();
        loadFromFile(filename);
        setupMesh();
    }

    // 清空所有数据
    void clearData() {
        vertices.clear();
        vertexElements.clear();
        edges.clear();
        edgeElements.clear();
        faces.clear();
        faceElements.clear();
        halfEdges.clear();
        indices.clear();
    }

    // 添加顶点
    size_t addVertex(const Vec3& position) {
        Vertex vertex;
        vertex.position = position;
        size_t vertexId = vertexElements.size();
        vertices.push_back(vertex);
        vertexElements.emplace_back(vertexId);
        return vertexId;
    }

    // 添加面（确保顶点序列为逆时针且法向量朝外）
    size_t addFace(const std::vector<size_t>& vertexIds) {
        size_t numVertices = vertexIds.size();
        if (numVertices < 3) {
            std::cerr << "Error: Face has less than 3 vertices!" << std::endl;
            exit(EXIT_FAILURE);
        }

        size_t faceId = faceElements.size();
        size_t startHalfEdgeId = halfEdges.size();

        // 为每个边创建半边
        for (size_t i = 0; i < numVertices; ++i) {
            HalfEdge he;
            size_t currentHalfEdgeId = halfEdges.size();
            he.id = currentHalfEdgeId;
            he.fromVertexId = vertexIds[i];
            he.toVertexId = vertexIds[(i + 1) % numVertices];

            // 查找对应的对向半边
            for (size_t oppoHeId : vertexElements[he.fromVertexId].incomingHalfEdgeIds) {
                const HalfEdge& oppoHe = halfEdges[oppoHeId];
                if (oppoHe.fromVertexId == he.toVertexId) {
                    he.oppositeHalfEdgeId = oppoHeId;
                    halfEdges[oppoHeId].oppositeHalfEdgeId = currentHalfEdgeId;
                    break;
                }
            }

            // 更新顶点的半边列表
            vertexElements[he.fromVertexId].outgoingHalfEdgeIds.push_back(currentHalfEdgeId);
            vertexElements[he.toVertexId].incomingHalfEdgeIds.push_back(currentHalfEdgeId);
            he.faceId = faceId;
            he.prevHalfEdgeId = (i == 0) ? (currentHalfEdgeId - 1 + numVertices) : (currentHalfEdgeId - 1);
            he.nextHalfEdgeId = (i == numVertices - 1) ? (currentHalfEdgeId + 1 - numVertices) : (currentHalfEdgeId + 1);

            // 关联边
            if (he.oppositeHalfEdgeId != INVALID_INDEX) {
                he.edgeId = halfEdges[he.oppositeHalfEdgeId].edgeId;
                edgeElements[he.edgeId].halfEdge2Id = currentHalfEdgeId;
            } else {
                he.edgeId = edges.size();
                EdgeElement newEdge(currentHalfEdgeId);
                newEdge.halfEdge1FromVertexId = he.fromVertexId;
                newEdge.halfEdge1ToVertexId = he.toVertexId;
                edgeElements.emplace_back(newEdge);
                edges.emplace_back(); // 添加空的边数据
            }

            halfEdges.emplace_back(he);
            indices.push_back(vertexIds[i]);
        }

        // 添加面
        faceElements.emplace_back(startHalfEdgeId, vertexIds);
        faces.emplace_back(); // 添加空的面数据

        return startHalfEdgeId;
    }

    // 设置OpenGL相关的缓冲区
    void setupMesh() {
        // 生成缓冲区和数组对象
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // 加载顶点数据
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        // 加载索引数据
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // 设置顶点属性指针
        // 位置
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // 法线
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        // 纹理坐标
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

        glBindVertexArray(0);
    }

    // 绘制网格
    void draw(Shader& shader) const {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0); // 解绑以防止误用
        glActiveTexture(GL_TEXTURE0); // 恢复默认纹理单元
    }

private:
    // 从文件加载数据
    void loadFromFile(const std::string& filename) {
        std::ifstream fin(filename);
        if (!fin.is_open()) {
            std::cerr << "Error: Failed to open file " << filename << std::endl;
            return;
        }

        std::string line;
        Vec3 vertexPosition;
        std::vector<size_t> faceVertexIndices(3, INVALID_INDEX);

        while (std::getline(fin, line)) {
            if (line.empty()) continue; // 跳过空行

            std::istringstream stream(line);
            char prefix;
            stream >> prefix;

            if (prefix == 'v') { // 顶点
                stream >> vertexPosition.x >> vertexPosition.y >> vertexPosition.z;
                addVertex(vertexPosition);
            } else if (prefix == 'f') { // 面
                stream >> faceVertexIndices[0] >> faceVertexIndices[1] >> faceVertexIndices[2];
                // 转换为0基索引
                for (size_t& idx : faceVertexIndices) {
                    if (idx == 0) {
                        std::cerr << "Error: Vertex indices in OBJ files should start from 1." << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    --idx;
                }
                addFace(faceVertexIndices);
            }
        }

        fin.close();
    }
};

// 实现HalfEdge的比较操作
bool HalfEdge::operator==(const HalfEdge& other) const {
    return id == other.id &&
           edgeId == other.edgeId &&
           prevHalfEdgeId == other.prevHalfEdgeId &&
           nextHalfEdgeId == other.nextHalfEdgeId &&
           oppositeHalfEdgeId == other.oppositeHalfEdgeId &&
           toVertexId == other.toVertexId &&
           fromVertexId == other.fromVertexId &&
           faceId == other.faceId &&
           deleted == other.deleted;
}

#endif // MESH_H
