#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <string>

/* We can use a function like this to print some GL capabilities of our adapter
to the log file. handy if we want to debug problems on other people's computers
*/
void logGLParameters();

/**
* Slice a vector<T>.
*/
template<typename T>
std::vector<T> slice(const std::vector<T>& v, int start = 0, int end = -1)
{
    int oldlen = v.size();
    int newlen;

    if (end == -1 && end >= oldlen)
    {
        newlen = oldlen - start;
    }
    else
    {
        newlen = end - start;
    }

    std::vector<T> nv(newlen);

    for (int i = 0; i < newlen; i++) {
        nv[i] = v[start + i];
    }
    return nv;
}

/**
* Get base directory from file path.
*/
std::string getBaseDir(const std::string& filepath);

/**
* Check if file exists.
*/
bool fileExists(const std::string& abs_filename);

/**
* Calculates face normal given v0, v1, v2.
* https://github.com/syoyo/tinyobjloader/blob/master/examples/viewer/viewer.cc
*/
void calcNormal(const float v0[3], const float v1[3], const float v2[3], float N[3]);

/**
* Check GL error.
* https://github.com/syoyo/tinyobjloader/blob/master/examples/viewer/viewer.cc
*/
void checkErrors(std::string desc);

#endif