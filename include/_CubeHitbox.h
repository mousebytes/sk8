#ifndef _CUBEHITBOX_H
#define _CUBEHITBOX_H

#include"_Collider.h"

class _CubeHitbox : public _Collider{
    public:
    // two corners that define the box
    Vector3 min;
    Vector3 max;

    _CubeHitbox(Vector3 vMin, Vector3 vMax, ColliderType type = COLLIDER_GENERAL);

    virtual void Draw() override;
    virtual _Collider* GetWorldSpaceCollider(const Vector3& pos, const Vector3& scale, const Vector3& rot) override;

    virtual bool CheckCollision(_Collider* other) override;
    virtual bool CheckCollisionWithCube(_CubeHitbox* cube) override;
    virtual bool CheckCollisionWithSphere(_SphereHitbox* sphere) override;
};


#endif //_CUBEHITBOX_H