#ifndef _TARGETMANAGER_H
#define _TARGETMANAGER_H

#include "_AnimatedModel.h"
#include "_AnimatedModelInstance.h"
#include "_SphereHitbox.h"
#include"_common.h"
#include"_Bullets.h"

// hlper struct to manage a target's state
struct _Target {
    _AnimatedModelInstance* instance;
    float lifetime;

    _Target(_AnimatedModelInstance* inst, float life) {
        instance = inst;
        lifetime = life;
    }

    ~_Target() {
        delete instance;
    }
};

class _TargetManager
{
public:
    _TargetManager(_AnimatedModel* targetBlueprint);
    virtual ~_TargetManager();

    // we need to know about the bullet manager to register new targets
    void RegisterBulletManager(_Bullets* bulletManager);
    void RegisterStaticCollider(_StaticModelInstance* model);

    void Update();
    void Draw();

private:
    void SpawnTarget();

    void UpdateTargetPhysics(_Target* target);

    _AnimatedModel* m_blueprint; // the visual model for all targets
    _Bullets* m_bulletManager; // pointer to the scene's bullet manager

    std::vector<_Target*> m_activeTargets;
    // list of things the targets can collide with (ground)
    vector<_StaticModelInstance*> m_collidableStaticModels;

    float m_spawnInterval; // how often (in sec) to spawn a new target
    float m_spawnTimer;    // countdown timer for spawning
    float m_targetLifetime; // how long (in sec) a target lives
};

#endif