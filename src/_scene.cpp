#include "_scene.h"

_Scene::_Scene()
{
    m_isCustomGame = false;
    //ctor
    terrainBlueprint = new _StaticModel();
    terrainInstance = new _StaticModelInstance(terrainBlueprint);
    m_inputs = new _inputs();
    m_camera = new _camera();
    //m_playButton = new _Button();

    m_landingTitle=new _Button();
    m_landingInstructions = new _Button();
    m_helpInfo = new _Button();
    m_playButton = new _Button();
    m_helpButton = new _Button();
    m_exitButton = new _Button();
    m_backButton = new _Button();

    // pause menu stuff
    m_resumeButton = new _Button();
    m_pauseHelpButton = new _Button();
    m_pauseMenuButton = new _Button();
    m_showPauseHelp = false;

    m_skybox = new _skyBox();

    m_player_blueprint = new _AnimatedModel();
    m_skateboardBlueprint = new _AnimatedModel();


    m_bulletBlueprint = new _StaticModel();
    m_bulletManager = nullptr;

    m_railBlueprint = new _StaticModel();
    m_railInstance = new _StaticModelInstance(m_railBlueprint);

    m_halfpipeBlueprint = new _StaticModel();
    m_halfpipeInstance = new _StaticModelInstance(m_halfpipeBlueprint);

    m_levelEditor = new _LevelEditor();
    m_editorResumeButton = new _Button();
    m_editorSaveButton = new _Button();
    m_editorExitButton = new _Button();
    m_editorButton = new _Button();
    m_playCustomButton = new _Button();

    m_customFloor = new _StaticModelInstance(terrainBlueprint);

    m_scaffoldBlueprint = new _StaticModel();
    m_stairsBlueprint = new _StaticModel();
    m_woodFloorBlueprint = new _StaticModel();
    m_sideBlueprint = new _StaticModel();

    m_backgroundImageButton = new _Button();

    m_scoreManager = new _ScoreManager();
}

_Scene::~_Scene()
{
    //dtor

    delete terrainBlueprint;
    delete terrainInstance;
    delete m_inputs;
    delete m_camera;

    delete m_landingTitle;
    delete m_landingInstructions;
    delete m_playButton;
    delete m_helpButton;
    delete m_exitButton;
    delete m_backButton;
    delete m_playCustomButton;
    
    for(auto* obj : m_customLevelObjects) delete obj;
    m_customLevelObjects.clear();

    delete m_skybox;

    delete m_player_blueprint;
    delete m_player;

    delete m_bulletManager;
    delete m_bulletBlueprint;
    
    delete m_resumeButton;
    delete m_pauseHelpButton;
    delete m_pauseMenuButton;

    delete m_railBlueprint;
    delete m_railInstance;
    delete m_skateboardBlueprint;

    delete m_halfpipeBlueprint;
    delete m_halfpipeInstance;

    delete m_levelEditor;
    delete m_editorResumeButton;
    delete m_editorSaveButton;
    delete m_editorExitButton;
    delete m_editorButton;

    delete m_customFloor;

    delete m_scaffoldBlueprint;
    delete m_stairsBlueprint;
    delete m_woodFloorBlueprint;
    delete m_sideBlueprint;

    delete m_backgroundImageButton;

    delete m_scoreManager;
}

void _Scene::reSizeScene(int width, int height)
{
    float aspectRatio = (float)width/(float)height;// keep track of the ratio
    glViewport(0,0,width,height); // adjust my viewport

    glMatrixMode(GL_PROJECTION);  // To setup ptrojection
    glLoadIdentity();             // calling identity matrix
    gluPerspective(45, aspectRatio,0.1,1000.0); // setting perspective projection

    //this->width = GetSystemMetrics(SM_CXSCREEN);
    //this->height= GetSystemMetrics(SM_CYSCREEN);

    this->width = width;
    this->height= height;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();             // calling identity matrix
}

void _Scene::initGL()
{
    _Time::Init();
    glShadeModel(GL_SMOOTH); // to handle GPU shaders
    glClearColor(0.0f,0.0f,0.0f,0.0f); // black background color
    glClearDepth(2.0f);         //depth test for layers
    glEnable(GL_DEPTH_TEST);    //activate depth test
    glDepthFunc(GL_LEQUAL);     // depth function type

    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // light properties
    // (x,y,z,w) w=1.0 for positional w=0.0 for directional
    GLfloat light_pos[] = {0.0f,5.0f,2.0f,1.0f};
    GLfloat light_diffuse[] = {1.0f,1.0f,1.0f,1.0f};
    GLfloat light_ambient[] = {0.2f,0.2f,0.2f,1.0f};
    //GLfloat light_specular[] = {0.5f,0.5f,0.5f,1.0f};

    // apply properties to light 0
    glLightfv(GL_LIGHT0,GL_POSITION,light_pos);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,light_diffuse);
    glLightfv(GL_LIGHT0,GL_AMBIENT,light_ambient);
    //glLightfv(GL_LIGHT0,GL_SPECULAR,light_specular);

    // use tex color for lighting
    // makes materials diffuse property track color from tex
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);

    initGameplay();
    initLandingPage();
    initMainMenu();
    initHelpScreen();
    initPauseMenu();
    initLevelEditor();

    m_sceneState = SceneState::LandingPage;
}

void _Scene::drawScene()
{
    _Time::Update();
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);//clear bits in each itteration
    glLoadIdentity();             // calling identity matrix

    switch (m_sceneState)
    {
        case SceneState::LandingPage:
            drawLandingPage();
            break;
        case SceneState::MainMenu:
            drawMainMenu();
            break;
        case SceneState::Paused:
            drawGameplay();   // Draw the frozen game world
            drawPauseMenu();  // Draw the pause menu on top
            break;
        case SceneState::Playing:
            updateGameplay();
            drawGameplay();
            break;
        case SceneState::Help:
            drawHelpScreen();
            break;
        case SceneState::LevelEditor:
            drawLevelEditor();
            break;
        case SceneState::EditorPaused:
            drawLevelEditor();     // Draw editor in background
            drawEditorPauseMenu(); // Draw overlay on top
            break;
        default:
            break;
    }
}


void _Scene::mouseMapping(int x, int y)
{
    GLint viewPort[4];
    GLdouble ModelViewM[16];
    GLdouble projectionM[16];
    GLfloat winX,winY,winZ;

    glGetDoublev(GL_MODELVIEW_MATRIX, ModelViewM);
    glGetDoublev(GL_PROJECTION_MATRIX,projectionM);
    glGetIntegerv(GL_VIEWPORT,viewPort);

    winX =(GLfloat)x;
    winY = (GLfloat)y;

    glReadPixels(x,(int)winY,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&winZ);
    gluUnProject(winX,winY,winZ,ModelViewM,projectionM,viewPort,&msX,&msY,&msZ);
}

int _Scene::winMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (m_sceneState)
    {
        case SceneState::LandingPage:
            handleLandingPageInput(uMsg,wParam,lParam);
            break;
        case SceneState::MainMenu:
            handleMainMenuInput(uMsg, wParam, lParam);
            break;
        case SceneState::Playing:
            handleGameplayInput(hWnd, uMsg, wParam, lParam);
            break;
        case SceneState::Help:
            handleHelpScreenInput(uMsg, wParam, lParam);
            break;
        case SceneState::Paused: 
            handlePauseMenuInput(uMsg, wParam, lParam);
            break;
        case SceneState::LevelEditor:
            handleLevelEditorInput(hWnd, uMsg, wParam, lParam);
            break;
        case SceneState::EditorPaused:
            handleEditorPauseInput(hWnd, uMsg, wParam, lParam);
            break; 
    }
    return 0;
}

void _Scene::handleLandingPageInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // check for a left mouse click or the enter key press
    if (uMsg == WM_LBUTTONDOWN)
    {
        m_sceneState = SceneState::MainMenu;
    }
    else if (uMsg == WM_KEYDOWN && wParam == VK_RETURN)
    {
        m_sceneState = SceneState::MainMenu;
    }
}

void _Scene::handleGameplayInput(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_KEYDOWN:

            if (wParam == VK_ESCAPE)
            {
                m_sceneState = SceneState::Paused;
                m_player->isFrozen = true; // freeze player logic
                break; // dont process other keys
            }

            m_inputs->wParam = wParam;
            // for free cam
            m_inputs->keyPressed(m_camera);
            // for player movement & doing eye cam stuff
            m_player->HandleKeys(wParam);

            if(wParam == '1'){
                isDebug=!isDebug;
            }
            else if(wParam=='2'){
                colliderDrawFace=!colliderDrawFace;
                if(!isDebug){
                    isDebug=true;
                    colliderDrawFace=true;
                }
            }

            break;
        case WM_KEYUP:

        break;

        case WM_LBUTTONDOWN:
            mouseMapping(LOWORD(lParam), HIWORD(lParam));
            // need to create local scope with {}
            {
                // fire from the camera's eye, in the camera's look direction
                Vector3 startPos = m_camera->eye;
                Vector3 direction = m_camera->des - m_camera->eye;
                //m_bulletManager->Fire(startPos, direction);
             }
            
            break;

        case WM_RBUTTONDOWN:

            break;

         case WM_MBUTTONDOWN:


            break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
            break;

        case WM_MOUSEMOVE:
            if(m_camera->isFreeCam) m_camera->handleMouse(hWnd, LOWORD(lParam), HIWORD(lParam), width / 2, height / 2);
            handleMouseMovement(hWnd, lParam);
            break;
        case WM_MOUSEWHEEL:

            break;

        default:
            break;
    }
}

void _Scene::handleEditorPauseInput(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE) {
        m_sceneState = SceneState::LevelEditor; // Resume
        return;
    }

    if (uMsg == WM_LBUTTONDOWN) {
        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);

        if (m_editorResumeButton->isClicked(mouseX, mouseY)) {
            m_sceneState = SceneState::LevelEditor;
        }
        else if (m_editorSaveButton->isClicked(mouseX, mouseY)) {
            m_levelEditor->SaveLevel("saves/level_custom.txt");
            m_sceneState = SceneState::LevelEditor;
        }
        else if (m_editorExitButton->isClicked(mouseX, mouseY)) {
            m_sceneState = SceneState::MainMenu;
            m_camera->isFreeCam=false;
        }
    }
}

void _Scene::handleLevelEditorInput(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_KEYDOWN:
             m_inputs->wParam = wParam;
             m_inputs->keyPressed(m_camera); 
             m_levelEditor->HandleKeyInput(wParam); // J, K, R
             
             // Go to Editor Pause Menu instead of Main Menu
             if(wParam == VK_ESCAPE) {
                 m_sceneState = SceneState::EditorPaused;
             }

             // --- Toggle Camera Lock on Enter ---
             if(wParam == VK_RETURN || wParam == VK_CONTROL) {
                 m_camera->isFreeCam = !m_camera->isFreeCam;
                 
                 // If we just re-enabled the camera, snap cursor to center immediately.
                 // This prevents the camera from "jumping" due to the mouse being far from center.
                 if (m_camera->isFreeCam) {
                     POINT pt = { width / 2, height / 2 };
                     ClientToScreen(hWnd, &pt);
                     SetCursorPos(pt.x, pt.y);
                 }
             }
             if(wParam == VK_SPACE){
                m_camera->camReset();
             }
             break;
        
        case WM_MOUSEMOVE:
             if(m_camera->isFreeCam) 
                 m_camera->handleMouse(hWnd, LOWORD(lParam), HIWORD(lParam), width/2, height/2);
             break;

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
             m_levelEditor->HandleMouseClick(uMsg, LOWORD(lParam), HIWORD(lParam));
             break;
    }
}

void _Scene::handleMainMenuInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_LBUTTONDOWN)
    {
        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);


        if (m_playButton->isClicked(mouseX, mouseY)) {
            m_sceneState = SceneState::Playing;
            m_player->isFrozen = false; // unfreeze player when starting game
            m_showPauseHelp = false; // reset pause help flag

            loadCampaignLevel();
        }
        else if (m_playCustomButton->isClicked(mouseX, mouseY)) {
            m_sceneState = SceneState::Playing;
            m_player->isFrozen = false;
            m_showPauseHelp = false;

            loadCustomLevel(); //Load the custom file!
        }
        else if (m_editorButton->isClicked(mouseX, mouseY)) {
            m_sceneState = SceneState::LevelEditor;
            m_player->isFrozen = false;
            // Initialize editor only when entering, or in InitGL if preferred
            //m_levelEditor->Init(width, height); // Ensure this is called at least once
            m_camera->camReset();      // Snap back to origin (0, 5, 10)
            m_camera->isFreeCam = true; // Allow WASD movement independent of player

            // Optional: Set a specific starting height for a bird's eye view
            // m_camera->eye.y = 20.0f; 
            // m_camera->rotAngle.y = 45.0f; // Look down
        }
        else if (m_helpButton->isClicked(mouseX, mouseY)) {
            m_sceneState = SceneState::Help;
        }
        else if (m_exitButton->isClicked(mouseX,mouseY)){
            exit(0);
        }

    }
}

void _Scene::handleHelpScreenInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_LBUTTONDOWN)
    {
        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);

        if (m_backButton->isClicked(mouseX, mouseY)) {
            m_sceneState = SceneState::MainMenu;
        }
    }
    else if (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE)
    {
        m_sceneState = SceneState::MainMenu;
    }
}

void _Scene::handlePauseMenuInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // always allow esc to resume regardless of overlay
    if (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE)
    {
        m_sceneState = SceneState::Playing;
        m_player->isFrozen = false;
        m_showPauseHelp = false; // hide help overlay
        return;
    }

    if (uMsg == WM_LBUTTONDOWN)
    {
        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);

        if (m_showPauseHelp)
        {
            // if help is showing, only check the back button
            if (m_backButton->isClicked(mouseX, mouseY))
            {
                m_showPauseHelp = false;
            }
        }
        else
        {
            // help is not showing, check main pause buttons
            if (m_resumeButton->isClicked(mouseX, mouseY))
            {
                m_sceneState = SceneState::Playing;
                m_player->isFrozen = false;
            }
            else if (m_pauseHelpButton->isClicked(mouseX, mouseY))
            {
                m_showPauseHelp = true;
            }
            else if (m_pauseMenuButton->isClicked(mouseX, mouseY))
            {
                m_sceneState = SceneState::MainMenu;
                // player will be unfrozen (isFrozen=false) when play is clicked from main menu
            }
        }
    }
}

void _Scene::initLandingPage()
{
    // positions are 2d pixels, assumes (0,0) is top left
    m_landingTitle->Init("images/sk8_logo.png", 400, 400, width/2, height/2, 0, 1, 1);
    m_landingInstructions->Init("images/enterclick.png", 300, 300, width/2, height/1.2f, 0, 1, 1);
    m_backgroundImageButton->Init("images/landing_bg.png",width,height,width/2,height/2,-1,1,1);
}

void _Scene::initGameplay()
{
    terrainBlueprint->LoadModel("models/terrain.obj","models/Terrain_Tex.png");
    terrainInstance->pos = Vector3(0,20,-5);
    terrainInstance->scale = Vector3(150,150,150);
    terrainInstance->SetPushable(true);
    terrainInstance->SetRotatable(true);

    Vector3 terrainMin = Vector3(-1, -.30,-1);
    // the y -0.8f means that it's 80% from the center towards the bottom (-1.0f)
    // note that it must always be between -1.0f & 1.0f
    Vector3 terrainMax = Vector3(1, -.25, 1);
    terrainInstance->AddCollider(new _CubeHitbox(terrainMin, terrainMax, COLLIDER_FLOOR));

    // ----- TERRAIN WALL COLLIDERS -----

    // Define a tall height for the walls in model space
    float wallMinY = -1.0f;
    float wallMaxY = 10.0f;
    float thickness = 0.1f; // local space thickness

    // North, South, East, West
    // are determined by camera yaw = 0
    // forward (W) = -Z - south
    // backward (S) = +Z - north
    // right (D) = +X - east
    // left (A) = -X - west


    // wall 1 far Z - just beyond the +1.0 z-edge
    Vector3 wallSouthMin = Vector3(-0.5f, wallMinY, 0.5f);
    Vector3 wallSouthMax = Vector3(0.5f, wallMaxY, 0.5f + thickness);
    terrainInstance->AddCollider(new _CubeHitbox(wallSouthMin, wallSouthMax, COLLIDER_WALL));

    // wall 2 naer z - just beyond the -1.0 z-edge
    Vector3 wallNorthMin = Vector3(-0.5f, wallMinY, -0.5f - thickness);
    Vector3 wallNorthMax = Vector3(0.5f, wallMaxY, -0.5f);
    terrainInstance->AddCollider(new _CubeHitbox(wallNorthMin, wallNorthMax, COLLIDER_WALL));

    // wall 3 right x  - just beyond the +1.0 x-edge
    Vector3 wallEastMin = Vector3(0.45f, wallMinY, -0.5f);
    Vector3 wallEastMax = Vector3(0.45f + thickness, wallMaxY, 0.5f);
    terrainInstance->AddCollider(new _CubeHitbox(wallEastMin, wallEastMax, COLLIDER_WALL));

    Vector3 sphereWallSouthEast = Vector3(0.3f,-0.3f,-0.4f);
    terrainInstance->AddCollider(new _SphereHitbox(sphereWallSouthEast,0.2f,COLLIDER_WALL));

    Vector3 sphereWallNorthEast = Vector3(0.4f,-0.3f,0.35f);
    terrainInstance->AddCollider(new _SphereHitbox(sphereWallNorthEast,0.3f,COLLIDER_WALL));

    // wall 4 left x - just beyond the -1.0 x-edge
    Vector3 wallWestMin = Vector3(-0.25f - thickness, wallMinY, -0.5f);
    Vector3 wallWestMax = Vector3(-0.25f, wallMaxY, 0.5f);
    terrainInstance->AddCollider(new _CubeHitbox(wallWestMin, wallWestMax, COLLIDER_WALL));

    // Flatten the terrain model to make a huge floor
    m_customFloor->pos = Vector3(0, 0, 0);
    m_customFloor->scale = Vector3(200, 1, 200);

    // Add a floor collider (Box from y=-5 to y=0)
    m_customFloor->AddCollider(new _CubeHitbox(
        Vector3(-1, -1.0f, -1), 
        Vector3(1, 0.0f, 1), 
        COLLIDER_FLOOR
    ));

    m_camera->camInit();

    m_skybox->skyBoxInit();
    m_skybox->tex[0] = m_skybox->textures->loadTexture("images/skybox/front.jpg");
    m_skybox->tex[1] = m_skybox->textures->loadTexture("images/skybox/back.jpg");
    m_skybox->tex[2] = m_skybox->textures->loadTexture("images/skybox/top.jpg");
    m_skybox->tex[3] = m_skybox->textures->loadTexture("images/skybox/bottom.jpg");
    m_skybox->tex[4] = m_skybox->textures->loadTexture("images/skybox/right.jpg");
    m_skybox->tex[5] = m_skybox->textures->loadTexture("images/skybox/left.jpg");

    m_player_blueprint->LoadTexture("models/player/Human_Atlas.png");
    m_player_blueprint->RegisterAnimation("idle","models/player/idle",1);
    m_player_blueprint->RegisterAnimation("walk", "models/player/walk",2);
    //m_player_blueprint->RegisterAnimation("walk","models/player/walk",2);


    m_skateboardBlueprint->LoadTexture("models/skateboard/colormap.png");
    m_skateboardBlueprint->RegisterAnimation("idle","models/skateboard/skateboard",1);
    m_player = new _Player(m_player_blueprint, m_skateboardBlueprint);
    m_player->RegisterStaticCollider(terrainInstance);
    m_player->RegisterStaticCollider(m_railInstance);
    

    // add sphere collider centered at (0,0,0) local space & r=1.0
    // note: model is norm -1 to +1
    //m_player->AddCollider(new _SphereHitbox(Vector3(0,0,0),1.0f));

    m_bulletBlueprint->LoadModel("models/bullet/untitled.obj","models/bullet/BulletAtlas.png");
    //m_bulletInstance->scale = Vector3(0.2,0.2,0.2);
    m_bulletManager = new _Bullets(m_bulletBlueprint);

    m_railBlueprint->LoadModel("models/skatepark assets/rail/rail.obj","models/skatepark assets/colormap.png");
    m_railInstance->pos = Vector3(5, -17, -10); // Position it in the world
    //m_railInstance->scale = Vector3(1, 1, 1); // Make it long and thin
    //m_railInstance->rotation.y = 15; // Angle it slightly
    // Add a collider for the rail
    m_railInstance->AddCollider(new _CubeHitbox(
        Vector3(-1, -1, -1), // Min corner (local space)
        Vector3(1, 1, 1),   // Max corner (local space)
        COLLIDER_RAIL                // Set the type
    ));

    // HALF PIPE
    m_halfpipeBlueprint->LoadModel("models/skatepark assets/halfpipe/halfpipe.obj", "models/skatepark assets/colormap.png");
    m_halfpipeInstance->AddCollider(new _CubeHitbox(Vector3(-1,-1,-1),Vector3(1,1,1),COLLIDER_HALFPIPE));
    m_halfpipeInstance->pos = Vector3(20,-13.5,-10);
    m_halfpipeInstance->scale = Vector3(4,4,4);
    //m_halfpipeInstance->rotation = Vector3(0,90,0);
    m_player->RegisterStaticCollider(m_halfpipeInstance);

    m_scaffoldBlueprint->LoadModel("models/skatepark assets/scaffold/scaffold.obj", "models/skatepark assets/colormap.png");
    m_stairsBlueprint->LoadModel("models/skatepark assets/stairs/stairs.obj", "models/skatepark assets/colormap.png");
    m_woodFloorBlueprint->LoadModel("models/skatepark assets/floor/floor.obj", "models/skatepark assets/colormap.png");
    m_sideBlueprint->LoadModel("models/skatepark assets/side piece/side.obj", "models/skatepark assets/colormap.png");

    m_scoreManager->Init();
    m_player->SetScoreManager(m_scoreManager);
}

void _Scene::initLevelEditor() {
    // Only call this once usually, or check if already init
    m_levelEditor->Init(width, height);

    if(m_player_blueprint) {
        m_levelEditor->SetReferenceBlueprint(m_player_blueprint);
    }

    // Initialize Pause Menu Buttons centered on screen
    int cX = width / 2;
    int cY = height / 2;
    
    m_editorResumeButton->Init("images/play-btn.png", 200, 60, cX, cY - 60, 0, 1, 1);
    m_editorSaveButton->Init("images/save-btn.png", 200, 60, cX, cY + 10, 0, 1, 1);
    m_editorExitButton->Init("images/exit-btn.png", 200, 60, cX, cY + 80, 0, 1, 1);
}

void _Scene::initMainMenu()
{
    // positions are 2d pixels, assumes (0,0) is top left
    m_playButton->Init("images/play-btn.png", 200, 70, width/2, height/2 - 120, 0, 1, 1);

    // PLAY CUSTOM BUTTON
    m_playCustomButton->Init("images/custom-btn.png", 200, 70, width/2, height/2 - 40, 0, 1, 1);
    // (You might need a new texture for this button, reusing play-btn for now)

    m_editorButton->Init("images/editor-btn.png", 200, 70, width/2, height/2 + 40, 0, 1, 1); 
    m_helpButton->Init("images/help-btn.png", 200, 70, width/2, height/2 + 120, 0, 1, 1);
    m_exitButton->Init("images/exit-btn.png", 200, 70, width/2, height/2 + 200, 0, 1, 1);
}

void _Scene::initHelpScreen()
{
    m_backButton->Init("images/exit-btn.png", 150, 50, 100, height - 100, 0, 1, 1); // Placeholder
    m_helpInfo->Init("images/help-page.png",width/2,height/2,width/2,height/2,0,1,1);
}

void _Scene::initPauseMenu()
{
    m_resumeButton->Init("images/play-btn.png", 200, 70, width/2, height/2 - 100, 0, 1, 1);
    m_pauseHelpButton->Init("images/help-btn.png", 200, 70, width/2, height/2, 0, 1, 1);
    m_pauseMenuButton->Init("images/exit-btn.png", 200, 70, width/2, height/2 + 100, 0, 1, 1);
}

void _Scene::draw2DOverlay()
{
    // switch to 2d orthographic mode
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);


    glMatrixMode(GL_PROJECTION);
    // should be popped in the calling function
    glPushMatrix();
    glLoadIdentity();
    // left, right, bottom, top
    gluOrtho2D(0,width,height,0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void _Scene::updateGameplay()
{
    m_player->UpdatePhysics();
    m_bulletManager->Update();
    m_scoreManager->Update(); // Updates popup positions/lifetimes

    // update cam set eye/des/up based on player
    m_player->UpdateCamera(m_camera);
}

void _Scene::drawGameplay()
{
    // reenable 3D states that the 2D overlay disables
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); // reenable culling for 3D
    glEnable(GL_BLEND);

    // update cam set eye/des/up based on player
    m_player->UpdateCamera(m_camera);

    m_camera->setUpCamera();
    // stop writing to the depth buffer
    // it was causing problems with a skybox-terrain interaction
    glDepthMask(GL_FALSE);
    m_skybox->drawSkyBox();
    // start writing to the depth buffer again
    glDepthMask(GL_TRUE);

    //terrainInstance->Draw();
    //m_railInstance->Draw();

    m_player->Draw();

    //m_halfpipeInstance->Draw();
    
    //m_bulletInstance->Draw();
    m_bulletManager->Draw();

    if (m_isCustomGame) 
    {
        m_customFloor->Draw();
        // Draw ONLY Custom Objects
        for(auto* obj : m_customLevelObjects) {
            obj->Draw();
        }
    }
    else 
    {
        // Draw ONLY Campaign Objects
        // TODO: (Eventually use a switch statement here for Level 1, 2, 3)
        terrainInstance->Draw();
        m_railInstance->Draw();
        m_halfpipeInstance->Draw();
    }

    // Draw this LAST (after player, map, etc) so text appears on top
    m_scoreManager->Draw(m_camera);

    // this used to be the gun area but could be used to
    // draw anything over the scene

    // after this point render ON TOP of the 3D world
    glClear(GL_DEPTH_BUFFER_BIT);

    // reset the viewport to the origin
    // we are no longer in the 3D world, we are at (0,0,0)
    // looking down the  zaxis.
    glLoadIdentity();

    
   
}

void _Scene::drawLevelEditor()
{
    if(m_sceneState == SceneState::LevelEditor) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    m_camera->setUpCamera();
    m_skybox->drawSkyBox(); // Ensure depth mask handling if needed

    m_levelEditor->Update(GetActiveWindow(), m_camera);
    m_levelEditor->Draw();
}

void _Scene::drawEditorPauseMenu()
{
    draw2DOverlay(); 

    // Semi-transparent dark overlay
    glEnable(GL_BLEND);
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(width, 0);
        glVertex2f(width, height);
        glVertex2f(0, height);
    glEnd();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDisable(GL_BLEND);

    // Draw Buttons
    m_editorResumeButton->Draw();
    m_editorSaveButton->Draw();
    m_editorExitButton->Draw();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}



void _Scene::drawLandingPage()
{
    draw2DOverlay(); // set up 2D drawing

    // draw the title
    m_backgroundImageButton->Draw();
    m_landingTitle->Draw();
    m_landingInstructions->Draw();
    

    // restore 3D projection
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void _Scene::drawMainMenu()
{
    draw2DOverlay(); // set up 2D drawing

    // draw the buttons
    m_backgroundImageButton->Draw();
    m_playButton->Draw();
    m_playCustomButton->Draw();
    m_helpButton->Draw();
    m_exitButton->Draw();
    m_editorButton->Draw();

    // restore 3D projection
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void _Scene::drawHelpScreen()
{
    draw2DOverlay(); // set up 2D drawing

    m_backButton->Draw();
    m_helpInfo->Draw();


    // restore 3D projection
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void _Scene::drawPauseMenu()
{
    draw2DOverlay(); // set up 2D drawing

    // draw a semi transparent black overlay
    glEnable(GL_BLEND);
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(width, 0);
        glVertex2f(width, height);
        glVertex2f(0, height);
    glEnd();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    


    if (m_showPauseHelp)
    {
        // draw the help info and a back button
        m_helpInfo->Draw();
        m_backButton->Draw(); // use the existing back button
    }
    else
    {
        // draw the pause buttons
        m_resumeButton->Draw();
        m_pauseHelpButton->Draw();
        m_pauseMenuButton->Draw();
    }

    // restore 3D projection
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void _Scene::handleMouseMovement(HWND hWnd, LPARAM lParam)
{
    if (m_sceneState == SceneState::Paused)
    {
        return; // don't move camera if paused
    }

    int mouseX = LOWORD(lParam);
    int mouseY = HIWORD(lParam);
    int centerX = width / 2;
    int centerY = height / 2;

    // if the mouse is at the center, ignore this "fake" event
    if (mouseX == centerX && mouseY == centerY) {
        return;
    }

    // calc delta from the center
    float deltaX = (float)(mouseX - centerX);
    float deltaY = (float)(mouseY - centerY);

    // Send deltas directly to the player
    m_player->HandleMouse(deltaX, deltaY);

    // reset mouse to center
    POINT centerPoint = { centerX, centerY };
    ClientToScreen(hWnd, &centerPoint);
    SetCursorPos(centerPoint.x, centerPoint.y);
}

void _Scene::loadCampaignLevel() {
    // DEBUG
    m_scoreManager->SetScoreObjective(5000, 60.0f); // 5000 points in 2 minutes
    // 1. Set Flag
    m_isCustomGame = false;

    // 2. Clear Custom Objects (Physics & Visuals)
    for(auto* obj : m_customLevelObjects) delete obj;
    m_customLevelObjects.clear();

    // 3. Reset Player Physics
    m_player->ClearColliders();
    
    // 4. Register Standard Level Colliders
    m_player->RegisterStaticCollider(terrainInstance);
    m_player->RegisterStaticCollider(m_railInstance);
    m_player->RegisterStaticCollider(m_halfpipeInstance);

    // 5. Reset Player Position
    m_player->m_body->pos = Vector3(0, 5, 0);
    m_player->ResetBoard();
}

void _Scene::loadCustomLevel() {
    m_isCustomGame = true;
    
    // 1. Clear objects
    for(auto* obj : m_customLevelObjects) delete obj;
    m_customLevelObjects.clear();

    // 2. Reset Physics
    m_player->ClearColliders();
    m_player->RegisterStaticCollider(m_customFloor);

    // 3. Load File
    ifstream file("saves/level_custom.txt");
    if (!file.is_open()) {
        cout << "No custom level found!" << endl;
        return;
    }

    string type;
    float x, y, z, rot, scale;
    while (file >> type >> x >> y >> z >> rot >> scale) {
        
        _StaticModel* blueprint = nullptr;
        
        // --- SELECT BLUEPRINT ---
        if(type == "rail") blueprint = m_railBlueprint;
        else if(type == "halfpipe") blueprint = m_halfpipeBlueprint;
        else if(type == "scaffold") blueprint = m_scaffoldBlueprint;
        else if(type == "stairs") blueprint = m_stairsBlueprint;
        else if(type == "wood_floor") blueprint = m_woodFloorBlueprint;
        else if(type == "side") blueprint = m_sideBlueprint;
        
        if (blueprint) {
            _StaticModelInstance* newObj = new _StaticModelInstance(blueprint);
            newObj->pos = Vector3(x, y, z);
            newObj->rotation.y = rot;
            newObj->scale = Vector3(scale, scale, scale);
            
            // --- ASSIGN COLLIDERS (Must match Level Editor logic) ---
            Vector3 cMin(-1,-1,-1);
            Vector3 cMax(1,1,1);

            if(type == "halfpipe") {
                newObj->AddCollider(new _CubeHitbox(cMin, cMax, COLLIDER_HALFPIPE));
            } 
            else if(type == "rail") {
                newObj->AddCollider(new _CubeHitbox(cMin, cMax, COLLIDER_RAIL));
            }
            else if(type == "stairs") {
                newObj->AddCollider(new _CubeHitbox(cMin, cMax, COLLIDER_STAIRS));
            }
            else if(type == "wood_floor") {
                // Thin Floor Logic
                newObj->AddCollider(new _CubeHitbox(Vector3(-1, -0.1f, -1), Vector3(1, 0.1f, 1), COLLIDER_FLOOR));
            }
            else {
                // Scaffolds and Sides are Walls
                newObj->AddCollider(new _CubeHitbox(cMin, cMax, COLLIDER_WALL));
            }
            
            m_customLevelObjects.push_back(newObj);
            m_player->RegisterStaticCollider(newObj);
        }
    }
    file.close();
    
    m_player->m_body->pos = Vector3(0, 1, 0);
    m_player->ResetBoard();
}