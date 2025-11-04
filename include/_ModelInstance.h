#ifndef _MODELINSTANCE_H
#define _MODELINSTANCE_H

#include<_MD2Model.h>
#include<_common.h>
#include"_AnimatedModel.h"

class _ModelInstance{
    public:
        //_ModelInstance(_MD2Model* modelAsset);
        _ModelInstance(_AnimatedModel* modelAsset);
        ~_ModelInstance();

        // animation logic
        void Update();
        // translation/rotation/drawing
        void Draw();

        void SetAnimation(int start,int end);
        void Actions(); 

        enum { STAND, WALKLEFT, WALKRIGHT, RUN, JUMP, ATTACK };
        int actionTrigger;

        vec3 pos;
        float dirAngleZ;
    private:

    //_MD2Model* blueprint;
    _AnimatedModel* blueprint;

    int currentFrame;
    float interp;
    int startFrame;
    int endFrame;

    void Animate(int start, int end, int* frame, float* interp);
    protected:
};


#endif // _MODELINSTANCE_H
