#include <iostream>
#include <sstream>
#include <tinyxml2.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include "util.h"
#include "ModelLoader.h"

using namespace glm;
using namespace std;
using namespace tinyxml2;

// simple OBJ loader
void loadOBJ(
    string path,
    vector<vec3>& vertices,
    vector<vec2>& uvs,
    vector<vec3>& normals,
    vector<unsigned int>& indices
) {
    cout << "Loading OBJ file: " << path << endl;

    vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    vector<vec3> temp_vertices;
    vector<vec2> temp_uvs;
    vector<vec3> temp_normals;
    indices.clear();

    FILE * file = fopen(path.c_str(), "r");
    if (file == NULL) {
        throw runtime_error("Can't open the file.\n");
    }

    while (1) {
        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.

        // else : parse lineHeader

        if (strcmp(lineHeader, "v") == 0) {
            vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            // Invert V coordinate since we will only use DDS texture,
            // which are inverted. Remove if you want to use TGA or BMP loaders.
            uv.y = -uv.y;
            temp_uvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
                &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
            if (matches != 9) {
                fclose(file);
                throw runtime_error("File can't be read by our simple parser.\n");
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices.push_back(uvIndex[0]);
            uvIndices.push_back(uvIndex[1]);
            uvIndices.push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
        else {
            // Probably a comment, eat up the rest of the line
            char stupidBuffer[1000];
            fgets(stupidBuffer, 1000, file);
        }
    }

    // For each vertex of each triangle
    for (unsigned int i = 0; i < vertexIndices.size(); i++) {
        // Get the indices of its attributes
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int uvIndex = uvIndices[i];
        unsigned int normalIndex = normalIndices[i];

        // Get the attributes thanks to the index
        vec3 vertex = temp_vertices[vertexIndex - 1];
        vec2 uv = temp_uvs[uvIndex - 1];
        vec3 normal = temp_normals[normalIndex - 1];

        // Put the attributes in buffers
        vertices.push_back(vertex);
        uvs.push_back(uv);
        normals.push_back(normal);
        indices.push_back(indices.size());
    }
    fclose(file);
}

void loadVTP(string path,
    vector<vec3>& vertices, vector<vec2>& uvs,
    vector<vec3>& normals,
    vector<unsigned int>& indices)
{
    indices.clear();
    const char* method = "PolygonalMesh::loadVtpFile()";
    XMLDocument vtp;
    assert(vtp.LoadFile(path.c_str()) == 0);

    XMLElement* root = vtp.FirstChildElement("VTKFile");
    assert(root != nullptr);
    assert(root->Attribute("type", "PolyData"));

    XMLElement* polydata = root->FirstChildElement("PolyData");
    assert(polydata != nullptr);
    XMLElement* piece = polydata->FirstChildElement("Piece");
    assert(piece != nullptr);
    XMLElement* enormals = piece->FirstChildElement("PointData");
    assert(enormals != nullptr);
    XMLElement* points = piece->FirstChildElement("Points");
    assert(points != nullptr);

    int numPoints, numPolys;
    piece->QueryIntAttribute("NumberOfPoints", &numPoints);
    piece->QueryIntAttribute("NumberOfPolys", &numPolys);

    //assert(enormals->Attribute("format", "ascii"));
    const char* normalsStr = enormals->FirstChildElement("DataArray")->FirstChild()->Value();
    stringstream sNorm(normalsStr);
    vector<vec3> tempNormals;
    do {
        vec3 normal;
        sNorm >> normal.x >> normal.y >> normal.z;
        tempNormals.push_back(normal);
    } while (sNorm.good());
    assert(tempNormals.size() == numPoints + 1);

    XMLElement* pointData = points->FirstChildElement("DataArray");
    assert(pointData != nullptr);
    assert(pointData->Attribute("format", "ascii"));
    const char* coordsStr = points->FirstChildElement("DataArray")->FirstChild()->Value();

    stringstream sCoord(coordsStr);
    vector<vec3> coordinates;
    do {
        vec3 coord;
        sCoord >> coord.x >> coord.y >> coord.z;
        coordinates.push_back(coord);
    } while (sCoord.good());
    coordinates.pop_back();  // the last is corrupted
    assert(coordinates.size() == numPoints);

    XMLElement* polys = piece->FirstChildElement("Polys");
    assert(polys != nullptr);

    // TODO beter implementation
    XMLElement *econnectivity, *eoffsets;
    if (polys->FirstChildElement("DataArray")->Attribute("Name", "connectivity"))
    {
        econnectivity = polys->FirstChildElement("DataArray");
        assert(econnectivity != nullptr);
        assert(econnectivity->Attribute("format", "ascii"));
    }
    else
    {
        throw runtime_error("Can't access connectivity");
    }

    if (polys->LastChildElement("DataArray")->Attribute("Name", "offsets"))
    {
        eoffsets = polys->LastChildElement("DataArray");
        assert(eoffsets != nullptr);
        assert(eoffsets->Attribute("format", "ascii"));
    }
    else
    {
        throw runtime_error("Can't access offsets");
    }

    // read offsets
    const char* offsetsStr = eoffsets->FirstChild()->Value();
    stringstream sOffsets(offsetsStr);
    vector<int> offsets;
    do {
        int offset;
        sOffsets >> offset;
        offsets.push_back(offset);
    } while (sOffsets.good());
    assert(offsets.size() == numPolys + 1);

    // read connectivity
    const char* connStr = econnectivity->FirstChild()->Value();
    stringstream sConn(connStr);
    vector<int> connectivity;
    do {
        int conn;
        sConn >> conn;
        connectivity.push_back(conn);
    } while (sConn.good());
    assert(connectivity.size() == offsets.back() + 1);

    // construct vertices
    int startPoly = 0;
    for (int i = 0; i < numPolys; ++i) {
        vector<int> face = slice(connectivity, startPoly, offsets[i]);
        int i1 = 0, i2 = 1, i3 = 2;
        while (i3 != face.size())
        {
            vertices.push_back(coordinates[face[i1]]);
            normals.push_back(tempNormals[face[i1]]);
            indices.push_back(indices.size());
            vertices.push_back(coordinates[face[i2]]);
            normals.push_back(tempNormals[face[i2]]);
            indices.push_back(indices.size());
            vertices.push_back(coordinates[face[i3]]);
            normals.push_back(tempNormals[face[i3]]);
            indices.push_back(indices.size());
            i2++;
            i3++;
        }
        startPoly = offsets[i];
    }
}

void loadOBJWithTiny(string path,
    vector<vec3>& vertices,
    vector<vec2>& uvs,
    vector<vec3>& normals,
    vector<unsigned int>& indices)
{
    // load obj
    tinyobj::attrib_t attrib;
    vector<tinyobj::shape_t> shapes;
    vector<tinyobj::material_t> materials;

    string err;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str()))
    {
        throw runtime_error(err);
    }

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            vec3 vertex = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2] };
            if (attrib.texcoords.size() != 0)
            {
                vec2 uv = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1 - attrib.texcoords[2 * index.texcoord_index + 1] };
                uvs.push_back(uv);
            }
            if (attrib.normals.size() != 0)
            {
                vec3 normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2] };
                normals.push_back(normal);
            }

            vertices.push_back(vertex);
            indices.push_back(indices.size());
        }
    }

    // TODO .mtl loader
}

Drawable::Drawable(string path)
{
    vector<vec3> vertices;
    vector<vec2> uvs;
    vector<vec3> normals;
    vector<unsigned int> indices;

    if (path.substr(path.size() - 3, 3) == "obj")
    {
        loadOBJWithTiny(path.c_str(), vertices, uvs, normals, indices);
    }
    else if (path.substr(path.size() - 3, 3) == "vtp")
    {
        loadVTP(path.c_str(), vertices, uvs, normals, indices);
    }
    else
    {
        throw runtime_error("File format not supported: " + path);
    }
    // VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // vertex VBO
    glGenBuffers(1, &verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3),
        &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    // normals VBO
    if (normals.size() != 0)
    {
        glGenBuffers(1, &normalsVBO);
        glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3),
            &normals[0], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);
    }

    // uvs VBO
    if (uvs.size() != 0)
    {
        glGenBuffers(1, &uvsVBO);
        glBindBuffer(GL_ARRAY_BUFFER, uvsVBO);
        glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2),
            &uvs[0], GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(2);
    }

    // Generate a buffer for the indices
    numIndices = indices.size();
    glGenBuffers(1, &elementVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
        &indices[0], GL_STATIC_DRAW);
}

Drawable::~Drawable()
{
    glDeleteBuffers(1, &verticesVBO);
    glDeleteBuffers(1, &uvsVBO);
    glDeleteBuffers(1, &normalsVBO);
    glDeleteBuffers(1, &elementVBO);
    glDeleteBuffers(1, &VAO);
}

void Drawable::bind()
{
    glBindVertexArray(VAO);
}

void Drawable::draw()
{
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, NULL);
    //glDrawArrays(GL_TRIANGLES, 0, numIndices);
}

#if MODEL_LOADER_EXPERIMENTAL == 1

#include "texture.h"
#include <algorithm>

void loadObjAndConvert(const char* filename,
    vector<DrawObject>& drawObjects,
    vector<tinyobj::material_t>& materials,
    map<string, GLuint>& textures,
    float bmin[3], float bmax[3])
{
    tinyobj::attrib_t attrib;
    vector<tinyobj::shape_t> shapes;

    string base_dir = getBaseDir(filename);
    if (base_dir.empty())
    {
        base_dir = ".";
    }
#ifdef _WIN32
    base_dir += "\\";
#else
    base_dir += "/";
#endif

    string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err,
        filename, base_dir.c_str());
    if (!err.empty())
    {
        cout << err << endl;
    }

    if (!ret)
    {
        throw runtime_error("Failed to load " + string(filename));
    }

    printf("# of vertices  = %d\n", (int)(attrib.vertices.size()) / 3);
    printf("# of normals   = %d\n", (int)(attrib.normals.size()) / 3);
    printf("# of texcoords = %d\n", (int)(attrib.texcoords.size()) / 2);
    printf("# of materials = %d\n", (int)materials.size());
    printf("# of shapes    = %d\n", (int)shapes.size());

    // Append `default` material
    materials.push_back(tinyobj::material_t());

    for (size_t i = 0; i < materials.size(); i++)
    {
        printf("material[%d].diffuse_texname = %s\n", int(i),
            materials[i].diffuse_texname.c_str());
    }

    // Load diffuse textures
    {
        for (size_t m = 0; m < materials.size(); m++)
        {
            tinyobj::material_t* mp = &materials[m];

            if (mp->diffuse_texname.length() > 0) {
                // Only load the texture if it is not already loaded
                if (textures.find(mp->diffuse_texname) == textures.end())
                {
                    GLuint texture_id;
                    /*int w, h;
                    int comp;*/

                    string texture_filename = mp->diffuse_texname;
                    if (!fileExists(texture_filename))
                    {
                        // Append base dir.
                        texture_filename = base_dir + mp->diffuse_texname;
                        if (!fileExists(texture_filename))
                        {
                            throw runtime_error("Unable to find file: " + mp->diffuse_texname);
                        }
                    }

                    //unsigned char* image = stbi_load(texture_filename.c_str(),
                    //    &w, &h, &comp, STBI_default);
                    //if (!image)
                    //{
                    //    cout << "Unable to load texture: " << texture_filename << endl;
                    //    return false;
                    //}
                    //cout << "Loaded texture: " << texture_filename << ", w = "
                    //    << w << ", h = " << h << ", comp = " << comp << endl;

                    //glGenTextures(1, &texture_id);
                    //glBindTexture(GL_TEXTURE_2D, texture_id);
                    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    //if (comp == 3)
                    //{
                    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB,
                    //        GL_UNSIGNED_BYTE, image);
                    //}
                    //else if (comp == 4)
                    //{
                    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                    //        GL_UNSIGNED_BYTE, image);
                    //}
                    //else
                    //{
                    //    assert(0); // TODO
                    //}
                    //glBindTexture(GL_TEXTURE_2D, 0);
                    //stbi_image_free(image);
                    texture_id = loadSOIL(texture_filename.c_str());
                    textures.insert(make_pair(mp->diffuse_texname, texture_id));
                }
            }
        }
    }

    bmin[0] = bmin[1] = bmin[2] = numeric_limits<float>::max();
    bmax[0] = bmax[1] = bmax[2] = -numeric_limits<float>::max();

    {
        for (size_t s = 0; s < shapes.size(); s++)
        {
            DrawObject o;
            vector<float> buffer;  // pos(3float), normal(3float), color(3float)
            for (size_t f = 0; f < shapes[s].mesh.indices.size() / 3; f++)
            {
                tinyobj::index_t idx0 = shapes[s].mesh.indices[3 * f + 0];
                tinyobj::index_t idx1 = shapes[s].mesh.indices[3 * f + 1];
                tinyobj::index_t idx2 = shapes[s].mesh.indices[3 * f + 2];

                int current_material_id = shapes[s].mesh.material_ids[f];

                if ((current_material_id < 0) ||
                    (current_material_id >= static_cast<int>(materials.size())))
                {
                    // Invalid material ID. Use default material.
                    // Default material is added to the last item in `materials`.
                    current_material_id = materials.size() - 1;
                }
                //if (current_material_id >= materials.size()) {
                //    cerr << "Invalid material index: " << current_material_id << endl;
                //}
                //
                float diffuse[3];
                for (size_t i = 0; i < 3; i++)
                {
                    diffuse[i] = materials[current_material_id].diffuse[i];
                }
                float tc[3][2];
                if (attrib.texcoords.size() > 0)
                {
                    assert(attrib.texcoords.size() > 2 * idx0.texcoord_index + 1);
                    assert(attrib.texcoords.size() > 2 * idx1.texcoord_index + 1);
                    assert(attrib.texcoords.size() > 2 * idx2.texcoord_index + 1);

                    // Flip Y coord.
                    tc[0][0] = attrib.texcoords[2 * idx0.texcoord_index];
                    tc[0][1] = 1.0f - attrib.texcoords[2 * idx0.texcoord_index + 1];
                    tc[1][0] = attrib.texcoords[2 * idx1.texcoord_index];
                    tc[1][1] = 1.0f - attrib.texcoords[2 * idx1.texcoord_index + 1];
                    tc[2][0] = attrib.texcoords[2 * idx2.texcoord_index];
                    tc[2][1] = 1.0f - attrib.texcoords[2 * idx2.texcoord_index + 1];
                }
                else
                {
                    tc[0][0] = 0.0f;
                    tc[0][1] = 0.0f;
                    tc[1][0] = 0.0f;
                    tc[1][1] = 0.0f;
                    tc[2][0] = 0.0f;
                    tc[2][1] = 0.0f;
                }

                float v[3][3];
                for (int k = 0; k < 3; k++)
                {
                    int f0 = idx0.vertex_index;
                    int f1 = idx1.vertex_index;
                    int f2 = idx2.vertex_index;
                    assert(f0 >= 0);
                    assert(f1 >= 0);
                    assert(f2 >= 0);

                    v[0][k] = attrib.vertices[3 * f0 + k];
                    v[1][k] = attrib.vertices[3 * f1 + k];
                    v[2][k] = attrib.vertices[3 * f2 + k];
                    bmin[k] = std::min(v[0][k], bmin[k]);
                    bmin[k] = std::min(v[1][k], bmin[k]);
                    bmin[k] = std::min(v[2][k], bmin[k]);
                    bmax[k] = std::max(v[0][k], bmax[k]);
                    bmax[k] = std::max(v[1][k], bmax[k]);
                    bmax[k] = std::max(v[2][k], bmax[k]);
                }

                float n[3][3];
                if (attrib.normals.size() > 0)
                {
                    int f0 = idx0.normal_index;
                    int f1 = idx1.normal_index;
                    int f2 = idx2.normal_index;
                    assert(f0 >= 0);
                    assert(f1 >= 0);
                    assert(f2 >= 0);
                    for (int k = 0; k < 3; k++) {
                        n[0][k] = attrib.normals[3 * f0 + k];
                        n[1][k] = attrib.normals[3 * f1 + k];
                        n[2][k] = attrib.normals[3 * f2 + k];
                    }
                }
                else
                {
                    // compute geometric normal
                    calcNormal(v[0], v[1], v[2], n[0]);
                    n[1][0] = n[0][0];
                    n[1][1] = n[0][1];
                    n[1][2] = n[0][2];
                    n[2][0] = n[0][0];
                    n[2][1] = n[0][1];
                    n[2][2] = n[0][2];
                }

                for (int k = 0; k < 3; k++)
                {
                    buffer.push_back(v[k][0]);
                    buffer.push_back(v[k][1]);
                    buffer.push_back(v[k][2]);
                    buffer.push_back(n[k][0]);
                    buffer.push_back(n[k][1]);
                    buffer.push_back(n[k][2]);
                    // Combine normal and diffuse to get color.
                    float normal_factor = 0.2;
                    float diffuse_factor = 1 - normal_factor;
                    float c[3] = {
                        n[k][0] * normal_factor + diffuse[0] * diffuse_factor,
                        n[k][1] * normal_factor + diffuse[1] * diffuse_factor,
                        n[k][2] * normal_factor + diffuse[2] * diffuse_factor
                    };
                    float len2 = c[0] * c[0] + c[1] * c[1] + c[2] * c[2];
                    if (len2 > 0.0f)
                    {
                        float len = sqrtf(len2);

                        c[0] /= len;
                        c[1] /= len;
                        c[2] /= len;
                    }
                    buffer.push_back(c[0] * 0.5 + 0.5);
                    buffer.push_back(c[1] * 0.5 + 0.5);
                    buffer.push_back(c[2] * 0.5 + 0.5);

                    buffer.push_back(tc[k][0]);
                    buffer.push_back(tc[k][1]);
                }
            }

            o.vb_id = 0;
            o.numTriangles = 0;

            // OpenGL viewer does not support texturing with per-face material.
            if (shapes[s].mesh.material_ids.size() > 0 &&
                shapes[s].mesh.material_ids.size() > s)
            {
                // use the material ID of the first face.
                o.material_id = shapes[s].mesh.material_ids[0];
            }
            else {
                o.material_id = materials.size() - 1; // = ID for default material.
            }
            printf("shape[%d] material_id %d\n", int(s), int(o.material_id));

            if (buffer.size() > 0) {
                glGenBuffers(1, &o.vb_id);
                glBindBuffer(GL_ARRAY_BUFFER, o.vb_id);
                glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), &buffer.at(0),
                    GL_STATIC_DRAW);
                // 3:vtx, 3:normal, 3:col, 2:texcoord
                o.numTriangles = buffer.size() / (3 + 3 + 3 + 2) / 3;

                printf("shape[%d] # of triangles = %d\n", static_cast<int>(s),
                    o.numTriangles);
            }

            drawObjects.push_back(o);
        }
    }

    printf("bmin = %f, %f, %f\n", bmin[0], bmin[1], bmin[2]);
    printf("bmax = %f, %f, %f\n", bmax[0], bmax[1], bmax[2]);
}

void draw(const vector<DrawObject>& drawObjects,
    const vector<tinyobj::material_t>& materials,
    const map<string, GLuint>& textures)
{
    glPolygonMode(GL_FRONT, GL_FILL);
    glPolygonMode(GL_BACK, GL_FILL);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);
    GLsizei stride = (3 + 3 + 3 + 2) * sizeof(float);
    for (size_t i = 0; i < drawObjects.size(); i++)
    {
        DrawObject o = drawObjects[i];
        if (o.vb_id < 1)
        {
            continue;
        }

        glBindBuffer(GL_ARRAY_BUFFER, o.vb_id);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glBindTexture(GL_TEXTURE_2D, 0);
        if ((o.material_id < materials.size()))
        {
            string diffuse_texname = materials[o.material_id].diffuse_texname;
            if (textures.find(diffuse_texname) != textures.end())
            {
                glBindTexture(GL_TEXTURE_2D, textures.at(diffuse_texname));
            }
        }
        glVertexPointer(3, GL_FLOAT, stride, (const void*)0);
        glNormalPointer(GL_FLOAT, stride, (const void*)(sizeof(float) * 3));
        glColorPointer(3, GL_FLOAT, stride, (const void*)(sizeof(float) * 6));
        glTexCoordPointer(2, GL_FLOAT, stride, (const void*)(sizeof(float) * 9));

        glDrawArrays(GL_TRIANGLES, 0, 3 * o.numTriangles);
        checkErrors("drawarrays");
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // draw wireframe
    glDisable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT, GL_LINE);
    glPolygonMode(GL_BACK, GL_LINE);

    glColor3f(0.0f, 0.0f, 0.4f);
    for (size_t i = 0; i < drawObjects.size(); i++)
    {
        DrawObject o = drawObjects[i];
        if (o.vb_id < 1)
        {
            continue;
        }

        glBindBuffer(GL_ARRAY_BUFFER, o.vb_id);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glVertexPointer(3, GL_FLOAT, stride, (const void*)0);
        glNormalPointer(GL_FLOAT, stride, (const void*)(sizeof(float) * 3));
        glColorPointer(3, GL_FLOAT, stride, (const void*)(sizeof(float) * 6));
        glTexCoordPointer(2, GL_FLOAT, stride, (const void*)(sizeof(float) * 9));

        glDrawArrays(GL_TRIANGLES, 0, 3 * o.numTriangles);
        checkErrors("drawarrays");
    }
}
#endif