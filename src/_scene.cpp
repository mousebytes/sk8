#include "_scene.h"

_Scene::_Scene()
{
    m_isCustomGame = false;
    //ctor

    // terrainBlueprint = new _StaticModel();
    // terrainInstance = new _StaticModelInstance(terrainBlueprint);

    // Instead, we initialize the floor blueprint here
    terrainBlueprint = new _StaticModel();
    // We will init the instance in initGameplay()

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

    // This will be our main floor for everything now
    m_customFloor = new _StaticModelInstance(terrainBlueprint);

    m_scaffoldBlueprint = new _StaticModel();
    m_stairsBlueprint = new _StaticModel();
    m_woodFloorBlueprint = new _StaticModel();
    m_sideBlueprint = new _StaticModel();

    // Initialize Spraycan
    m_sprayCanBlueprint = new _StaticModel();

    m_backgroundImageButton = new _Button();

    m_scoreManager = new _ScoreManager();

    // Init Level Progression Vars
    m_currentLevelIndex = 1;
    m_levelTransitionTimer = 0.0f;
    m_levelCompleteTriggered = false;

    m_particleSystem = new _ParticleSystem();

    //Init Sound Manager
    m_sounds = new _sounds();
    m_sounds->initSounds();

    m_creditsButton = new _Button();
    m_creditsImage = new _Button();
}

_Scene::~_Scene()
{
    //dtor

    delete terrainBlueprint;
    //delete terrainInstance; // Removed
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

    // Cleanup Tags
    for(auto* tag : m_activeTags) delete tag;
    m_activeTags.clear();

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
    delete m_sprayCanBlueprint;

    delete m_backgroundImageButton;


    delete m_particleSystem;

    delete m_scoreManager;
    if(m_sounds) delete m_sounds;

    delete m_creditsButton;
    delete m_creditsImage;
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
    initCreditsScreen();

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
        case SceneState::Credits:
            drawCreditsScreen();
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
        case SceneState::Credits:
            handleCreditsScreenInput(uMsg, wParam, lParam);
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
                // --- CHECK IF WON/LOST FIRST ---
                GameState state = m_scoreManager->GetState();
                
                // If Lost, OR if Won and it is the Final Level -> ESC goes to Main Menu
                if (state == GAME_LOST || (state == GAME_WON && m_currentLevelIndex == 3)) {
                     m_sceneState = SceneState::MainMenu;
                     m_camera->isFreeCam = false;
                     // Reset cursor
                     POINT pt = { width / 2, height / 2 };
                     ClientToScreen(hWnd, &pt);
                     SetCursorPos(pt.x, pt.y);
                     return; 
                }

                // Otherwise toggle pause
                m_sceneState = SceneState::Paused;
                m_player->isFrozen = true; 
                m_player->PauseSkateSound();
                break; 
            }

            m_inputs->wParam = wParam;
            m_camera->HandleKeys(uMsg, wParam);
            m_player->HandleKeys(uMsg, wParam);

            if(wParam == '1'){ isDebug=!isDebug; }
            else if(wParam=='2'){
                colliderDrawFace=!colliderDrawFace;
                if(!isDebug){ isDebug=true; colliderDrawFace=true; }
            }
            
            else if(wParam == '7') loadCampaignLevel1();
            else if(wParam == '8') loadCampaignLevel2();
            else if(wParam == '9') loadCampaignLevel3();

            break;
        case WM_KEYUP:
            m_player->HandleKeys(uMsg, wParam);
            m_camera->HandleKeys(uMsg, wParam);
            break;

        case WM_LBUTTONDOWN:
            mouseMapping(LOWORD(lParam), HIWORD(lParam));
            {
                Vector3 startPos = m_camera->eye;
                Vector3 direction = m_camera->des - m_camera->eye;
             }
            break;

        case WM_MOUSEMOVE:
            if(m_camera->isFreeCam) m_camera->handleMouse(hWnd, LOWORD(lParam), HIWORD(lParam), width / 2, height / 2);
            handleMouseMovement(hWnd, lParam);
            break;

        default: break;
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

             // Removed m_inputs->keyPressed(m_camera);
             m_camera->HandleKeys(uMsg, wParam);

             m_levelEditor->HandleKeyInput(wParam); // J, K, R

             // Go to Editor Pause Menu instead of Main Menu
             if(wParam == VK_ESCAPE) {
                 m_sceneState = SceneState::EditorPaused;
             }

             // --- Toggle Camera Lock on Enter ---
             if(wParam == VK_RETURN || wParam == VK_CONTROL) {
                 m_camera->isFreeCam = !m_camera->isFreeCam;

                 // If we just re-enabled the camera, snap cursor to center immediately.
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
        case WM_KEYUP:
             m_camera->HandleKeys(uMsg, wParam);
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

            loadCampaignLevel(); // Defaults to level 1
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
            m_camera->camReset();
            m_camera->isFreeCam = true;
        }
        else if (m_helpButton->isClicked(mouseX, mouseY)) {
            m_sceneState = SceneState::Help;
        }
        else if (m_exitButton->isClicked(mouseX,mouseY)){
            exit(0);
        }
        else if (m_creditsButton->isClicked(mouseX, mouseY)) {
            m_sceneState = SceneState::Credits;
        }

    }
}

void _Scene::handleCreditsScreenInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                m_player->StopSkateSound();
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

    // --- SETUP UNIFIED FLOOR ---
    // Flatten the terrain model to make a huge floor
    // This is the SAME logic used in custom levels
    m_customFloor->pos = Vector3(0, 0, 0);
    m_customFloor->scale = Vector3(200, 1, 200);

    // Add a floor collider (Box from y=-5 to y=0)
    m_customFloor->AddCollider(new _CubeHitbox(
        Vector3(-1, -1.0f, -1),
        Vector3(1, 0.0f, 1),
        COLLIDER_FLOOR
    ));

    // REMOVED OLD TERRAIN INSTANCE INIT

    m_camera->camInit();

    m_skybox->skyBoxInit();
    m_skybox->tex[0] = m_skybox->textures->loadTexture("images/skybox/front.jpg");
    m_skybox->tex[1] = m_skybox->textures->loadTexture("images/skybox/back.jpg");
    m_skybox->tex[2] = m_skybox->textures->loadTexture("images/skybox/top.jpg");
    m_skybox->tex[3] = m_skybox->textures->loadTexture("images/skybox/bottom.jpg");
    m_skybox->tex[4] = m_skybox->textures->loadTexture("images/skybox/right.jpg");
    m_skybox->tex[5] = m_skybox->textures->loadTexture("images/skybox/left.jpg");

    m_player_blueprint->LoadTexture("models/alternatePlayers/man1/man_tex.png");
    m_player_blueprint->RegisterAnimation("idle","models/alternatePlayers/man1/idle",1);
    m_player_blueprint->RegisterAnimation("kick", "models/alternatePlayers/man1/kick",2);
    m_player_blueprint->RegisterAnimation("walk", "models/alternatePlayers/man1/walk",2);
    m_player_blueprint->RegisterAnimation("idleWalk","models/alternatePlayers/man1/idleWalk",1);


    m_skateboardBlueprint->LoadTexture("models/skateboard/colormap.png");
    m_skateboardBlueprint->RegisterAnimation("idle","models/skateboard/skateboard",1);
    m_player = new _Player(m_player_blueprint, m_skateboardBlueprint);

    m_player->SetParticleSystem(m_particleSystem);
    m_particleSystem->Init();

    // Register the unified floor initially
    m_player->RegisterStaticCollider(m_customFloor);

    m_sprayCanBlueprint->LoadModel("models/spraycan/spraycan.obj", "models/spraycan/map.png");


    m_bulletBlueprint->LoadModel("models/bullet/untitled.obj","models/bullet/BulletAtlas.png");
    m_bulletManager = new _Bullets(m_bulletBlueprint);

    m_railBlueprint->LoadModel("models/skatepark assets/rail/rail.obj","models/skatepark assets/colormap.png");
    m_railInstance->pos = Vector3(5, -17, -10); // Position it in the world
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

    m_scaffoldBlueprint->LoadModel("models/skatepark assets/scaffold/scaffold.obj", "models/skatepark assets/colormap.png");
    m_stairsBlueprint->LoadModel("models/skatepark assets/stairs/stairs.obj", "models/skatepark assets/colormap.png");
    m_woodFloorBlueprint->LoadModel("models/skatepark assets/floor/floor.obj", "models/skatepark assets/colormap.png");
    m_sideBlueprint->LoadModel("models/skatepark assets/side piece/side.obj", "models/skatepark assets/colormap.png");

    m_scoreManager->Init();
    m_player->SetScoreManager(m_scoreManager);

    m_player->SetSoundManager(m_sounds);

    m_scoreManager->SetSoundManager(m_sounds);

    m_sounds->playMusic("sounds/KickPush.mp3");
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
    m_creditsButton->Init("images/help-btn.png", 150, 50, width - 100, height - 100, 0, 1, 1);
}

void _Scene::initCreditsScreen()
{
    // Load the full screen credits image
    m_creditsImage->Init("images/menus/Credits.png", width, height, width/2, height/2, 0, 1, 1);
    
    // We can reuse the back button from the Help screen logic
    // But we need to ensure it's initialized. 
    // (It is initialized in initHelpScreen, which is called in initGL, so we are safe)
}

void _Scene::initHelpScreen()
{
    m_helpInfo->Init("images/menus/MainHelp.png",width,height,width/2,height/2,0,1,1);
    m_backButton->Init("images/exit-btn.png", 150, 50, 100, height - 100, 0, 1, 1); // Placeholder
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
    m_particleSystem->Update();

    // Update Camera (Free Cam logic runs here if active)
    m_camera->Update();

    m_bulletManager->Update();
    m_scoreManager->Update(); // Updates popup positions/lifetimes

    // --- Update Tags ---
    CheckTagCollisions();

    // Animate Tags (Spin)
    for(auto* tag : m_activeTags) {
        tag->rotation.y += 100.0f * _Time::deltaTime;
    }

    // update cam set eye/des/up based on player
    // Note: If FreeCam is active, Player::UpdateCamera returns early
    m_player->UpdateCamera(m_camera);

    // --- LEVEL PROGRESSION LOGIC ---
    if (!m_isCustomGame) { // Only progress in campaign
        if (m_scoreManager->GetState() == GAME_WON) {

            if (!m_levelCompleteTriggered) {
                m_levelCompleteTriggered = true;
                m_levelTransitionTimer = 3.0f; // Wait 3 seconds
            }
        }
    }

    if (m_levelCompleteTriggered) {
        m_levelTransitionTimer -= _Time::deltaTime;

        if (m_levelTransitionTimer <= 0.0f) {
             m_levelCompleteTriggered = false;

             // Only auto-advance if we are NOT on the final level (Level 3)
             if (m_currentLevelIndex < 3) {
                 m_currentLevelIndex++;

                 if (m_currentLevelIndex == 2) {
                     loadCampaignLevel2();
                 }
                 else if (m_currentLevelIndex == 3) {
                     loadCampaignLevel3();
                 }
             }
        }
    }
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

    // Draw the Unified Floor
    m_customFloor->Draw();

    m_player->Draw();
    m_particleSystem->Draw();

    m_bulletManager->Draw();

    // --- DRAW TAGS ---
    for(auto* tag : m_activeTags) {
        tag->Draw();
    }

    // Custom level objects (used for both custom games AND campaign now)
    for(auto* obj : m_customLevelObjects) {
        obj->Draw();
    }

    // Draw this LAST (after player, map, etc) so text appears on top
    m_scoreManager->Draw(m_camera,width,height);

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

    // Update Camera Movement in Editor Loop
    m_camera->Update();

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
    //glDisable(GL_BLEND);

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
    draw2DOverlay(); //setup 2d drawing

    m_backgroundImageButton->Draw();
    m_playButton->Draw();
    m_playCustomButton->Draw();
    m_helpButton->Draw();
    m_exitButton->Draw();
    m_editorButton->Draw();
    
    // Add this:
    m_creditsButton->Draw();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void _Scene::drawCreditsScreen()
{
    draw2DOverlay();

    m_creditsImage->Draw();
    m_backButton->Draw();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}


void _Scene::drawHelpScreen()
{
    draw2DOverlay(); // set up 2D drawing

    m_helpInfo->Draw();
    m_backButton->Draw();
    


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

// ---------------------------------------------
// HELPER: GENERIC LEVEL LOADER
// ---------------------------------------------
// Now handles both Custom Level logic AND Campaign Level Logic
// plus the "tag" type.
void _Scene::loadCustomLevel() {
    m_player->StopSkateSound();
    m_isCustomGame = true;
    m_scoreManager->SetFreePlay();

    // Clear old
    for(auto* obj : m_customLevelObjects) delete obj;
    m_customLevelObjects.clear();
    for(auto* tag : m_activeTags) delete tag;
    m_activeTags.clear();

    // Reset Physics
    m_player->ClearColliders();
    m_player->RegisterStaticCollider(m_customFloor); // Use unified floor

    // Reuse helper
    ifstream file("saves/level_custom.txt");
    if (!file.is_open()) {
        cout << "No custom level found!" << endl;
        return;
    }

    string type;
    float x, y, z, rot, scale;
    while (file >> type >> x >> y >> z >> rot >> scale) {

        // CHECK FOR TAG
        if (type == "tag") {
            _StaticModelInstance* t = new _StaticModelInstance(m_sprayCanBlueprint);
            t->pos = Vector3(x, y, z);
            t->rotation.y = rot;
            t->scale = Vector3(scale, scale, scale);
            m_activeTags.push_back(t);
            continue; // Skip the rest of loop
        }

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

            // --- ASSIGN COLLIDERS ---
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
                newObj->AddCollider(new _CubeHitbox(Vector3(-1, -0.1f, -1), Vector3(1, 0.1f, 1), COLLIDER_FLOOR));
            }
            else {
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

// Helper func to parse file string
void LoadLevelFromFile(string filename,
                       vector<_StaticModelInstance*>& objects,
                       vector<_StaticModelInstance*>& tags,
                       _Player* player,
                       _StaticModel* railBP,
                       _StaticModel* pipeBP,
                       _StaticModel* scaffBP,
                       _StaticModel* stairBP,
                       _StaticModel* woodBP,
                       _StaticModel* sideBP,
                       _StaticModel* tagBP)
{
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Failed to open level: " << filename << endl;
        return;
    }

    string type;
    float x, y, z, rot, scale;
    while (file >> type >> x >> y >> z >> rot >> scale) {

        // TAG LOGIC
        if (type == "tag") {
            _StaticModelInstance* t = new _StaticModelInstance(tagBP);
            t->pos = Vector3(x, y, z);
            t->rotation.z = rot; // Tags use Z rot for spin/tilt sometimes, or Y
            t->scale = Vector3(scale, scale, scale);
            tags.push_back(t);
            continue;
        }

        _StaticModel* blueprint = nullptr;
        if(type == "rail") blueprint = railBP;
        else if(type == "halfpipe") blueprint = pipeBP;
        else if(type == "scaffold") blueprint = scaffBP;
        else if(type == "stairs") blueprint = stairBP;
        else if(type == "wood_floor") blueprint = woodBP;
        else if(type == "side") blueprint = sideBP;

        if (blueprint) {
            _StaticModelInstance* newObj = new _StaticModelInstance(blueprint);
            newObj->pos = Vector3(x, y, z);
            newObj->rotation.y = rot;
            newObj->scale = Vector3(scale, scale, scale);

            Vector3 cMin(-1,-1,-1);
            Vector3 cMax(1,1,1);

            if(type == "halfpipe") newObj->AddCollider(new _CubeHitbox(cMin, cMax, COLLIDER_HALFPIPE));
            else if(type == "rail") newObj->AddCollider(new _CubeHitbox(cMin, cMax, COLLIDER_RAIL));
            else if(type == "stairs") newObj->AddCollider(new _CubeHitbox(cMin, cMax, COLLIDER_STAIRS));
            else if(type == "wood_floor") newObj->AddCollider(new _CubeHitbox(Vector3(-1, -0.1f, -1), Vector3(1, 0.1f, 1), COLLIDER_FLOOR));
            else newObj->AddCollider(new _CubeHitbox(cMin, cMax, COLLIDER_WALL));

            objects.push_back(newObj);
            player->RegisterStaticCollider(newObj);
        }
    }
    file.close();
}

// ---------------------------------------------
// CAMPAIGN LEVEL MANAGEMENT
// ---------------------------------------------

void _Scene::loadCampaignLevel() {
    loadCampaignLevel1();
}

void _Scene::loadCampaignLevel1() {
    m_player->StopSkateSound();
    m_isCustomGame = false;
    m_currentLevelIndex = 1;
    m_levelCompleteTriggered = false;

    m_scoreManager->SetFinalLevel(false);

    // 1. Clear old objects
    for(auto* obj : m_customLevelObjects) delete obj;
    m_customLevelObjects.clear();
    for(auto* tag : m_activeTags) delete tag;
    m_activeTags.clear();

    // 2. Set Objective
    m_scoreManager->SetScoreObjective(5000, 60.0f);

    // 3. Register Standard Physics (Unified Floor)
    m_player->ClearColliders();
    m_player->RegisterStaticCollider(m_customFloor); // Use unified floor

    // 4. LOAD FROM FILE
    LoadLevelFromFile("saves/level1.txt", m_customLevelObjects, m_activeTags, m_player,
                      m_railBlueprint, m_halfpipeBlueprint, m_scaffoldBlueprint,
                      m_stairsBlueprint, m_woodFloorBlueprint, m_sideBlueprint, m_sprayCanBlueprint);

    // 5. Reset Player
    m_player->m_body->pos = Vector3(0, 5, 0);
    m_player->ResetBoard();
}

void _Scene::loadCampaignLevel2() {
    m_player->StopSkateSound();
    m_isCustomGame = false;
    m_currentLevelIndex = 2;
    m_levelCompleteTriggered = false;

    m_scoreManager->SetFinalLevel(false);

    for(auto* obj : m_customLevelObjects) delete obj;
    m_customLevelObjects.clear();
    for(auto* tag : m_activeTags) delete tag;
    m_activeTags.clear();

    m_player->ClearColliders();
    m_player->RegisterStaticCollider(m_customFloor);

    LoadLevelFromFile("saves/level2.txt", m_customLevelObjects, m_activeTags, m_player,
                      m_railBlueprint, m_halfpipeBlueprint, m_scaffoldBlueprint,
                      m_stairsBlueprint, m_woodFloorBlueprint, m_sideBlueprint, m_sprayCanBlueprint);

    // DYNAMIC OBJECTIVE: Count the tags we just loaded!
    int totalTags = m_activeTags.size();
    m_scoreManager->SetTagObjective(totalTags, 120.0f);

    m_player->m_body->pos = Vector3(0, 5, 0);
    m_player->ResetBoard();
}

void _Scene::loadCampaignLevel3() {
    m_player->StopSkateSound();
    m_isCustomGame = false;
    m_currentLevelIndex = 3;
    m_levelCompleteTriggered = false;

    m_scoreManager->SetFinalLevel(true);

    for(auto* obj : m_customLevelObjects) delete obj;
    m_customLevelObjects.clear();
    for(auto* tag : m_activeTags) delete tag;
    m_activeTags.clear();

    m_player->ClearColliders();
    m_player->RegisterStaticCollider(m_customFloor);

    LoadLevelFromFile("saves/level3.txt", m_customLevelObjects, m_activeTags, m_player,
                      m_railBlueprint, m_halfpipeBlueprint, m_scaffoldBlueprint,
                      m_stairsBlueprint, m_woodFloorBlueprint, m_sideBlueprint, m_sprayCanBlueprint);

    // DYNAMIC OBJECTIVE: Count the tags we just loaded!
    int totalTags = m_activeTags.size();
    m_scoreManager->SetTagObjective(totalTags, 90.0f);

    m_player->m_body->pos = Vector3(0, 5, 0);
    m_player->ResetBoard();
}

void _Scene::SpawnTagsLevel2() {
    // Deprecated - logic moved to saves/level2.txt
}

void _Scene::CheckTagCollisions() {
    if (m_activeTags.empty()) return;

    for (auto it = m_activeTags.begin(); it != m_activeTags.end(); ) {
        _StaticModelInstance* tag = *it;

        Vector3 playerPos = m_player->m_body->pos;
        Vector3 tagPos = tag->pos;

        float distSq = pow(playerPos.x - tagPos.x, 2) +
                       pow(playerPos.y - tagPos.y, 2) +
                       pow(playerPos.z - tagPos.z, 2);

        if (distSq < (1.5f * 1.5f)) {
            m_scoreManager->CollectTag();
            delete tag;
            it = m_activeTags.erase(it);
        } else {
            ++it;
        }
    }
}
