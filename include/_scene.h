#ifndef _SCENE_H
#define _SCENE_H

#include<_common.h>
#include<_light.h>
#include<_model.h>
#include<_inputs.h>
#include<_textureloader.h>
#include<_parallax.h>
#include<_skybox.h>
#include<_sprite.h>
#include<_timer.h>
#include<_camera.h>
#include<_collisioncheck.h>
#include"_StaticModelInstance.h"
#include"_inputs.h"
#include"_Button.h"
#include"_skybox.h"
#include"_Collider.h"
#include"_CubeHitbox.h"
#include"_SphereHitbox.h"
#include"_AnimatedModelInstance.h"
#include"_Player.h"
#include"_Bullets.h"
#include"_LevelEditor.h"

class _Scene
{
    public:
        _Scene();           //constructor
        virtual ~_Scene();  //Destructor

        int clickCnt =0;

        void reSizeScene(int width, int height);  // resize window
        void initGL();                            // initialize GL graphics
        void drawScene();                         // render scene
        int winMsg(HWND,UINT,WPARAM,LPARAM);      // to get keyboard interrupts and pass it to inputs
        void mouseMapping(int,int);
        double msX,msY,msZ;

        int width, height;  // keep record of the screen size

        void initGameplay();
        void drawGameplay();
        void updateGameplay();
        void handleGameplayInput(HWND,UINT,WPARAM,LPARAM);

        // --- Landing Page Scene ---
        _Button *m_landingTitle;
        _Button *m_landingInstructions;
        _Button *m_backgroundImageButton;
        void initLandingPage();
        void drawLandingPage();
        void handleLandingPageInput(UINT,WPARAM,LPARAM);

        // --- Main Menu Scene ---
        _Button *m_playButton;
        _Button* m_playCustomButton;
        _Button *m_helpButton;
        _Button *m_exitButton;
        _skyBox *m_skybox;
        void initMainMenu();
        void drawMainMenu();
        void handleMainMenuInput(UINT,WPARAM,LPARAM);

        
        // --- Help Scene ---
        _Button *m_backButton;
        _Button *m_helpInfo;
        void initHelpScreen();
        void drawHelpScreen();
        void handleHelpScreenInput(UINT,WPARAM,LPARAM);

        // --- Pause Menu ---
        _Button *m_resumeButton;
        _Button *m_pauseHelpButton;
        _Button *m_pauseMenuButton;
        bool m_showPauseHelp; // To toggle help overlay
        void initPauseMenu();
        void drawPauseMenu();
        void handlePauseMenuInput(UINT,WPARAM,LPARAM);

        // --- Level Editor Methods ---
        _Button* m_editorButton;  // Button on main menu
        void initLevelEditor();   // Called once
        void drawLevelEditor();   // Render loop
        void handleLevelEditorInput(HWND, UINT, WPARAM, LPARAM); // Input loop
        vector<_StaticModelInstance*> m_customLevelObjects;

        bool m_isCustomGame;
        void loadCampaignLevel();
        void loadCustomLevel();

        _StaticModelInstance* m_customFloor;


        void drawEditorPauseMenu();
        void handleEditorPauseInput(HWND, UINT, WPARAM, LPARAM);

        _Button* m_editorResumeButton;
        _Button* m_editorSaveButton;
        _Button* m_editorExitButton;


    protected:

    private:

        _StaticModel *terrainBlueprint;
        _StaticModelInstance *terrainInstance;

        // grind rail
        _StaticModel* m_railBlueprint;
        _StaticModelInstance* m_railInstance;

        _inputs *m_inputs;
        _camera *m_camera;
        _AnimatedModel *m_player_blueprint;
        _Player *m_player;
        _AnimatedModel *m_skateboardBlueprint;

        _StaticModel *m_bulletBlueprint;
        _Bullets *m_bulletManager;

        _StaticModel *m_halfpipeBlueprint;
        _StaticModelInstance *m_halfpipeInstance;

        // --- NEW BLUEPRINTS FOR CUSTOM LEVEL ---
        _StaticModel* m_scaffoldBlueprint;
        _StaticModel* m_stairsBlueprint;
        _StaticModel* m_woodFloorBlueprint;
        _StaticModel* m_sideBlueprint;


        enum SceneState {LandingPage, MainMenu, Help, Playing, Paused, LevelEditor, EditorPaused};
        SceneState m_sceneState;

        _LevelEditor* m_levelEditor;

        void draw2DOverlay();
        void handleMouseMovement(HWND hWnd, LPARAM lParam);
};

#endif // _SCENE_H
