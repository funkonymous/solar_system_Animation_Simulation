#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <GL/glew.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>

static std::vector<unsigned int> INDICES_DEFAUTL_VALUE = std::vector<unsigned int>();

/**
* A very simple .obj loader. Use only for teaching purposes. Use loadOBJWithTiny()
* instead.
*/
void loadOBJ(
    std::string path,
    std::vector<glm::vec3>& verticies,
    std::vector<glm::vec2>& uvs,
    std::vector<glm::vec3>& normals,
    std::vector<unsigned int>& indices = INDICES_DEFAUTL_VALUE
);

/**
* A .vtp loader.
*/
void loadVTP(
    std::string path,
    std::vector<glm::vec3>& verticies,
    std::vector<glm::vec2>& uvs,
    std::vector<glm::vec3>& normals,
    std::vector<unsigned int>& indices = INDICES_DEFAUTL_VALUE
);

/**
* An .obj loader that uses tinyobjloader library. Any mesh (quad) is triangulated.
*
* https://github.com/syoyo/tinyobjloader
*/
void loadOBJWithTiny(
    std::string path,
    std::vector<glm::vec3>& verticies,
    std::vector<glm::vec2>& uvs,
    std::vector<glm::vec3>& normals,
    std::vector<unsigned int>& indices = INDICES_DEFAUTL_VALUE
);

class Drawable
{
public:
    GLuint VAO, verticesVBO, uvsVBO, normalsVBO, elementVBO;
    unsigned int numIndices;

    Drawable(std::string path);
    ~Drawable();
    void bind();
    /* Bind VAO before calling draw */
    void draw();
};

/*****************************Experimental************************************/

#define MODEL_LOADER_EXPERIMENTAL 0

#if MODEL_LOADER_EXPERIMENTAL == 1
#include <tiny_obj_loader.h>
#include <map>

/**
* https://github.com/syoyo/tinyobjloader/blob/master/examples/viewer/viewer.cc
*
*/
typedef struct
{
    GLuint vb_id;  // vertex buffer id
    int numTriangles;
    size_t material_id;
} DrawObject;

void loadObjAndConvert(const char* filename,
    std::vector<DrawObject>& drawObjects,
    std::vector<tinyobj::material_t>& materials,
    std::map<std::string, GLuint>& textures,
    float bmin[3], float bmax[3]);

void draw(const std::vector<DrawObject>& drawObjects,
    const std::vector<tinyobj::material_t>& materials,
    const std::map<std::string, GLuint>& textures);

#endif

#endif
