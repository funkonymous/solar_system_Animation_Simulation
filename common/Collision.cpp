#include "Collision.h"
#include "Sphere.h"
using namespace glm;

void handleSphereSphereCollision(Sphere& sphere1, Sphere& sphere2);
bool checkForBoxSphereCollision(glm::vec3& pos, const float& r,
    const float& size, glm::vec3& n);
bool checkForSphereSphereCollision(vec3& pos1, const float& r1, vec3& pos2, const float& r2,
	vec3& n1, vec3& n2);


void handleSphereSphereCollision(Sphere& sphere1, Sphere& sphere2) {
	vec3 n1, n2;
	vec3 v1, v2;
	if (checkForSphereSphereCollision(sphere1.x, sphere1.r, sphere2.x, sphere2.r, n1, n2)) {
		v1 = sphere1.v;
		v2 = sphere2.v;
		sphere1.x += v1*vec3(10.0);
		sphere1.v = v1 - 2 * sphere2.m / (sphere1.m + sphere2.m) * dot(v1-v2,n1) * n1 ;
		sphere1.P = sphere1.m * sphere1.v;
		sphere2.x += v2*vec3(10.0);
		sphere2.v = v2 - 2 * sphere1.m / (sphere1.m + sphere2.m) * dot(v2 - v1, n2) * n2;
		sphere2.P = sphere2.m * sphere2.v;
	}
}

bool checkForBoxSphereCollision(vec3& pos, const float& r, const float& size, vec3& n) {
    if (pos.x - r <= 0) {
        //correction
        float dis = -(pos.x - r);
        pos = pos + vec3(dis, 0, 0);

        n = vec3(-1, 0, 0);
    } else if (pos.x + r >= size) {
        //correction
        float dis = size - (pos.x + r);
        pos = pos + vec3(dis, 0, 0);

        n = vec3(1, 0, 0);
    } else if (pos.y - r <= 0) {
        //correction
        float dis = -(pos.y - r);
        pos = pos + vec3(0, dis, 0);

        n = vec3(0, -1, 0);
    } else if (pos.y + r >= size) {
        //correction
        float dis = size - (pos.y + r);
        pos = pos + vec3(0, dis, 0);

        n = vec3(0, 1, 0);
    } else if (pos.z - r <= 0) {
        //correction
        float dis = -(pos.z - r);
        pos = pos + vec3(0, 0, dis);

        n = vec3(0, 0, -1);
    } else if (pos.z + r >= size) {
        //correction
        float dis = size - (pos.z + r);
        pos = pos + vec3(0, 0, dis);

        n = vec3(0, 0, 1);
    } else {
        return false;
    }
    return true;
}


bool checkForSphereSphereCollision(vec3& pos1, const float& r1, vec3& pos2, const float& r2, vec3& n1, vec3& n2) {
	
	if (length(pos1 - pos2) <= (r1 + r2)*500.0*149597870700.0) {
		n1 = normalize(pos1 - pos2);
		n2 = -n1;
		return true;
	}
	else {
		return false;
	}
}

