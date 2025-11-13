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
#include<_3dmodelloader.h>
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
#include"_TargetManager.h"

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
        void initLandingPage();
        void drawLandingPage();
        void handleLandingPageInput(UINT,WPARAM,LPARAM);

        // --- Main Menu Scene ---
        _Button *m_playButton;
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


    protected:

    private:

        _StaticModel *terrainBlueprint;
        _StaticModelInstance *terrainInstance;

        _inputs *m_inputs;
        _camera *m_camera;
        _AnimatedModel *m_player_blueprint;
        _Player *m_player;

        _StaticModel *m_bulletBlueprint;
        _Bullets *m_bulletManager;

        _AnimatedModel* m_targetBlueprint;
        _TargetManager* m_targetManager;

        _StaticModel* m_gunBlueprint;
        _StaticModelInstance* m_gunInstance;

        enum SceneState {LandingPage, MainMenu, Help, Playing, Paused};
        SceneState m_sceneState;

        void draw2DOverlay();
        void handleMouseMovement(HWND hWnd, LPARAM lParam);
};

#endif // _SCENE_H
