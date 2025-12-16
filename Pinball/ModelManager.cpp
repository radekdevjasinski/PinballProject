#include "ModelManager.hpp"

#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <GL/freeglut.h>

ModelManager::ModelManager() : sceneScale(1.0f) {}

void ModelManager::processNode(aiNode* node, const aiScene* scene) {
    // przetworz wszystkie siatki w aktualnym wezle
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    // rekurencyjnie przetwórz dzieci wêz³a
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

        if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
            materialColors.push_back({ color.r, color.g, color.b });
        }
        else {
            materialColors.push_back({ 0.8f, 0.8f, 0.8f });
        }
    }
    meshes.clear();
    processNode(scene->mRootNode, scene);

    return true;
}

void ModelManager::draw() {
    for (const auto& mesh : meshes) {
        if (mesh.materialIndex < materialColors.size()) {
            const auto& color = materialColors[mesh.materialIndex];
            glColor3f(color.r, color.g, color.b);
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

void ModelManager::getGeometryData(std::vector<float>& outVertices, std::vector<uint32_t>& outIndices) {
    outVertices.clear();
    outIndices.clear();
    uint32_t baseVertex = 0;

    for (const auto& mesh : meshes) {
        // wierzcho³ki
        for (const auto& vertex : mesh.vertices) {
            outVertices.push_back(vertex.x);
            outVertices.push_back(vertex.y);
            outVertices.push_back(vertex.z);
        }

        //indeksy, uwzglêdniaj¹c offset z poprzednich siatek
        for (unsigned int index : mesh.indices) {
            outIndices.push_back(baseVertex + index);
        }

        // aktualizuj offset dla nastêpnej siatki
        baseVertex += mesh.vertices.size();
    }
}