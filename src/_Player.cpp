#include "_Player.h"

_Player::_Player(_AnimatedModel* modelBlueprint, _AnimatedModel* boardBlueprint)
{
    m_body = new _AnimatedModelInstance(modelBlueprint);
    m_skateboard = new _AnimatedModelInstance(boardBlueprint);
    
    // set up the player's physics collider
    // add sphere collider centered at (0,0,0) local space & r=1.0
    // note: model is norm -1 to +1
    m_body->AddCollider(new _SphereHitbox(Vector3(0,0,0), 1.0f));
    
    // set initial position
    m_body->pos = Vector3(0, 1, 0); // start slightly above ground
    ResetBoard();

    // init player brain variables
    m_cameraPitch = 20.0f;  // Start camera slightly angled down
    m_playerYaw = 0.0f;     // Player starts facing forward
    m_cameraYaw = 0.0f;     // Camera starts directly behind player
    
    // --- SKATE PHYSICS VARS ---
    m_acceleration = 150.0f; // units per second^2
    m_maxSpeed = 300.0f;     // units per second
    m_turnSpeed = 1080.0f;   // degrees per second
    m_friction = 0.3f;      // Damping factor (higher = stops faster)
    m_jumpForce = 8.0f;     // Initial upward velocity
    m_state = STATE_AIR;    // Start in air, will be set to grounded by physics

    // --- CAMERA VARS ---
    m_camDistance = 8.0f;   // 8 units away from player
    m_camHeight = 2.0f;     // looks at a point 2 units above player's origin
    m_mouseSensitivity = 0.1f;

    m_currentRail = nullptr;

    isFrozen = false;

    m_skateboardOffset = Vector3(0,-1.0,0);
    m_skateboard->scale = Vector3(0.5,0.5,0.5);
}

_Player::~_Player()
{
    delete m_body;
    delete m_skateboard;
}

void _Player::ResetBoard(){
    m_isOnBoard = true;
    m_skateboard->pos = m_body->pos + m_skateboardOffset;
    m_skateboard->rotation = m_body->rotation;
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
    /*if(m_cameraPitch < 5.0f) { // Don't let camera go below the ground
        m_cameraPitch = 5.0f;
    }*/
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

    // --- TURNING LOGIC ---
    if (wParam == 'A') // Turn Left
    {
        float turnAmount = m_turnSpeed * _Time::deltaTime;
        m_playerYaw += turnAmount;

        // Convert angle change to radians
        float rad = turnAmount * PI / 180.0f;
        float cosTheta = cos(rad);
        float sinTheta = sin(rad);

        // Capture old velocity
        float oldX = rb->velocity.x;
        float oldZ = rb->velocity.z;

        // Rotate velocity vector by +theta
        // Formula: x' = x*cos - z*sin | z' = x*sin + z*cos
        // Adapted for this coordinate system (Yaw 0 = -Z):
        rb->velocity.x = oldX * cosTheta + oldZ * sinTheta;
        rb->velocity.z = -oldX * sinTheta + oldZ * cosTheta;
    }
    if (wParam == 'D') // Turn Right
    {
        float turnAmount = m_turnSpeed * _Time::deltaTime;
        m_playerYaw -= turnAmount;

        // Convert angle change to radians (Negative for right turn)
        float rad = -turnAmount * PI / 180.0f;
        float cosTheta = cos(rad);
        float sinTheta = sin(rad);

        // Capture old velocity
        float oldX = rb->velocity.x;
        float oldZ = rb->velocity.z;

        // Rotate velocity vector by -theta
        rb->velocity.x = oldX * cosTheta + oldZ * sinTheta;
        rb->velocity.z = -oldX * sinTheta + oldZ * cosTheta;
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

    // --- BAIL / RECALL MECHANIC ---
    if (wParam == 'F') {
        if (m_isOnBoard) {
            // Fall off: Player hops, board stays behind
            m_isOnBoard = false;
            m_state = STATE_BAILED;
            rb->velocity.y = 5.0f; // Hop off
        } else {
            // Recall: Board teleports back to player
            ResetBoard();
            m_state = STATE_AIR;
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
        m_state = STATE_GRINDING;

        // --- Calculate Rail Direction Vectors ---
        // Convert degrees to radians
        float railYawRad = m_currentRail->rotation.y * PI / 180.0f;

        // Calculate the rail's forward unit vector 
        // (Matches standard OpenGL rotation: x = -sin, z = -cos)
        Vector3 railDir;
        railDir.x = -sin(railYawRad); 
        railDir.y = 0;
        railDir.z = -cos(railYawRad);
        railDir.normalize();

        // --- Determine Momentum Direction ---
        // Get current horizontal velocity direction
        Vector3 currentVel = rb->velocity;
        currentVel.y = 0; 

        // Use Dot Product to check if player is moving with or against the rail
        // Dot > 0: Same direction, Dot < 0: Opposite direction
        float dot = (currentVel.x * railDir.x) + (currentVel.z * railDir.z);

        // If moving opposite to the rail's defined forward vector, flip the direction
        if (dot < 0) {
            railDir = railDir * -1.0f;
        }

        // --- Snap Velocity (Preserve Momentum) ---
        // Get the player's current speed magnitude
        float speed = sqrt(rb->velocity.x * rb->velocity.x + rb->velocity.z * rb->velocity.z);
        
        // Apply that speed purely along the aligned rail direction
        rb->velocity.x = railDir.x * speed;
        rb->velocity.z = railDir.z * speed;
        rb->velocity.y = 0; // No gravity while grinding

        // --- Snap Position (Top-Center Logic) ---
        // We project the player's position onto the rail's infinite line
        // to snap X/Z to the center, but preserve their progress along the rail.
        
        // Vector from Rail Origin to Player
        Vector3 diff = m_body->pos - m_currentRail->pos;
        
        // Project 'diff' onto the normalized 'railDir'
        float distAlongRail = (diff.x * railDir.x) + (diff.z * railDir.z);
        
        // Calculate the new snapped position on the rail line
        Vector3 newPos = m_currentRail->pos + (railDir * distAlongRail);
        
        // Calculate Height: Rail Pos Y + Rail Top Bound + Player Collider Radius
        // Assuming the rail collider is a box from -1 to 1 (Height = 1.0 * scale.y)
        float railTopY = m_currentRail->pos.y + (m_currentRail->scale.y * 1.0f);
        newPos.y = railTopY + 1.0f; // +1.0 for player's sphere radius so they sit on top

        // Apply the position snap
        m_body->pos = newPos;

        // --- Visual Rotation (Grind Stance) ---
        // Rotate 90 degrees relative to the rail to look like a boardslide
        // We base this on the original rail rotation
        m_playerYaw = m_currentRail->rotation.y + 90.0f;
        
        // Keep the 'grounded' flag true so gravity doesn't apply next frame
        rb->isGrounded = true; 
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

    // --- BOARD PHYSICS SYNC ---
    if(m_isOnBoard){
        // board follows player with offset
        m_skateboard->pos = m_body->pos + m_skateboardOffset;
        m_skateboard->rotation = m_body->rotation;
    } else {
        // TODO: add roll away logic, for now it stays in place
        // add a rigidbody to the board
    }

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
    m_skateboard->Draw();
}