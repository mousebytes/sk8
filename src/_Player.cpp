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
    m_cameraPitch = 0.0f;
    m_playerYaw = 0.0f;
    m_moveSpeed = 70.0f;
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
    // apply mouse movement to our brain's rotations
    m_playerYaw -= deltaX * m_mouseSensitivity;
    m_cameraPitch -= deltaY * m_mouseSensitivity;

    // clamp vertical rotation
    if(m_cameraPitch > 89.0f) {
        m_cameraPitch = 89.0f;
    }
    if(m_cameraPitch < -89.0f) {
        m_cameraPitch = -89.0f;
    }

    // apply the player's yaw (left/right) to the model's rotation
    // the model will turn with the camera
    m_body->rotation.y = m_playerYaw;
}

void _Player::HandleKeys(WPARAM wParam)
{
    
    // calculate forward and right vectors based on player's yaw
    Vector3 fwd;
    fwd.x = -sin(m_playerYaw * PI / 180.0);
    fwd.z = -cos(m_playerYaw * PI / 180.0);

    Vector3 right;
    right.x = cos(m_playerYaw * PI / 180.0);
    right.z = -sin(m_playerYaw * PI / 180.0);

    Vector3 moveDir = Vector3(0,0,0);

    // check which key was pressed and build a movement direction
    if (wParam == 'W') moveDir = moveDir + fwd;
    if (wParam == 'S') moveDir = moveDir - fwd;
    if (wParam == 'A') moveDir = moveDir - right;
    if (wParam == 'D') moveDir = moveDir + right;

    // set the body's velocity
    // we only set X and Z, Y (gravity) is handled in UpdatePhysics
    m_body->velocity.x = moveDir.x * m_moveSpeed;
    m_body->velocity.z = moveDir.z * m_moveSpeed;
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
    // We check for wall collisions before checking for ground
    // we must assume m_body has colliders, pos, scale, and rotation
    // we also assume m_body->colliders[0] is the player's main hitbox
    if (m_body->colliders.empty()) return; // safety check

    _Collider* playerMainCollider = m_body->colliders[0];
    
    // check x axis movement
    bool hitX = false;
    Vector3 predictedPosX = m_body->pos;
    predictedPosX.x += m_body->velocity.x * _Time::deltaTime; // predict x only move
    
    // get player's collider at this predicted X position
    _Collider* playerPredictedColliderX = playerMainCollider->GetWorldSpaceCollider(predictedPosX, m_body->scale, m_body->rotation);
    
    if(playerPredictedColliderX)
    {
        for(_StaticModelInstance* staticModel : m_collidableStaticModels) {
            for (_Collider* staticCollider : staticModel->colliders) {
                // get the static collider in world space
                _Collider* staticWorldCollider = staticCollider->GetWorldSpaceCollider(staticModel->pos, staticModel->scale, staticModel->rotation);
                if(staticWorldCollider) {
                    // check for collision AND if the collider is a wall
                    if (playerPredictedColliderX->CheckCollision(staticWorldCollider) && staticCollider->m_type == COLLIDER_WALL) {
                        hitX = true;
                    }
                    delete staticWorldCollider; // clean up temp collider
                }
                if(hitX) break;
            }
            if(hitX) break;
        }
        delete playerPredictedColliderX; // clean up temp collider
    }

    // check z axis movement
    bool hitZ = false;
    Vector3 predictedPosZ = m_body->pos;
    predictedPosZ.z += m_body->velocity.z * _Time::deltaTime; // predict z only move
    
    // get player's collider at this predicted Z position
    _Collider* playerPredictedColliderZ = playerMainCollider->GetWorldSpaceCollider(predictedPosZ, m_body->scale, m_body->rotation);
    
    if(playerPredictedColliderZ)
    {
        for(_StaticModelInstance* staticModel : m_collidableStaticModels) {
            for (_Collider* staticCollider : staticModel->colliders) {
                _Collider* staticWorldCollider = staticCollider->GetWorldSpaceCollider(staticModel->pos, staticModel->scale, staticModel->rotation);
                if(staticWorldCollider) {
                    // check for collision AND if the collider is a wall
                    if (playerPredictedColliderZ->CheckCollision(staticWorldCollider) && staticCollider->m_type == COLLIDER_WALL) {
                        hitZ = true;
                    }
                    delete staticWorldCollider; // clean up temp collider
                }
                if(hitZ) break;
            }
            if(hitZ) break;
        }
        delete playerPredictedColliderZ; // clean up temp collider
    }

    // zero out velocity for the axis that hit
    if (hitX) m_body->velocity.x = 0;
    if (hitZ) m_body->velocity.z = 0;

    // --- Ground Check & Gravity Logic ---
    // now we check for ground, after wall slides are handled
    m_body->isGrounded = false;
    
    // get the collider at our current position
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
                    // check collision at our current position
                    if (playerCurrentCollider->CheckCollision(staticWorldCollider))
                    {
                        // check if it's a floor
                        if (staticCollider->m_type == COLLIDER_FLOOR)
                        {
                            m_body->isGrounded = true;
                            if(m_body->velocity.y < 0) { // if falling
                                m_body->velocity.y = 0; // stop falling
                            }
                            
                            // TODO: Add push-up logic here if player is sunk into the floor
                            // (ex: m_body->pos.y = staticWorldCollider->max.y + playerRadius)
                        }
                    }
                    delete staticWorldCollider; // clean up temp collider
                }
                if (m_body->isGrounded) break;
            }
            if (m_body->isGrounded) break;
        }
        delete playerCurrentCollider; // clean up temp collider
    }

    // TODO: add a loop here to check against m_collidableAnimatedModels
    // (ex: for player enemy, player NPC, etc)
    // for (_AnimatedModelInstance* animModel : m_collidableAnimatedModels) { ... }

    // applies gravity if not grounded
    // and movement velocity (now corrected for walls)
    m_body->Update();

    // ---- ANIMATION STATE SELECTION ----
    if (!m_body->isGrounded) {
        // m_body->PlayAnimation("jump", 1.0f); 
        m_body->PlayAnimation("idle", 1.0f); 
    } else if (abs(m_body->velocity.x) > 0.01f || abs(m_body->velocity.z) > 0.01f) {
        // m_body->PlayAnimation("run", 1.0f);
        m_body->PlayAnimation("walk", 1.0f); 
    } else{
        // player is on the ground and not moving
        m_body->PlayAnimation("idle", 1.0f);
    }

    // reset horz velocity so the player stops when
    // no keys are pressed next frame
    m_body->velocity.x = 0;
    m_body->velocity.z = 0;
}

void _Player::UpdateCamera(_camera* cam)
{
    // don't force the cam into pos if it's in free cam
    if(cam->isFreeCam) {isFrozen=true; return;}
    else {isFrozen=false;}

    // how high the eyes are above the model's origin
    float eyeHeight = 1.1f; 
    
    // set camera's eye position to the player's body position
    cam->eye = m_body->pos;
    // add eye height
    cam->eye.y += eyeHeight; 

    // set camera's rotation angles
    cam->rotAngle.x = m_playerYaw;
    cam->rotAngle.y = m_cameraPitch;

    // calculate the "look at" point (des)
    // spherical to cartesian conversion
    float pitchRad = m_cameraPitch * PI / 180.0;
    float yawRad = m_playerYaw * PI / 180.0;

    cam->des.x = cam->eye.x - sin(yawRad) * cos(pitchRad);
    cam->des.z = cam->eye.z - cos(yawRad) * cos(pitchRad);
    cam->des.y = cam->eye.y + sin(pitchRad);
    
    // set the up vector (always Y-up)
    cam->up = Vector3(0, 1, 0);
}

void _Player::Draw()
{
    // we just tell our body to draw itself
    m_body->Draw();
}