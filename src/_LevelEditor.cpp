#include "_LevelEditor.h"

// --- HELPER: Get distance from Center to Bottom/Top based on type ---
static float GetHalfHeight(string type, float scaleY) {
    if (type == "wood_floor") return 0.1f * scaleY; // Thin floor
    if (type == "rail") return 0.5f * scaleY;       // Rails are lower
    if (type == "tag") return 0.5f * scaleY;        // Tags are small
    return 1.0f * scaleY;                           // Standard blocks (Scaffold, Ramp, etc)
}

_LevelEditor::_LevelEditor() {
    m_ghostObject = nullptr;
    m_hoveredObject = nullptr;
    m_floorPlane = nullptr;
    m_refPlayer = nullptr;
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
    if (m_refPlayer) delete m_refPlayer;
}

void _LevelEditor::SetReferenceBlueprint(_AnimatedModel* bp) {
    if (m_refPlayer) delete m_refPlayer;
    
    m_refPlayer = new _AnimatedModelInstance(bp);
    
    m_refPlayer->pos = Vector3(0, 1, 0); 
    m_refPlayer->GetRigidBody()->gravity = 0;
    
    m_refPlayer->scale = Vector3(1, 1, 1);
    m_refPlayer->PlayAnimation("idle", 1.0f);
}

void _LevelEditor::Init(int width, int height) {
    // Load Blueprints
    _StaticModel* rail = new _StaticModel();
    rail->LoadModel("models/skatepark assets/rail/rail.obj", "models/skatepark assets/colormap.png");
    m_blueprints["rail"] = rail;

    _StaticModel* halfpipe = new _StaticModel();
    halfpipe->LoadModel("models/skatepark assets/halfpipe/halfpipe.obj", "models/skatepark assets/colormap.png");
    m_blueprints["halfpipe"] = halfpipe;
    
    // Create the Huge Floor Plane
    _StaticModel* floorModel = new _StaticModel();
    floorModel->LoadModel("models/terrain.obj", "models/Terrain_Tex.png");
    m_blueprints["floor"] = floorModel;

    m_floorPlane = new _StaticModelInstance(floorModel);
    m_floorPlane->pos = Vector3(0, -0.1f, 0); // Slightly below y=0 to show grid
    m_floorPlane->scale = Vector3(100, 1, 100); // HUGE area
    m_floorPlane->rotation = Vector3(0,0,0);

    _StaticModel* scaffold = new _StaticModel();
    scaffold->LoadModel("models/skatepark assets/scaffold/scaffold.obj", "models/skatepark assets/colormap.png");
    m_blueprints["scaffold"] = scaffold;

    _StaticModel* stairs = new _StaticModel();
    stairs->LoadModel("models/skatepark assets/stairs/stairs.obj", "models/skatepark assets/colormap.png");
    m_blueprints["stairs"] = stairs;

    _StaticModel* woodFloor = new _StaticModel();
    woodFloor->LoadModel("models/skatepark assets/floor/floor.obj", "models/skatepark assets/colormap.png");
    m_blueprints["wood_floor"] = woodFloor;

    _StaticModel* side = new _StaticModel();
    side->LoadModel("models/skatepark assets/side piece/side.obj", "models/skatepark assets/colormap.png");
    m_blueprints["side"] = side;

    // --- ADD TAG BLUEPRINT ---
    _StaticModel* tag = new _StaticModel();
    tag->LoadModel("models/spraycan/spraycan.obj", "models/spraycan/map.png");
    m_blueprints["tag"] = tag;

    // Create UI
    int startY = 60;
    int gapY = 60;

    // Rail
    _Button* btnRail = new _Button();
    btnRail->Init("images/editorItems/rail.png", 80, 50, 60, startY, 0, 1, 1); 
    m_itemButtons.push_back(btnRail);
    m_itemNames.push_back("rail");

    //Halfpipe
    _Button* btnPipe = new _Button();
    btnPipe->Init("images/editorItems/ramp.png", 80, 50, 60, startY + gapY, 0, 1, 1); 
    m_itemButtons.push_back(btnPipe);
    m_itemNames.push_back("halfpipe");

    //Scaffold
    _Button* btnScaff = new _Button();
    btnScaff->Init("images/editorItems/scaffolding.png", 80, 50, 60, startY + gapY*2, 0, 1, 1); 
    m_itemButtons.push_back(btnScaff);
    m_itemNames.push_back("scaffold");

    //Stairs
    _Button* btnStairs = new _Button();
    btnStairs->Init("images/editorItems/stairs.png", 80, 50, 60, startY + gapY*3, 0, 1, 1); 
    m_itemButtons.push_back(btnStairs);
    m_itemNames.push_back("stairs");

    //Wood Floor
    _Button* btnWFloor = new _Button();
    btnWFloor->Init("images/editorItems/floor.png", 80, 50, 60, startY + gapY*4, 0, 1, 1); 
    m_itemButtons.push_back(btnWFloor);
    m_itemNames.push_back("wood_floor");

    /*
    //Side Piece
    _Button* btnSide = new _Button();
    btnSide->Init("images/play-btn.png", 80, 50, 60, startY + gapY*5, 0, 1, 1); 
    m_itemButtons.push_back(btnSide);
    m_itemNames.push_back("side");
    */

    //TAG BUTTON
    _Button* btnTag = new _Button();
    btnTag->Init("images/editorItems/spraycan.png", 80, 50, 60, startY + gapY*5, 0, 1, 1); 
    m_itemButtons.push_back(btnTag);
    m_itemNames.push_back("tag");

    SetGhost("scaffold");
    
    //AUTO LOAD ON START
    LoadLevel("saves/level_custom.txt");
}

void _LevelEditor::SetGhost(string type) {
    if (m_blueprints.find(type) == m_blueprints.end()) return;
    if (m_ghostObject) delete m_ghostObject;

    m_selectedType = type;
    m_ghostObject = new _StaticModelInstance(m_blueprints[type]);
    m_ghostObject->modelName = type; // Ensure name is set for logic checks
    
    m_ghostObject->scale = Vector3(1,1,1);
    
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
    //Get Ray
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

    //Check against all placed objects
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

    // Just tick the animation, DO NOT move the position
    if (m_refPlayer) {
        m_refPlayer->Update();
    }

    // Check for Hover
    m_hoveredObject = RaycastCheck(hWnd, cam);

    

    if (m_ghostObject) {
        m_ghostObject->pos = m_cursorPos;
        

        // Calculate how tall the GHOST is (distance from center to bottom)
        float ghostHalfHeight = GetHalfHeight(m_selectedType, m_ghostObject->scale.y);

        if (m_hoveredObject) {
            // Calculate the exact TOP of the hovered object
            float hoveredHalfHeight = GetHalfHeight(m_hoveredObject->modelName, m_hoveredObject->scale.y);
            float hoveredTop = m_hoveredObject->pos.y + hoveredHalfHeight;

            // Place ghost on top
            m_ghostObject->pos.y = hoveredTop + ghostHalfHeight;
        }
        else {
            // Place ghost on ground (0 + height)
            m_ghostObject->pos.y = 0.0f + ghostHalfHeight;
        }

        m_ghostObject->rotation.y = m_currentRotation;
    }
}

void _LevelEditor::HandleKeyInput(WPARAM wParam) {
    if (wParam == 'R' || wParam == 'r') {
        m_currentRotation += 90.0f;
        if (m_currentRotation >= 360.0f) m_currentRotation -= 360.0f;
    }
    if (wParam == 'J' || wParam == 'j') ScaleGhost(0.5f);
    if (wParam == 'K' || wParam == 'k') ScaleGhost(-0.5f);
    
    if (wParam == VK_BACK) {
        if (m_ghostObject) {
            m_ghostObject->scale = Vector3(1.0f, 1.0f, 1.0f);
        }
    }
}

bool _LevelEditor::HandleMouseClick(UINT uMsg, int mouseX, int mouseY) {
    
    //UI INTERACTION
    if (uMsg == WM_LBUTTONDOWN) {
        for(size_t i=0; i<m_itemButtons.size(); i++) {
            if(m_itemButtons[i]->isClicked(mouseX, mouseY)) {
                SetGhost(m_itemNames[i]);
                return true; 
            }
        }
    }

    //RIGHT CLICK DELETE
    if (uMsg == WM_RBUTTONDOWN) {
        if (m_hoveredObject) {
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

    //PLACEMENT & STACKING (Left Click)
    if (uMsg == WM_LBUTTONDOWN && m_ghostObject) {
        
        _StaticModelInstance* newObj = new _StaticModelInstance(m_blueprints[m_selectedType]);
        
        newObj->modelName = m_selectedType;
        newObj->scale = m_ghostObject->scale; // Apply Scale FIRST

        // --- DYNAMIC PLACEMENT LOGIC ---
        float newObjHalfHeight = GetHalfHeight(m_selectedType, newObj->scale.y);

        if (m_hoveredObject) {
            newObj->pos.x = m_hoveredObject->pos.x;
            newObj->pos.z = m_hoveredObject->pos.z;
            
            // Dynamic Stacking
            float hoveredHalfHeight = GetHalfHeight(m_hoveredObject->modelName, m_hoveredObject->scale.y);
            float hoveredTop = m_hoveredObject->pos.y + hoveredHalfHeight;
            
            newObj->pos.y = hoveredTop + newObjHalfHeight;
            newObj->rotation = m_ghostObject->rotation;
        }
        else {
             // Standard Placement on Ground
             newObj->pos = m_ghostObject->pos;
             newObj->pos.y = 0.0f + newObjHalfHeight;
             newObj->rotation = m_ghostObject->rotation;
        }

        // --- COLLIDER ASSIGNMENT ---
        // Assign specific physics types based on the selected object name
        Vector3 cMin(-1, -1, -1);
        Vector3 cMax(1, 1, 1);

        if(m_selectedType == "rail") {
             newObj->AddCollider(new _CubeHitbox(cMin, cMax, COLLIDER_RAIL));
        }
        else if(m_selectedType == "halfpipe") {
             newObj->AddCollider(new _CubeHitbox(cMin, cMax, COLLIDER_HALFPIPE));
        }
        else if(m_selectedType == "stairs") {
             newObj->AddCollider(new _CubeHitbox(cMin, cMax, COLLIDER_STAIRS));
        }
        else if(m_selectedType == "wood_floor") {
             newObj->AddCollider(new _CubeHitbox(Vector3(-1, -0.1f, -1), Vector3(1, 0.1f, 1), COLLIDER_FLOOR));
        }
        else if(m_selectedType == "tag") {
             // Tag has no physical collider for physics, but we give it a trigger volume 
             // (or just a small hitbox so we can select it in editor)
             newObj->AddCollider(new _SphereHitbox(Vector3(0,0,0), 1.0f, COLLIDER_GENERAL));
        }
        else {
             // Scaffolds and Side pieces are Walls (Solid blocks)
             newObj->AddCollider(new _CubeHitbox(cMin, cMax, COLLIDER_WALL));
        }

        m_placedObjects.push_back(newObj);
        return true;
    }

    return false;
}

void _LevelEditor::Draw() {
    
    // Draw the HUGE floor
    if(m_floorPlane) m_floorPlane->Draw();

    // Draw Reference Player
    if (m_refPlayer) {
        m_refPlayer->Draw();
    }

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
        //glDisable(GL_BLEND);
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
        string type = obj->modelName;
        if (type.empty()) type = "rail";

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
            
            //Restore Properties
            newObj->pos = Vector3(x, y, z);
            newObj->rotation.y = rot;
            newObj->scale = Vector3(scale, scale, scale);
            newObj->modelName = type;

            //Restore Physics
            Vector3 cMin(-1, -1, -1);
            Vector3 cMax(1, 1, 1);

            if(type == "wood_floor") {
                 newObj->AddCollider(new _CubeHitbox(Vector3(-1, -0.1f, -1), Vector3(1, 0.1f, 1), COLLIDER_FLOOR));
            }
            else if(type == "rail") {
                 newObj->AddCollider(new _CubeHitbox(cMin, cMax, COLLIDER_RAIL));
            }
            else if(type == "halfpipe") {
                 newObj->AddCollider(new _CubeHitbox(cMin, cMax, COLLIDER_HALFPIPE));
            }
            else if(type == "stairs") {
                 newObj->AddCollider(new _CubeHitbox(cMin, cMax, COLLIDER_STAIRS));
            }
            else if(type == "tag") {
                 newObj->AddCollider(new _SphereHitbox(Vector3(0,0,0), 1.0f, COLLIDER_GENERAL));
            }
            else {
                 newObj->AddCollider(new _CubeHitbox(cMin, cMax, COLLIDER_WALL));
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