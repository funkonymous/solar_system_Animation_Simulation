#include <GL/glew.h>
#include <iostream>
#include <cmath>
using namespace std;
#include "util.h"

void logGLParameters() {
    GLenum params[] = {
        GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
        GL_MAX_CUBE_MAP_TEXTURE_SIZE,
        GL_MAX_DRAW_BUFFERS,
        GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
        GL_MAX_TEXTURE_IMAGE_UNITS,
        GL_MAX_TEXTURE_SIZE,
        GL_MAX_VARYING_FLOATS,
        GL_MAX_VERTEX_ATTRIBS,
        GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
        GL_MAX_VERTEX_UNIFORM_COMPONENTS,
        GL_MAX_VIEWPORT_DIMS,
        GL_STEREO,
    };
    const char *names[] = {
        "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
        "GL_MAX_CUBE_MAP_TEXTURE_SIZE",
        "GL_MAX_DRAW_BUFFERS",
        "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
        "GL_MAX_TEXTURE_IMAGE_UNITS",
        "GL_MAX_TEXTURE_SIZE",
        "GL_MAX_VARYING_FLOATS",
        "GL_MAX_VERTEX_ATTRIBS",
        "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
        "GL_MAX_VERTEX_UNIFORM_COMPONENTS",
        "GL_MAX_VIEWPORT_DIMS",
        "GL_STEREO",
    };

    cout << "GL Context Parameters:" << endl;

    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *version = glGetString(GL_VERSION);;
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported: " << version << endl;

    // integers - only works if the order is 0-10 integer return types
    for (int i = 0; i < 10; i++) {
        int v = 0;
        glGetIntegerv(params[i], &v);
        cout << names[i] << " " << v << endl;
    }
    // others
    int v[2];
    v[0] = v[1] = 0;
    glGetIntegerv(params[10], v);
    cout << names[10] << " " << v[0] << " " << v[1] << endl;
    unsigned char s = 0;
    glGetBooleanv(params[11], &s);
    cout << names[11] << " " << s << endl;
    cout << "-----------------------------" << endl;
}

std::string getBaseDir(const std::string & filepath)
{
    if (filepath.find_last_of("/\\") != std::string::npos)
    {
        return filepath.substr(0, filepath.find_last_of("/\\"));
    }
    return "";
}

bool fileExists(const std::string & abs_filename)
{
    bool ret;
    FILE *fp = fopen(abs_filename.c_str(), "rb");
    if (fp)
    {
        ret = true;
        fclose(fp);
    }
    else
    {
        ret = false;
    }

    return ret;
}

void calcNormal(const float v0[3], const float v1[3], const float v2[3], float N[3])
{
    float v10[3];
    v10[0] = v1[0] - v0[0];
    v10[1] = v1[1] - v0[1];
    v10[2] = v1[2] - v0[2];

    float v20[3];
    v20[0] = v2[0] - v0[0];
    v20[1] = v2[1] - v0[1];
    v20[2] = v2[2] - v0[2];

    N[0] = v20[1] * v10[2] - v20[2] * v10[1];
    N[1] = v20[2] * v10[0] - v20[0] * v10[2];
    N[2] = v20[0] * v10[1] - v20[1] * v10[0];

    float len2 = N[0] * N[0] + N[1] * N[1] + N[2] * N[2];
    if (len2 > 0.0f)
    {
        float len = sqrt(len2);

        N[0] /= len;
        N[1] /= len;
    }
}

void checkErrors(std::string desc)
{
    GLenum e = glGetError();
    if (e != GL_NO_ERROR)
    {
        printf("OpenGL error in \"%s\": %d (%d)\n", desc.c_str(), e, e);
        throw runtime_error("");
    }
}
