#include "_Player.h"

_Player::_Player(_AnimatedModel* modelBlueprint)
{
    m_body = new _AnimatedModelInstance(modelBlueprint);
    
    // set up the player's physics collider
    // add sphere collider centered at (0,0,0) local space & r=1.0
    // note: model is norm -1 to +1
    m_body->AddCollider(new _SphereHitbox(Vector3(0,0,0), 1.0f));
    
    // set initial position
    m_body->pos = Vector3(0, 1, 0); // start slightly above ground

    // init player brain variables
    m_cameraPitch = 20.0f;  // Start camera slightly angled down
    m_playerYaw = 0.0f;     // Player starts facing forward
    m_cameraYaw = 0.0f;     // Camera starts directly behind player
    
    // --- SKATE PHYSICS VARS ---
    m_acceleration = 30.0f; // units per second^2
    m_maxSpeed = 500.0f;     // units per second
    m_turnSpeed = 720.0f;   // degrees per second
    m_friction = 1.0f;      // Damping factor (higher = stops faster)
    m_jumpForce = 8.0f;     // Initial upward velocity
    m_state = STATE_AIR;    // Start in air, will be set to grounded by physics

    // --- CAMERA VARS ---
    m_camDistance = 8.0f;   // 8 units away from player
    m_camHeight = 2.0f;     // looks at a point 2 units above player's origin
    m_mouseSensitivity = 0.1f;

    isFrozen = false;
}

_Player::~_Player()
{
    delete m_body;
}

void _Player::HandleMouse(float deltaX, float deltaY)
{
    if(isFrozen) return;
    
    // Mouse X controls the camera's YAW (horizontal orbit)
    m_cameraYaw -= deltaX * m_mouseSensitivity;

    // Mouse Y controls the camera's PITCH (vertical orbit)
    m_cameraPitch += deltaY * m_mouseSensitivity;

    // clamp vertical rotation
    if(m_cameraPitch > 89.0f) {
        m_cameraPitch = 89.0f;
    }
    if(m_cameraPitch < 5.0f) { // Don't let camera go below the ground
        m_cameraPitch = 5.0f;
    }
}

void _Player::HandleKeys(WPARAM wParam)
{
    // calculate forward vector based on player's yaw
    Vector3 fwd;
    fwd.x = -sin(m_playerYaw * PI / 180.0);
    fwd.z = -cos(m_playerYaw * PI / 180.0);
    fwd.normalize(); // Ensure it's a unit vector

    // check which key was pressed
    if (wParam == 'W') // Accelerate
    {
        // Add force in the forward direction
        m_body->velocity.x += fwd.x * m_acceleration * _Time::deltaTime;
        m_body->velocity.z += fwd.z * m_acceleration * _Time::deltaTime;
    }
    if (wParam == 'S') // Brake / Reverse
    {
         // Add force in the backward direction
        m_body->velocity.x -= fwd.x * m_acceleration * _Time::deltaTime;
        m_body->velocity.z -= fwd.z * m_acceleration * _Time::deltaTime;
    }
    if (wParam == 'A') // Turn Left
    {
        m_playerYaw += m_turnSpeed * _Time::deltaTime;
    }
    if (wParam == 'D') // Turn Right
    {
        m_playerYaw -= m_turnSpeed * _Time::deltaTime;
    }
    if (wParam == VK_SPACE) // Jump (Ollie)
    {
        if (m_state == STATE_GROUNDED) // Can only jump if on the ground
        {
            m_body->velocity.y = m_jumpForce;
            m_state = STATE_AIR; // We are now in the air
            m_body->isGrounded = false; // Manually set this, physics will confirm
        }
    }
}

void _Player::RegisterStaticCollider(_StaticModelInstance* model)
{
    m_collidableStaticModels.push_back(model);
}

void _Player::RegisterAnimatedCollider(_AnimatedModelInstance* model)
{
    m_collidableAnimatedModels.push_back(model);
}

void _Player::ClearColliders()
{
    m_collidableStaticModels.clear();
    m_collidableAnimatedModels.clear();
}

void _Player::UpdatePhysics()
{
    if(isFrozen) return;

    // --- Wall Collision & Sliding Logic ---
    if (m_body->colliders.empty()) return; // safety check

    _Collider* playerMainCollider = m_body->colliders[0];
    
    // check x axis movement
    bool hitX = false;
    Vector3 predictedPosX = m_body->pos;
    predictedPosX.x += m_body->velocity.x * _Time::deltaTime;
    _Collider* playerPredictedColliderX = playerMainCollider->GetWorldSpaceCollider(predictedPosX, m_body->scale, m_body->rotation);
    
    if(playerPredictedColliderX)
    {
        for(_StaticModelInstance* staticModel : m_collidableStaticModels) {
            for (_Collider* staticCollider : staticModel->colliders) {
                _Collider* staticWorldCollider = staticCollider->GetWorldSpaceCollider(staticModel->pos, staticModel->scale, staticModel->rotation);
                if(staticWorldCollider) {
                    if (playerPredictedColliderX->CheckCollision(staticWorldCollider) && staticCollider->m_type == COLLIDER_WALL) {
                        hitX = true;
                    }
                    delete staticWorldCollider; 
                }
                if(hitX) break;
            }
            if(hitX) break;
        }
        delete playerPredictedColliderX; 
    }

    // check z axis movement
    bool hitZ = false;
    Vector3 predictedPosZ = m_body->pos;
    predictedPosZ.z += m_body->velocity.z * _Time::deltaTime;
    _Collider* playerPredictedColliderZ = playerMainCollider->GetWorldSpaceCollider(predictedPosZ, m_body->scale, m_body->rotation);
    
    if(playerPredictedColliderZ)
    {
        for(_StaticModelInstance* staticModel : m_collidableStaticModels) {
            for (_Collider* staticCollider : staticModel->colliders) {
                _Collider* staticWorldCollider = staticCollider->GetWorldSpaceCollider(staticModel->pos, staticModel->scale, staticModel->rotation);
                if(staticWorldCollider) {
                    if (playerPredictedColliderZ->CheckCollision(staticWorldCollider) && staticCollider->m_type == COLLIDER_WALL) {
                        hitZ = true;
                    }
                    delete staticWorldCollider;
                }
                if(hitZ) break;
            }
            if(hitX) break; // Already hit X, no need to keep checking Z
        }
        delete playerPredictedColliderZ; 
    }

    // zero out velocity for the axis that hit
    if (hitX) m_body->velocity.x = 0;
    if (hitZ) m_body->velocity.z = 0;

    // --- Ground Check & Gravity Logic ---
    m_body->isGrounded = false;
    
    _Collider* playerCurrentCollider = playerMainCollider->GetWorldSpaceCollider(m_body->pos, m_body->scale, m_body->rotation);
    
    if (playerCurrentCollider)
    {
        for(_StaticModelInstance* staticModel : m_collidableStaticModels)
        {
            for (_Collider* staticCollider : staticModel->colliders)
            {
                _Collider* staticWorldCollider = staticCollider->GetWorldSpaceCollider(staticModel->pos, staticModel->scale, staticModel->rotation);
                if (staticWorldCollider)
                {
                    if (playerCurrentCollider->CheckCollision(staticWorldCollider))
                    {
                        if (staticCollider->m_type == COLLIDER_FLOOR)
                        {
                            m_body->isGrounded = true;
                            m_state = STATE_GROUNDED; // Update our state
                            if(m_body->velocity.y < 0) { 
                                m_body->velocity.y = 0; 
                            }
                        }
                    }
                    delete staticWorldCollider;
                }
                if (m_body->isGrounded) break;
            }
            if (m_body->isGrounded) break;
        }
        delete playerCurrentCollider;
    }

    // If no ground was detected, we are in the air
    if (!m_body->isGrounded) {
        m_state = STATE_AIR;
    }

    // --- Apply Friction & Speed Cap ---
    if (m_state == STATE_GROUNDED) 
    {
        // Calculate horizontal speed
        float horizSpeed = sqrt(m_body->velocity.x * m_body->velocity.x + m_body->velocity.z * m_body->velocity.z);

        if (horizSpeed > 0.01f) // If we are moving
        {
            // Apply friction (damp
            float frictionAmount = m_friction * _Time::deltaTime;
            m_body->velocity.x -= m_body->velocity.x * frictionAmount;
            m_body->velocity.z -= m_body->velocity.z * frictionAmount;

            // Cap speed
            if (horizSpeed > m_maxSpeed)
            {
                m_body->velocity.x = (m_body->velocity.x / horizSpeed) * m_maxSpeed;
                m_body->velocity.z = (m_body->velocity.z / horizSpeed) * m_maxSpeed;
            }
        }
        else // Stop micromovements
        {
            m_body->velocity.x = 0;
            m_body->velocity.z = 0;
        }
    }
    // (In the air, we might want less or no friction for air control)

    // Apply the player's turning rotation to the model
    m_body->rotation.y = m_playerYaw;

    // applies gravity (if not grounded)
    // and movement velocity (now corrected for walls, friction, and speed)
    m_body->Update();

    // ---- ANIMATION STATE SELECTION ----
    float currentSpeed = sqrt(m_body->velocity.x * m_body->velocity.x + m_body->velocity.z * m_body->velocity.z);

    if (m_state == STATE_AIR) {
        // m_body->PlayAnimation("jump", 1.0f); // TODO: Add jump/air animation
        m_body->PlayAnimation("idle", 1.0f); // Placeholder
    } else if (currentSpeed > 0.1f) {
        // m_body->PlayAnimation("push", 1.0f); // TODO: Add pushing/rolling animation
        m_body->PlayAnimation("walk", 1.0f);
    } else {
        // player is on the ground and not moving
        m_body->PlayAnimation("idle", 1.0f);
    }

}

void _Player::UpdateCamera(_camera* cam)
{
    // don't force the cam into pos if it's in free cam
    if(cam->isFreeCam) {isFrozen=true; return;}
    else {isFrozen=false;}
    
    // Set the "look at" point (the target)
    // This is a point slightly above the player's body
    cam->des = m_body->pos;
    cam->des.y += m_camHeight; 

    // Calculate camera's eye position using spherical coordinates (orbit)
    float pitchRad = m_cameraPitch * PI / 180.0;
    float yawRad = m_cameraYaw * PI / 180.0;

    cam->eye.x = cam->des.x - sin(yawRad) * cos(pitchRad) * m_camDistance;
    cam->eye.z = cam->des.z - cos(yawRad) * cos(pitchRad) * m_camDistance;
    cam->eye.y = cam->des.y + sin(pitchRad) * m_camDistance;
    
    // Set the up vector (always Y-up)
    cam->up = Vector3(0, 1, 0);

    cam->rotAngle.x = m_cameraYaw;
    cam->rotAngle.y = m_cameraPitch;
}

void _Player::Draw()
{
    // we just tell our body to draw itself
    m_body->Draw();
}