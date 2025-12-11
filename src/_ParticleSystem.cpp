#include "_ParticleSystem.h"

_ParticleSystem::_ParticleSystem() {
    // Reserve memory to prevent frequent reallocations
    m_particles.reserve(1000);
}

_ParticleSystem::~_ParticleSystem() {
    m_particles.clear();
}

void _ParticleSystem::Init() {
    // Setup initial state if needed (textures, etc.)
}

float _ParticleSystem::RandomFloat(float min, float max) {
    // i hate having to use static cast for this kinda stuff
    return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
}

void _ParticleSystem::EmitSparks(Vector3 pos, int count) {
    for(int i = 0; i < count; i++) {
        Particle p;
        p.pos = pos;
        
        // Random velocity bursting upwards and outwards
        p.velocity.x = RandomFloat(-2.0f, 2.0f);
        p.velocity.y = RandomFloat(2.0f, 5.0f);  // Upward burst
        p.velocity.z = RandomFloat(-2.0f, 2.0f);
        
        p.color = Vector3(1.0f, 0.8f, 0.0f);
        p.maxLife = RandomFloat(0.3f, 0.6f); // Short life
        p.life = p.maxLife;
        p.size = RandomFloat(0.05f, 0.1f);
        p.useGravity = true;

        m_particles.push_back(p);
    }
}

void _ParticleSystem::EmitDust(Vector3 pos, Vector3 playerVel, int count) {
    for(int i = 0; i < count; i++) {
        Particle p;
        
        // Spawn slightly randomized around the wheel area
        p.pos.x = pos.x + RandomFloat(-0.2f, 0.2f);
        p.pos.y = pos.y + RandomFloat(0.0f, 0.1f);
        p.pos.z = pos.z + RandomFloat(-0.2f, 0.2f);

        // Dust moves slightly opposite to player or just puffs up
        p.velocity.x = -playerVel.x * 0.2f + RandomFloat(-0.5f, 0.5f);
        p.velocity.y = RandomFloat(0.1f, 0.5f);
        p.velocity.z = -playerVel.z * 0.2f + RandomFloat(-0.5f, 0.5f);

        p.color = Vector3(0.9f, 0.9f, 0.9f); // White/Grey
        p.maxLife = RandomFloat(0.5f, 1.0f);
        p.life = p.maxLife;
        p.size = RandomFloat(0.05f, 0.15f);
        p.useGravity = false; // Dust floats

        m_particles.push_back(p);
    }
}

void _ParticleSystem::Update() {
    for (auto it = m_particles.begin(); it != m_particles.end();) {
        // update Age
        it->life -= _Time::deltaTime;

        if (it->life <= 0.0f) {
            it = m_particles.erase(it);
            continue;
        }

        // Apply Physics
        it->pos.x += it->velocity.x * _Time::deltaTime;
        it->pos.y += it->velocity.y * _Time::deltaTime;
        it->pos.z += it->velocity.z * _Time::deltaTime;

        if(it->useGravity) {
            it->velocity.y -= 9.8f * _Time::deltaTime;
        }

        ++it;
    }
}

void _ParticleSystem::Draw() {
    glDisable(GL_LIGHTING);       // particles glow/are unlit
    glDisable(GL_TEXTURE_2D);     // we are drawing simple colored squares
    glDisable(GL_CULL_FACE);
    
    // enable blending for transparency later
    // glEnable(GL_BLEND); 
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glBegin(GL_QUADS);
    for (Particle& p : m_particles) {
        
        float lifeRatio = p.life / p.maxLife;

        // --- COLOR LOGIC ---
        if (p.useGravity) {
            // SPARKS: fade to white
            // Interpolate from Color -> 1,1,1
            float r = 1.0f + (p.color.x - 1.0f) * lifeRatio;
            float g = 1.0f + (p.color.y - 1.0f) * lifeRatio;
            float b = 1.0f + (p.color.z - 1.0f) * lifeRatio;
            glColor3f(r, g, b);
        } else {
            // DUST: fade to black
            // Interpolate from 0,0,0 -> Color (Scaling by lifeRatio does this)
            glColor3f(p.color.x * lifeRatio, p.color.y * lifeRatio, p.color.z * lifeRatio);
        }

        float s = p.size;
        
        // FRONT FACE (XY Plane)
        glVertex3f(p.pos.x - s, p.pos.y - s, p.pos.z);
        glVertex3f(p.pos.x + s, p.pos.y - s, p.pos.z);
        glVertex3f(p.pos.x + s, p.pos.y + s, p.pos.z);
        glVertex3f(p.pos.x - s, p.pos.y + s, p.pos.z);
        
        // SIDE FACE (YZ Plane)
        glVertex3f(p.pos.x, p.pos.y - s, p.pos.z - s);
        glVertex3f(p.pos.x, p.pos.y - s, p.pos.z + s);
        glVertex3f(p.pos.x, p.pos.y + s, p.pos.z + s);
        glVertex3f(p.pos.x, p.pos.y + s, p.pos.z - s);

        // TOP FACE (XZ Plane)
        glVertex3f(p.pos.x - s, p.pos.y, p.pos.z - s);
        glVertex3f(p.pos.x + s, p.pos.y, p.pos.z - s);
        glVertex3f(p.pos.x + s, p.pos.y, p.pos.z + s);
        glVertex3f(p.pos.x - s, p.pos.y, p.pos.z + s);
    }
    glEnd();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
    glColor3f(1, 1, 1);
}