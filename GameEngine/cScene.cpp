#include "cScene.h"
#include <iostream>
#include <fstream>

using namespace std;

void cScene::CreateScene(std::string sceneFileName) {
    //std::string sceneFileName = "D:/Graphics1/GameEngine/sceneFileWithNormals.txt";
    std::string token = "";

    ifstream sceneFile(sceneFileName);

    std::cout << "Loading Scene!" << std::endl;
    
    if (!sceneFile.is_open()) {
        cout << "Trying to open file: " << sceneFileName << endl;
        cout << "Unable to open file because it is already open!" << endl;
        return;
    }
    //else {
    //    cout << "Scenefile open successful!" << endl;
    //}

    // Taking input for number of meshes to load
    while (token != "number_of_meshes") {
        sceneFile >> token;
    }
    sceneFile >> numberOfMeshesToLoad;

    // Taking input for mesh paths
    while (token != "mesh_paths") {
        sceneFile >> token;
    }

    // Pushing back the mesh paths to the vector pathOfMeshesToLoad
    for (unsigned int indexOfMeshesToLoad = 0; indexOfMeshesToLoad != numberOfMeshesToLoad; indexOfMeshesToLoad++) {
        sceneFile >> meshPath;
        meshPath.erase(std::remove(meshPath.begin(), meshPath.end(), '"'), meshPath.end());
        for (char& ch : meshPath) {
            if (ch == '\\') {
                ch = '/';
            }
        }
        //pathOfMeshesToLoad.push_back(meshPath);
        cLoadModels plyModel;
        
        sceneFile >> plyModel.numberOfTextures;     // Directly loading number of textures to the model

        // Checking if it is cubeMapObject
        size_t position = meshPath.find("cubeMapObject.ply");

        // Check if the cubeMapObject.ply is found
        if (position != std::string::npos) {
            plyModel.bIsCubeMap = true;
        }
        else {
            plyModel.bIsCubeMap = false;
        }

        plyModel.LoadPlyModel(meshPath);
        plyModel.ModelFileName = meshPath;
        pModels.push_back(plyModel);
    }

    // Taking input for mesh transforms
    while (token != "mesh_transforms") {
        sceneFile >> token;
    }

    int modelIndex = 0;
    for (unsigned int indexOfMeshTransforms = 0; indexOfMeshTransforms != numberOfMeshesToLoad; indexOfMeshTransforms++) { // Remember whenever you need to call transforms of any model you just need to pass their index!

        sceneFile >> pModels[modelIndex].pMeshTransform.x;
        sceneFile >> pModels[modelIndex].pMeshTransform.y;
        sceneFile >> pModels[modelIndex].pMeshTransform.z;
        pModels[modelIndex].position = glm::vec3(pModels[modelIndex].pMeshTransform.x, pModels[modelIndex].pMeshTransform.y, pModels[modelIndex].pMeshTransform.z);

        sceneFile >> pModels[modelIndex].pMeshTransform.xScale;
        sceneFile >> pModels[modelIndex].pMeshTransform.yScale;
        sceneFile >> pModels[modelIndex].pMeshTransform.zScale;
        pModels[modelIndex].scale = glm::vec3(pModels[modelIndex].pMeshTransform.xScale, pModels[modelIndex].pMeshTransform.yScale, pModels[modelIndex].pMeshTransform.zScale);

        sceneFile >> pModels[modelIndex].pMeshTransform.xRotation;
        sceneFile >> pModels[modelIndex].pMeshTransform.yRotation;
        sceneFile >> pModels[modelIndex].pMeshTransform.zRotation;
        pModels[modelIndex].rotation = glm::vec3(pModels[modelIndex].pMeshTransform.xRotation, pModels[modelIndex].pMeshTransform.yRotation, pModels[modelIndex].pMeshTransform.zRotation);

        modelIndex++;
    }

    while (token != "mesh_textures") {
        sceneFile >> token;
    }

    std::cout << "Loading textures!" << std::endl;

    for (unsigned int modelIndex = 0; modelIndex != numberOfMeshesToLoad; modelIndex++) {       // And here we goooooooooo! Now we can Import max 192 textures for single model!
        for (unsigned int indexOfMeshTextures = 0; indexOfMeshTextures != pModels[modelIndex].numberOfTextures; indexOfMeshTextures++) {
            //std::vector<std::string> textureFilePaths(pModels[modelIndex].numberOfTextures);

            pModels[modelIndex].textureFilePaths.resize(pModels[modelIndex].numberOfTextures);
            pModels[modelIndex].textures.resize(pModels[modelIndex].numberOfTextures);

            sceneFile >> pModels[modelIndex].textureFilePaths[indexOfMeshTextures];

            pModels[modelIndex].textureFilePaths[indexOfMeshTextures].erase(std::remove(pModels[modelIndex].textureFilePaths[indexOfMeshTextures].begin(), pModels[modelIndex].textureFilePaths[indexOfMeshTextures].end(), '"'), pModels[modelIndex].textureFilePaths[indexOfMeshTextures].end());
            for (char& ch : pModels[modelIndex].textureFilePaths[indexOfMeshTextures]) {
                if (ch == '\\') {
                    ch = '/';
                }
            }

            //std::cout << "Texture file read successful!: " << pModels[modelIndex].textureFilePaths[indexOfMeshTextures] << std::endl;
        }
    }
    //while (token != "mesh_material") {
    //    sceneFile >> token;
    //}
        
    //int materialModelIndex = 0;
    //for (unsigned int indexOfMaterial = 0; indexOfMaterial != numberOfMeshesToLoad; indexOfMaterial++) {
    //    sceneFile >> pModels[indexOfMaterial].pMaterial.shininess;

    //    sceneFile >> pModels[indexOfMaterial].pMaterial.diffuse.x;
    //    sceneFile >> pModels[indexOfMaterial].pMaterial.diffuse.y;
    //    sceneFile >> pModels[indexOfMaterial].pMaterial.diffuse.z;

    //    sceneFile >> pModels[indexOfMaterial].pMaterial.specular.x;
    //    sceneFile >> pModels[indexOfMaterial].pMaterial.specular.y;
    //    sceneFile >> pModels[indexOfMaterial].pMaterial.specular.z;
    //    materialModelIndex++;
    //}
}

//void cScene::ExportMaterial(GLuint shaderProgram, int numberOfMaterials)
//{
//    if (glGetUniformLocation(shaderProgram, "material.shininess") == -1 ||
//        glGetUniformLocation(shaderProgram, "material.diffuse") == -1 ||
//        glGetUniformLocation(shaderProgram, "material.specular") == -1) {
//        std::cerr << "Error: Material uniform location not found in shader." << std::endl;
//        return;
//    }
//
//    for (int indexOfMaterial = 0; indexOfMaterial != numberOfMaterials; indexOfMaterial++) {
//        const cLoadModels::sMaterial& material = pModels[indexOfMaterial].pMaterial;
//        glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), material.shininess);
//        glUniform3fv(glGetUniformLocation(shaderProgram, "material.diffuse"), 1, glm::value_ptr(material.diffuse));
//        glUniform3fv(glGetUniformLocation(shaderProgram, "material.specular"), 1, glm::value_ptr(material.specular));
//    }
//}