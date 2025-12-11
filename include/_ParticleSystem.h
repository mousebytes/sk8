#ifndef _PARTICLESYSTEM_H
#define _PARTICLESYSTEM_H

#include "_common.h"

struct Particle {
    Vector3 pos;
    Vector3 velocity;
    Vector3 color;     // RGB
    float life;        // Remaining life in seconds
    float maxLife;     // Total life duration
    float size;
    bool useGravity;   // Should gravity affect this particle?
};

class _ParticleSystem {
public:
    _ParticleSystem();
    ~_ParticleSystem();

    void Init();
    void Update();
    void Draw();

    // Specific Emitters
    void EmitSparks(Vector3 pos, int count);
    void EmitDust(Vector3 pos, Vector3 playerVel, int count);

private:
    std::vector<Particle> m_particles;
    
    // Helper to get random float between min and max
    float RandomFloat(float min, float max);
};

#endif // _PARTICLESYSTEM_H