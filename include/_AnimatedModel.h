#ifndef _ANIMATEDMODEL_H
#define _ANIMATEDMODEL_H

#include<_common.h>
#include"_StaticModel.h"

/*
EX for naming schema:
SK8/
├── models/
│   └── skater/
│       │
│       ├── skater_texture.jpg   <-- (The ONE texture for the whole model)
│       │
│       ├── idle_00.obj          <-- Animation 1 (Idle)
│       ├── idle_01.obj
│       ├── idle_02.obj
│       │
│       ├── push_00.obj          <-- Animation 2 (Push)
│       ├── push_01.obj
│       ├── push_02.obj
│       ├── ...
│       │
│       ├── ollie_00.obj         <-- Animation 3 (Ollie)
│       ├── ollie_01.obj
│       ├── ...
│
└── main.cpp
*/

class _AnimatedModel{
    public:
        _AnimatedModel();
        ~_AnimatedModel();

        // loads an animation from sequence of OBJs
        // ex: LoadAnimation("models/skater/push",10);
        // would load push_00.obj, push_01.obj, ..., push_09.obj
        bool LoadAnimation(const char* baseName, int frameCount);

        void FreeModel();

        //render function which does lerp
        void RenderInterpolated(int frameA, int frameB, float interp);

        int GetFrameCount();
    private:
        // flipbook of frames
        std::vector<ObjModel*> m_Frames;
        _textureLoader myTex;
        GLuint modelTexID;
    protected:
};

#endif //_ANIMATEDMODEL_H