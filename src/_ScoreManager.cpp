#include "_ScoreManager.h"

_ScoreManager::_ScoreManager() {
    m_totalScore = 0;
    m_currentComboScore = 0;
    m_multiplier = 1;
    m_hudFont = new _fonts();
    m_popupFont = new _fonts();
    
    m_winTex = new _textureLoader();
    m_loseTex = new _textureLoader();
    
    m_timeLimit = 0.0f;
    m_currentTime = 0.0f;
    m_targetScore = 0;
    m_tagsCollected = 0;
    m_tagsTarget = 0;

    m_isTimed = false;
    m_isTagMode = false;
    m_isFinalLevel = false;
    m_gameState = GAME_PLAYING;

    m_balanceValue = 0.0f;
    m_showBalanceMeter = false;

    m_soundMgr = nullptr;
}

_ScoreManager::~_ScoreManager() {
    delete m_hudFont;
    delete m_popupFont;
    delete m_winTex;
    delete m_loseTex;
    for (auto p : m_popups) delete p;
    m_popups.clear();
}

void _ScoreManager::Init() {
    m_hudFont->initFonts("images/fontsheet.png", 15, 8);
    m_popupFont->initFonts("images/fontsheet.png", 15, 8);

    m_winTex->loadTexture("images/menus/WinScreen.png");
    m_loseTex->loadTexture("images/menus/GameOver.png");
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
    SpawnPopup(Vector3(0,0,0), "TAGGED! " + to_string(m_tagsCollected) + " of " + to_string(m_tagsTarget));

    if(m_isTagMode){
        if (m_tagsCollected >= m_tagsTarget) {
            m_gameState = GAME_WON;
            SpawnPopup(Vector3(0,-1,0), "LEVEL COMPLETE!");
        }
    }
    else{
        SpawnPopup(Vector3(0,0,0), "TAGGED! " + to_string(m_tagsCollected) + " of " + to_string(m_tagsTarget));
    }


}

void _ScoreManager::Update() {
    //Update Timer
    if (m_gameState == GAME_PLAYING && m_isTimed) {
        m_currentTime -= _Time::deltaTime;

        if (m_currentTime <= 0.0f) {
            m_currentTime = 0.0f;
            m_gameState = GAME_LOST;
            SpawnPopup(Vector3(0,0,0), "TIME UP!");
        }
    }

    //Check Score Win Condition
    if (m_gameState == GAME_PLAYING && !m_isTagMode && m_targetScore > 0) {
        if (m_totalScore >= m_targetScore) {
            m_gameState = GAME_WON;
            SpawnPopup(Vector3(0,0,0), "TARGET REACHED!");
        }
    }

    //Update Popups
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

void _ScoreManager::Draw(_camera* cam, int screenWidth, int screenHeight) {
glPushMatrix(); 
    glLoadIdentity(); 
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    // --- DRAW HUD ---

    //Score (Top Left)
    m_hudFont->setPosition(-2.1f, 1.0f, -3.0f);
    m_hudFont->setSize(0.1f, 0.1f);

    string scoreStr = "Score: " + to_string(m_totalScore);
    if (m_currentComboScore > 0) {
        scoreStr += " + " + to_string(m_currentComboScore);
        if (m_multiplier > 1) scoreStr += " x" + to_string(m_multiplier);
    }
    m_hudFont->drawText(scoreStr);

    //Timer (Top Center)
    if (m_isTimed) DrawTimer();

    //Objectives (Top Right)
    DrawObjectives();

    // --- FPS COUNTER ---
    // Render at Top Right
    if (_Time::deltaTime > 0.0f) {
        // Calculate Frames Per Second
        int fps = (int)(1.0f / _Time::deltaTime);
        string fpsStr = to_string(fps) + " FPS";

        // Position: Far Right (X=1.8), Top (Y=1.0)
        m_hudFont->setPosition(1.8f, -1.2f, -3.0f);
        m_hudFont->setSize(0.03f, 0.03f); // Slightly smaller font
        
        glColor3f(0.0f, 1.0f, 0.0f); // Green Color
        m_hudFont->drawText(fpsStr);
        glColor3f(1.0f, 1.0f, 1.0f); // Reset Color
    }

    //Win/Loss Messages (Center Screen or Fullscreen Overlay)
    if (m_gameState != GAME_PLAYING) DrawWinLoss(screenWidth, screenHeight);
    
    //Popups (Center Screen Overlay)
    for (auto p : m_popups) {
        float scale = 0.1f;
        float spacing = scale * 1.5f;
        float textWidth = p->text.length() * spacing;

        // Center text
        m_popupFont->setPosition(-textWidth / 2.0f + p->pos.x, p->pos.y, -3.0f);
        m_popupFont->setSize(scale, scale);
        m_popupFont->drawText(p->text);
    }

    // draw the balance meter for grinding
    DrawBalanceMeter();

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
    m_hudFont->setPosition(0.8f, -1.0f, -3.0f); // Top Right

    if (m_isTagMode) {
        string tagStr = "Tags: " + to_string(m_tagsCollected) + " of " + to_string(m_tagsTarget);
        m_hudFont->drawText(tagStr);
    }
    else if (m_targetScore > 0) {
        string targetStr = "Goal: " + to_string(m_targetScore);
        m_hudFont->drawText(targetStr);
    }
}

void _ScoreManager::DrawWinLoss(int width, int height) {
    
    bool drawOverlay = false;
    _textureLoader* texToDraw = nullptr;

    // --- LOGIC: CHOOSE IMAGE OR TEXT ---
    if (m_gameState == GAME_LOST) {
        texToDraw = m_loseTex; // Always show GameOver.png on failure
        drawOverlay = true;
    }
    else if (m_gameState == GAME_WON) {
        if (m_isFinalLevel) {
            texToDraw = m_winTex; // Show WinScreen.png only on final level
            drawOverlay = true;
        }
        else {
            // Standard Text for intermediate levels
            m_hudFont->setSize(0.2f, 0.2f); 
            string msg = "COMPLETE!";
            float txtW = msg.length() * (0.2f * 1.5f);
            m_hudFont->setPosition(-txtW/2, 0.2f, -3.0f);
            glColor3f(0.0f, 1.0f, 0.0f); 
            m_hudFont->drawText(msg);
            glColor3f(1.0f, 1.0f, 1.0f);
        }
    }

    // --- DRAW FULLSCREEN OVERLAY IF NEEDED ---
    if (drawOverlay && texToDraw) {
        // Temporarily switch to 2D Ortho Mode to draw the image covering screen
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, width, height, 0); // Top-left origin
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        
        // --- DISABLE CULLING FOR THIS QUAD ---
        glDisable(GL_CULL_FACE); 
        
        glColor3f(1, 1, 1);

        texToDraw->bindTexture();

        glBegin(GL_QUADS);
            glTexCoord2f(0, 0); glVertex2f(0, 0);
            glTexCoord2f(1, 0); glVertex2f(width, 0);
            glTexCoord2f(1, 1); glVertex2f(width, height);
            glTexCoord2f(0, 1); glVertex2f(0, height);
        glEnd();

        // Restore State
        glEnable(GL_CULL_FACE);

        glPopMatrix(); // Pop ModelView
        glMatrixMode(GL_PROJECTION);
        glPopMatrix(); // Pop Projection
        glMatrixMode(GL_MODELVIEW);
        
        // Re-disable depth test as we are still inside Draw() 
        glDisable(GL_DEPTH_TEST); 
    }
}

void _ScoreManager::SetFinalLevel(bool flag){
    m_isFinalLevel=flag;
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

    // --- SOUND LOGIC ---
    if (m_soundMgr) {
        int soundIndex = m_multiplier - 1;

        if(soundIndex >= 1){
            // Cap at combo_6.wav if you don't have more files
            if(soundIndex > 6) soundIndex = 6; 

            string filename = "sounds/combo_" + to_string(soundIndex) + ".wav";
            m_soundMgr->playSFX(filename.c_str(), 0.5f);
        }
    }
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

        if (m_multiplier > 6 && m_soundMgr) {
            m_soundMgr->playSFX("sounds/final_combo.wav", 0.7f);
        }
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

void _ScoreManager::SetFreePlay() {
    // Reset Scores
    m_totalScore = 0;
    m_currentComboScore = 0;
    m_multiplier = 1;

    // Clear Timers and Objectives
    m_timeLimit = 0.0f;
    m_currentTime = 0.0f;
    m_targetScore = 0;
    m_tagsCollected = 0;
    m_tagsTarget = 0;

    // Disable Flags
    m_isTimed = false;      // No timer
    m_isTagMode = false;    // No tag collecting
    m_gameState = GAME_PLAYING; // Ensure we aren't stuck in "Game Won/Lost"

    // Clear old popups (optional, stops "Time Up" from lingering)
    for (auto p : m_popups) delete p;
    m_popups.clear();
}

void _ScoreManager::SetBalanceValue(float val, bool show) {
    m_balanceValue = val;
    m_showBalanceMeter = show;
}

// Add this helper function
void _ScoreManager::DrawBalanceMeter() {
    if (!m_showBalanceMeter) return;

    // Dimensions
    float barWidth = 0.6f;
    float barHeight = 0.05f;
    float yPos = -0.5f; // Near bottom of screen

    glDisable(GL_TEXTURE_2D); // Draw solid colors

    //Draw Background (Black)
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        glVertex3f(-barWidth/2 - 0.02f, yPos - barHeight, -3.0f);
        glVertex3f( barWidth/2 + 0.02f, yPos - barHeight, -3.0f);
        glVertex3f( barWidth/2 + 0.02f, yPos + barHeight, -3.0f);
        glVertex3f(-barWidth/2 - 0.02f, yPos + barHeight, -3.0f);
    glEnd();

    //Draw The Needle/Marker
    // Map -1..1 to screen X coords
    float needleX = m_balanceValue * (barWidth / 2.0f);

    // Color changes based on danger (Green -> Red)
    float danger = abs(m_balanceValue);
    glColor3f(danger, 1.0f - danger, 0.0f);

    glBegin(GL_QUADS);
        glVertex3f(needleX - 0.02f, yPos - barHeight, -3.0f);
        glVertex3f(needleX + 0.02f, yPos - barHeight, -3.0f);
        glVertex3f(needleX + 0.02f, yPos + barHeight, -3.0f);
        glVertex3f(needleX - 0.02f, yPos + barHeight, -3.0f);
    glEnd();

    glEnable(GL_TEXTURE_2D);
    glColor3f(1,1,1);
}

void _ScoreManager::SetSoundManager(_sounds* mgr)
{
    m_soundMgr = mgr;
}
