#ifndef _SPHEREHITBOX_H
#define _SPHEREHITBOX_H

#include "_Collider.h"

class _SphereHitbox : public _Collider {
public:
    Vector3 center; // model space center (offset from origin)
    float radius;

    _SphereHitbox(Vector3 center, float radius, ColliderType type = COLLIDER_GENERAL);
    
    virtual void Draw() override;
    virtual _Collider* GetWorldSpaceCollider(const Vector3& pos, const Vector3& scale, const Vector3& rot) override;

    virtual bool CheckCollision(_Collider* other) override;
    virtual bool CheckCollisionWithCube(_CubeHitbox* aabb) override;
    virtual bool CheckCollisionWithSphere(_SphereHitbox* sphere) override;
};

#endif // _SPHEREHITBOX_H