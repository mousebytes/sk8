#ifndef _SCOREMANAGER_H
#define _SCOREMANAGER_H

#include "_common.h"
#include "_fonts.h"
#include "_camera.h"

// Simple struct for floating text data
struct ScorePopup {
    Vector3 pos;
    string text;
    float lifeTime; // 0.0 to 1.0
    float floatSpeed;
};

class _ScoreManager {
public:
    _ScoreManager();
    ~_ScoreManager();

    void Init();
    void Update();
    void Draw(_camera* cam); // Needs camera to make popups face user/billboard

    // --- Gameplay Methods ---
    void AddScore(int points);            // Instant points (pickups)
    void AddTrickScore(int points);       // Accumulates in "current combo"
    void AddMultiplier(int amount = 1);   // Increases combo multiplier
    void LandCombo();                     // Banks the combo score
    void Bail();                          // Loses the combo score

private:
    // Scoring State
    int m_totalScore;
    int m_currentComboScore;
    int m_multiplier;

    // Resources
    _fonts* m_hudFont;     // For the main score
    _fonts* m_popupFont;   // For floating text

    // Active Popups
    vector<ScorePopup*> m_popups;

    // Helper to spawn text
    void SpawnPopup(Vector3 pos, string text, int points = 0);
};

#endif