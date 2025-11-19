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

    m_preGrindYaw = 0.0f;

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
        if (m_state == STATE_GROUNDED || m_state == STATE_GRINDING || m_state == STATE_BAILED)
        {
            // If we are jumping OUT of a grind, restore original rotation
            if (m_state == STATE_GRINDING) {
                m_playerYaw = m_preGrindYaw;
            }

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
    bool isOnVert = false;
    
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
                            if (rb->velocity.y <= 0.1) 
                            {
                                isOnRail = true; // we are on rail
                                m_currentRail = staticModel; // store rail
                            }
                        }
                        // halfpipe logic
                        else if (staticCollider->m_type == COLLIDER_HALFPIPE)
                        {
                            // --- 1. ROBUST LOCAL SPACE TRANSFORMATION ---
                            Vector3 relPos = m_body->pos - staticModel->pos;
                        
                            // Convert rotation to radians
                            float rad = staticModel->rotation.y * PI / 180.0f;
                            float c = cos(rad);
                            float s = sin(rad);
                        
                            // Apply Inverse Rotation (World -> Local)
                            float localX = relPos.x * c - relPos.z * s;
                            float localZ = relPos.x * s + relPos.z * c;
                        
                            // --- 2. DIMENSIONS & BOUNDS ---
                            float halfWidth = staticModel->scale.x;
                            float halfDepth = staticModel->scale.z; 
                            float halfHeight = staticModel->scale.y;
                        
                            // Check Width (Side to side)
                            if (localX < -halfWidth - 0.5f || localX > halfWidth + 0.5f) continue;
                        
                            // --- 3. CURVE LOGIC ---
                            float radius = halfHeight * 2.0f;
                            float wallZ = -halfDepth;
                            float distFromWall = localZ - wallZ;
                        
                            // Prevent going through the back
                            if (distFromWall < -0.5f) continue;
                            // Clamp flat ground
                            if (distFromWall > radius) distFromWall = radius;
                        
                            // Circle Math: xCircle is 0 at bottom, R at top
                            float xCircle = radius - distFromWall;
                            if (xCircle < 0) xCircle = 0;
                        
                            // Calculate raw surface height at this position
                            float circleY = radius - sqrt(pow(radius, 2) - pow(xCircle, 2));
                            float pipeBottomY = staticModel->pos.y - halfHeight;
                        
                            // --- 4. OFFSET CALCULATIONS (THE FIX) ---

                            // A. Calculate Slope Angle
                            float slopeRad = asin(xCircle / radius); // 0 at bottom, PI/2 at top
                        
                            // B. Geometry Correction: 
                            // To keep a sphere on a slope, we divide radius by cos(theta).
                            // As the wall gets vertical, this value gets huge, so we clamp it.
                            float cosTheta = cos(slopeRad);
                            if(cosTheta < 0.3f) cosTheta = 0.3f; // Clamp to prevent shooting to space

                            float geometryOffset = 1.0f / cosTheta; 
                        
                            // C. Manual Visual Offset:
                            // Tweak this value to push the model further out visually!
                            float visualTweak = 2.5f; 
                        
                            float finalY = pipeBottomY + circleY + geometryOffset + visualTweak;
                        
                            // --- 5. APPLY PHYSICS ---
                            if (m_body->pos.y <= finalY + 2.0f && m_body->pos.y >= pipeBottomY - 1.0f)
                            {
                                isOnVert = true;
                                rb->isGrounded = true;
                                m_body->pos.y = finalY;

                                // Stop falling
                                if(rb->velocity.y < 0) rb->velocity.y = 0;
                            
                                // Apply Pitch Rotation
                                // Convert radians to degrees for rotation
                                float slopeDeg = slopeRad * (180.0f / PI);

                                m_body->rotation.x = slopeDeg;
                                m_body->rotation.z = 0;
                            }
                        }
                        else if (staticCollider->m_type == COLLIDER_STAIRS)
                        {
                            // --- 1. LOCAL SPACE TRANSFORM (Standard) ---
                            Vector3 relPos = m_body->pos - staticModel->pos;
                            float rad = staticModel->rotation.y * PI / 180.0f;
                            float c = cos(rad);
                            float s = sin(rad);
                            float localX = relPos.x * c - relPos.z * s;
                            float localZ = relPos.x * s + relPos.z * c;
                        
                            // --- 2. BOUNDS CHECK ---
                            float halfWidth = staticModel->scale.x;
                            float halfDepth = staticModel->scale.z;
                            float halfHeight = staticModel->scale.y;
                        
                            // Check Width (Side to side)
                            if (localX < -halfWidth - 0.5f || localX > halfWidth + 0.5f) continue;
                            // Check Depth (Front to back)
                            if (localZ < -halfDepth - 0.5f || localZ > halfDepth + 0.5f) continue;
                        
                            // --- 3. LINEAR SLOPE LOGIC ---
                            // Map localZ (-halfDepth to +halfDepth) to Height (0 to 2*halfHeight)

                            // Normalize Z to 0.0 - 1.0 range
                            // Assuming Front of stairs is at -halfDepth and Top is at +halfDepth
                            float totalDepth = halfDepth * 2.0f;
                            float distFromFront = localZ - (-halfDepth); // 0 at front
                            float t = distFromFront / totalDepth;
                        
                            // Clamp t (so we have a flat spot at top and bottom)
                            if (t < 0.0f) t = 0.0f;
                            if (t > 1.0f) t = 1.0f;
                        
                            // Calculate target height (Linear interpolation)
                            float rampHeight = t * (halfHeight * 2.0f);

                            float pipeBottomY = staticModel->pos.y - halfHeight;
                            float targetWorldY = pipeBottomY + rampHeight;
                        
                            // --- 4. SNAP TO STAIRS ---
                            // Check if player is close enough to the surface to snap
                            if (m_body->pos.y <= targetWorldY + 2.0f && m_body->pos.y >= pipeBottomY - 0.5f)
                            {
                                rb->isGrounded = true;
                                m_body->pos.y = targetWorldY + 1.0f; // +1.0 offset for player center

                                if(rb->velocity.y < 0) rb->velocity.y = 0;
                            
                                // --- 5. VISUAL TILT (Optional) ---
                                // Calculate slope angle: tan(theta) = height / length
                                float slopeAngle = atan2(halfHeight * 2.0f, totalDepth) * (180.0f / PI);

                                // If we are facing up the stairs, tilt up. If down, tilt down.
                                // For now, just simple pitch based on object rotation
                                // m_body->rotation.x = -slopeAngle; 
                            }
                        }
                    }
                    delete staticWorldCollider;
                }
                //if (rb->isGrounded) break; // Ground check is highest priority
            }
            //if (rb->isGrounded) break;
        }
        delete playerCurrentCollider;
    }

    // --- unified State Machine & Friction ---
    if (rb->isGrounded) // This is true only if we hit a COLLIDER_FLOOR
    {   
        // If we are on vert, use specific state, otherwise standard grounded
        if (m_isOnBoard) {
            // Standard skating logic
            m_state = isOnVert ? STATE_VERT : STATE_GROUNDED;
        } else {
            // Walking logic: force state to BAILED (or WALKING) so we know we are on foot
            m_state = STATE_BAILED;
        }
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

        // RESET VISUAL ROTATION IF NOT ON VERT
        if(!isOnVert) {
            m_body->rotation.x = 0.0f;
            // Rotate back to flat slowly in air
            m_body->rotation.x *= 0.95f;
        }
    }
    else if (isOnRail) // This is true if we hit a rail AND NOT the floor
    {
        // 1. Capture Entry Angle Only Once
        if (m_state != STATE_GRINDING) {
            m_preGrindYaw = m_playerYaw; // Save the direction we were facing
        }
        m_state = STATE_GRINDING;

        // --- Calculate Rail Direction Vectors ---
        // Convert degrees to radians
        float railYawRad = m_currentRail->rotation.y * PI / 180.0f;

        // Calculate the rail's forward unit vector 
        Vector3 railDir;
        railDir.x = -sin(railYawRad); 
        railDir.y = 0;
        railDir.z = -cos(railYawRad);
        railDir.normalize();

        // --- Determine Direction based on INTENT (Entry Angle) ---
        // We use m_preGrindYaw instead of current velocity to prevent jitter
        //float preYawRad = m_preGrindYaw * PI / 180.0f;
        //Vector3 intentDir;
        //intentDir.x = -sin(preYawRad);
        //intentDir.y = 0;
        //intentDir.z = -cos(preYawRad);

        Vector3 intentDir = rb->velocity;
        intentDir.normalize();

        // Check if our entry angle was with or against the rail
        float dot = (intentDir.x * railDir.x) + (intentDir.z * railDir.z);

        // If we entered 'backwards', flip the rail direction for the duration of the grind
        if (dot < 0) {
            railDir = railDir * -1.0f;
        }

        // --- Snap Velocity (Preserve Momentum) ---
        float speed = sqrt(rb->velocity.x * rb->velocity.x + rb->velocity.z * rb->velocity.z);
        
        // Apply speed along the stabilized rail direction
        rb->velocity.x = railDir.x * speed;
        rb->velocity.z = railDir.z * speed;
        rb->velocity.y = 0; 

        // --- Snap Position (Top-Center Logic) ---
        Vector3 diff = m_body->pos - m_currentRail->pos;
        float distAlongRail = (diff.x * railDir.x) + (diff.z * railDir.z);
        Vector3 newPos = m_currentRail->pos + (railDir * distAlongRail);
        
        float railTopY = m_currentRail->pos.y + (m_currentRail->scale.y * 1.0f);
        newPos.y = railTopY + 0.9f; 

        m_body->pos = newPos;

        // --- Visual Rotation (Grind Stance) ---
        // Calculate rotation based on the MOVEMENT direction (railDir), not the static rail
        // This ensures if we grind 'backwards', we face the correct perpendicular direction
        // atan2(sin, cos) -> atan2(-x, -z) because x=-sin, z=-cos
        float moveAngle = atan2(-railDir.x, -railDir.z) * 180.0f / PI;
        m_playerYaw = moveAngle + 90.0f;
        
        rb->isGrounded = true;
    }
    else // Not on floor, not on rail
    {
        if (m_state == STATE_GRINDING) {
            m_playerYaw = m_preGrindYaw;
        }

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
        // Calculate rotated offset so board orbits the player pivot
        Vector3 rotatedOffset = CalculateBoardOffset(m_skateboardOffset, m_body->rotation);
        
        m_skateboard->pos = m_body->pos + rotatedOffset;
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
    // ... existing draw calls ...
    m_body->Draw();
    m_skateboard->Draw();

    // === DEBUG DRAWING FOR HALF-PIPE ===
    if (isDebug) // Only draw if debug mode is on (Press '1')
    {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glLineWidth(3.0f); // Make lines thick

        for (auto* staticModel : m_collidableStaticModels)
        {
            // 1. Setup Vectors (Match Physics)
    float radY = staticModel->rotation.y * PI / 180.0f;
    Vector3 fwdVec(sin(radY), 0, cos(radY)); 
    Vector3 relPos = m_body->pos - staticModel->pos;
    float localZ = (relPos.x * fwdVec.x) + (relPos.z * fwdVec.z);

    // 2. Calc T
    float halfDepth = staticModel->scale.z;
    float rampStart = halfDepth + 1.0f; 
    float totalLen = (halfDepth * 2.0f) + 1.0f; 
    float distFromFront = rampStart - localZ;
    float t = distFromFront / totalLen;

    // Skip drawing if out of bounds (matches physics continue)
    if (t > 1.0f || t < -0.1f) continue; 
    if (t < 0.0f) t = 0.0f;

    // 3. Height (Pivot Fix)
    float halfHeight = staticModel->scale.y;
    float pipeBottomY = staticModel->pos.y - halfHeight; 
    float curveHeight = (t * t) * (halfHeight * 2.0f); 
    float targetWorldY = pipeBottomY + curveHeight; // Raw surface height

    // 4. Draw
    glColor3f(1.0f, 1.0f, 0.0f); 
    glBegin(GL_LINES);
        glVertex3f(m_body->pos.x, m_body->pos.y, m_body->pos.z);
        glVertex3f(m_body->pos.x, targetWorldY, m_body->pos.z);
    glEnd();
        }
        
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
    }
}

Vector3 _Player::CalculateBoardOffset(Vector3 baseOffset, Vector3 rotation) {
    // Convert to Radians
    float radX = rotation.x * PI / 180.0f;
    float radY = rotation.y * PI / 180.0f;

    // 1. Apply Pitch (Rotation around X)
    // Original offset is (0, -1, 0)
    // y' = y*cos(x) - z*sin(x)
    // z' = y*sin(x) + z*cos(x)
    float y1 = baseOffset.y * cos(radX) - baseOffset.z * sin(radX);
    float z1 = baseOffset.y * sin(radX) + baseOffset.z * cos(radX);
    float x1 = baseOffset.x; // unchanged by X rot

    // 2. Apply Yaw (Rotation around Y)
    // x'' = x'*cos(y) + z'*sin(y)
    // z'' = -x'*sin(y) + z'*cos(y)
    float x2 = x1 * cos(radY) + z1 * sin(radY);
    float z2 = -x1 * sin(radY) + z1 * cos(radY);
    float y2 = y1; // unchanged by Y rot

    return Vector3(x2, y2, z2);
}

void _Player::ResetBoard(){
    m_isOnBoard = true;
    
    // Calculate rotated offset
    Vector3 rotatedOffset = CalculateBoardOffset(m_skateboardOffset, m_body->rotation);
    
    m_skateboard->pos = m_body->pos + rotatedOffset;
    m_skateboard->rotation = m_body->rotation;
}