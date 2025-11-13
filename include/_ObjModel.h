#ifndef _OBJMODEL_H
#define _OBJMODEL_H
#include"_common.h"
#include"_textureloader.h"

struct ObjModel {
    std::vector<Vector3> vertices;
    std::vector<Vector2> tex_coords;
    std::vector<Vector3> normals;
    std::vector<std::vector<Vector3>> faces;
    std::string filename;

    void readObj();
    void Draw();
    void init(std::string filename);
};

#endif // _OBJMODEL_H
