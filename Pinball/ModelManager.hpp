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
struct MaterialData {
    float r, g, b, a;
};
class ModelManager {
private:
    std::vector<MeshEntry> staticMeshes; 
    std::vector<MeshEntry> flipperLMeshes; 
    std::vector<MeshEntry> flipperRMeshes; 
    std::vector<MaterialData> materialColors;
    aiVector3D sceneCenter;
    float sceneScale;

    void processNode(aiNode* node, const aiScene* scene);
    MeshEntry processMesh(aiMesh* mesh, const aiScene* scene);
    void extractGeometry(const std::vector<MeshEntry>& meshes, 
        std::vector<float>& outVertices, std::vector<uint32_t>& outIndices);


public:
    ModelManager();
    bool loadModel(const std::string& path);
    void drawFlipperL();
    void drawFlipperR();
    void drawTable();
    void drawMeshList(const std::vector<MeshEntry>& meshList);
    void getGeometryData(std::vector<float>& outVertices, std::vector<uint32_t>& outIndices);
    void getFlipperLGeometry(std::vector<float>& outVertices, std::vector<uint32_t>& outIndices);
    void getFlipperRGeometry(std::vector<float>& outVertices, std::vector<uint32_t>& outIndices);
};