#include "_SphereHitbox.h"
#include "_CubeHitbox.h" 

_SphereHitbox::_SphereHitbox(Vector3 center, float radius, ColliderType type) {
    this->center = center;
    this->radius = radius;
    m_type = type;
}

void _SphereHitbox::Draw() {
    // draw wireframe sphere
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    

    glPushMatrix();
    glColor4f(0.0f, 1.0f, 0.0f,0.5f); // Green
    glTranslatef(center.x, center.y, center.z);

    //glutSolidSphere()

    if(colliderDrawFace){
        glutSolidSphere(radius,16,16);
    }else{
        const int segments = 16;

        // XY Circle
        glBegin(GL_LINE_LOOP);
        for(int i = 0; i < segments; i++) {
            float theta = 2.0f * PI * float(i) / float(segments);
            float x = radius * cosf(theta);
            float y = radius * sinf(theta);
            glVertex3f(x, y, 0);
        }
        glEnd();

        // XZ Circle
        glBegin(GL_LINE_LOOP);
        for(int i = 0; i < segments; i++) {
            float theta = 2.0f * PI * float(i) / float(segments);
            float x = radius * cosf(theta);
            float z = radius * sinf(theta);
            glVertex3f(x, 0, z);
        }
        glEnd();

        // YZ Circle
        glBegin(GL_LINE_LOOP);
        for(int i = 0; i < segments; i++) {
            float theta = 2.0f * PI * float(i) / float(segments);
            float y = radius * cosf(theta);
            float z = radius * sinf(theta);
            glVertex3f(0, y, z);
        }
    glEnd();
    }
    
    

    glPopMatrix();
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glColor4f(1.0f, 1.0f, 1.0f,1.0f);
}

_Collider* _SphereHitbox::GetWorldSpaceCollider(const Vector3& pos, const Vector3& scale, const Vector3& rot) {
    // rotation is irrelevant for a sphere, so we ignore rot
    
    // apply scale to the center offset
    Vector3 scaledCenter = Vector3(center.x * scale.x, center.y * scale.y, center.z * scale.z);

    // apply pos
    Vector3 worldCenter = scaledCenter + pos;
    
    // apply scale to radius, we take the largest scale component
    // to ensure the sphere fully encloses the scaled model space
    float worldRadius = radius * std::max({scale.x, scale.y, scale.z});
    
    // return a new, temporary world space sphere
    return new _SphereHitbox(worldCenter, worldRadius, this->m_type);
}


bool _SphereHitbox::CheckCollision(_Collider* other) {
    return other->CheckCollisionWithSphere(this);
}

bool _SphereHitbox::CheckCollisionWithCube(_CubeHitbox* cube) {
    return cube->CheckCollisionWithSphere(this);
}

bool _SphereHitbox::CheckCollisionWithSphere(_SphereHitbox* sphere) {
    
    float distSq = pow(this->center.x - sphere->center.x, 2) +
                   pow(this->center.y - sphere->center.y, 2) +
                   pow(this->center.z - sphere->center.z, 2);

    float radiiSum = this->radius + sphere->radius;
    float radiiSumSq = radiiSum * radiiSum;

    // compare
    return distSq < radiiSumSq;
}