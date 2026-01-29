#include "ModelManager.hpp"

#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <GL/freeglut.h>

ModelManager::ModelManager() : sceneScale(1.0f) {}

void ModelManager::processNode(aiNode* node, const aiScene* scene) {
    std::string nodeName = node->mName.C_Str();

    std::vector<MeshEntry>* targetMeshList = &staticMeshes;

    if (nodeName.find("Flipper.L") != std::string::npos) {
        targetMeshList = &flipperLMeshes;
    }
    else if (nodeName.find("Flipper.R") != std::string::npos) {
        targetMeshList = &flipperRMeshes;
    }

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        targetMeshList->push_back(processMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

MeshEntry ModelManager::processMesh(aiMesh* mesh, const aiScene* scene) {
    MeshEntry newMesh;
    newMesh.materialIndex = mesh->mMaterialIndex;

    // pobierz wierzcholki i normalne
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        newMesh.vertices.push_back(mesh->mVertices[i]);

        if (mesh->HasNormals()) {
            newMesh.normals.push_back(mesh->mNormals[i]);
        }
        else {
            newMesh.normals.push_back(aiVector3D(0.0f, 1.0f, 0.0f));
        }
    }

    // pobierz sciany
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            newMesh.indices.push_back(face.mIndices[j]);
        }
    }

    return newMesh;
}

bool ModelManager::loadModel(const std::string& path) {
    Assimp::Importer importer;

	// flagi: triangulate - zamieñ na trójk¹ty, flip UVs - odwróæ wspó³rzêdne UV, gen smooth normals - wyg³adŸ normalne, join identical vertices - po³¹cz identyczne wierzcho³ki
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "B³¹d Assimp: " << importer.GetErrorString() << std::endl;
        return false;
    }

    materialColors.clear();
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* material = scene->mMaterials[i];
        aiColor3D color(0.8f, 0.8f, 0.8f);
        float opacity = 1.0f;

        material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        material->Get(AI_MATKEY_OPACITY, opacity);

        materialColors.push_back({ color.r, color.g, color.b, opacity });
    }
    staticMeshes.clear();
    flipperLMeshes.clear();
    flipperRMeshes.clear();
    processNode(scene->mRootNode, scene);

    return true;
}

void ModelManager::drawMeshList(const std::vector<MeshEntry>& meshList) {
    for (const auto& mesh : meshList) {
        if (mesh.materialIndex < materialColors.size()) {
            const auto& mat = materialColors[mesh.materialIndex];
            glColor4f(mat.r, mat.g, mat.b, mat.a);
        }
        else {
            glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
        }

        glBegin(GL_TRIANGLES);
        for (unsigned int i = 0; i < mesh.indices.size(); i++) {
            unsigned int index = mesh.indices[i];
            if (!mesh.normals.empty()) {
                glNormal3f(mesh.normals[index].x, mesh.normals[index].y, mesh.normals[index].z);
            }
            glVertex3f(mesh.vertices[index].x, mesh.vertices[index].y, mesh.vertices[index].z);
        }
        glEnd();
    }
}

void ModelManager::drawFlipperL() {
    drawMeshList(flipperLMeshes);
}

void ModelManager::drawFlipperR() {
    drawMeshList(flipperRMeshes);
}

void ModelManager::drawTable() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawMeshList(staticMeshes);
    glDisable(GL_BLEND);

}

void ModelManager::extractGeometry(const std::vector<MeshEntry>& meshes, std::vector<float>& outVertices, std::vector<uint32_t>& outIndices) {
    outVertices.clear();
    outIndices.clear();
    uint32_t baseVertex = 0;

    for (const auto& mesh : meshes) {
        for (const auto& vertex : mesh.vertices) {
            outVertices.push_back(vertex.x);
            outVertices.push_back(vertex.y);
            outVertices.push_back(vertex.z);
        }
        for (unsigned int index : mesh.indices) {
            outIndices.push_back(baseVertex + index);
        }
        baseVertex += mesh.vertices.size();
    }
}

void ModelManager::getGeometryData(std::vector<float>& outVertices, std::vector<uint32_t>& outIndices) {
    extractGeometry(staticMeshes, outVertices, outIndices);
}

void ModelManager::getFlipperLGeometry(std::vector<float>& outVertices, std::vector<uint32_t>& outIndices) {
    extractGeometry(flipperLMeshes, outVertices, outIndices);
}

void ModelManager::getFlipperRGeometry(std::vector<float>& outVertices, std::vector<uint32_t>& outIndices) {
    extractGeometry(flipperRMeshes, outVertices, outIndices);
}