#include "cRenderModel.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void cRenderModel::Render(GLuint shaderProgram, cLoadModels* model)
{
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	if (model->bIsCubeMap) {
		model->bIsVisible = true;
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
	}
	glUniform1f(glGetUniformLocation(shaderProgram, "bIsMovingTexture"), model->bIsMovingTexture);

	glUniform1i(glGetUniformLocation(shaderProgram, "bIsStensil"), model->bIsStensil);
	glUniform1i(glGetUniformLocation(shaderProgram, "bIsVisible"), model->bIsVisible);
	glUniform1i(glGetUniformLocation(shaderProgram, "bIsReflective"), model->bIsReflective);
	glUniform1i(glGetUniformLocation(shaderProgram, "bIsRefractive"), model->bIsRefractive);
	glUniform1i(glGetUniformLocation(shaderProgram, "bIsTransparent"), model->bIsTransparent);
	glUniform1i(glGetUniformLocation(shaderProgram, "bIsCubeMap"), model->bIsCubeMap);

	if (model->bIsReflective) {
		glUniform1f(glGetUniformLocation(shaderProgram, "reflectiveIndex"), model->reflectiveIndex);
	}
	if (model->bIsRefractive) {
		glUniform1f(glGetUniformLocation(shaderProgram, "refractiveIndex"), model->refractiveIndex);
	}

	for (int textureIndex = 0; textureIndex != model->numberOfTextures; textureIndex++) {		// Needed this otherwise every model will have same last loaded texture
		if (!model->bIsCubeMap) {
			if (model->bIsTransparent) {
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glDisable(GL_CULL_FACE);
				//glDisable(GL_DEPTH_TEST);
				glDepthMask(GL_FALSE);
				glUniform1i(glGetUniformLocation(shaderProgram, "bIsTransparent"), true);
				glUniform1f(glGetUniformLocation(shaderProgram, "transparencyIndex"), model->transparencyIndex);
			}
			else {
				glEnable(GL_CULL_FACE);
				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);
				//glDepthMask(GL_TRUE);
				glUniform1i(glGetUniformLocation(shaderProgram, "bIsTransparent"), false);
			}
			glActiveTexture(GL_TEXTURE0 + textureIndex);   // 0 is texture unit
			glBindTexture(GL_TEXTURE_2D, model->textures[textureIndex]);
			std::string textureString = "textureSamplers[" + std::to_string(textureIndex) + "]";		// done this as I have array of texture units in shader
			glUniform1i(glGetUniformLocation(shaderProgram, textureString.c_str()), textureIndex);  // textureIndex is texture unit
		}
		else if (model->bIsCubeMap) {
			glBindTexture(GL_TEXTURE_CUBE_MAP, model->cubeMapTextureID);
		}
	}
	glBindVertexArray(model->VAO_ID);

	glm::mat4 modelMat = model->CreateModelMatrix(shaderProgram, *model);      // Creation of model matrix with arguements passed in sceneFile.txt
	model->GenerateTransformedVertices(modelMat);            // this is here because collisions want transformed vertices
	
	unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

	glDrawElements(GL_TRIANGLES, model->numberOfIndices, GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);
	
	if (model->bIsCubeMap) {
		model->bIsVisible = false;
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
	}
	if (model->bIsTransparent) {
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		//glEnable(GL_DEPTH_TEST);
	}
}

void cRenderModel::DrawDebugSphere(cLoadModels* sphereModel, glm::vec3 position, glm::vec4 RGBA, float scale, GLuint shaderProgram)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	sphereModel->pMeshTransform.x = position.x;
	sphereModel->pMeshTransform.y = position.y;
	sphereModel->pMeshTransform.z = position.z;
	
	sphereModel->pVertex->r = RGBA.r;
	sphereModel->pVertex->g = RGBA.g;
	sphereModel->pVertex->b = RGBA.b;
	sphereModel->pVertex->a = RGBA.a;

	sphereModel->scale.x = scale;
	sphereModel->scale.y = scale;
	sphereModel->scale.z = scale;

	sphereModel->bIsWireframe = true;

    Render(shaderProgram, sphereModel);
}

void cRenderModel::DrawModelAtLocation(cLoadModels* model, glm::vec3 position, GLuint shaderProgram)
{
	model->pMeshTransform.x = position.x;
	model->pMeshTransform.y = position.y;
	model->pMeshTransform.z = position.z;

	Render(shaderProgram, model);
}
