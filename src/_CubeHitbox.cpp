#include"_CubeHitbox.h"
#include"_SphereHitbox.h"

bool isDebug = false;
bool colliderDrawFace = false;

_CubeHitbox::_CubeHitbox(Vector3 vMin, Vector3 vMax, ColliderType type) {
    min = vMin;
    max = vMax;
    m_type = type;
}

void _CubeHitbox::Draw() {
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glColor4f(1.0f, 0, 0,0.5f);

    if (colliderDrawFace) {
        // draw all 6 faces of the cube as solid quads
        glBegin(GL_QUADS);
            // top face (Y+)
            glNormal3f(0.0f, 1.0f, 0.0f);
            glVertex3f(min.x, max.y, min.z); // top left near
            glVertex3f(max.x, max.y, min.z); // top right near
            glVertex3f(max.x, max.y, max.z); // top right far
            glVertex3f(min.x, max.y, max.z); // top left far

            // bottom face (Y-)
            glNormal3f(0.0f, -1.0f, 0.0f);
            glVertex3f(min.x, min.y, min.z); // bottom left near
            glVertex3f(min.x, min.y, max.z); // bottom left far
            glVertex3f(max.x, min.y, max.z); // bottom right far
            glVertex3f(max.x, min.y, min.z); // bottom right near

            // front face (Z+)
            glNormal3f(0.0f, 0.0f, 1.0f);
            glVertex3f(min.x, min.y, max.z); // bottom left
            glVertex3f(min.x, max.y, max.z); // top left
            glVertex3f(max.x, max.y, max.z); // top right
            glVertex3f(max.x, min.y, max.z); // bottom right

            // back face (Z-)
            glNormal3f(0.0f, 0.0f, -1.0f);
            glVertex3f(min.x, min.y, min.z); // bottom left
            glVertex3f(max.x, min.y, min.z); // bottom right
            glVertex3f(max.x, max.y, min.z); // top right
            glVertex3f(min.x, max.y, min.z); // top left

            // left face (X-)
            glNormal3f(-1.0f, 0.0f, 0.0f);
            glVertex3f(min.x, min.y, min.z); // bottom near
            glVertex3f(min.x, max.y, min.z); // top near
            glVertex3f(min.x, max.y, max.z); // top far
            glVertex3f(min.x, min.y, max.z); // bottom far

            // right face (X+)
            glNormal3f(1.0f, 0.0f, 0.0f);
            glVertex3f(max.x, min.y, min.z); // bottom near
            glVertex3f(max.x, min.y, max.z); // bottom far
            glVertex3f(max.x, max.y, max.z); // top far
            glVertex3f(max.x, max.y, min.z); // top near
            
        glEnd();
    }
    else{
        // Top face
        glBegin(GL_LINE_LOOP);
            glVertex3f(min.x, max.y, min.z);
            glVertex3f(max.x, max.y, min.z);
            glVertex3f(max.x, max.y, max.z);
            glVertex3f(min.x, max.y, max.z);
        glEnd();

        // Bottom face
        glBegin(GL_LINE_LOOP);
            glVertex3f(min.x, min.y, min.z);
            glVertex3f(max.x, min.y, min.z);
            glVertex3f(max.x, min.y, max.z);
            glVertex3f(min.x, min.y, max.z);
        glEnd();

    

    // Connecting vertical lines 
    glBegin(GL_LINES);
        glVertex3f(min.x, min.y, min.z); glVertex3f(min.x, max.y, min.z);
        glVertex3f(max.x, min.y, min.z); glVertex3f(max.x, max.y, min.z);
        glVertex3f(max.x, min.y, max.z); glVertex3f(max.x, max.y, max.z);
        glVertex3f(min.x, min.y, max.z); glVertex3f(min.x, max.y, max.z);
    glEnd();
    }

    //glColor3f(1.0f, 0.0f, 0.0f); // Red
    
    

    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glColor4f(1.0f, 1.0f, 1.0f,1.0f);
}

_Collider* _CubeHitbox::GetWorldSpaceCollider(const Vector3& pos, const Vector3& scale, const Vector3& rot) {
    // cubic hitboxes do not support rotation.
    // will only apply position and scale.
    
    // apply scale
    Vector3 scaledMin = Vector3(min.x * scale.x, min.y * scale.y, min.z * scale.z);
    Vector3 scaledMax = Vector3(max.x * scale.x, max.y * scale.y, max.z * scale.z);

    // apply pos
    Vector3 worldMin = scaledMin + pos;
    Vector3 worldMax = scaledMax + pos;
    
    // return a new, temporary world space cube hitbox
    return new _CubeHitbox(worldMin, worldMax, this->m_type);;
}


bool _CubeHitbox::CheckCollision(_Collider* other) {
    return other->CheckCollisionWithCube(this);
}

bool _CubeHitbox::CheckCollisionWithCube(_CubeHitbox* cube) {
    
    // check x axis for separation
    if (this->max.x < cube->min.x || this->min.x > cube->max.x) {
        return false;
    }
    // check y axis for separation
    if (this->max.y < cube->min.y || this->min.y > cube->max.y) {
        return false;
    }
    // check z axis for separation
    if (this->max.z < cube->min.z || this->min.z > cube->max.z) {
        return false;
    }
    // no separation found they are overlapping
    return true;
}

bool _CubeHitbox::CheckCollisionWithSphere(_SphereHitbox* sphere) {
    // cube vs sphere logic
    
    // find the point on the cube closest to the sphere's center
    float closestX = std::max(this->min.x, std::min(sphere->center.x, this->max.x));
    float closestY = std::max(this->min.y, std::min(sphere->center.y, this->max.y));
    float closestZ = std::max(this->min.z, std::min(sphere->center.z, this->max.z));

    // get distance squared between this closest point and the sphere center
    float distSq = pow(closestX - sphere->center.x, 2) +
                   pow(closestY - sphere->center.y, 2) +
                   pow(closestZ - sphere->center.z, 2);

    // if the distance squared is less than the radius squared, they collide
    return distSq < (sphere->radius * sphere->radius);
}