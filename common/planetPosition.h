#ifndef PLANETPOSITION_HPP
#define PLANETPOSITION_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


using namespace glm;

// planets pose functions

glm::vec3 EarthPos(float t);
glm::vec3 MoonPos(float t);

glm::vec3 ForceCalc(float M1, vec3 Pos1, float M2, vec3 Pos2);

#endif
