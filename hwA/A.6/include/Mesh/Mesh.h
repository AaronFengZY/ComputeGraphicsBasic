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
    // ????
    std::vector<Vertex> vertices;
    std::vector<VertexElement> vertexElements;
    std::vector<Edge> edges;
    std::vector<EdgeElement> edgeElements;
    std::vector<Face> faces;
    std::vector<FaceElement> faceElements;
    std::vector<HalfEdge> halfEdges;
    std::vector<unsigned int> indices;

    // OpenGL??
    unsigned int VAO;
    unsigned int VBO, EBO;

    // ????
    Mesh() {
        clearData();
    }

    Mesh(const std::string& filename) {
        clearData();
        loadFromFile(filename);
        setupMesh();
    }

    Mesh(std::string filename, bool _need_to_calc_normal = false, bool _setup_mesh = true) {
        clearData();

        // textures.clear(); // ???? textures ?????????????

        loadFromFile(filename); // ?????????
        if (_need_to_calc_normal)
            calcNormal();
        
        if (_setup_mesh)
            setupMesh();
    }
    

    // ??????
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

    // ????
    size_t addVertex(const Vec3& position) {
        Vertex vertex;
        vertex.position = position;
        size_t vertexId = vertexElements.size();
        vertices.push_back(vertex);
        vertexElements.emplace_back(vertexId);
        return vertexId;
    }

    // ?????????????????????
    size_t addFace(const std::vector<size_t>& vertexIds) {
        size_t numVertices = vertexIds.size();
        if (numVertices < 3) {
            std::cerr << "Error: Face has less than 3 vertices!" << std::endl;
            exit(EXIT_FAILURE);
        }

        size_t faceId = faceElements.size();
        size_t startHalfEdgeId = halfEdges.size();

        // ????????
        for (size_t i = 0; i < numVertices; ++i) {
            HalfEdge he;
            size_t currentHalfEdgeId = halfEdges.size();
            he.id = currentHalfEdgeId;
            he.fromVertexId = vertexIds[i];
            he.toVertexId = vertexIds[(i + 1) % numVertices];

            // ?????????
            for (size_t oppoHeId : vertexElements[he.fromVertexId].incomingHalfEdgeIds) {
                const HalfEdge& oppoHe = halfEdges[oppoHeId];
                if (oppoHe.fromVertexId == he.toVertexId) {
                    he.oppositeHalfEdgeId = oppoHeId;
                    halfEdges[oppoHeId].oppositeHalfEdgeId = currentHalfEdgeId;
                    break;
                }
            }

            // ?????????
            vertexElements[he.fromVertexId].outgoingHalfEdgeIds.push_back(currentHalfEdgeId);
            vertexElements[he.toVertexId].incomingHalfEdgeIds.push_back(currentHalfEdgeId);
            he.faceId = faceId;
            he.prevHalfEdgeId = (i == 0) ? (currentHalfEdgeId - 1 + numVertices) : (currentHalfEdgeId - 1);
            he.nextHalfEdgeId = (i == numVertices - 1) ? (currentHalfEdgeId + 1 - numVertices) : (currentHalfEdgeId + 1);

            // ???
            if (he.oppositeHalfEdgeId != INVALID_INDEX) {
                he.edgeId = halfEdges[he.oppositeHalfEdgeId].edgeId;
                edgeElements[he.edgeId].halfEdge2Id = currentHalfEdgeId;
            } else {
                he.edgeId = edges.size();
                EdgeElement newEdge(currentHalfEdgeId);
                newEdge.halfEdge1FromVertexId = he.fromVertexId;
                newEdge.halfEdge1ToVertexId = he.toVertexId;
                edgeElements.emplace_back(newEdge);
                edges.emplace_back(); // ???????
            }

            halfEdges.emplace_back(he);
            indices.push_back(vertexIds[i]);
        }

        // ???
        faceElements.emplace_back(startHalfEdgeId, vertexIds);
        faces.emplace_back(); // ???????

        return startHalfEdgeId;
    }

    // ??OpenGL??????
    void setupMesh() {
        // ??????????
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // ??????
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        // ??????
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // ????????
        // ??
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // ??
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        // ????
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

        glBindVertexArray(0);
    }

    // ????
    void draw(Shader& shader) const {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0); // ???????
        glActiveTexture(GL_TEXTURE0); // ????????
    }

        // ??????????
    void calcNormal() {
        // ?????????
        for (size_t f = 0; f < faceElements.size(); f++) {
            const FaceElement& face = faceElements[f];
            size_t start_halfedge_id = face.startHalfEdgeId;
            size_t halfedge_id = start_halfedge_id;

            // ???????
            size_t vids[3];
            for (int i = 0; i < 3; ++i) {
                const HalfEdge& he = halfEdges[halfedge_id];
                vids[i] = he.fromVertexId;
                halfedge_id = he.nextHalfEdgeId;
            }

            Vec3 v0 = vertices[vids[0]].position;
            Vec3 v1 = vertices[vids[1]].position;
            Vec3 v2 = vertices[vids[2]].position;

            Vec3 edge1 = v1 - v0;
            Vec3 edge2 = v2 - v0;
            Vec3 normal = glm::normalize(glm::cross(edge1, edge2));

            // ???????
            faces[f].normal = glm::vec4(normal, 0.0f); // ?? w ?????
        }

        // ??????????
        for (size_t vh = 0; vh < vertexElements.size(); vh++) {
            Vec3 norm_wArea(0.0f);

            // ????????
            for (size_t he_id : vertexElements[vh].incomingHalfEdgeIds) {
                const HalfEdge& he = halfEdges[he_id];
                size_t face_id = he.faceId;
                if (face_id == INVALID_INDEX) continue; // ???????

                // ?????????
                Vec3 face_normal = glm::vec3(faces[face_id].normal);

                // ??????
                // ???????
                size_t prev_he_id = halfEdges[he.prevHalfEdgeId].oppositeHalfEdgeId;
                if (prev_he_id == INVALID_INDEX) continue;

                const HalfEdge& prev_he = halfEdges[prev_he_id];
                Vec3 v0 = vertices[vh].position;
                Vec3 v1 = vertices[prev_he.fromVertexId].position;
                Vec3 v2 = vertices[he.fromVertexId].position;

                Vec3 edge1 = glm::normalize(v1 - v0);
                Vec3 edge2 = glm::normalize(v2 - v0);
                float angle = acos(glm::clamp(glm::dot(edge1, edge2), -1.0f, 1.0f));

                norm_wArea += face_normal * angle;
            }

            // ?????
            vertices[vh].normal = glm::normalize(norm_wArea);
        }
    }

private:
    // ???????
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
            if (line.empty()) continue; // ????

            std::istringstream stream(line);
            char prefix;
            stream >> prefix;

            if (prefix == 'v') { // ??
                stream >> vertexPosition.x >> vertexPosition.y >> vertexPosition.z;
                addVertex(vertexPosition);
            } else if (prefix == 'f') { // ?
                stream >> faceVertexIndices[0] >> faceVertexIndices[1] >> faceVertexIndices[2];
                // ???0???
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

// ??HalfEdge?????
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


