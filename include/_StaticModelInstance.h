#ifndef _STATICMODELINSTANCE_H
#define _STATICMODELINSTANCE_H

#include "_StaticModel.h" // Needs the blueprint
#include "_common.h"
#include"_Collider.h"

class _StaticModelInstance {
public:
    _StaticModelInstance(_StaticModel* modelAsset);
    ~_StaticModelInstance();

    void Draw();

    // each instance should have its own pos and rot
    Vector3 pos;
    Vector3 scale;
    Vector3 rotation;

    vector<_Collider*> colliders;

    

    void SetPushable(bool flag);
    void Push(float x, float y, float z);
    void SetRotatable(bool flag);
    void Rotate(float x, float y, float z);

    void AddCollider(_Collider* collider);
    void DrawColliders();
    bool CheckCollision(_StaticModelInstance *other);

    // deprecated funcs
    void RotateSmoothly(float x, float y, float z, float interp);

    

private:
    _StaticModel* blueprint;

    // can this thing move
    bool isPushable;
    // can this thing rotate
    bool isRotatable;

};

#endif // _STATICMODELINSTANCE_H
