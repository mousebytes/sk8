#ifndef _COLLIDER_H
#define _COLLIDER_H

#include "_common.h"

enum ColliderType {
    COLLIDER_GENERAL,
    COLLIDER_FLOOR,
    COLLIDER_WALL,
    COLLIDER_BULLET,
    COLLIDER_TARGET
};

// forward declare the types
class _CubeHitbox;
class _SphereHitbox;

class _Collider {
public:
    _Collider() : m_type(COLLIDER_GENERAL) {}
    virtual ~_Collider() {}

    ColliderType m_type;

    // draws the colliders wireframe
    virtual void Draw() = 0;

    // creates & returns a new temp worldspace
    // version of this collider, the caller
    // should delete this new collider
    virtual _Collider* GetWorldSpaceCollider(const Vector3& pos, const Vector3& scale, const Vector3& rot) = 0;

    // primary collision check
    virtual bool CheckCollision(_Collider* other) = 0;

    // type specific collision check
    virtual bool CheckCollisionWithCube(_CubeHitbox* cube) = 0;
    virtual bool CheckCollisionWithSphere(_SphereHitbox* sphere) = 0;
};

#endif // _COLLIDER_H
