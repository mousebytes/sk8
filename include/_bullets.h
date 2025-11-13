#ifndef _BULLETS_H
#define _BULLETS_H

#include "_common.h"
#include "_StaticModelInstance.h"
#include"_AnimatedModelInstance.h"
#include"_SphereHitbox.h";
#include"_CubeHitbox.h"

// forward declare helper struct
struct _Bullet;

class _Bullets{
    public:
        _Bullets(_StaticModel* bulletBlueprint);
        ~_Bullets();

        // spawns a new bullet at pos
        // and the direction the bullet should travel
        void Fire(Vector3 startPos, Vector3 direction);
        // updates all active bullets (pos,lifetime)
        void Update();

        //draw all active bullets
        void Draw();

        // methods to register potential targets
        void RegisterStaticTarget(_StaticModelInstance* model);
        void RegisterAnimatedTarget(_AnimatedModelInstance* model);
        void ClearTargets();
        void UnregisterAnimatedTarget(_AnimatedModelInstance* model);

    private:
        _StaticModel* m_blueprint; // we don't own this _scene does
        list<_Bullet*> m_activeBullets;

        float m_bulletSpeed;
        float m_bulletLifetime;

        // models the bullets can collide with
        std::vector<_StaticModelInstance*> m_staticTargets;
        std::vector<_AnimatedModelInstance*> m_animatedTargets;
};



#endif // _BULLETS_H