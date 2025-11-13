#include "_Hitbox.h"
// extern var that's defined in _common.h but has gotta be defined in a .cpp
// and this just makes the most sense to me
//bool isDebug=true;
_Hitbox::_Hitbox() {
    // Default 1x1x1 cube
    min = Vector3(-0.5f, -0.5f, -0.5f);
    max = Vector3(0.5f, 0.5f, 0.5f);
}

_Hitbox::_Hitbox(Vector3 vMin, Vector3 vMax) {
    min = vMin;
    max = vMax;
}

void _Hitbox::Draw() {

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    glColor3f(1.0f, 0.0f, 0.0f);
    
    // Draw the 12 edges of the box
    
    // Bottom face (4 lines)
    glBegin(GL_LINE_LOOP);
        glVertex3f(min.x, min.y, min.z);
        glVertex3f(max.x, min.y, min.z);
        glVertex3f(max.x, min.y, max.z);
        glVertex3f(min.x, min.y, max.z);
    glEnd();

    // Top face (4 lines)
    glBegin(GL_LINE_LOOP);
        glVertex3f(min.x, max.y, min.z);
        glVertex3f(max.x, max.y, min.z);
        glVertex3f(max.x, max.y, max.z);
        glVertex3f(min.x, max.y, max.z);
    glEnd();

    // Connecting vertical lines 
    glBegin(GL_LINES);
        glVertex3f(min.x, min.y, min.z);
        glVertex3f(min.x, max.y, min.z);

        glVertex3f(max.x, min.y, min.z);
        glVertex3f(max.x, max.y, min.z);

        glVertex3f(max.x, min.y, max.z);
        glVertex3f(max.x, max.y, max.z);

        glVertex3f(min.x, min.y, max.z);
        glVertex3f(min.x, max.y, max.z);
    glEnd();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
}

bool _Hitbox::isColliding(_Hitbox& other)
{
    // the collision test is true if the boxes overlap on all 3 axes
    // a collision is absent if one box is fully to one side of the other
    // on any single axis

    // check x axis for separation
    if (this->max.x < other.min.x || this->min.x > other.max.x) {
        return false;
    }

    // check y axis for separation
    if (this->max.y < other.min.y || this->min.y > other.max.y) {
        return false;
    }

    // check z axis for separation
    if (this->max.z < other.min.z || this->min.z > other.max.z) {
        return false;
    }

    // if no separation was found on any axis, they must be overlapping
    return true;
}

bool _Hitbox::isPointInside(Vector3 point)
{
    // check if the point is within the min and max bounds on all 3 axes
    return (point.x >= this->min.x && point.x <= this->max.x &&
            point.y >= this->min.y && point.y <= this->max.y &&
            point.z >= this->min.z && point.z <= this->max.z);
}