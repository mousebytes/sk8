#ifndef _ANIMATEDMODEL_H
#define _ANIMATEDMODEL_H

#include<_common.h>
#include"_ObjModel.h"

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

class _AnimatedModel {
public:
    _AnimatedModel();
    ~_AnimatedModel();

    // call this once before registering animations
    bool LoadTexture(char* texpath);

    // loads an animation sequence and stores it with a name (key)
    bool RegisterAnimation(std::string name, const char* baseName, int frameCount);

    void FreeModel();

    // draws an animation given a name & two frames & interp factor
    void Draw(std::string animName, int frameA, int frameB, float interp);

    // gets the frame count for a specific animation
    int GetFrameCount(std::string animName);

private:

    // this map stores all loaded animation sequences by name
    std::map<std::string, std::vector<ObjModel*>> m_Animations;

    _textureLoader myTex;
    GLuint modelTexID;

    // a temporary model to hold the interpolated result
    ObjModel m_ScratchFrame;

protected:
};

#endif //_ANIMATEDMODEL_H