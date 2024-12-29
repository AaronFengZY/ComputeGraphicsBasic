#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Shader/Shader.h"
#include "camera_class/camera.h"
#include "Mesh/Mesh.h"
#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>


#define STB_IMAGE_IMPLEMENTATION

using namespace std;

// 该函数声明用于加载单个纹理。
// 输入参数：纹理文件路径和纹理的环绕模式（如 GL_CLAMP_TO_EDGE 或 GL_REPEAT）
// 返回值：加载的纹理的ID。
unsigned int load_single_texture(char const * path, GLuint WRAP_MODE);

class TerrainEngine {
private:
    GLuint skyboxVAO, skyboxVBO;
    GLuint skyBox_Textures[5]; // 天空盒纹理数组
    GLuint water_Texture; // 水面纹理
    GLuint landTex, detailTex; // 地面纹理和细节纹理
    Shader skyShader, waterShader; // 天空盒和水面着色器

    float cloudSpeed = 0.01, waterSpeed = 0.3f, waterAlpha = 0.56f, waterScale = 0.3f; // 水面的相关参数

    Mesh landMesh; // 地形网格对象，需要实现zyMesh类，来处理地形的顶点和网格

public:
    Shader landShader; // 地形着色器

    // 常量：天空盒的顶点数量和属性步幅
    static const GLsizei skyBox_verts_num = 36; 
    static const GLsizei skyBox_attrib_stride = 5;
    static const float cubeVertices[skyBox_attrib_stride * skyBox_verts_num]; // 天空盒的顶点数据

    // 构造函数：初始化所有的着色器和纹理
    TerrainEngine(std::string skybox_vs, std::string skybox_fs, std::string water_vs, std::string water_fs,
                  std::string land_vs, std::string land_fs);

    // 加载所有纹理，包括天空盒、水面、地面、细节纹理和高度图
    void loadTextures(std::vector<std::string> skyboxFiles, std::string waterFile, 
                       std::string landFile, std::string detailFile, std::string heightMapFile);

    // 渲染天空盒
    void drawSkybox(glm::mat4 const &model, glm::mat4 const &view, glm::mat4 const &proj, float deltaTime);

    // 渲染水面
    void drawWater(const glm::mat4 &model, const glm::mat4 &view, const glm::mat4 &proj, const Camera &camera, float deltaTime);

    // 渲染地形
    void drawLand(glm::mat4 const &model, glm::mat4 const &view, glm::mat4 const &proj, bool isUp);

    // 处理窗口大小变化时的回调函数
    void framebufferSizeCallback(GLFWwindow* window, int width, int height);

    // 加载高度图并生成地形
    std::string loadHeightMap(std::string &hmapFile);
};


// 在 main.cpp 中定义 cubeVertices
const float TerrainEngine::cubeVertices[TerrainEngine::skyBox_attrib_stride * TerrainEngine::skyBox_verts_num] = {
    // 这些是天空盒的顶点数据，每个顶点包括位置和纹理坐标（5个元素）
    // 每个面包含2个三角形，每个三角形由3个顶点组成，共有6个面
    // 每个顶点有5个属性：位置（3个浮点数），纹理坐标（2个浮点数）
    // 所以每个顶点数据占5个浮点数，总共有36个顶点

    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,           // back
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,            // right
    0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,

    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,           // front
    0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,           // left
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,           // top
    0.5f,  0.5f, -0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,           // bottom: water
    0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
};


// 该函数加载指定文件的高度图，处理数据生成地形的顶点位置和索引，并将这些数据写入一个OBJ文件。
// 函数返回生成的OBJ文件路径。
std::string TerrainEngine::loadHeightMap(std::string &hmapFile) {
    float max_height = 255.0f;
    int width, height, nChannels;

    // 加载高度图文件
    unsigned char *raw_data_char = stbi_load(hmapFile.c_str(), &width, &height, &nChannels, 1);
    if (raw_data_char == NULL) {
        printf("Error: invalid heightmap!\n");
    }

    std::cout << "width: " << width << " height: " << height << std::endl;
    
    float max_x = (float)width, max_y = (float)height;
    
    std::vector<float> land_pos;
    std::vector<unsigned int> land_indices;
    std::string objName = "./resource/land.obj";
    
    FILE *fid = fopen(objName.c_str(), "w+");
    unsigned tri_cnt = 0;

    // 遍历高度图的每个像素，生成对应的顶点位置
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            land_pos.push_back((float)row / max_y);
            land_pos.push_back((float)((int)raw_data_char[row * width + col]) / max_height);  // 注意计算方式 row * width + col
            land_pos.push_back((float)col / max_x);
            fprintf(fid, "v %f %f %f\n", land_pos[3 * tri_cnt], land_pos[3 * tri_cnt + 1], land_pos[3 * tri_cnt + 2]);
            tri_cnt++;
        }
    }

    std::cout << "Height map loaded." << std::endl;

    // 生成地形的索引数据
    tri_cnt = 0;
    for (int row = 0; row < height - 1; row++) {
        for (int col = 0; col < width - 1; col++) {
            // 生成两个三角形，形成一个矩形
            land_indices.push_back(row * width + col);
            land_indices.push_back(row * width + col + 1);
            land_indices.push_back((row + 1) * width + col + 1);
            fprintf(fid, "f %u %u %u\n", land_indices[3 * tri_cnt] + 1, land_indices[3 * tri_cnt + 1] + 1, land_indices[3 * tri_cnt + 2] + 1);
            tri_cnt++;

            // 第二个三角形
            land_indices.push_back((row + 1) * width + col + 1);
            land_indices.push_back((row + 1) * width + col);
            land_indices.push_back(row * width + col);
            fprintf(fid, "f %u %u %u\n", land_indices[3 * tri_cnt] + 1, land_indices[3 * tri_cnt + 1] + 1, land_indices[3 * tri_cnt + 2] + 1);
            tri_cnt++;
        }
    }
    
    fclose(fid);

    std::cout << "Height map loaded." << std::endl;

    return objName;  // 返回生成的OBJ文件路径
}

// 该构造函数初始化天空盒、地形、和水面等相关着色器，并生成对应的VAO和VBO。
// 该过程会加载着色器程序和纹理，用于渲染不同的地形和水面效果。
TerrainEngine::TerrainEngine(std::string skybox_vs, std::string skybox_fs, std::string water_vs, std::string water_fs,
                             std::string land_vs, std::string land_fs) :
    skyBox_Textures{0}, skyShader(skybox_vs.c_str(), skybox_fs.c_str()), waterShader(water_vs.c_str(), water_fs.c_str()),
    landShader(land_vs.c_str(), land_fs.c_str()) {

    std::cout << "Shaders loaded." << std::endl;

    // Generate VAO, VBO for skybox
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    // 加载天空盒的顶点数据
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // 映射VBO并检查数据
    float* vboData = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
    if (vboData) {
        std::cout << "VBO Data: " << std::endl;
        for (int i = 0; i < 36 * skyBox_attrib_stride; ++i) { // 36个顶点
            std::cout << vboData[i] << " ";
            if ((i + 1) % 5 == 0) std::cout << std::endl; // 每行显示5个数
        }

        glUnmapBuffer(GL_ARRAY_BUFFER);
    } else {
        std::cerr << "Failed to map VBO for reading!" << std::endl;
    }

    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, skyBox_attrib_stride * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1); // texture coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, skyBox_attrib_stride * sizeof(float), (void*)(3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    std::cout << "Skybox VAO and VBO set up." << std::endl;
}

void TerrainEngine::loadTextures(std::vector<std::string> skyboxFiles, std::string waterFile,
                                  std::string landFile, std::string detailFile, std::string heightMapFile) {
    // 确保天空盒文件路径数量为5个
    assert(skyboxFiles.size() == 5);
    for (unsigned i = 0; i < skyboxFiles.size(); i++) {
        std::cout << "Loading skybox texture: " << skyboxFiles[i].c_str() << std::endl;
        skyBox_Textures[i] = load_single_texture(skyboxFiles[i].c_str(), GL_CLAMP_TO_EDGE);
        assert(skyBox_Textures[i] != 0);
        if (skyBox_Textures[i] == 0) {
            std::cerr << "Failed to load texture: " << skyboxFiles[i] << std::endl;
        } else {
            std::cout << "Loaded texture: " << skyboxFiles[i] << " with ID: " << skyBox_Textures[i] << std::endl;
        }
    }

    // 加载水面纹理
    water_Texture = load_single_texture(waterFile.c_str(), GL_REPEAT);
    assert(water_Texture != 0);

    // 加载地面和细节纹理
    landTex = load_single_texture(landFile.c_str(), GL_CLAMP_TO_EDGE);
    detailTex = load_single_texture(detailFile.c_str(), GL_CLAMP_TO_EDGE);
    assert(landTex != 0 && detailTex != 0);

    std::cout << "before load_height_map" << std::endl;

    // 获取并处理高度图，生成地形网格数据
    landMesh = Mesh(loadHeightMap(heightMapFile), false, false);  // 不设置纹理和法线

    std::cout << "landMesh.vertexList.size() = " << landMesh.vertices.size() << std::endl;

    // 计算每个顶点的纹理坐标
    for (size_t iv = 0; iv < landMesh.vertices.size(); iv++) {
        // 根据地形的坐标计算纹理坐标
        landMesh.vertices[iv].texCoords.x = landMesh.vertices[iv].position.z;
        landMesh.vertices[iv].texCoords.y = landMesh.vertices[iv].position.x;
    }

    // 完成地形网格的设置
    landMesh.setupMesh();
}

// 该函数用于加载一个单一的纹理文件，并返回纹理ID。
// 纹理参数指定纹理的环绕模式（如重复或夹紧）。
unsigned int load_single_texture(char const * path, GLuint WRAP_MODE) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);

    if (data) {
        std::cout << "Texture loaded: " << path << ", width: " << width << ", height: " << height << ", components: " << nrComponents << std::endl;
    } else {
        std::cout << "Texture failed to load: " << path << std::endl;
    }

    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WRAP_MODE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WRAP_MODE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void TerrainEngine::drawSkybox(glm::mat4 const & model, glm::mat4 const & view, glm::mat4 const & proj, float deltaTime) {
    static float x_shift = 0, y_shift = 0;
    x_shift += deltaTime;  // 更新x轴云层偏移
    y_shift += deltaTime;  // 更新y轴云层偏移
    glm::vec3 transVec = glm::vec3(cloudSpeed) * glm::vec3(cos(x_shift), 0.0, cos(y_shift));  // 计算偏移向量

    skyShader.use();
    skyShader.setMat4("model", glm::translate(model, transVec));  // 设置模型矩阵
    skyShader.setMat4("view", view);  // 设置视图矩阵
    skyShader.setMat4("projection", proj);  // 设置投影矩阵
    skyShader.setInt("texture", 0);  // 设置纹理单元

    glBindVertexArray(skyboxVAO);  // 绑定Skybox VAO
    for (unsigned i = 0; i < 5; i++) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, skyBox_Textures[i]);  // 绑定天空盒纹理
        if (skyBox_Textures[i] == 0) {
            std::cerr << "Error: Texture ID for face " << i << " is invalid." << std::endl;
        }
        glDrawArrays(GL_TRIANGLES, i * 6, 6);  // 绘制每个面
        glBindTexture(GL_TEXTURE_2D, 0);  // 解绑纹理
    }
    glBindVertexArray(0);  // 解绑VAO
}

void TerrainEngine::drawWater(glm::mat4 const &model, glm::mat4 const &view, glm::mat4 const &proj, Camera const &camera, float deltaTime) {
    const static glm::mat4 mirror_y({
        {1, 0, 0, 0},
        {0, -1, 0, 0},  // y轴翻转
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    });

    // 计算镜像模型
    const static glm::mat4 mirror_y_skybox_model = mirror_y * model;
    const static glm::mat4 mirror_y_land_model = mirror_y * model;

    // 启用深度测试
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);  // 默认深度测试模式

    // 绘制天空盒（镜像）
    drawSkybox(mirror_y_skybox_model, view, proj, deltaTime);

    // 绘制地面（镜像）
    drawLand(mirror_y_land_model, view, proj, false);

    // 动态水面效果
    static float x_shift = 0, y_shift = 0;
    x_shift += deltaTime * waterSpeed;  // 更新水面x轴位移
    y_shift += deltaTime * waterSpeed * 0.8f;  // 更新水面y轴位移

    // 开启混合模式绘制水面
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // 设置混合模式
    glDepthMask(GL_FALSE);  // 禁用深度写入

    glm::mat4 water_model = model;
    water_model[3][1] = 0.04 * 50.0f;  // 设置水面Y坐标

    waterShader.use();
    waterShader.setMat4("view", view);  // 设置视图矩阵
    waterShader.setMat4("projection", proj);  // 设置投影矩阵
    waterShader.setMat4("model", water_model);  // 设置水面模型矩阵
    waterShader.setFloat("texture_scale", 10.0f);  // 设置纹理缩放
    waterShader.setFloat("xShift", waterScale * sin(x_shift));  // 设置水面x轴偏移
    waterShader.setFloat("yShift", waterScale * sin(y_shift));  // 设置水面y轴偏移
    waterShader.setFloat("water_alpha", waterAlpha);  // 设置水面透明度
    waterShader.setInt("texture", 0);  // 设置纹理单元

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, water_Texture);  // 绑定水面纹理

    glBindVertexArray(skyboxVAO);  // 使用skyBox的VAO来绘制水面
    glDrawArrays(GL_TRIANGLES, 5 * 6, 6);  // 绘制水面
    glBindVertexArray(0);

    // 关闭混合模式并恢复深度写入
    glDepthMask(GL_TRUE);  // 启用深度写入
    glDisable(GL_BLEND);
}

void TerrainEngine::drawLand(glm::mat4 const & model, glm::mat4 const & view, glm::mat4 const & proj, bool isUp) {
    landShader.use();
    landShader.setFloat("heightScale", 0.05f);  // 设置地面高度缩放
    landShader.setMat4("model", model);  // 设置模型矩阵
    landShader.setMat4("view", view);  // 设置视图矩阵
    landShader.setMat4("projection", proj);  // 设置投影矩阵
    
    glBindVertexArray(landMesh.VAO);  // 绑定地面VAO

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, landTex);  // 绑定地面纹理
    landShader.setInt("land_Texture", 0);  // 设置地面纹理单元
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, detailTex);  // 绑定细节纹理
    landShader.setInt("detail_Texture", 1);  // 设置细节纹理单元
    landShader.setFloat("detail_scale", 30.0);  // 设置细节纹理的缩放
    landShader.setBool("isUp", isUp);  // 设置是否是上面
    if (isUp) {
        landShader.setFloat("offset", -0.48f);  // 上面偏移
    } else {
        landShader.setFloat("offset", 0.44f);  // 下面偏移
    }
    
    glDrawElements(GL_TRIANGLES, landMesh.indices.size(), GL_UNSIGNED_INT, 0);  // 绘制地面网格
    glBindVertexArray(0);  // 解绑VAO
}

// 窗口大小变化时的回调函数
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    std::cout << "Window resized to " << width << "x" << height << std::endl;
    glViewport(0, 0, width, height);  // 更新OpenGL视口大小
}

// 初始化相机
Camera camera(glm::vec3(5.0f, -15.0f, 5.0f));  // 设置相机初始位置

// 初始化鼠标位置
double lastX = 400.0f;  // 初始鼠标X坐标
double lastY = 300.0f;  // 初始鼠标Y坐标
bool firstMouse = true;  // 判断是否是第一次接收鼠标移动事件

// 鼠标移动回调函数
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;  // 记录初始鼠标位置
        lastY = ypos;
        firstMouse = false;
    }

    // 计算鼠标偏移量
    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos;  // y轴反向
    lastX = xpos;
    lastY = ypos;

    // 处理鼠标移动
    camera.ProcessMouseMovement(xOffset, yOffset);
}

// 设置时间追踪变量
float deltaTime = 0.0f;  
float lastFrame = 0.0f; 

// 键盘输入回调函数
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    // 切换鼠标游标模式
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        int cursorMode = glfwGetInputMode(window, GLFW_CURSOR);
        glfwSetInputMode(window, GLFW_CURSOR, (cursorMode == GLFW_CURSOR_DISABLED) ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }

    // 控制相机移动
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_W) camera.ProcessKeyboard(FORWARD, deltaTime);
        if (key == GLFW_KEY_S) camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (key == GLFW_KEY_A) camera.ProcessKeyboard(LEFT, deltaTime);
        if (key == GLFW_KEY_D) camera.ProcessKeyboard(RIGHT, deltaTime);
        if (key == GLFW_KEY_SPACE) camera.ProcessKeyboard(UP, deltaTime);
        if (key == GLFW_KEY_LEFT_SHIFT) camera.ProcessKeyboard(DOWN, deltaTime);
    }
}

// 回调函数：处理鼠标滚轮
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll((float)yoffset);
}

int main() {

    // 初始化GLFW
    if (!glfwInit()) {
        std::cerr << "初始化GLFW失败！" << std::endl;
        return -1;
    }

    // 设置纹理加载时翻转
    stbi_set_flip_vertically_on_load(true);

    // 设置OpenGL版本（4.6）
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建GLFW窗口和OpenGL上下文
    GLFWwindow* window = glfwCreateWindow(800, 600, "Terrain Engine", nullptr, nullptr);
    if (!window) {
        std::cerr << "创建GLFW窗口失败！" << std::endl;
        glfwTerminate();
        return -1;
    }

    // 设置OpenGL上下文为当前
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // 注册鼠标回调函数
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 注册键盘回调函数
    glfwSetKeyCallback(window, key_callback);
    // 隐藏光标
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 使用GLAD加载OpenGL函数指针
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "初始化GLAD失败！" << std::endl;
        return -1;
    }

    // 启用OpenGL设置
    glEnable(GL_DEPTH_TEST); // 启用深度测试
    glEnable(GL_CULL_FACE);  // 启用面剔除（我们剔除背面）
    glCullFace(GL_BACK);     // 剔除背面（默认）
    glLineWidth(1.0f);       // 设置线条宽度
    glPointSize(2.0f);       // 设置点大小

    std::cout << "当前路径: " << std::filesystem::current_path() << std::endl;

    // 创建TerrainEngine对象
    TerrainEngine engine(
        "./include/Shader/vs/skybox.vs",  // skybox顶点着色器
        "./include/Shader/fs/skybox.fs",  // skybox片段着色器
        "./include/Shader/vs/water.vs",  // 水面顶点着色器
        "./include/Shader/fs/water.fs",  // 水面片段着色器
        "./include/Shader/vs/land.vs",   // 地面顶点着色器
        "./include/Shader/fs/land.fs"    // 地面片段着色器
    );

    std::cout << "TerrainEngine着色器创建成功！" << std::endl;

    // 加载纹理
    std::vector<std::string> skyboxFiles = {
        "./data/SkyBox/SkyBox0.bmp", "./data/SkyBox/SkyBox1.bmp",
        "./data/SkyBox/SkyBox2.bmp", "./data/SkyBox/SkyBox3.bmp",
        "./data/SkyBox/SkyBox4.bmp"
    };
    engine.loadTextures(skyboxFiles, "./data/SkyBox/SkyBox5.bmp", 
                         "./data/terrain-texture3.bmp", "./data/detail.bmp", "./data/heightmap.bmp");

    std::cout << "纹理加载成功！" << std::endl;

    glm::vec3 lightPos(0.2f, 0.2f, 0.2f);  // 设置光源位置

    // 设置目标FPS
    const float targetFPS = 30.0f;
    const float targetDeltaTime = 1.0f / targetFPS; // 每帧的时间

    // 禁用面剔除，避免在渲染过程中遮挡
    glDisable(GL_CULL_FACE);     // 禁用面剔除


    std::cout << "键盘控制：" << std::endl;
    std::cout << "W - 向前移动" << std::endl;
    std::cout << "S - 向后移动" << std::endl;
    std::cout << "A - 向左移动" << std::endl;
    std::cout << "D - 向右移动" << std::endl;
    std::cout << "空格键 - 向上移动" << std::endl;
    std::cout << "左Shift - 向下移动" << std::endl;
    std::cout << "P - 切换光标模式" << std::endl;


    // 主渲染循环
    while (!glfwWindowShouldClose(window)) {
        // 计算时间差
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 延迟以保持目标FPS
        if (deltaTime < targetDeltaTime) {
            std::this_thread::sleep_for(std::chrono::duration<float>(targetDeltaTime - deltaTime));
            deltaTime = targetDeltaTime;
        }

        // 处理事件
        glfwPollEvents();

        // 清空缓冲区
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 计算矩阵
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(100.0f), 800.0f / 600.0f, 0.1f, 10000.0f);

        // 设置地面相关着色器参数
        model = glm::scale(model, glm::vec3(50.0f));
        engine.landShader.setVec3("lightPos", lightPos);
        engine.landShader.setVec3("viewPos", camera.Position);

        // 绘制地面
        engine.drawLand(model, view, projection, true);

        // 绘制水面
        engine.drawWater(model, view, projection, camera, deltaTime);

        // 绘制天空盒
        glDepthFunc(GL_ALWAYS); // 禁用深度测试
        engine.drawSkybox(model, view, projection, deltaTime);
        glDepthFunc(GL_LESS);   // 恢复深度测试

        // 交换缓冲区
        glfwSwapBuffers(window);
    }

    // 清理并终止GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}