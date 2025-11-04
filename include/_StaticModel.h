#ifndef _STATIC_MODEL_H
#define _STATIC_MODEL_H
#include<_common.h>
#include<_textureloader.h>

struct ObjModel {
    std::vector<Vector3> vertices;
    std::vector<Vector2> tex_coords;
    std::vector<Vector3> normals;
    std::vector<std::vector<Vector3>> faces;
    std::string filename;
    
    // Add these functions
    void readObj();
    void Draw();
    void init(std::string filename);
};

class _StaticModel{
    public:
        _StaticModel();
        ~_StaticModel();

        bool LoadModel(const char* filename);
        void Draw();
        void FreeModel();
    private:
        // hold geometry
        ObjModel myModel;
        _textureLoader myTex;
        GLuint modelTexID;
    protected:

};



#endif // _STATIC_MODEL_H