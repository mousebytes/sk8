#include "_Rigidbody.h"

_Rigidbody::_Rigidbody() {
    velocity = Vector3(0,0,0);
    gravity = 9.8f;
    isGrounded = false;
}

_Rigidbody::~_Rigidbody() {
    // destruct
}

void _Rigidbody::Update(Vector3& position) {
    // Apply Gravity
    if(!isGrounded){
        velocity.y -= gravity * _Time::deltaTime;
    }

    // Apply Velocity to Position
    position.x += velocity.x * _Time::deltaTime;
    position.y += velocity.y * _Time::deltaTime;
    position.z += velocity.z * _Time::deltaTime;
}