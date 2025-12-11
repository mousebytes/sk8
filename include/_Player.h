#ifndef _PLAYER_H
#define _PLAYER_H
#include"_camera.h"
#include"_inputs.h"
#include"_common.h"
#include"_AnimatedModelInstance.h"
#include"_StaticModelInstance.h"
#include"_ScoreManager.h"
#include"_ParticleSystem.h"

enum PlayerState {
    STATE_GROUNDED,
    STATE_AIR,
    STATE_GRINDING,
    STATE_BAILED, // for when off board
    STATE_VERT
};

class _Player{
    public:
    _Player(_AnimatedModel* modelBlueprint, _AnimatedModel* boardBlueprint);
    ~_Player();

    // player body
    _AnimatedModelInstance* m_body;
    _AnimatedModelInstance* m_skateboard;

    

    // --- Player Physics ---
    float m_playerYaw;      // The direction the player's body is facing (Y-axis rotation)
    float m_acceleration;   // How fast the player gains speed
    float m_maxSpeed;       // The maximum speed the player can reach by pushing
    float m_turnSpeed;      // How fast the player can turn (degrees per second)
    float m_friction;       // How much (or little) the player slows down naturally
    float m_jumpForce;      // Upward velocity applied when jumping
    PlayerState m_state;    // Current state (grounded, air, etc.)

    float m_airTime;        // Tracks duration in STATE_AIR

    // --- Walking Physics Vars
    float m_walkSpeed;      // Max walking speed
    float m_walkAccel;      // Walking acceleration (snappy)
    float m_walkFriction;   // High friction to stop instantly
    float m_walkTurnSpeed;  // Turning speed on foot

    // --- Grind Physics ---
    float m_grindBalance;       // Current balance (-1.0 to 1.0)
    float m_grindBalanceVel;    // How fast balance is slipping
    float m_grindInstability;   // How hard it is to balance (increases over time)

    float m_preGrindYaw;

    bool m_isOnBoard;
    Vector3 m_skateboardOffset;

    // --- Camera Control ---
    float m_cameraYaw;      // Camera's horizontal orbit around player
    float m_cameraPitch;    // Camera's vertical orbit around player
    float m_camDistance;    // How far the camera is from the player
    float m_camHeight;      // How high the camera's "look at" point is
    float m_mouseSensitivity;

    vector<_StaticModelInstance*> m_collidableStaticModels;
    vector<_AnimatedModelInstance*> m_collidableAnimatedModels;

    _StaticModelInstance* m_currentRail;

    bool isFrozen;

    // --- Kickflip Vars ---
    bool m_isKickflipping;
    float m_kickflipProgress;

    _ParticleSystem *m_particleSystem;

    void SetParticleSystem(_ParticleSystem* p_sys) { m_particleSystem = p_sys; }

    // takes raw input deltas
    void HandleMouse(float deltaX, float deltaY);

    // take key presses
    bool inputW, inputA, inputS, inputD, inputH;
    bool inputSpace;

    // HandleKeys accepts the message type (to distinguish Up/Down)
    void HandleKeys(UINT uMsg, WPARAM wParam);

    // public methods to register colliders
    void RegisterStaticCollider(_StaticModelInstance* model);
    void RegisterAnimatedCollider(_AnimatedModelInstance* model);
    void ClearColliders();

    // handle gravity, collisions, apply velocity
    void UpdatePhysics();
    void UpdatePhysicsBoard(); // skating physics
    void UpdatePhysicsWalk();  // walking physics

    // set camera position and lookat
    void UpdateCamera(_camera* cam);

    // calls m_body->draw for now
    void Draw();

    // helper to reset board pos
    void ResetBoard();

    // helper to rotate the offset
    Vector3 CalculateBoardOffset(Vector3 baseOffset, Vector3 rotation);

    _ScoreManager* m_scoreMgr; 
    float m_scoreAccumulator; // Used to track partial points during grinds

    
    // Update Set Function
    void SetScoreManager(_ScoreManager* mgr) { m_scoreMgr = mgr; }
};



#endif //_PLAYER_H