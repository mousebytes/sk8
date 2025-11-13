#include"_TargetManager.h"

_TargetManager::_TargetManager(_AnimatedModel* targetBlueprint){
    m_blueprint=targetBlueprint;
    m_bulletManager=nullptr;

    m_spawnInterval=1.0f;
    m_spawnTimer=m_spawnInterval;
    m_targetLifetime=10.0f;
}

_TargetManager::~_TargetManager(){
    for(_Target* target : m_activeTargets){
        delete target;
    }
    m_activeTargets.clear();
    m_collidableStaticModels.clear();
}

void _TargetManager::RegisterBulletManager(_Bullets* bulletManager){
    m_bulletManager = bulletManager;
}

void _TargetManager::RegisterStaticCollider(_StaticModelInstance* model)
{
    m_collidableStaticModels.push_back(model);
}

void _TargetManager::SpawnTarget(){
    // can't spawn if don't know bullet manager
    if(!m_bulletManager) return;

    _AnimatedModelInstance* newInstance = new _AnimatedModelInstance(m_blueprint);

    // spawn at a random X,Z position
    float randX = (rand()%5)-2.5f;
    float randZ = (rand()%5)-2.5f;
    newInstance->pos=Vector3(randX,10.0f,randZ);

    newInstance->AddCollider(new _SphereHitbox(Vector3(0,0,0),1.0f,COLLIDER_TARGET));

    //custom gravity factor
    newInstance->gravity = 2.5f;

    // tell target to play default animation
    newInstance->PlayAnimation("idle", 1.0f);

    _Target* newTarget = new _Target(newInstance,m_targetLifetime);

    m_activeTargets.push_back(newTarget);

    m_bulletManager->RegisterAnimatedTarget(newInstance);
}

void _TargetManager::UpdateTargetPhysics(_Target* target)
{
    _AnimatedModelInstance* targetInstance = target->instance;

    // only check if the target has a collider
    if (targetInstance->colliders.empty()) return;

    _Collider* targetMainCollider = targetInstance->colliders[0];

    // get the target's collider at its current world position
    _Collider* targetCurrentCollider = targetMainCollider->GetWorldSpaceCollider(
                                            targetInstance->pos,
                                            targetInstance->scale,
                                            targetInstance->rotation);

    if (targetCurrentCollider)
    {
        // check against all registered static colliders (the terrain)
        for(_StaticModelInstance* staticModel : m_collidableStaticModels)
        {
            for (_Collider* staticCollider : staticModel->colliders)
            {
                _Collider* staticWorldCollider = staticCollider->GetWorldSpaceCollider(
                                                    staticModel->pos,
                                                    staticModel->scale,
                                                    staticModel->rotation);
                if (staticWorldCollider)
                {
                    // check for collision
                    if (targetCurrentCollider->CheckCollision(staticWorldCollider))
                    {
                        // check if it's a floor
                        if (staticCollider->m_type == COLLIDER_FLOOR)
                        {
                            targetInstance->isGrounded = true; // <-- THE FIX!

                            if(targetInstance->velocity.y < 0) { // if falling
                                targetInstance->velocity.y = 0; // stop falling
                            }
                        }
                    }
                    delete staticWorldCollider;
                }
                if (targetInstance->isGrounded) break;
            }
            if (targetInstance->isGrounded) break;
        }
        delete targetCurrentCollider;
    }
}

void _TargetManager::Update(){
    // spawn logic
    // only tick the timer if the target list is empty
    // can remove this outer if loop
    if (m_activeTargets.empty())
    {
        m_spawnTimer -= _Time::deltaTime;
        if(m_spawnTimer <= 0.0f){
            SpawnTarget();
            // reset timer
            m_spawnTimer = m_spawnInterval;
        }
    }

    if(m_activeTargets.empty()) return;
    // update & deletion logic
    for(auto it = m_activeTargets.begin(); it!=m_activeTargets.end();){
        _Target* target = *it;

        // apply gravity & movement
        target->instance->Update();

        // check for ground collision (updates model's isGrounded flag)
        UpdateTargetPhysics(target);

        target->lifetime -= _Time::deltaTime;

        // check for deletion conditions
        bool isDead = false;

        if(target->lifetime<=0.0f) isDead = true; // lifetime expired
        if(target->instance->isHit) isDead=true; // shot
        if(target->instance->isGrounded) isDead=true; // hit floor

        if(isDead){
            // unregister from bullet manager
            if(m_bulletManager){
                m_bulletManager->UnregisterAnimatedTarget(target->instance);
            }

            delete target;
            it = m_activeTargets.erase(it);
        }
        else{
            ++it;
        }
    }
}

void _TargetManager::Draw(){
    for(_Target* target : m_activeTargets){
        target->instance->Draw();
    }
}
