#include"_Bullets.h"

//a helper struct to manage bullet's state
// managed by _Bullets

struct _Bullet{
    _StaticModelInstance* instance;
    Vector3 velocity;
    // time in seconds before bullet is destroyed
    float lifetime;

    // creates a new bullet with blueprint, world space pos
    // velocity (dir*speed) and how long the bullet should live
    _Bullet(_StaticModel* blueprint, Vector3 pos, Vector3 vel, float life, float yaw, float pitch){
        instance = new _StaticModelInstance(blueprint);
        instance->pos = pos;
        velocity=vel;
        lifetime=life;

        instance->rotation.x = -pitch;
        instance->rotation.y = yaw;
        instance->rotation.z = 0.0f; // No roll

        // DEBUG change scale of bullets here
        instance->scale = Vector3(0.2,0.2,0.2);

        instance->AddCollider(new _SphereHitbox(Vector3(0,0,0), 0.5f, COLLIDER_BULLET));
    }
    
    ~_Bullet(){
        delete instance;
    }

    // update bullets pos & lifetime, true if expired false otherwise
    bool Update(){
        // apply velocity & no gravity
        instance->pos.x += velocity.x * _Time::deltaTime;
        instance->pos.y += velocity.y * _Time::deltaTime;
        instance->pos.z += velocity.z * _Time::deltaTime;

        // countdown lifetime
        lifetime-=_Time::deltaTime;
        return lifetime<=0.0f;
    }

    // draw bullets model instance
    void Draw(){
        instance->Draw();
    }
};


_Bullets::_Bullets(_StaticModel* bulletBlueprint){
    m_blueprint = bulletBlueprint;
    // units per second
    m_bulletSpeed=150.0f;
    // lifetime in seconds
    m_bulletLifetime=3.0f;
}

_Bullets::~_Bullets(){
    for(_Bullet* bullet : m_activeBullets){
        delete bullet;
    }
    m_activeBullets.clear();

    // clear lists (but don't delete pointers, scene owns them)
    m_staticTargets.clear();
    m_animatedTargets.clear();
}

void _Bullets::RegisterStaticTarget(_StaticModelInstance* model)
{
    m_staticTargets.push_back(model);
}

void _Bullets::RegisterAnimatedTarget(_AnimatedModelInstance* model)
{
    m_animatedTargets.push_back(model);
}

void _Bullets::UnregisterAnimatedTarget(_AnimatedModelInstance* model)
{
    // find and remove the model from our target list
    m_animatedTargets.erase(
        remove(m_animatedTargets.begin(), m_animatedTargets.end(), model),
        m_animatedTargets.end()
    );
}

void _Bullets::ClearTargets()
{
    m_staticTargets.clear();
    m_animatedTargets.clear();
}

void _Bullets::Fire(Vector3 startPos, Vector3 direction){
    // calc vel
    direction.normalize();
    Vector3 velocity = direction*m_bulletSpeed;

    const float RAD_TO_DEG = 180.0f / PI;

    // calculate yaw (rotation around the y axis)
    float yaw = atan2(direction.x, direction.z) * RAD_TO_DEG;

    // calculate Pitch (rotation around the x axis)
    // compare 'y' (height) to the
    // length of the vector on the XZ plane
    float xzLength = sqrt(direction.x * direction.x + direction.z * direction.z);
    float pitch = atan2(direction.y, xzLength) * RAD_TO_DEG;

    // create & add the new bullet to our active list
    _Bullet* newBullet = new _Bullet(m_blueprint,startPos,velocity,m_bulletLifetime,yaw,pitch);
    m_activeBullets.push_back(newBullet);
}

void _Bullets::Update(){
    // iterate & update all bullets
    for(auto it = m_activeBullets.begin(); it!=m_activeBullets.end();){
        _Bullet* bullet = *it;

        // check life & update pos
        bool isDead = bullet->Update();
        bool hitTarget=false;

        if (!bullet->instance->colliders.empty())
        {
            // get the bullet's single world-space collider
            _Collider* bulletWorldCol = bullet->instance->colliders[0]->GetWorldSpaceCollider(
                                            bullet->instance->pos,
                                            bullet->instance->scale,
                                            bullet->instance->rotation);

            if (bulletWorldCol)
            {
                // check against all registered static targets
                for (_StaticModelInstance* target : m_staticTargets)
                {
                    // check against all colliders on that target
                    for (_Collider* targetModelCol : target->colliders)
                    {
                        // we only care about colliders of type COLLIDER_TARGET
                        if (targetModelCol->m_type == COLLIDER_TARGET)
                        {
                            // get the target's world space collider
                            _Collider* targetWorldCol = targetModelCol->GetWorldSpaceCollider(
                                                            target->pos,
                                                            target->scale,
                                                            target->rotation);

                            if (targetWorldCol)
                            {
                                //  collision check
                                if (bulletWorldCol->CheckCollision(targetWorldCol))
                                {
                                    hitTarget = true; // mark bullet for deletion

                                    // --- TODO: FUNCTIONALITY HERE ---
                                    target->pos.y += 1.0f;
                                }
                                delete targetWorldCol; // clean up temp target collider
                            }
                        }
                        if (hitTarget) break; // stop checking colliders on this target
                    }
                    if (hitTarget) break; // stop checking other targets
                }

                // animated instance hit checking
                if(!hitTarget) // only check if we haven't hit a static target
                {
                    for (_AnimatedModelInstance* target : m_animatedTargets)
                    {
                        for (_Collider* targetModelCol : target->colliders)
                        {
                            if (targetModelCol->m_type == COLLIDER_TARGET)
                            {
                                _Collider* targetWorldCol = targetModelCol->GetWorldSpaceCollider(
                                                                target->pos,
                                                                target->scale,
                                                                target->rotation);
                                if (targetWorldCol)
                                {
                                    if (bulletWorldCol->CheckCollision(targetWorldCol))
                                    {
                                        hitTarget = true; // mark bullet for deletion
                                        // set its hit flag
                                        target->isHit = true; 
                                    }
                                    delete targetWorldCol;
                                }
                            }
                            if (hitTarget) break;
                        }
                        if (hitTarget) break;
                    }
                }

                delete bulletWorldCol; // clean up temp bullet collider
            }
        }

        if(isDead || hitTarget){
            // delete bullet obj & its instance
            delete bullet;
            it=m_activeBullets.erase(it);
        }else{
            // next item
            ++it;
        }
    }
}

void _Bullets::Draw(){
    for(_Bullet* bullet : m_activeBullets){
        bullet->Draw();
    }
}