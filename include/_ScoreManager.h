#ifndef _SCOREMANAGER_H
#define _SCOREMANAGER_H

#include "_common.h"
#include "_fonts.h"
#include "_camera.h"
#include "_sounds.h"
// Simple struct for floating text data
struct ScorePopup {
    Vector3 pos;
    string text;
    float lifeTime; // 0.0 to 1.0
    float floatSpeed;
};

enum GameState {
    GAME_PLAYING,
    GAME_WON,
    GAME_LOST
};

class _ScoreManager {
public:
    _ScoreManager();
    ~_ScoreManager();

    void Init();
    void Update();
    void Draw(_camera* cam, int screenWidth, int screenHeight); 

    // --- Gameplay Methods ---
    void AddScore(int points);
    void AddTrickScore(int points);
    void AddMultiplier(int amount = 1);
    void RegisterAirTime(float time);
    void LandCombo();                     
    void Bail();   
    void SetFinalLevel(bool flag);                       
    
    // --- Level Objectives ---
    // Setup for Level 1: "Reach X Score in Y Seconds"
    void SetScoreObjective(int targetScore, float timeLimit);

    // Setup for Level 2: "Tag X Spots"
    void SetTagObjective(int totalTags, float timeLimit);

    void SetFreePlay(); // Resets everything for custom/sandbox mode

    void CollectTag(); // Call this when player touches a graffiti spot

    GameState GetState() { return m_gameState; }

    void SetBalanceValue(float val, bool show); // val is -1.0 to 1.0
    void SetSoundManager(_sounds* mgr);

private:
    // Scoring State
    int m_totalScore;
    int m_currentComboScore;
    int m_multiplier;

    // Objectives State
    float m_timeLimit;
    float m_currentTime;

    int m_targetScore;       // For Score Attack
    int m_tagsCollected;     // For Tag Attack
    int m_tagsTarget;        // Total tags needed

    bool m_isTimed;
    bool m_isTagMode;
    bool m_isFinalLevel;
    
    GameState m_gameState;

    // Resources
    _fonts* m_hudFont;
    _fonts* m_popupFont;

    _sounds* m_soundMgr;

    _textureLoader* m_winTex;
    _textureLoader* m_loseTex;

    // Active Popups
    vector<ScorePopup*> m_popups;

    // Helper to spawn text
    void SpawnPopup(Vector3 pos, string text, int points = 0);

    // Helpers for HUD
    void DrawTimer();
    void DrawObjectives();
    void DrawWinLoss(int width, int height);

    float m_balanceValue;
    bool m_showBalanceMeter;
    void DrawBalanceMeter();

    _sounds* m_soundMgr;
};

#endif
