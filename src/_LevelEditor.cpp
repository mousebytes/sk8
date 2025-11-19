#include "_LevelEditor.h"

_LevelEditor::_LevelEditor() {
    m_ghostObject = nullptr;
    m_hoveredObject = nullptr;
    m_floorPlane = nullptr;
    m_gridSize = 1.0f;
    m_currentRotation = 0.0f;
    m_selectedType = "";
}

_LevelEditor::~_LevelEditor() {
    if (m_ghostObject) delete m_ghostObject;
    if (m_floorPlane) delete m_floorPlane;
    
    for (auto* btn : m_itemButtons) delete btn;
    delete m_saveButton;
    delete m_loadButton;
    delete m_exitButton;

    for (auto const& pair : m_blueprints) {
        delete pair.second;
    }
    ClearLevel();
}

void _LevelEditor::Init(int width, int height) {
    // 1. Load Blueprints
    _StaticModel* rail = new _StaticModel();
    rail->LoadModel("models/skatepark assets/rail/rail.obj", "models/skatepark assets/colormap.png");
    m_blueprints["rail"] = rail;

    _StaticModel* halfpipe = new _StaticModel();
    halfpipe->LoadModel("models/skatepark assets/halfpipe/halfpipe.obj", "models/skatepark assets/colormap.png");
    m_blueprints["halfpipe"] = halfpipe;
    
    // 2. Create the Huge Floor Plane
    _StaticModel* floorModel = new _StaticModel();
    floorModel->LoadModel("models/terrain.obj", "models/Terrain_Tex.png");
    m_blueprints["floor"] = floorModel;

    m_floorPlane = new _StaticModelInstance(floorModel);
    m_floorPlane->pos = Vector3(0, -0.1f, 0); // Slightly below y=0 to show grid
    m_floorPlane->scale = Vector3(100, 1, 100); // HUGE area
    m_floorPlane->rotation = Vector3(0,0,0);


    // 3. Create UI
    int startY = 60;
    int gapY = 70;

    _Button* btnRail = new _Button();
    btnRail->Init("images/play-btn.png", 80, 50, 60, startY, 0, 1, 1); 
    m_itemButtons.push_back(btnRail);
    m_itemNames.push_back("rail");

    _Button* btnPipe = new _Button();
    btnPipe->Init("images/play-btn.png", 80, 50, 60, startY + gapY, 0, 1, 1); 
    m_itemButtons.push_back(btnPipe);
    m_itemNames.push_back("halfpipe");

    m_saveButton = new _Button();
    m_saveButton->Init("images/play-btn.png", 100, 40, width - 60, 40, 0, 1, 1);
    
    m_loadButton = new _Button();
    m_loadButton->Init("images/play-btn.png", 100, 40, width - 60, 90, 0, 1, 1);

    m_exitButton = new _Button();
    m_exitButton->Init("images/exit-btn.png", 100, 40, width - 60, height - 40, 0, 1, 1);

    SetGhost("rail");
    
    // 4. AUTO LOAD ON START
    LoadLevel("level_custom.txt");
}

void _LevelEditor::SetGhost(string type) {
    if (m_blueprints.find(type) == m_blueprints.end()) return;
    if (m_ghostObject) delete m_ghostObject;

    m_selectedType = type;
    m_ghostObject = new _StaticModelInstance(m_blueprints[type]);
    
    if(type == "halfpipe") m_ghostObject->scale = Vector3(4,4,4);
    else m_ghostObject->scale = Vector3(1,1,1);
    
    m_ghostObject->rotation.y = m_currentRotation;
}

void _LevelEditor::ScaleGhost(float amount) {
    if (!m_ghostObject) return;
    m_ghostObject->scale.x += amount;
    m_ghostObject->scale.y += amount;
    m_ghostObject->scale.z += amount;
    if (m_ghostObject->scale.x < 0.1f) m_ghostObject->scale = Vector3(0.1f, 0.1f, 0.1f);
}

// Simple Ray-Box Intersection helper
bool RayAABBIntersection(Vector3 rayOrigin, Vector3 rayDir, Vector3 boxMin, Vector3 boxMax, float& t) {
    float tmin = (boxMin.x - rayOrigin.x) / rayDir.x;
    float tmax = (boxMax.x - rayOrigin.x) / rayDir.x;
    if (tmin > tmax) std::swap(tmin, tmax);

    float tymin = (boxMin.y - rayOrigin.y) / rayDir.y;
    float tymax = (boxMax.y - rayOrigin.y) / rayDir.y;
    if (tymin > tymax) std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax)) return false;
    if (tymin > tmin) tmin = tymin;
    if (tymax < tmax) tmax = tymax;

    float tzmin = (boxMin.z - rayOrigin.z) / rayDir.z;
    float tzmax = (boxMax.z - rayOrigin.z) / rayDir.z;
    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax)) return false;
    if (tzmin > tmin) tmin = tzmin;
    if (tzmax < tmax) tmax = tzmax;

    t = tmin;
    return true;
}

_StaticModelInstance* _LevelEditor::RaycastCheck(HWND hWnd, _camera* cam) {
    // 1. Get Ray
    POINT mousePos;
    GetCursorPos(&mousePos);
    ScreenToClient(hWnd, &mousePos);

    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLdouble posX, posY, posZ;

    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    float winX = (float)mousePos.x;
    float winY = (float)viewport[3] - (float)mousePos.y;

    gluUnProject(winX, winY, 0.0, modelview, projection, viewport, &posX, &posY, &posZ);
    Vector3 rayStart(posX, posY, posZ);
    gluUnProject(winX, winY, 1.0, modelview, projection, viewport, &posX, &posY, &posZ);
    Vector3 rayEnd(posX, posY, posZ);

    Vector3 rayDir = rayEnd - rayStart;
    rayDir.normalize();

    // 2. Check against all placed objects
    _StaticModelInstance* closestObj = nullptr;
    float closestDist = 99999.0f;

    for (auto* obj : m_placedObjects) {
        // Approximate bounds based on pos & scale
        float sX = obj->scale.x; 
        float sY = obj->scale.y; 
        float sZ = obj->scale.z;

        // World box approx: pos + (localMin * scale) -> assuming -1 to 1 local model
        Vector3 boxMin = obj->pos + Vector3(-1 * sX, -1 * sY, -1 * sZ);
        Vector3 boxMax = obj->pos + Vector3(1 * sX, 1 * sY, 1 * sZ);

        float t = 0;
        if (RayAABBIntersection(rayStart, rayDir, boxMin, boxMax, t)) {
            if (t > 0 && t < closestDist) {
                closestDist = t;
                closestObj = obj;
            }
        }
    }
    return closestObj;
}

void _LevelEditor::Update(HWND hWnd, _camera* cam) {
    m_cursorPos = GetWorldPosFromMouse(hWnd, cam);
    
    // Snap to Grid
    m_cursorPos.x = round(m_cursorPos.x / m_gridSize) * m_gridSize;
    m_cursorPos.z = round(m_cursorPos.z / m_gridSize) * m_gridSize;
    m_cursorPos.y = 0; 

    // Check for Hover
    m_hoveredObject = RaycastCheck(hWnd, cam);

    if (m_ghostObject) {
        m_ghostObject->pos = m_cursorPos;
        //if(m_selectedType == "halfpipe") m_ghostObject->pos.y = 0.0f; 
        //else m_ghostObject->pos.y = 4.0f;
        if(m_selectedType == "rail") m_ghostObject->pos.y = m_ghostObject->scale.y/2;
        else m_ghostObject->pos.y = m_ghostObject->scale.y;
        m_ghostObject->rotation.y = m_currentRotation;
    }
}

void _LevelEditor::HandleKeyInput(WPARAM wParam) {
    if (wParam == 'R' || wParam == 'r') {
        m_currentRotation += 90.0f;
        if (m_currentRotation >= 360.0f) m_currentRotation -= 360.0f;
    }
    if (wParam == 'J' || wParam == 'j') ScaleGhost(0.2f);
    if (wParam == 'K' || wParam == 'k') ScaleGhost(-0.2f);
}

bool _LevelEditor::HandleMouseClick(UINT uMsg, int mouseX, int mouseY) {
    
    // 1. UI INTERACTION (Only on Left Click)
    if (uMsg == WM_LBUTTONDOWN) {
        for(size_t i=0; i<m_itemButtons.size(); i++) {
            if(m_itemButtons[i]->isClicked(mouseX, mouseY)) {
                SetGhost(m_itemNames[i]);
                return true; 
            }
        }
        if(m_saveButton->isClicked(mouseX, mouseY)) { SaveLevel("level_custom.txt"); return true; }
        if(m_loadButton->isClicked(mouseX, mouseY)) { LoadLevel("level_custom.txt"); return true; }
        if(m_exitButton->isClicked(mouseX, mouseY)) { return true; /* Signal to exit in Scene */ }
    }

    // 2. OBJECT DELETION (Right Click)
    if (uMsg == WM_RBUTTONDOWN) {
        if (m_hoveredObject) {
            // Find and remove from vector
            for (auto it = m_placedObjects.begin(); it != m_placedObjects.end(); ++it) {
                if (*it == m_hoveredObject) {
                    delete *it;
                    m_placedObjects.erase(it);
                    m_hoveredObject = nullptr;
                    break;
                }
            }
            return true;
        }
    }

    // 3. PLACEMENT (Left Click)
    if (uMsg == WM_LBUTTONDOWN && m_ghostObject && !m_hoveredObject) {
        _StaticModelInstance* newObj = new _StaticModelInstance(m_blueprints[m_selectedType]);
        newObj->pos = m_ghostObject->pos;
        newObj->rotation = m_ghostObject->rotation;
        newObj->scale = m_ghostObject->scale;

        if(m_selectedType == "rail") 
             newObj->AddCollider(new _CubeHitbox(Vector3(-1, -1, -1), Vector3(1, 1, 1), COLLIDER_RAIL));
        else if(m_selectedType == "halfpipe") 
             newObj->AddCollider(new _CubeHitbox(Vector3(-1, -1, -1), Vector3(1, 1, 1), COLLIDER_HALFPIPE));

        m_placedObjects.push_back(newObj);
    }
    return false;
}

void _LevelEditor::Draw() {
    
    // Draw the HUGE floor
    if(m_floorPlane) m_floorPlane->Draw();

    // Draw placed objects
    for (auto* obj : m_placedObjects) {
        obj->Draw();

        // Highlight Hovered Object
        if (obj == m_hoveredObject) {
            // Draw Hitbox or Tint
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            glColor3f(1.0f, 0.0f, 0.0f); // RED HIGHLIGHT
            
            // Simple wireframe box around the position
            glPushMatrix();
            glTranslatef(obj->pos.x, obj->pos.y, obj->pos.z);
            glScalef(obj->scale.x, obj->scale.y, obj->scale.z);
            glutWireCube(2.0); // Assuming model is -1 to 1
            glPopMatrix();

            glEnable(GL_LIGHTING);
            glEnable(GL_TEXTURE_2D);
            glColor4f(1,1,1,1);
        }
    }

    // Draw ghost
    if (m_ghostObject) {
        glEnable(GL_BLEND);
        glColor4f(1, 1, 1, 0.5f); 
        m_ghostObject->Draw();
        glColor4f(1, 1, 1, 1);
        glDisable(GL_BLEND);
    }

    // Draw Grid
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_LINES);
    for(int i=-50; i<=50; i+=2) {
        glVertex3f((float)i, 0.05f, -50); // Lift slightly above floor
        glVertex3f((float)i, 0.05f, 50);
        glVertex3f(-50, 0.05f, (float)i);
        glVertex3f(50, 0.05f, (float)i);
    }
    glEnd();
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    // Draw UI Overlay
    glDisable(GL_CULL_FACE);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    GLint viewPort[4];
    glGetIntegerv(GL_VIEWPORT, viewPort);
    gluOrtho2D(0, viewPort[2], viewPort[3], 0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);
    for(auto* btn : m_itemButtons) btn->Draw();
    m_saveButton->Draw();
    m_loadButton->Draw();
    m_exitButton->Draw();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void _LevelEditor::SaveLevel(string filename) {
    ofstream file(filename);
    if (!file.is_open()) return;
    for (auto* obj : m_placedObjects) {
        string type = "rail";
        if(!obj->colliders.empty() && obj->colliders[0]->m_type == COLLIDER_HALFPIPE) type = "halfpipe";

        file << type << " " 
             << obj->pos.x << " " << obj->pos.y << " " << obj->pos.z << " "
             << obj->rotation.y << " " 
             << obj->scale.x << endl; 
    }
    file.close();
    cout << "Level Saved!" << endl;
}

void _LevelEditor::LoadLevel(string filename) {
    ClearLevel();
    ifstream file(filename);
    if (!file.is_open()) return;

    string type;
    float x, y, z, rot, scale;
    while (file >> type >> x >> y >> z >> rot >> scale) {
        if (m_blueprints.find(type) != m_blueprints.end()) {
            _StaticModelInstance* newObj = new _StaticModelInstance(m_blueprints[type]);
            newObj->pos = Vector3(x, y, z);
            newObj->rotation.y = rot;
            newObj->scale = Vector3(scale, scale, scale);
            
            if(type == "halfpipe") {
                newObj->AddCollider(new _CubeHitbox(Vector3(-1,-1,-1),Vector3(1,1,1),COLLIDER_HALFPIPE));
            } else {
                newObj->AddCollider(new _CubeHitbox(Vector3(-1,-1,-1),Vector3(1,1,1),COLLIDER_RAIL));
            }
            m_placedObjects.push_back(newObj);
        }
    }
    file.close();
    cout << "Level Loaded!" << endl;
}

void _LevelEditor::ClearLevel() {
    for (auto* obj : m_placedObjects) delete obj;
    m_placedObjects.clear();
}

Vector3 _LevelEditor::GetWorldPosFromMouse(HWND hWnd, _camera* cam) {
    POINT mousePos;
    GetCursorPos(&mousePos);
    ScreenToClient(hWnd, &mousePos);

    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLdouble posX, posY, posZ;

    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    float winX = (float)mousePos.x;
    float winY = (float)viewport[3] - (float)mousePos.y;

    gluUnProject(winX, winY, 0.0, modelview, projection, viewport, &posX, &posY, &posZ);
    Vector3 rayStart(posX, posY, posZ);
    gluUnProject(winX, winY, 1.0, modelview, projection, viewport, &posX, &posY, &posZ);
    Vector3 rayEnd(posX, posY, posZ);

    Vector3 dir = rayEnd - rayStart;
    dir.normalize();

    if (abs(dir.y) > 0.001f) {
        float t = (0.0f - rayStart.y) / dir.y;
        if (t >= 0) return rayStart + (dir * t);
    }
    return Vector3(0,0,0);
}