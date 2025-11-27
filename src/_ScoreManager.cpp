#include "_ScoreManager.h"

_ScoreManager::_ScoreManager() {
    m_totalScore = 0;
    m_currentComboScore = 0;
    m_multiplier = 1;
    m_hudFont = new _fonts();
    m_popupFont = new _fonts();
}

_ScoreManager::~_ScoreManager() {
    delete m_hudFont;
    delete m_popupFont;
    for (auto p : m_popups) delete p;
    m_popups.clear();
}

void _ScoreManager::Init() {
    m_hudFont->initFonts("images/fontsheet.png", 15, 8);
    m_popupFont->initFonts("images/fontsheet.png", 15, 8);
}

void _ScoreManager::Update() {
    for (auto it = m_popups.begin(); it != m_popups.end();) {
        ScorePopup* p = *it;
        p->pos.y += p->floatSpeed * _Time::deltaTime;
        p->lifeTime -= _Time::deltaTime;

        if (p->lifeTime <= 0) {
            delete p;
            it = m_popups.erase(it);
        } else {
            ++it;
        }
    }
}

void _ScoreManager::Draw(_camera* cam) {
    
    // We want both the HUD and the Popups to be in Screen Space now
    // So we invoke the 2D overlay setup immediately
    
    glPushMatrix(); // Save the current World View (Camera)
    glLoadIdentity(); // Reset view to (0,0,0) looking down -Z
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    
    // 1. DRAW POPUPS (Now Screen Space & Centered)
    for (auto p : m_popups) {
        // Calculate text width to center it
        // (Assuming scale 0.1 and spacing factor 1.5 from _fonts.cpp)
        float scale = 0.1f;
        float spacing = scale * 1.5f;
        float textWidth = p->text.length() * spacing;
        
        // Offset X by half the width to center
        float xOffset = -textWidth / 2.0f;

        // Position relative to screen center (0,0)
        // Z = -3.0 puts it in the view frustum
        m_popupFont->setPosition(xOffset + p->pos.x, p->pos.y, -3.0f);
        m_popupFont->setSize(scale, scale); 
        m_popupFont->drawText(p->text);
    }

    // 2. DRAW HUD (Top Left)
    // Z = -3.0 puts it comfortably in front of the camera lens
    m_hudFont->setPosition(-1.5f, 1.0f, -3.0f); 
    m_hudFont->setSize(0.1f, 0.1f); 
    
    string scoreStr = "Score: " + to_string(m_totalScore);
    if (m_currentComboScore > 0) {
        scoreStr += " + " + to_string(m_currentComboScore);
        if (m_multiplier > 1) scoreStr += " x" + to_string(m_multiplier);
    }
    
    m_hudFont->drawText(scoreStr);
    
    // Restore World State
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glPopMatrix(); // Restore the World View for the rest of the scene
}

void _ScoreManager::AddScore(int points) {
    m_totalScore += points;
    // Spawn at center (0,0,0) relative to screen
    SpawnPopup(Vector3(0,0,0), "+" + to_string(points)); 
}

void _ScoreManager::AddTrickScore(int points) {
    m_currentComboScore += points;
}

void _ScoreManager::AddMultiplier(int amount) {
    m_multiplier += amount;
}

void _ScoreManager::LandCombo() {
    if (m_currentComboScore > 0) {
        int final = m_currentComboScore * m_multiplier;
        m_totalScore += final;
        // Spawn at center (0,0,0) relative to screen
        SpawnPopup(Vector3(0,0,0), "COMBO: " + to_string(final));
        m_currentComboScore = 0;
        m_multiplier = 1;
    }
}

void _ScoreManager::Bail() {
    if (m_currentComboScore > 0) {
        // Spawn at center (0,0,0) relative to screen
        SpawnPopup(Vector3(0,0,0), "BAIL!");
        m_currentComboScore = 0;
        m_multiplier = 1;
    }
}

void _ScoreManager::SpawnPopup(Vector3 pos, string text, int points) {
    ScorePopup* p = new ScorePopup();
    p->pos = pos;
    p->text = text;
    p->lifeTime = 2.0f;
    
    // Reduced float speed because screen coordinates are much smaller than world coordinates
    // 3.0f was flying off screen instantly
    p->floatSpeed = 0.5f; 
    
    m_popups.push_back(p);
}