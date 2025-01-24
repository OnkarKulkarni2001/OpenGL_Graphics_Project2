#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "cLoadModels.h"
#include "FlyCam.h"
#include "cShaderCompiler.h"
#include "cVAOManager.h"
#include "cScene.h"
#include "IPlayer.h"
#include "cPlayer.h"
#include "cAiEnemy.h"
#include "cLightManager.h"
#include "cLightMover.h"
#include "cPhysicsUpdated.h"
#include "cRenderModel.h"
#include "cLua.h"
#include "cTextureCreator.h"
#include <sstream>
#include <algorithm>

glm::vec3 g_CameraLocation;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

float getRandomFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

glm::vec3 getRandom_vec3(glm::vec3 min, glm::vec3 max)
{
    return glm::vec3(
        getRandomFloat(min.x, max.x),
        getRandomFloat(min.y, max.y),
        getRandomFloat(min.z, max.z));
}

std::string getStringVec3(glm::vec3 theVec3)
{
    std::stringstream ssVec;
    ssVec << "(" << theVec3.x << ", " << theVec3.y << ", " << theVec3.z << ")";
    return ssVec.str();
}

bool IsCloser(cLoadModels& modelA, cLoadModels& modelB) {
    return (glm::distance(g_CameraLocation, modelA.position) > glm::distance(g_CameraLocation, modelB.position));
}

void SortModelsByDistanceFromCamera(std::vector<cLoadModels>& transparentObjects) {
    //for (int i = 0; i != transparentObjects.size(); i++) {
        std::sort( transparentObjects.begin(), transparentObjects.end(), IsCloser);
    //}
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version (3.3 core profile)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);                  // glTexStorage2D is not supported on version 3, need to use 4!!!
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Triangle", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Refer cShaderCompiler class for more info.
    cShaderCompiler shader;
    GLuint shaderProgram = shader.CompileShader();
    glUseProgram(shaderProgram);

    // Import Scene
    cScene scene;
    scene.CreateScene("../sceneFileWithNormals.txt");
    //scene.ExportMaterial(shaderProgram, scene.numberOfMeshesToLoad);            // Considering number of materials = number of meshes to load

    cVAOManager VAOManager;
    for (int i = 0; i != scene.pModels.size(); i++) {
        VAOManager.GettingModelReadyToRender(scene.pModels[i]);         // This thing is new just because I created whole new VAO thing which creates several different VAOs and now I can render a single model multiple times
    }

    cRenderModel renderer;

    cLightManager lightManager;
    lightManager.LoadLights("../lightsFile.txt");

    std::cout << "This may take a while..." << std::endl;
    // Camera Initialization
    FlyCam flyCam(800, 600, glm::vec3(0, 9, -16.0), 0.0f);
    flyCam.camSpeed = 1.0f;
    flyCam.camYaw = 91.0f;
    flyCam.camPitch = 4.0f;


    cLightMover lightMover(lightManager, flyCam, 1);


    float deltaTime = 0;
    float startTime, endTime;

    for (int modelIndex = 0; modelIndex != scene.numberOfMeshesToLoad; modelIndex++) {
        scene.pModels[modelIndex].pTransformedVertices = new cLoadModels::sTransformedVertex[scene.pModels[modelIndex].numberOfVertices];
    
        glm::mat4 model = scene.pModels[modelIndex].CreateModelMatrix(shaderProgram, scene.pModels[modelIndex]);      // Creation of model matrix with arguements passed in sceneFile.txt
        scene.pModels[modelIndex].GenerateTransformedVertices(model);
    }   // Used for initializing the pTransformedVertices, Nothing new xD

    // Starting physics
   // cPhysicsUpdated physicsEngine(scene);

    startTime = glfwGetTime();


    // ------------------------------------------Texture---------------------------------------

    //    BaseColor.bmp     <--------------------- This order should be followed if you have every kind of map
    //    Normal.bmp
    //    Roughness.bmp
    //    Height.bmp
    //    Metallic.bmp

    cTextureCreator textureCreator;

    for (int modelIndex = 0; modelIndex != scene.numberOfMeshesToLoad; modelIndex++) {
        if (scene.pModels[modelIndex].bIsCubeMap == false) {
            textureCreator.LoadTextures24Bit(shaderProgram, scene.pModels[modelIndex], true);
        }
        else {
            textureCreator.LoadCubeMap24Bit(shaderProgram, true, scene.pModels[modelIndex].cubeMapTextureID,
                scene.pModels[modelIndex].textureFilePaths[0], scene.pModels[modelIndex].textureFilePaths[1],
                scene.pModels[modelIndex].textureFilePaths[2], scene.pModels[modelIndex].textureFilePaths[3],
                scene.pModels[modelIndex].textureFilePaths[4], scene.pModels[modelIndex].textureFilePaths[5]);
        }
    }
    scene.pModels[5].bIsTransparent = true;
    scene.pModels[5].transparencyIndex = 0.3;
    scene.pModels[5].bIsReflective = true;
    scene.pModels[5].reflectiveIndex = 0.5;
    scene.pModels[5].bIsRefractive = true;
    scene.pModels[5].refractiveIndex = 1.5;

    scene.pModels[3].bIsStensil = true;

    scene.pModels[13].bIsTransparent = true;
    scene.pModels[13].transparencyIndex = 0.5f;
    scene.pModels[14].bIsTransparent = true;
    scene.pModels[14].transparencyIndex = 0.5f;
    scene.pModels[15].bIsTransparent = true;
    scene.pModels[15].transparencyIndex = 0.5f;
    scene.pModels[16].bIsTransparent = true;
    scene.pModels[16].transparencyIndex = 0.5f;
    scene.pModels[17].bIsTransparent = true;
    scene.pModels[17].transparencyIndex = 0.5f;
    scene.pModels[18].bIsTransparent = true;
    scene.pModels[18].transparencyIndex = 0.5f;
    scene.pModels[19].bIsTransparent = true;
    scene.pModels[19].transparencyIndex = 0.5f;

    scene.pModels[13].bIsReflective = true;
    scene.pModels[13].reflectiveIndex = 1.0f;
    scene.pModels[14].bIsReflective = true;
    scene.pModels[14].reflectiveIndex = 1.0f;
    scene.pModels[15].bIsReflective = true;
    scene.pModels[15].reflectiveIndex = 1.0f;
    scene.pModels[16].bIsReflective = true;
    scene.pModels[16].reflectiveIndex = 1.0f;
    scene.pModels[17].bIsReflective = true;
    scene.pModels[17].reflectiveIndex = 1.0f;
    scene.pModels[18].bIsReflective = true;
    scene.pModels[18].reflectiveIndex = 1.0f;
    scene.pModels[19].bIsReflective = true;
    scene.pModels[19].reflectiveIndex = 1.0f;

    //"..\assets\models\dragonBall1Star.ply" 1
    //    "..\assets\models\dragonBall7Star.ply" 1
    //    "..\assets\models\dragonBall6Star.ply" 1
    //    "..\assets\models\dragonBall3Star.ply" 1
    //    "..\assets\models\dragonBall2StarNew.ply" 1
    //    "..\assets\models\dragonBall5Star.ply" 1
    //    "..\assets\models\dragonBall4Star.ply" 1
    //glEnable(GL_BLEND);                                 // For transparency
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ------------------------------------------Texture---------------------------------------

    // -----------------------------------------------LUA----------------------------------------------
    
    cLua lua;
    lua_State* state = luaL_newstate();
    lua.InitLua(state);
    //lua.ExecuteLuaScript(state, "game_logic.lua");      // Need to implement this script game_logic.lua

    // -----------------------------------------------LUA----------------------------------------------

    bool bIsHPressed = false;
    bool bShowHUD = false;
    scene.pModels[3].bIsVisible = false;

    std::vector<cLoadModels> transparentObjects;
    transparentObjects.clear();
    for (int i = 0; i != scene.pModels.size(); i++) {
        if (scene.pModels[i].bIsTransparent) {
            transparentObjects.push_back(scene.pModels[i]);
        }
    }

    scene.pModels[18].bIsMovingTexture = true;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Render loop
    while (!glfwWindowShouldClose(window)) {

        g_CameraLocation = flyCam.camLocation;
        //std::cout << getStringVec3(g_CameraLocation) << std::endl;
        // Input handling
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        glfwGetWindowSize(window, &flyCam.camWidth, &flyCam.camHeight);

        std::string cameraPositionInTitle = "Camera Location: (" + to_string(flyCam.camLocation.x) + ", " + to_string(flyCam.camLocation.y) + ", " + to_string(flyCam.camLocation.z) + ") Camera Speed: " + to_string(flyCam.camSpeed) + " Camera Roll: " + to_string(flyCam.camRoll) + " Cam yaw: " + to_string(flyCam.camYaw) + " Cam pitch: " + to_string(flyCam.camPitch);
        glfwSetWindowTitle(window, cameraPositionInTitle.c_str());

        endTime = glfwGetTime();
        deltaTime = endTime - startTime;
        //std::cout << deltaTime << " Time passed" << std::endl;

        flyCam.cameraMatrix(45.0f, 0.1f, 11000.0f, shaderProgram, "camMatrix", window);

        // Rendering commands here
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glEnable(GL_DEPTH_TEST);   // Enable depth testing
        glEnable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader program and draw the triangle
        glUseProgram(shaderProgram);

        //lightMover.MoveForward();
        //lightMover.MoveUp();
        //lightMover.MoveRight();

        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
            scene.pModels.clear();
            scene.CreateScene("../sceneFileWithNormals.txt");
            for (int i = 0; i != scene.pModels.size(); i++) {
                VAOManager.GettingModelReadyToRender(scene.pModels[i]);         // This thing is new just because I created whole new VAO thing which creates several different VAOs and now I can render a single model multiple times
            }
            for (int modelIndex = 0; modelIndex != scene.numberOfMeshesToLoad; modelIndex++) {
                scene.pModels[modelIndex].pTransformedVertices = new cLoadModels::sTransformedVertex[scene.pModels[modelIndex].numberOfVertices];

                glm::mat4 model = scene.pModels[modelIndex].CreateModelMatrix(shaderProgram, scene.pModels[modelIndex]);      // Creation of model matrix with arguements passed in sceneFile.txt
                scene.pModels[modelIndex].GenerateTransformedVertices(model);
            }
            lightManager.LoadLights("../lightsFile.txt");
            for (int modelIndex = 0; modelIndex != scene.numberOfMeshesToLoad; modelIndex++) {
                if (scene.pModels[modelIndex].bIsCubeMap == false) {
                    textureCreator.LoadTextures24Bit(shaderProgram, scene.pModels[modelIndex], true);
                }
                else {
                    textureCreator.LoadCubeMap24Bit(shaderProgram, true, scene.pModels[modelIndex].cubeMapTextureID,
                        scene.pModels[modelIndex].textureFilePaths[0], scene.pModels[modelIndex].textureFilePaths[1],
                        scene.pModels[modelIndex].textureFilePaths[2], scene.pModels[modelIndex].textureFilePaths[3],
                        scene.pModels[modelIndex].textureFilePaths[4], scene.pModels[modelIndex].textureFilePaths[5]);
                }
            }
        }



        // ------------------------------------------------------------------------------------------------------------------------------
        // You can call movement functions from light mover class for lights here now and then call turn on lights function of light manager
        lightManager.TurnOnLights(shaderProgram, 10);
        // ------------------------------------------------------------------------------------------------------------------------------
        // You can create player objects here and make them move from here
        // ------------------------------------------------------------------------------------------------------------------------------

        //glm::vec3 cameraPosition = flyCam.camLocation;

        scene.pModels[5].pMeshTransform.xRotation += 0.01 * deltaTime;
        std::cout << scene.pModels[5].pMeshTransform.xRotation << std::endl;
        // Cube map object position should go with camera (scene.pModels[2] is cubeMapObject)


        for (int i = 0; i != scene.pModels.size(); i++) {
            if (scene.pModels[i].bIsCubeMap) {
                scene.pModels[i].pMeshTransform.x = flyCam.camLocation.x;
                scene.pModels[i].pMeshTransform.y = flyCam.camLocation.y;
                scene.pModels[i].pMeshTransform.z = flyCam.camLocation.z;
            }
        }

        // Show HUD object
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
            if (!bIsHPressed) {
                bShowHUD = !bShowHUD;
                scene.pModels[3].bIsVisible = bShowHUD;
            }
            bIsHPressed = true;
        }
        else {
            bIsHPressed = false;
        }

        if (bShowHUD) {
            scene.pModels[3].pMeshTransform.xScale = 0.09;
            scene.pModels[3].pMeshTransform.yScale = 0.1;
            scene.pModels[3].pMeshTransform.zScale = 0.16;
            scene.pModels[3].pMeshTransform.x = flyCam.camLocation.x + flyCam.camForwardVector.x * 20.0f;
            scene.pModels[3].pMeshTransform.y = flyCam.camLocation.y + flyCam.camForwardVector.y * 20.0f;
            scene.pModels[3].pMeshTransform.z = flyCam.camLocation.z + flyCam.camForwardVector.z * 20.0f;
            glm::vec3 camOverlayPosition = glm::vec3(scene.pModels[3].pMeshTransform.x, scene.pModels[3].pMeshTransform.y, scene.pModels[3].pMeshTransform.z);
            glm::vec3 direction = glm::normalize(flyCam.camLocation - camOverlayPosition);

            glm::quat rotation = glm::quatLookAt(direction, flyCam.camUpVector);
            glm::vec3 eulerAngles = glm::eulerAngles(rotation);

            glm::quat adjustedRotation = rotation * glm::quat(glm::vec3(glm::radians(-90.0f), 0.0f, glm::radians(90.0f))); // Example: Rotate 90 degrees around Y
            glm::vec3 adjustedEulerAngles = glm::eulerAngles(adjustedRotation);

            // Apply adjusted angles
            scene.pModels[3].pMeshTransform.xRotation = glm::degrees(adjustedEulerAngles.x);
            scene.pModels[3].pMeshTransform.yRotation = glm::degrees(adjustedEulerAngles.y);
            scene.pModels[3].pMeshTransform.zRotation = glm::degrees(adjustedEulerAngles.z);
        }
        else {
            scene.pModels[3].pMeshTransform.xScale = 0.0;
            scene.pModels[3].pMeshTransform.yScale = 0.0;
            scene.pModels[3].pMeshTransform.zScale = 0.0;
        }
        lightManager.lights[2].position = glm::vec3(scene.pModels[4].pMeshTransform.x, scene.pModels[4].pMeshTransform.y, scene.pModels[4].pMeshTransform.z);

        //glCullFace(GL_BACK);
        //glEnable(GL_CULL_FACE);
        //glDisable(GL_BLEND);
        //glEnable(GL_DEPTH_TEST);
        //glDepthMask(GL_TRUE);

        for (int i = 0; i != scene.pModels.size(); i++) {
            if (scene.pModels[i].bIsWireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else {
                shader.SetSceneView(window);        // Press 1, 2 or 3 for different viewModes like wireframe, fill or point
            }
            if (!scene.pModels[i].bIsTransparent){
                renderer.Render(shaderProgram, &scene.pModels[i]);
            }
        }
        SortModelsByDistanceFromCamera(transparentObjects);

        glUniform1f(glGetUniformLocation(shaderProgram, "deltaTime"), deltaTime);

        glm::mat4 view = glm::lookAt(flyCam.camLocation, flyCam.camLocation + flyCam.camForwardVector, flyCam.camUpVector);
        glm::vec3 cameraPosition = glm::vec3(glm::inverse(view)[3]);
                //// Sort transparent models back-to-front based on camera position
        //// Calculate camera position once before sorting
        //std::sort(transparentObjects.begin(), transparentObjects.end(),
        //    [&cameraPosition](cLoadModels* a, cLoadModels* b) {
        //        // Compute distances for valid models
        //        float distanceA = glm::distance(cameraPosition, a->position);
        //        float distanceB = glm::distance(cameraPosition, b->position);

        //        // Sort by distance: farther objects should come first
        //        return distanceA > distanceB;
        //    });
        
        //SortModelsByDistanceFromCamera(transparentObjects);
        //glDisable(GL_CULL_FACE);
        //glEnable(GL_BLEND);
        //glDisable(GL_DEPTH_TEST);
        //glDepthMask(GL_FALSE);

        for (int j = 0; j != transparentObjects.size(); j++) {
            renderer.Render(shaderProgram, &transparentObjects[j]);
        }
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        //glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        // Swap buffers and poll IO events (keys pressed/released, mouse moved, etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    // -----------------------------------------------LUA----------------------------------------------
    
    lua_close(state);   // Lua cleanup

    // -----------------------------------------------LUA----------------------------------------------



    // Cleanup
    VAOManager.VAOVBOCleanup();
    
    // Terminate GLFW
    glfwTerminate();
    return 0;
}