#ifndef _RIGIDBODY_H
#define _RIGIDBODY_H

#include "_common.h"

class _Rigidbody {
public:
    _Rigidbody();
    virtual ~_Rigidbody();

    Vector3 velocity;
    float gravity;
    bool isGrounded;

    // Updates position based on velocity and gravity
    void Update(Vector3& position);
};

#endif // _RIGIDBODY_H