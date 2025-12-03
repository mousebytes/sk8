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
    m_acceleration = 15.0f; // units per second^2
    m_maxSpeed = 30.0f;     // units per second
    m_turnSpeed = 360.0f;   // degrees per second
    m_friction = 0.3f;      // Damping factor (higher = stops faster)
    m_jumpForce = 8.0f;     // Initial upward velocity
    m_state = STATE_AIR;    // Start in air, will be set to grounded by physics
    m_airTime = 0.0f;

    // --- WALK PHYSICS VARS ---
    m_walkSpeed = 60.0f;         // Much slower than skating
    m_walkAccel = 60.0f;        // High acceleration for instant start
    m_walkFriction = 8.0f;       // High friction for instant stop
    m_walkTurnSpeed = 360.0f;

    m_preGrindYaw = 0.0f;
    m_scoreAccumulator = 0.0f; // Init accumulator

    // --- GRIND MINIGAME VARS ---
    m_grindBalance = 0.0f;
    m_grindBalanceVel = 0.0f;
    m_grindInstability = 1.0f;

    // --- CAMERA VARS ---
    m_camDistance = 8.0f;   // 8 units away from player
    m_camHeight = 2.0f;     // looks at a point 2 units above player's origin
    m_mouseSensitivity = 0.1f;

    m_currentRail = nullptr;

    isFrozen = false;

    m_skateboardOffset = Vector3(0,-1.05,0);
    m_skateboard->scale = Vector3(0.5,0.5,0.5);

    m_isKickflipping = false;
    m_kickflipProgress = 0.0f;

    // Init Inputs
    inputW = false;
    inputA = false;
    inputS = false;
    inputD = false;
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
}

void _Player::HandleKeys(UINT uMsg, WPARAM wParam)
{
    // Determine if key is being pressed or released
    bool isDown = (uMsg == WM_KEYDOWN);

    // 1. UPDATE STATE FLAGS (For Continuous Movement)
    switch(wParam)
    {
        case 'W': inputW = isDown; break;
        case 'S': inputS = isDown; break;
        case 'A': inputA = isDown; break;
        case 'D': inputD = isDown; break;
    }

    // 2. HANDLE ONE-SHOT ACTIONS (Instant Triggers)
    // We only want these to happen ONCE when the key is pressed down
    if (uMsg == WM_KEYDOWN) 
    {
        _Rigidbody *rb = m_body->GetRigidBody();

        // --- JUMP / GRIND JUMP ---
        if (wParam == VK_SPACE) 
        {
            if (m_state == STATE_GRINDING) {
                 // Jump out of grind
                 m_state = STATE_AIR;
                 m_body->GetRigidBody()->isGrounded = false;
                 m_body->GetRigidBody()->velocity.y = m_jumpForce;
                 m_playerYaw = m_preGrindYaw; // Restore rotation
                 if(m_scoreMgr) m_scoreMgr->SetBalanceValue(0, false);
            }
            else if (m_isOnBoard) {
                // Skating Jump
                if (m_state == STATE_GROUNDED || m_state == STATE_BAILED)
                {
                    rb->velocity.y = m_jumpForce;
                    m_state = STATE_AIR;
                    rb->isGrounded = false;
                }
                // Kickflip (In Air)
                else if (m_state == STATE_AIR && !m_isKickflipping) 
                {
                    m_isKickflipping = true;
                    m_kickflipProgress = 0.0f;
                    if(m_scoreMgr) m_scoreMgr->AddTrickScore(100); 
                }
            }
            else {
                // Walking Jump
                if (rb->isGrounded) rb->velocity.y = 5.0f;
            }
        }

        // --- TOGGLE BOARD ---
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

// ---------------------------------------------------------
// MAIN PHYSICS DISPATCHER
// ---------------------------------------------------------
void _Player::UpdatePhysics()
{
    if (isFrozen) return;

    // =========================================================
    // APPLY CONTINUOUS INPUT FORCES
    // This logic runs every frame, properly scaled by DeltaTime
    // =========================================================
    
    _Rigidbody *rb = m_body->GetRigidBody();

    // Calculate forward vector
    Vector3 fwd;
    fwd.x = -sin(m_playerYaw * PI / 180.0);
    fwd.z = -cos(m_playerYaw * PI / 180.0);
    fwd.normalize();

    if (m_state == STATE_GRINDING) {
        // --- GRIND BALANCE CONTROLS ---
        float correctionForce = 0.05f * _Time::deltaTime;
        
        // Use A/D to correct balance
        if (inputA) m_grindBalanceVel -= correctionForce;
        if (inputD) m_grindBalanceVel += correctionForce;
    }
    else if (!m_isOnBoard) {
        // --- WALKING MOVEMENT ---
        if (inputW) {
            rb->velocity.x += fwd.x * m_walkAccel * _Time::deltaTime;
            rb->velocity.z += fwd.z * m_walkAccel * _Time::deltaTime;
        }
        if (inputS) {
            rb->velocity.x -= fwd.x * m_walkAccel * _Time::deltaTime;
            rb->velocity.z -= fwd.z * m_walkAccel * _Time::deltaTime;
        }
        if (inputA) m_playerYaw += m_walkTurnSpeed * _Time::deltaTime;
        if (inputD) m_playerYaw -= m_walkTurnSpeed * _Time::deltaTime;

        // Cap walking speed
        float currentSpeed = sqrt(rb->velocity.x * rb->velocity.x + rb->velocity.z * rb->velocity.z);
        if(currentSpeed > m_walkSpeed) {
            rb->velocity.x = (rb->velocity.x / currentSpeed) * m_walkSpeed;
            rb->velocity.z = (rb->velocity.z / currentSpeed) * m_walkSpeed;
        }
    }
    else {
        // --- SKATING MOVEMENT ---
        if (inputW) {
            rb->velocity.x += fwd.x * m_acceleration * _Time::deltaTime;
            rb->velocity.z += fwd.z * m_acceleration * _Time::deltaTime;
        }
        if (inputS) {
            rb->velocity.x -= fwd.x * m_acceleration * _Time::deltaTime;
            rb->velocity.z -= fwd.z * m_acceleration * _Time::deltaTime;
        }
        
        // Turning (Tank Controls)
        if (inputA || inputD) {
            float turnDir = (inputA ? 1.0f : -1.0f);
            float turnAmount = m_turnSpeed * turnDir * _Time::deltaTime;
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
    }
    // =========================================================

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
    
    if (m_scoreMgr) m_scoreMgr->Bail();

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

    if (hitX) rb->velocity.x = 0;
    if (hitZ) rb->velocity.z = 0;

    // 2. GROUND DETECTION (Simple Floor & Stairs)
    // ------------------------------------------------------------------
    rb->isGrounded = false;
    
    _Collider* playerCurrent = playerMainCollider->GetWorldSpaceCollider(m_body->pos, m_body->scale, m_body->rotation);
    
    if (playerCurrent) {
        for(_StaticModelInstance* staticModel : m_collidableStaticModels) {
            for (_Collider* staticCollider : staticModel->colliders) {
                if (staticCollider->m_type == COLLIDER_FLOOR || staticCollider->m_type == COLLIDER_STAIRS) {
                    
                    _Collider* worldCol = staticCollider->GetWorldSpaceCollider(staticModel->pos, staticModel->scale, staticModel->rotation);
                    if (worldCol) {
                        if (playerCurrent->CheckCollision(worldCol)) {
                            rb->isGrounded = true; 
                            if(rb->velocity.y < 0) rb->velocity.y = 0;

                            // --- STAIR CLIMBING LOGIC ---
                            // If walking and hitting stairs, snap to top
                            if (staticCollider->m_type == COLLIDER_STAIRS) {
                                float stairTop = staticModel->pos.y + staticModel->scale.y;
                                // Snap player center to 1.0 unit above surface
                                m_body->pos.y = stairTop + 1.0f;
                            }
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
    m_body->rotation.x = 0;
    m_body->rotation.z = 0;

    if (rb->isGrounded) 
    {
        float frictionAmount = m_walkFriction * _Time::deltaTime;
        rb->velocity.x -= rb->velocity.x * frictionAmount;
        rb->velocity.z -= rb->velocity.z * frictionAmount;
        
        if(abs(rb->velocity.x) < 0.1f) rb->velocity.x = 0;
        if(abs(rb->velocity.z) < 0.1f) rb->velocity.z = 0;
    }
    
    // 4. ANIMATION & UPDATE
    if(rb->velocity.x != 0 || rb->velocity.z != 0) {
        m_body->PlayAnimation("walk", 1.0f);
    } else {
        m_body->PlayAnimation("idleWalk", 1.0f);
    }

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
                // Ignore rails for stopping velocity when skating
                // BUT treat STAIRS as a wall when skating!
                if (staticCollider->m_type == COLLIDER_WALL || staticCollider->m_type == COLLIDER_STAIRS) { 
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
                if (staticCollider->m_type == COLLIDER_WALL || staticCollider->m_type == COLLIDER_STAIRS) {
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
                                
                                    float slopeRad = asin(xCircle / radius);
                                    float cosTheta = cos(slopeRad);
                                    if(cosTheta < 0.3f) cosTheta = 0.3f; 
                                    float geometryOffset = 1.0f / cosTheta; 
                                    float visualTweak = 2.5f; 
                                    float finalY = pipeBottomY + circleY + geometryOffset + visualTweak;
                                
                                    if (m_body->pos.y <= finalY + 2.0f && m_body->pos.y >= pipeBottomY - 1.0f)
                                    {
                                        isOnVert = true;
                                        rb->isGrounded = true;
                                        m_body->pos.y = finalY;
                                        if(rb->velocity.y < 0) rb->velocity.y = 0;
                                    
                                        float slopeDeg = slopeRad * (180.0f / PI);
                                        m_body->rotation.x = slopeDeg;
                                        m_body->rotation.z = 0;
                                    }
                                }
                            }
                        }
                        // D. Stairs
                        else if (staticCollider->m_type == COLLIDER_STAIRS) {
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
        
        // check if we just landed from air or grind
        if (m_state == STATE_AIR || m_state == STATE_GRINDING) {
            
            // --- FIX: Restore Yaw if coming from grind ---
            if(m_state == STATE_GRINDING) {
                 m_playerYaw = m_preGrindYaw;
            }

            // --- NEW AIR TIME SCORING ---
            if(m_state == STATE_AIR && m_scoreMgr) {
                m_scoreMgr->RegisterAirTime(m_airTime);
            }
            m_airTime = 0.0f; // Reset

            // --- KICKFLIP RESET ---
            // Reset kickflip state on landing
            m_isKickflipping = false;
            m_kickflipProgress = 0.0f;

            if(m_scoreMgr) {
                m_scoreMgr->LandCombo();
                m_scoreMgr->SetBalanceValue(0, false); // Hide grind meter if we land
            }
        }

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
        // Reset lean from grind
        m_body->rotation.z = 0.0f;
    }
    else if (isOnRail) {
        
        // --- ENTER GRIND STATE ---
        if (m_state != STATE_GRINDING) {
            // Just landed on rail
            if (m_state == STATE_AIR && m_scoreMgr) {
                m_scoreMgr->RegisterAirTime(m_airTime);
            }
            m_airTime = 0.0f; 
            
            m_preGrindYaw = m_playerYaw;
            if(m_scoreMgr) m_scoreMgr->AddMultiplier(1);

            m_state = STATE_GRINDING;
            
            // --- KICKFLIP RESET ---
            // Reset kickflip state on grind
            m_isKickflipping = false;
            m_kickflipProgress = 0.0f;

            // Reset Balance logic
            m_grindBalance = 0.0f;
            m_grindBalanceVel = 0.002f; // Give a tiny initial push so it doesn't sit perfect
            m_grindInstability = 0.2f; // Start easy
        }

        // --- GRIND MINIGAME PHYSICS ---
        
        // 1. Instability grows over time
        m_grindInstability += 0.05f * _Time::deltaTime;

        // 2. Apply "Inverted Pendulum" force
        m_grindBalanceVel += m_grindBalance * 0.1f * m_grindInstability * _Time::deltaTime;
        
        // 3. Apply Velocity
        m_grindBalance += m_grindBalanceVel;

        // 4. Update Visuals
        if(m_scoreMgr) m_scoreMgr->SetBalanceValue(m_grindBalance, true);
        
        // Rotate player model to show leaning
        m_body->rotation.x = m_grindBalance * -45.0f; 

        // 5. FAIL CONDITION (Bail)
        if (abs(m_grindBalance) > 1.0f) {
            m_state = STATE_BAILED;
            m_isOnBoard = false; // Force player off board
            
            // Push player off the rail sideways
            rb->velocity.x += (m_grindBalance > 0 ? 5.0f : -5.0f);
            rb->velocity.y = 2.0f; // Little hop off
            
            if(m_scoreMgr) {
                m_scoreMgr->Bail(); // Reset Streak
                m_scoreMgr->SetBalanceValue(0, false); // Hide UI
            }
        }
        else {
            // --- EXISTING RAIL MOVEMENT CODE ---
            // Add points while grinding 
            if(m_scoreMgr) {
                m_scoreAccumulator += 100.0f * _Time::deltaTime;
                if (m_scoreAccumulator >= 1.0f) {
                    int pointsToAdd = (int)m_scoreAccumulator;
                    m_scoreMgr->AddTrickScore(pointsToAdd);
                    m_scoreAccumulator -= pointsToAdd;
                }
            }
            
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

            float moveAngle = atan2(-railDir.x, -railDir.z) * 180.0f / PI;
            m_playerYaw = moveAngle + 90.0f;
            
            rb->isGrounded = true;
        }
    }
    else {
        // --- FIX: Restore Yaw if falling off rail ---
        if (m_state == STATE_GRINDING) {
            m_playerYaw = m_preGrindYaw;
        }

        m_state = STATE_AIR;
        // --- TRACK AIR TIME ---
        m_airTime += _Time::deltaTime;
        
        // Hide grind meter in air
        if(m_scoreMgr) m_scoreMgr->SetBalanceValue(0, false);
    }

    // 4. ANIMATION & SYNC
    // ------------------------------------------------------------------
    m_body->rotation.y = m_playerYaw;
    m_body->Update();

    if(m_isOnBoard){
        Vector3 rotatedOffset = CalculateBoardOffset(m_skateboardOffset, m_body->rotation);
        m_skateboard->pos = m_body->pos + rotatedOffset;
        m_skateboard->rotation = m_body->rotation;

        // --- KICKFLIP ANIMATION LOGIC ---
        if (m_isKickflipping) {
            float spinSpeed = 720.0f; // degrees per second
            m_kickflipProgress += spinSpeed * _Time::deltaTime;

            if (m_kickflipProgress >= 360.0f) {
                m_kickflipProgress = 0.0f;
                m_isKickflipping = false;
            }
            // Apply spin to Roll (Z axis)
            m_skateboard->rotation.z += m_kickflipProgress;
        }
    }

    // Choose Animation
    float speed = sqrt(rb->velocity.x * rb->velocity.x + rb->velocity.z * rb->velocity.z);
    
    if (m_state == STATE_GRINDING) m_body->PlayAnimation("idle", 1.0f); 
    else if (m_state == STATE_AIR) m_body->PlayAnimation("idle", 1.0f); 
    else if (speed > 0.1f) m_body->PlayAnimation("kick", 1.0f); 
    else m_body->PlayAnimation("idle", 1.0f);
}



void _Player::UpdateCamera(_camera* cam)
{
    if(cam->isFreeCam) {isFrozen=true; return;}
    else {isFrozen=false;}
    
    cam->des = m_body->pos;
    cam->des.y += m_camHeight; 

    float pitchRad = m_cameraPitch * PI / 180.0;
    float yawRad = m_cameraYaw * PI / 180.0;

    cam->eye.x = cam->des.x - sin(yawRad) * cos(pitchRad) * m_camDistance;
    cam->eye.z = cam->des.z - cos(yawRad) * cos(pitchRad) * m_camDistance;
    cam->eye.y = cam->des.y + sin(pitchRad) * m_camDistance;
    
    cam->up = Vector3(0, 1, 0);

    cam->rotAngle.x = m_cameraYaw;
    cam->rotAngle.y = m_cameraPitch;
}

void _Player::Draw()
{
    m_body->Draw();
    m_skateboard->Draw();

    if (isDebug) 
    {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glLineWidth(3.0f); 

        for (auto* staticModel : m_collidableStaticModels)
        {
            float radY = staticModel->rotation.y * PI / 180.0f;
            Vector3 fwdVec(sin(radY), 0, cos(radY)); 
            Vector3 relPos = m_body->pos - staticModel->pos;
            float localZ = (relPos.x * fwdVec.x) + (relPos.z * fwdVec.z);

            float halfDepth = staticModel->scale.z;
            float rampStart = halfDepth + 1.0f; 
            float totalLen = (halfDepth * 2.0f) + 1.0f; 
            float distFromFront = rampStart - localZ;
            float t = distFromFront / totalLen;

            if (t > 1.0f || t < -0.1f) continue; 
            if (t < 0.0f) t = 0.0f;

            float halfHeight = staticModel->scale.y;
            float pipeBottomY = staticModel->pos.y - halfHeight; 
            float curveHeight = (t * t) * (halfHeight * 2.0f); 
            float targetWorldY = pipeBottomY + curveHeight; 

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
    float radX = rotation.x * PI / 180.0f;
    float radY = rotation.y * PI / 180.0f;

    float y1 = baseOffset.y * cos(radX) - baseOffset.z * sin(radX);
    float z1 = baseOffset.y * sin(radX) + baseOffset.z * cos(radX);
    float x1 = baseOffset.x; 

    float x2 = x1 * cos(radY) + z1 * sin(radY);
    float z2 = -x1 * sin(radY) + z1 * cos(radY);
    float y2 = y1; 

    return Vector3(x2, y2, z2);
}

void _Player::ResetBoard(){
    m_isOnBoard = true;
    Vector3 rotatedOffset = CalculateBoardOffset(m_skateboardOffset, m_body->rotation);
    m_skateboard->pos = m_body->pos + rotatedOffset;
    m_skateboard->rotation = m_body->rotation;
}