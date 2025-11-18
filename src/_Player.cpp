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

    m_currentRail = nullptr;

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
    _Rigidbody *rb = m_body->GetRigidBody();
    // calculate forward vector based on player's yaw
    Vector3 fwd;
    fwd.x = -sin(m_playerYaw * PI / 180.0);
    fwd.z = -cos(m_playerYaw * PI / 180.0);
    fwd.normalize(); // Ensure it's a unit vector

    // check which key was pressed
    if (wParam == 'W') // Accelerate
    {
        // Add force in the forward direction
        rb->velocity.x += fwd.x * m_acceleration * _Time::deltaTime;
        rb->velocity.z += fwd.z * m_acceleration * _Time::deltaTime;
    }
    if (wParam == 'S') // Brake / Reverse
    {
         // Add force in the backward direction
        rb->velocity.x -= fwd.x * m_acceleration * _Time::deltaTime;
        rb->velocity.z -= fwd.z * m_acceleration * _Time::deltaTime;
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
        // --- Allow jumping from ground or grind ---
        if (m_state == STATE_GROUNDED || m_state == STATE_GRINDING)
        {
            rb->velocity.y = m_jumpForce;
            m_state = STATE_AIR; // We are now in the air
            rb->isGrounded = false; // manually set this
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

    _Rigidbody *rb = m_body->GetRigidBody();
    m_currentRail = nullptr;

    _Collider* playerMainCollider = m_body->colliders[0];
    
    // check x axis movement
    bool hitX = false;
    Vector3 predictedPosX = m_body->pos;
    predictedPosX.x += rb->velocity.x * _Time::deltaTime;
    _Collider* playerPredictedColliderX = playerMainCollider->GetWorldSpaceCollider(predictedPosX, m_body->scale, m_body->rotation);
    
    if(playerPredictedColliderX)
    {
        for(_StaticModelInstance* staticModel : m_collidableStaticModels) {
            for (_Collider* staticCollider : staticModel->colliders) {
                _Collider* staticWorldCollider = staticCollider->GetWorldSpaceCollider(staticModel->pos, staticModel->scale, staticModel->rotation);
                if(staticWorldCollider) {
                    // --- ignore rails for wall collision ---
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
    predictedPosZ.z += rb->velocity.z * _Time::deltaTime;
    _Collider* playerPredictedColliderZ = playerMainCollider->GetWorldSpaceCollider(predictedPosZ, m_body->scale, m_body->rotation);
    
    if(playerPredictedColliderZ)
    {
        for(_StaticModelInstance* staticModel : m_collidableStaticModels) {
            for (_Collider* staticCollider : staticModel->colliders) {
                _Collider* staticWorldCollider = staticCollider->GetWorldSpaceCollider(staticModel->pos, staticModel->scale, staticModel->rotation);
                if(staticWorldCollider) {
                    // --- ignore rails for wall collision ---
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
    if (hitX) rb->velocity.x = 0;
    if (hitZ) rb->velocity.z = 0;


    // --- unified Ground & Rail Check Logic ---
    rb->isGrounded = false; // Reset at the start of the check
    bool isOnRail = false;
    
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
                            rb->isGrounded = true; // We are on the floor
                        }
                        else if (staticCollider->m_type == COLLIDER_RAIL)
                        {
                            isOnRail = true; // We are on a rail
                            m_currentRail = staticModel; // store the rail
                        }
                    }
                    delete staticWorldCollider;
                }
                if (rb->isGrounded) break; // Ground check is highest priority
            }
            if (rb->isGrounded) break;
        }
        delete playerCurrentCollider;
    }

    // --- unified State Machine & Friction ---
    if (rb->isGrounded) // This is true only if we hit a COLLIDER_FLOOR
    {
        m_state = STATE_GROUNDED;
        if(rb->velocity.y < 0) { 
            rb->velocity.y = 0; 
        }

        // --- Apply Friction & Speed Cap (Only on Ground) ---
        float horizSpeed = sqrt(rb->velocity.x * rb->velocity.x + rb->velocity.z * rb->velocity.z);
        if (horizSpeed > 0.01f) // If we are moving
        {
            float frictionAmount = m_friction * _Time::deltaTime;
            rb->velocity.x -= rb->velocity.x * frictionAmount;
            rb->velocity.z -= rb->velocity.z * frictionAmount;

            if (horizSpeed > m_maxSpeed)
            {
                rb->velocity.x = (rb->velocity.x / horizSpeed) * m_maxSpeed;
                rb->velocity.z = (rb->velocity.z / horizSpeed) * m_maxSpeed;
            }
        }
        else // Stop micro-movements
        {
            rb->velocity.x = 0;
            rb->velocity.z = 0;
        }
    }
    else if (isOnRail) // This is true if we hit a rail AND NOT the floor
    {
        // Check if we *just* landed on the rail in this frame
        if (m_state != STATE_GRINDING && m_currentRail != nullptr)
        {
            // --- SNAP ROTATION ---
            // Set the player's yaw to match the rail's yaw
            m_playerYaw = m_currentRail->rotation.y;

            // --- SNAP VELOCITY ---
            // First, get the player's current horizontal speed
            float horizSpeed = sqrt(rb->velocity.x * rb->velocity.x + rb->velocity.z * rb->velocity.z);
            
            // Second, get the rail's forward direction vector
            float railYawRad = m_currentRail->rotation.y * PI / 180.0f;
            Vector3 railForward;
            railForward.x = -sin(railYawRad); // Same as player's 'W' key logic
            railForward.z = -cos(railYawRad);
            
            // Finally, set the player's velocity to that speed in the rail's direction
            rb->velocity.x = railForward.x * horizSpeed;
            rb->velocity.z = railForward.z * horizSpeed;
        }

        m_state = STATE_GRINDING;
        rb->velocity.y = 0; // Negate gravity
        
        // we Update()
        // into thinking we are "grounded" so it doesn't apply gravity
        rb->isGrounded = true; 
        
        // (No friction is applied while grinding)
    }
    else // Not on floor, not on rail
    {
        m_state = STATE_AIR; 
        // rb->isGrounded is already false, so gravity will be applied
        // in m_body->Update()
    }
    // ---------------------------------


    // Apply the player's turning rotation to the model
    m_body->rotation.y = m_playerYaw;

    // applies gravity (if in STATE_AIR)
    // and movement velocity (now corrected for walls, friction, and speed)
    m_body->Update();

    // ---- UNIFIED ANIMATION STATE SELECTION ----
    float currentSpeed = sqrt(rb->velocity.x * rb->velocity.x + rb->velocity.z * rb->velocity.z);

    if (m_state == STATE_AIR) {
        // m_body->PlayAnimation("jump", 1.0f); // TODO: Add jump/air animation
        m_body->PlayAnimation("idle", 1.0f); // Placeholder
    }
    else if (m_state == STATE_GRINDING)
    {
        // m_body->PlayAnimation("grind", 1.0f); // TODO: Add grind animation
        m_body->PlayAnimation("idle", 1.0f); // Placeholder
    }
    else if (currentSpeed > 0.1f) { // Grounded and moving
        // m_body->PlayAnimation("push", 1.0f); // TODO: Add pushing/rolling animation
        m_body->PlayAnimation("walk", 1.0f); // Placeholder
    } 
    else { // Grounded and not moving
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