#ifndef _ANIMATEDMODELINSTANCE_H
#define _ANIMATEDMODELINSTANCE_H

#include<_common.h>
#include"_AnimatedModel.h"
#include"_Collider.h"
#include"_StaticModelInstance.h"
#include"_CubeHitbox.h"
#include"_SphereHitbox.h"


class _AnimatedModelInstance{
    public:
        //_AnimatedModelInstance(_MD2Model* modelAsset);
        _AnimatedModelInstance(_AnimatedModel* modelAsset);
        ~_AnimatedModelInstance();

        // animation logic
        void Update();
        // translation/rotation/drawing
        void Draw();

        void PlayAnimation(string name, float speed = 1.0f);

        enum { STAND, WALKLEFT, WALKRIGHT, RUN, JUMP, ATTACK };
        int actionTrigger;

        Vector3 pos;
        Vector3 rotation;
        Vector3 scale;

        Vector3 velocity;
        float gravity;

        bool isGrounded;
        bool isHit;


        vector<_Collider*> colliders;
        void AddCollider(_Collider* collider);
        void DrawColliders();
        // checks collisions against static model instances
        bool CheckCollision(_StaticModelInstance* other);
    private:

    //_MD2Model* blueprint;
    _AnimatedModel* blueprint;

    string m_currentAnimationName;
    int m_currentFrame;
    float m_interp;
    float m_animationSpeed;

    protected:
};


#endif //_ANIMATEDMODELINSTANCE_H
