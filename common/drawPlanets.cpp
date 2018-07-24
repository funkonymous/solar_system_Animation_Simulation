/*
Drawing part of the project
DrawingPlanets:
	Inputs :
		Projection Matrix from camera
		View Matrix from camera
		Position from animation or simulation
		Rotation from simulation
		Tilt is a constant
		Size is a constant
		light Position is always the position of the sun
		Texture case for sun-background or planets
		Diffuse Texture and Specular Texture for texture mapping
	Action :
		Binding and passing through uniforms all data exeprt shadows
		to shader programms
ComputeShadowPrism
	Inputs :
		Earth's Position
		Moon's Position
		The planet we refer to
	Action :
		Computes the shadow prism's euler angle and propagates
		the object's center and the object's that causes the shadow
		center
*/

#include <GL/glew.h>
#include "drawPlanets.h"


using namespace glm;
enum  { other = 0, moon, earth };

void DrawingPlanets(mat4 projectionMatrix, mat4 viewMatrix, vec3 position, float rotation, float tilt, 
	float size, vec3 lightPos, int caseTexture, GLuint diffuseTexture, GLuint specularTexture)
{	
	
	glBindVertexArray(objVAO);
	// compute model matrix
	mat4 modelMatrix  = glm::translate(mat4(), position)*
		glm::rotate(mat4(), tilt, vec3(0, 0, 1))*
		glm::rotate(mat4(), rotation, vec3(0, 1, 0))*
		glm::scale(mat4(), vec3(size, size, size));

	// transfer uniforms to GPU
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);
	glUniform3f(lightLocation, lightPos.x, lightPos.y, lightPos.z); // light
	glUniform1f(TextureCaseLocation, caseTexture);

	// bind textures and transmit diffuse and specular maps to the GPU
	glUniform1i(diffuceColorSampler, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseTexture);
	glUniform1i(specularColorSampler, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, specularTexture);
	
	glUniform1i(difSpace, 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, diffuseTextureSpaceShip);
	glUniform1i(specSpace, 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, specularTextureSpaceShip);
	
	// draw
	glDrawArrays(GL_TRIANGLES, 0, objVertices.size());
	
}

void ComputeShadowPrism(vec3 EarthPos, vec3 MoonPos,int planet) {
	float earthShadowPrism = 0.0;
	float moonShadowPrism = 0.0;
	// Euler angle for earth
	if (length(MoonPos) <= length(EarthPos)) {
		earthShadowPrism = length(MoonPos) /sqrt( pow(length(MoonPos),2)+pow(moonRad*RadiiFactor,2));
	};
	// Euler angle for moon
	if (length(MoonPos) >= length(EarthPos)) {
		moonShadowPrism = length(EarthPos) / sqrt(pow(length(EarthPos), 2) + pow(earthRad*RadiiFactor, 2));
	};

	switch (planet) {
		case earth:
			glUniform1f(ShadowL, earthShadowPrism);
			glUniform3f(centerL, MoonPos.x, MoonPos.y, MoonPos.z);
			glUniform3f(center2L, EarthPos.x, EarthPos.y, EarthPos.z);
			break;
		case moon:
			glUniform1f(ShadowL, moonShadowPrism);
			glUniform3f(centerL, EarthPos.x, EarthPos.y, EarthPos.z);
			glUniform3f(center2L, MoonPos.x, MoonPos.y, MoonPos.z);
			break;
		default:
			glUniform1f(ShadowL, 0.0f);
			break;
	};
}