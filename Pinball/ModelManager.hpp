#pragma once

#include <vector>
#include <string>
#include <assimp/scene.h> 
struct Color {
    float r, g, b;
};

struct MeshEntry {
    std::vector<aiVector3D> vertices;
    std::vector<aiVector3D> normals;
    std::vector<unsigned int> indices;
    unsigned int materialIndex;
};

class ModelManager {
private:
    std::vector<MeshEntry> meshes;
    std::vector<Color> materialColors;
    aiVector3D sceneCenter;
    float sceneScale;

    void processNode(aiNode* node, const aiScene* scene);
    MeshEntry processMesh(aiMesh* mesh, const aiScene* scene);

public:
    ModelManager();
    bool loadModel(const std::string& path);
    void draw();
    void getGeometryData(std::vector<float>& outVertices, std::vector<uint32_t>& outIndices);
};