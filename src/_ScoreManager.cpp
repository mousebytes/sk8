#include "_ScoreManager.h"

_ScoreManager::_ScoreManager() {
    m_totalScore = 0;
    m_currentComboScore = 0;
    m_multiplier = 1;
    m_hudFont = new _fonts();
    m_popupFont = new _fonts();
    
    m_timeLimit = 0.0f;
    m_currentTime = 0.0f;
    m_targetScore = 0;
    m_tagsCollected = 0;
    m_tagsTarget = 0;
    
    m_isTimed = false;
    m_isTagMode = false;
    m_gameState = GAME_PLAYING;
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

void _ScoreManager::SetScoreObjective(int targetScore, float timeLimit) {
    m_targetScore = targetScore;
    m_timeLimit = timeLimit;
    m_currentTime = timeLimit;
    
    m_isTimed = true;
    m_isTagMode = false;
    m_gameState = GAME_PLAYING;
    
    m_totalScore = 0; // Reset score on level start
}

void _ScoreManager::SetTagObjective(int totalTags, float timeLimit) {
    m_tagsTarget = totalTags;
    m_tagsCollected = 0;
    m_timeLimit = timeLimit;
    m_currentTime = timeLimit;
    
    m_isTimed = (timeLimit > 0); // Timer is optional for tagging
    m_isTagMode = true;
    m_gameState = GAME_PLAYING;
    
    m_totalScore = 0;
}

void _ScoreManager::CollectTag() {
    if (m_gameState != GAME_PLAYING) return;
    
    m_tagsCollected++;
    SpawnPopup(Vector3(0,0,0), "TAGGED! " + to_string(m_tagsCollected) + "/" + to_string(m_tagsTarget));
    
    if (m_tagsCollected >= m_tagsTarget) {
        m_gameState = GAME_WON;
        SpawnPopup(Vector3(0,0,0), "LEVEL COMPLETE!");
    }
}

void _ScoreManager::Update() {
    // 1. Update Timer
    if (m_gameState == GAME_PLAYING && m_isTimed) {
        m_currentTime -= _Time::deltaTime;
        
        if (m_currentTime <= 0.0f) {
            m_currentTime = 0.0f;
            m_gameState = GAME_LOST;
            SpawnPopup(Vector3(0,0,0), "TIME UP!");
        }
    }

    // 2. Check Score Win Condition
    if (m_gameState == GAME_PLAYING && !m_isTagMode && m_targetScore > 0) {
        if (m_totalScore >= m_targetScore) {
            m_gameState = GAME_WON;
            SpawnPopup(Vector3(0,0,0), "TARGET REACHED!");
        }
    }

    // 3. Update Popups
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
    
    glPushMatrix(); 
    glLoadIdentity(); 
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    
    // --- DRAW HUD ---
    
    // 1. Score (Top Left)
    m_hudFont->setPosition(-2.1f, 1.0f, -3.0f); 
    m_hudFont->setSize(0.1f, 0.1f); 
    
    string scoreStr = "Score: " + to_string(m_totalScore);
    if (m_currentComboScore > 0) {
        scoreStr += " + " + to_string(m_currentComboScore);
        if (m_multiplier > 1) scoreStr += " x" + to_string(m_multiplier);
    }
    m_hudFont->drawText(scoreStr);

    // 2. Timer (Top Center)
    if (m_isTimed) DrawTimer();

    // 3. Objectives (Top Right)
    DrawObjectives();

    // 4. Win/Loss Messages (Center Screen)
    if (m_gameState != GAME_PLAYING) DrawWinLoss();
    
    // 5. Popups (Center Screen Overlay)
    for (auto p : m_popups) {
        float scale = 0.1f;
        float spacing = scale * 1.5f;
        float textWidth = p->text.length() * spacing;
        
        // Center text
        m_popupFont->setPosition(-textWidth / 2.0f + p->pos.x, p->pos.y, -3.0f);
        m_popupFont->setSize(scale, scale); 
        m_popupFont->drawText(p->text);
    }
    
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glPopMatrix(); 
}

void _ScoreManager::DrawTimer() {
    // Format Seconds into MM:SS
    int minutes = (int)m_currentTime / 60;
    int seconds = (int)m_currentTime % 60;
    
    string minStr = (minutes < 10 ? "0" : "") + to_string(minutes);
    string secStr = (seconds < 10 ? "0" : "") + to_string(seconds);
    string timeStr = minStr + ":" + secStr;
    
    // Center logic (approximate)
    m_hudFont->setPosition(1.3f, 1.0f, -3.0f); 
    m_hudFont->setSize(0.1f, 0.1f);
    
    // Color code urgency
    if (m_currentTime < 10.0f) glColor3f(1.0f, 0.0f, 0.0f); // Red
    else glColor3f(1.0f, 1.0f, 1.0f); // White
    
    m_hudFont->drawText(timeStr);
    glColor3f(1.0f, 1.0f, 1.0f); // Reset
}

void _ScoreManager::DrawObjectives() {
    m_hudFont->setSize(0.08f, 0.08f); // Slightly smaller
    m_hudFont->setPosition(1.0f, -1.0f, -3.0f); // Top Right
    
    if (m_isTagMode) {
        string tagStr = "Tags: " + to_string(m_tagsCollected) + "/" + to_string(m_tagsTarget);
        m_hudFont->drawText(tagStr);
    } 
    else if (m_targetScore > 0) {
        string targetStr = "Goal: " + to_string(m_targetScore);
        m_hudFont->drawText(targetStr);
    }
}

void _ScoreManager::DrawWinLoss() {
    m_hudFont->setSize(0.2f, 0.2f); // Big Text
    
    if (m_gameState == GAME_WON) {
        string msg = "COMPLETE!";
        // Center text math
        float width = msg.length() * (0.2f * 1.5f);
        m_hudFont->setPosition(-width/2, 0.2f, -3.0f);
        glColor3f(0.0f, 1.0f, 0.0f); // Green
        m_hudFont->drawText(msg);
    }
    else if (m_gameState == GAME_LOST) {
        string msg = "FAILED";
        float width = msg.length() * (0.2f * 1.5f);
        m_hudFont->setPosition(-width/2, 0.2f, -3.0f);
        glColor3f(1.0f, 0.0f, 0.0f); // Red
        m_hudFont->drawText(msg);
    }
    glColor3f(1.0f, 1.0f, 1.0f);
}

void _ScoreManager::AddScore(int points) {
    if(m_gameState != GAME_PLAYING) return;
    m_totalScore += points;
    SpawnPopup(Vector3(0,0,0), "+" + to_string(points)); 
}

void _ScoreManager::AddTrickScore(int points) {
    if(m_gameState != GAME_PLAYING) return;
    m_currentComboScore += points;
}

void _ScoreManager::AddMultiplier(int amount) {
    if(m_gameState != GAME_PLAYING) return;
    m_multiplier += amount;
}

void _ScoreManager::RegisterAirTime(float time) {
    if(m_gameState != GAME_PLAYING) return;
    
    // Threshold: Ignore tiny hops (less than 0.5s)
    if (time > 0.5f) {
        int points = (int)(time * 100.0f); // 500 points per second
        m_currentComboScore += points;
        
        // Spawn special popup
        string msg = "AIR TIME: " + to_string(points);
        SpawnPopup(Vector3(0,0.5f,0), msg);
    }
}

void _ScoreManager::LandCombo() {
    if(m_gameState != GAME_PLAYING) {
        m_currentComboScore = 0;
        m_multiplier = 1;
        return;
    }
    if (m_currentComboScore > 0) {
        int final = m_currentComboScore * m_multiplier;
        m_totalScore += final;
        SpawnPopup(Vector3(0,0,0), "COMBO: " + to_string(final));
        m_currentComboScore = 0;
        m_multiplier = 1;
    }
}

void _ScoreManager::Bail() {
    if (m_currentComboScore > 0) {
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
    p->floatSpeed = 0.5f; 
    m_popups.push_back(p);
}