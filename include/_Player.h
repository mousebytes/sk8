#ifndef _PLAYER_H
#define _PLAYER_H
#include"_camera.h"
#include"_inputs.h"
#include"_common.h"
#include"_AnimatedModelInstance.h"
#include"_StaticModelInstance.h"

class _Player{
    public:
    _Player(_AnimatedModel* modelBlueprint);
    ~_Player();

    // player body
    _AnimatedModelInstance* m_body;

    float m_cameraPitch; // up/down look (x axis rotation)
    float m_playerYaw; // left/right look (y axis rotation)

    float m_moveSpeed;
    float m_mouseSensitivity;

    vector<_StaticModelInstance*> m_collidableStaticModels;
    vector<_AnimatedModelInstance*> m_collidableAnimatedModels;

    bool isFrozen;

    // takes raw input deltas
    void HandleMouse(float deltaX, float deltaY);

    // take key presses
    void HandleKeys(WPARAM wParam);

    // public methods to register colliders
    void RegisterStaticCollider(_StaticModelInstance* model);
    void RegisterAnimatedCollider(_AnimatedModelInstance* model);
    void ClearColliders();

    // handle gravity, collisions, apply velocity
    void UpdatePhysics();

    // set camera position and lookat
    void UpdateCamera(_camera* cam);

    // calls m_body->draw for now
    void Draw();
};


#endif //_PLAYER_H