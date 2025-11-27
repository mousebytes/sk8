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

    // --- WALK PHYSICS VARS ---
    m_walkSpeed = 60.0f;         // Much slower than skating
    m_walkAccel = 400.0f;        // High acceleration for instant start
    m_walkFriction = 8.0f;       // High friction for instant stop
    m_walkTurnSpeed = 800.0f;

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

// In sk8-temp/src/_Player.cpp

void _Player::HandleKeys(WPARAM wParam)
{
    _Rigidbody *rb = m_body->GetRigidBody();
    
    // Calculate forward vector
    Vector3 fwd;
    fwd.x = -sin(m_playerYaw * PI / 180.0);
    fwd.z = -cos(m_playerYaw * PI / 180.0);
    fwd.normalize();

    // --- 1. WALKING INPUT (Bailed Mode) ---
    if (!m_isOnBoard)
    {
        if (wParam == 'W') 
        {
            rb->velocity.x += fwd.x * m_walkAccel * _Time::deltaTime;
            rb->velocity.z += fwd.z * m_walkAccel * _Time::deltaTime;
        }
        if (wParam == 'S') 
        {
            rb->velocity.x -= fwd.x * m_walkAccel * _Time::deltaTime;
            rb->velocity.z -= fwd.z * m_walkAccel * _Time::deltaTime;
        }
        if (wParam == 'A') m_playerYaw += m_walkTurnSpeed * _Time::deltaTime;
        if (wParam == 'D') m_playerYaw -= m_walkTurnSpeed * _Time::deltaTime;

        // Cap velocity immediately for "snappy" walking feel
        float currentSpeed = sqrt(rb->velocity.x * rb->velocity.x + rb->velocity.z * rb->velocity.z);
        if(currentSpeed > m_walkSpeed) {
             rb->velocity.x = (rb->velocity.x / currentSpeed) * m_walkSpeed;
             rb->velocity.z = (rb->velocity.z / currentSpeed) * m_walkSpeed;
        }

        // Jump Logic (Walking)
        if (wParam == VK_SPACE && rb->isGrounded)
        {
            rb->velocity.y = 5.0f; // Lower jump height when off board
        }
    }
    // --- 2. SKATING INPUT (On Board Mode) ---
    else 
    {
        // Existing Skating Controls
        if (wParam == 'W') 
        {
            rb->velocity.x += fwd.x * m_acceleration * _Time::deltaTime;
            rb->velocity.z += fwd.z * m_acceleration * _Time::deltaTime;
        }
        if (wParam == 'S') 
        {
            rb->velocity.x -= fwd.x * m_acceleration * _Time::deltaTime;
            rb->velocity.z -= fwd.z * m_acceleration * _Time::deltaTime;
        }
        if (wParam == 'A') 
        {
            // Existing Tank Control Turning Logic
            float turnAmount = m_turnSpeed * _Time::deltaTime;
            m_playerYaw += turnAmount;
            
            // Rotate velocity vector
            float rad = turnAmount * PI / 180.0f;
            float cosTheta = cos(rad);
            float sinTheta = sin(rad);
            float oldX = rb->velocity.x;
            float oldZ = rb->velocity.z;
            rb->velocity.x = oldX * cosTheta + oldZ * sinTheta;
            rb->velocity.z = -oldX * sinTheta + oldZ * cosTheta;
        }
        if (wParam == 'D') 
        {
            // Same as 'A' but negative turnAmount...
            float turnAmount = m_turnSpeed * _Time::deltaTime;
            m_playerYaw -= turnAmount;
            
            float rad = -turnAmount * PI / 180.0f;
            float cosTheta = cos(rad);
            float sinTheta = sin(rad);
            float oldX = rb->velocity.x;
            float oldZ = rb->velocity.z;
            rb->velocity.x = oldX * cosTheta + oldZ * sinTheta;
            rb->velocity.z = -oldX * sinTheta + oldZ * cosTheta;
        }

        if (wParam == VK_SPACE) 
        {
            if (m_state == STATE_GROUNDED || m_state == STATE_GRINDING || m_state == STATE_BAILED)
            {
                if (m_state == STATE_GRINDING) m_playerYaw = m_preGrindYaw;
                rb->velocity.y = m_jumpForce;
                m_state = STATE_AIR;
                rb->isGrounded = false;
            }
        }
    }

    // --- SHARED INPUTS ---
    // Toggle Board/Bailed State
    if (wParam == 'F') {
        if (m_isOnBoard) {
            m_isOnBoard = false;
            m_state = STATE_BAILED;
            rb->velocity.y = 5.0f; 
        } else {
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

// In sk8-temp/src/_Player.cpp

// ---------------------------------------------------------
// MAIN PHYSICS DISPATCHER
// ---------------------------------------------------------
void _Player::UpdatePhysics()
{
    if (isFrozen) return;

    if (m_isOnBoard) {
        UpdatePhysicsBoard();
    } else {
        UpdatePhysicsWalk();
    }
}

// ---------------------------------------------------------
// WALKING PHYSICS (Bailed Mode)
// ---------------------------------------------------------
void _Player::UpdatePhysicsWalk()
{
    if (m_body->colliders.empty()) return; // safety check
    if (isFrozen) return;

    _Rigidbody *rb = m_body->GetRigidBody();

    // 1. WALL COLLISION
    // ------------------------------------------------------------------
    _Collider* playerMainCollider = m_body->colliders[0];
    bool hitX = false;
    bool hitZ = false;

    // Check X Axis
    Vector3 predictedPosX = m_body->pos;
    predictedPosX.x += rb->velocity.x * _Time::deltaTime;
    _Collider* colX = playerMainCollider->GetWorldSpaceCollider(predictedPosX, m_body->scale, m_body->rotation);
    
    if(colX) {
        for(_StaticModelInstance* staticModel : m_collidableStaticModels) {
            for (_Collider* staticCollider : staticModel->colliders) {
                // Walk mode interacts with walls
                if (staticCollider->m_type == COLLIDER_WALL) {
                    _Collider* worldCol = staticCollider->GetWorldSpaceCollider(staticModel->pos, staticModel->scale, staticModel->rotation);
                    if(worldCol) {
                        if(colX->CheckCollision(worldCol)) hitX = true;
                        delete worldCol;
                    }
                }
                if(hitX) break;
            }
            if(hitX) break;
        }
        delete colX;
    }

    // Check Z Axis
    Vector3 predictedPosZ = m_body->pos;
    predictedPosZ.z += rb->velocity.z * _Time::deltaTime;
    _Collider* colZ = playerMainCollider->GetWorldSpaceCollider(predictedPosZ, m_body->scale, m_body->rotation);
    
    if(colZ) {
        for(_StaticModelInstance* staticModel : m_collidableStaticModels) {
            for (_Collider* staticCollider : staticModel->colliders) {
                if (staticCollider->m_type == COLLIDER_WALL) {
                    _Collider* worldCol = staticCollider->GetWorldSpaceCollider(staticModel->pos, staticModel->scale, staticModel->rotation);
                    if(worldCol) {
                        if(colZ->CheckCollision(worldCol)) hitZ = true;
                        delete worldCol;
                    }
                }
                if(hitZ) break;
            }
            if(hitZ) break;
        }
        delete colZ;
    }

    // Stop velocity on hit
    if (hitX) rb->velocity.x = 0;
    if (hitZ) rb->velocity.z = 0;

    // 2. GROUND DETECTION (Simple Floor & Stairs)
    // ------------------------------------------------------------------
    rb->isGrounded = false;
    
    _Collider* playerCurrent = playerMainCollider->GetWorldSpaceCollider(m_body->pos, m_body->scale, m_body->rotation);
    
    if (playerCurrent) {
        for(_StaticModelInstance* staticModel : m_collidableStaticModels) {
            for (_Collider* staticCollider : staticModel->colliders) {
                // Walking interacts with Floors and Stairs
                if (staticCollider->m_type == COLLIDER_FLOOR || staticCollider->m_type == COLLIDER_STAIRS) {
                    
                    _Collider* worldCol = staticCollider->GetWorldSpaceCollider(staticModel->pos, staticModel->scale, staticModel->rotation);
                    if (worldCol) {
                        if (playerCurrent->CheckCollision(worldCol)) {
                            // Standard Ground Check
                            rb->isGrounded = true; 
                            
                            // Prevent falling through floor
                            if(rb->velocity.y < 0) rb->velocity.y = 0;
                            
                            // Basic stair step-up logic (if needed, or just treat as slope/floor)
                            // For this simple walk mode, treating stairs as floor usually works fine 
                            // if the collider boxes are ramped or small enough.
                        }
                        delete worldCol;
                    }
                }
            }
        }
        delete playerCurrent;
    }

    // 3. MOVEMENT PHYSICS (Snappy, High Friction)
    // ------------------------------------------------------------------
    m_state = STATE_BAILED;

    // Force Upright Rotation (No tilting while walking)
    m_body->rotation.x = 0;
    m_body->rotation.z = 0;

    if (rb->isGrounded) 
    {
        // High Friction for instant stopping
        float frictionAmount = m_walkFriction * _Time::deltaTime;
        rb->velocity.x -= rb->velocity.x * frictionAmount;
        rb->velocity.z -= rb->velocity.z * frictionAmount;
        
        // Hard stop if slow enough (prevents micro-sliding)
        if(abs(rb->velocity.x) < 0.1f) rb->velocity.x = 0;
        if(abs(rb->velocity.z) < 0.1f) rb->velocity.z = 0;
    }
    
    // 4. ANIMATION & UPDATE
    // ------------------------------------------------------------------
    if(rb->velocity.x != 0 || rb->velocity.z != 0) {
        m_body->PlayAnimation("walk", 1.0f);
    } else {
        m_body->PlayAnimation("idle", 1.0f);
    }

    // Apply rotation based on camera/input direction
    m_body->rotation.y = m_playerYaw;
    m_body->Update();
}

// ---------------------------------------------------------
// SKATING PHYSICS
// ---------------------------------------------------------
void _Player::UpdatePhysicsBoard()
{
    if (m_body->colliders.empty()) return;
    _Rigidbody *rb = m_body->GetRigidBody();
    m_currentRail = nullptr;

    // 1. WALL COLLISION (Ignore Rails)
    // ------------------------------------------------------------------
    bool hitX = false;
    bool hitZ = false;
    _Collider* playerMainCollider = m_body->colliders[0];

    // Check X
    Vector3 predictedPosX = m_body->pos;
    predictedPosX.x += rb->velocity.x * _Time::deltaTime;
    _Collider* colX = playerMainCollider->GetWorldSpaceCollider(predictedPosX, m_body->scale, m_body->rotation);
    if(colX) {
        for(_StaticModelInstance* staticModel : m_collidableStaticModels) {
            for (_Collider* staticCollider : staticModel->colliders) {
                // Ignore rails and stairs for stopping velocity when skating
                if (staticCollider->m_type == COLLIDER_WALL) { 
                    _Collider* worldCol = staticCollider->GetWorldSpaceCollider(staticModel->pos, staticModel->scale, staticModel->rotation);
                    if(worldCol && colX->CheckCollision(worldCol)) hitX = true;
                    delete worldCol;
                }
            }
        }
        delete colX;
    }

    // Check Z
    Vector3 predictedPosZ = m_body->pos;
    predictedPosZ.z += rb->velocity.z * _Time::deltaTime;
    _Collider* colZ = playerMainCollider->GetWorldSpaceCollider(predictedPosZ, m_body->scale, m_body->rotation);
    if(colZ) {
        for(_StaticModelInstance* staticModel : m_collidableStaticModels) {
            for (_Collider* staticCollider : staticModel->colliders) {
                if (staticCollider->m_type == COLLIDER_WALL) {
                    _Collider* worldCol = staticCollider->GetWorldSpaceCollider(staticModel->pos, staticModel->scale, staticModel->rotation);
                    if(worldCol && colZ->CheckCollision(worldCol)) hitZ = true;
                    delete worldCol;
                }
            }
        }
        delete colZ;
    }

    if (hitX) rb->velocity.x = 0;
    if (hitZ) rb->velocity.z = 0;

    // 2. COMPLEX GROUND DETECTION (Floor, Rails, Vert)
    // ------------------------------------------------------------------
    rb->isGrounded = false;
    bool isOnRail = false;
    bool isOnVert = false;

    _Collider* playerCurrent = playerMainCollider->GetWorldSpaceCollider(m_body->pos, m_body->scale, m_body->rotation);

    if (playerCurrent) {
        for(_StaticModelInstance* staticModel : m_collidableStaticModels) {
            for (_Collider* staticCollider : staticModel->colliders) {
                _Collider* worldCol = staticCollider->GetWorldSpaceCollider(staticModel->pos, staticModel->scale, staticModel->rotation);
                if (worldCol) {
                    if (playerCurrent->CheckCollision(worldCol)) {
                        
                        // A. Floor
                        if (staticCollider->m_type == COLLIDER_FLOOR) {
                            rb->isGrounded = true;
                        }
                        // B. Rail
                        else if (staticCollider->m_type == COLLIDER_RAIL) {
                            if (rb->velocity.y <= 0.1) {
                                isOnRail = true;
                                m_currentRail = staticModel;
                            }
                        }
                        // C. Halfpipe (Vert Logic)
                        else if (staticCollider->m_type == COLLIDER_HALFPIPE) {
                            // Local Space Transform
                            Vector3 relPos = m_body->pos - staticModel->pos;
                            float rad = staticModel->rotation.y * PI / 180.0f;
                            float c = cos(rad);
                            float s = sin(rad);
                            float localX = relPos.x * c - relPos.z * s;
                            float localZ = relPos.x * s + relPos.z * c;
                        
                            float halfWidth = staticModel->scale.x;
                            float halfDepth = staticModel->scale.z; 
                            float halfHeight = staticModel->scale.y;
                        
                            // Check bounds
                            if (localX >= -halfWidth - 0.5f && localX <= halfWidth + 0.5f) {
                                float radius = halfHeight * 2.0f;
                                float wallZ = -halfDepth;
                                float distFromWall = localZ - wallZ;
                            
                                if (distFromWall >= -0.5f) {
                                    if (distFromWall > radius) distFromWall = radius;
                                
                                    float xCircle = radius - distFromWall;
                                    if (xCircle < 0) xCircle = 0;
                                
                                    float circleY = radius - sqrt(pow(radius, 2) - pow(xCircle, 2));
                                    float pipeBottomY = staticModel->pos.y - halfHeight;
                                
                                    // Calculate Visual & Geometry Offsets
                                    float slopeRad = asin(xCircle / radius);
                                    float cosTheta = cos(slopeRad);
                                    if(cosTheta < 0.3f) cosTheta = 0.3f; 
                                    float geometryOffset = 1.0f / cosTheta; 
                                    float visualTweak = 2.5f; 
                                    float finalY = pipeBottomY + circleY + geometryOffset + visualTweak;
                                
                                    // Check Height threshold
                                    if (m_body->pos.y <= finalY + 2.0f && m_body->pos.y >= pipeBottomY - 1.0f)
                                    {
                                        isOnVert = true;
                                        rb->isGrounded = true;
                                        m_body->pos.y = finalY;
                                        if(rb->velocity.y < 0) rb->velocity.y = 0;
                                    
                                        // Apply rotation
                                        float slopeDeg = slopeRad * (180.0f / PI);
                                        m_body->rotation.x = slopeDeg;
                                        m_body->rotation.z = 0;
                                    }
                                }
                            }
                        }
                        // D. Stairs
                        else if (staticCollider->m_type == COLLIDER_STAIRS) {
                             // Simplified Stairs collision (treat as ramp for now)
                             rb->isGrounded = true;
                             if(rb->velocity.y < 0) rb->velocity.y = 0;
                        }
                    }
                    delete worldCol;
                }
            }
        }
        delete playerCurrent;
    }

    // 3. STATE MACHINE & MOMENTUM
    // ------------------------------------------------------------------
    if (rb->isGrounded) {
        m_state = isOnVert ? STATE_VERT : STATE_GROUNDED;
        if(rb->velocity.y < 0) rb->velocity.y = 0;

        // Low Friction (Momentum-based)
        float horizSpeed = sqrt(rb->velocity.x * rb->velocity.x + rb->velocity.z * rb->velocity.z);
        if (horizSpeed > 0.01f) {
            float frictionAmount = m_friction * _Time::deltaTime;
            rb->velocity.x -= rb->velocity.x * frictionAmount;
            rb->velocity.z -= rb->velocity.z * frictionAmount;

            if (horizSpeed > m_maxSpeed) {
                rb->velocity.x = (rb->velocity.x / horizSpeed) * m_maxSpeed;
                rb->velocity.z = (rb->velocity.z / horizSpeed) * m_maxSpeed;
            }
        }

        // Return rotation to flat if not on vert
        if(!isOnVert) m_body->rotation.x *= 0.95f;
    }
    else if (isOnRail) {
        // Rail Snapping Logic
        if (m_state != STATE_GRINDING) m_preGrindYaw = m_playerYaw;
        m_state = STATE_GRINDING;
        
        float railYawRad = m_currentRail->rotation.y * PI / 180.0f;
        Vector3 railDir(-sin(railYawRad), 0, -cos(railYawRad));
        railDir.normalize();

        Vector3 intentDir = rb->velocity;
        intentDir.normalize();

        if ((intentDir.x * railDir.x) + (intentDir.z * railDir.z) < 0) {
            railDir = railDir * -1.0f;
        }

        float speed = sqrt(rb->velocity.x * rb->velocity.x + rb->velocity.z * rb->velocity.z);
        rb->velocity.x = railDir.x * speed;
        rb->velocity.z = railDir.z * speed;
        rb->velocity.y = 0; 

        // Snap Position
        Vector3 diff = m_body->pos - m_currentRail->pos;
        float distAlongRail = (diff.x * railDir.x) + (diff.z * railDir.z);
        Vector3 newPos = m_currentRail->pos + (railDir * distAlongRail);
        float railTopY = m_currentRail->pos.y + (m_currentRail->scale.y * 1.0f);
        newPos.y = railTopY + 0.9f; 
        m_body->pos = newPos;

        // Visual Rotation
        float moveAngle = atan2(-railDir.x, -railDir.z) * 180.0f / PI;
        m_playerYaw = moveAngle + 90.0f;
        
        rb->isGrounded = true;
    }
    else {
        m_state = STATE_AIR;
    }

    // 4. ANIMATION & SYNC
    // ------------------------------------------------------------------
    m_body->rotation.y = m_playerYaw;
    m_body->Update();

    if(m_isOnBoard){
        Vector3 rotatedOffset = CalculateBoardOffset(m_skateboardOffset, m_body->rotation);
        m_skateboard->pos = m_body->pos + rotatedOffset;
        m_skateboard->rotation = m_body->rotation;
    }

    // Choose Animation
    float speed = sqrt(rb->velocity.x * rb->velocity.x + rb->velocity.z * rb->velocity.z);
    
    if (m_state == STATE_GRINDING) m_body->PlayAnimation("idle", 1.0f); 
    else if (m_state == STATE_AIR) m_body->PlayAnimation("idle", 1.0f); 
    else if (speed > 0.1f) m_body->PlayAnimation("walk", 1.0f); 
    else m_body->PlayAnimation("idle", 1.0f);
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