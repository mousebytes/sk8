#ifndef _STATIC_MODEL_H
#define _STATIC_MODEL_H
#include<_common.h>
#include<_textureloader.h>
#include"_ObjModel.h"

class _StaticModel{
    public:
        _StaticModel();
        ~_StaticModel();

        bool LoadModel(const char* filename,char* texpath);
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
