#ifndef DRAWPLANETS_HPP
#define DRAWPLANETS_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

using namespace glm;

extern GLuint projectionMatrixLocation, viewMatrixLocation, modelMatrixLocation;
extern GLuint lightLocation;
extern GLuint diffuceColorSampler, specularColorSampler;
extern GLuint diffuseTextureSpaceShip, specularTextureSpaceShip, difSpace, specSpace;
extern GLuint objVAO;
extern GLuint objVerticiesVBO, objUVVBO, objNormalsVBO;
extern GLuint TextureCaseLocation;
extern std::vector<vec3> objVertices, objNormals;
extern std::vector<vec2> objUVs;
extern GLuint ShadowL,centerL,center2L;
extern float earthRad, moonRad, RadiiFactor;


void DrawingPlanets(mat4 projectionMatrix, mat4 viewMatrix, vec3 position, float rotation, float tilt,
	float size, vec3 lightPos, int caseTexture, GLuint diffuseTexture, GLuint specularTexture);

void ComputeShadowPrism(vec3 EarthPos, vec3 MoonPos, int planet);

#endif
