#ifndef _SCENE_H
#define _SCENE_H

#include"_common.h"
#include"_light.h"
#include"_model.h"
#include"_inputs.h"
#include"_textureloader.h"
#include"_parallax.h"
#include"_skybox.h"
#include"_sprite.h"
#include"_timer.h"
//#include"_3dmodelloader.h"
#include"_MD2Model.h"
#include"_ModelInstance.h"
#include"_camera.h"
#include "_Time.h"
#include"_AnimatedModel.h"

class _Scene
{
    public:
        _Scene();           //constructor
        virtual ~_Scene();  //Destructor

        _light *myLight = new _light();   //light settings
        _model *myModel = new _model();   //create a model
        _inputs *myInput = new _inputs(); // input handle
        _textureLoader *myTexture = new _textureLoader();// for loading images
        _parallax *myPrlx = new _parallax();
        _skyBox *mySkyBox = new _skyBox();
        _sprite *mySprite = new _sprite();
        _timer *myTime = new _timer();
        _camera *myCam = new _camera();

        //_3DModelLoader *mdl3D = new _3DModelLoader();
        //_3DModelLoader *mdl3DW = new _3DModelLoader();

        // new model system

        //// assets from disk
        //_MD2Model* playerModel;
        //_MD2Model* weaponModel;

        //// assets in world
        //_ModelInstance* player;
        //_ModelInstance* weapon;

        _AnimatedModel* skaterModel;
        _ModelInstance* skater;


        void reSizeScene(int width, int height);  // resize window
        void initGL();                            // initialize GL graphics
        void drawScene();                         // render scene
        int winMsg(HWND,UINT,WPARAM,LPARAM);      // to get keyboard interrupts and pass it to inputs
        // x & y
        void mouseMapping(int,int);

        double msX,msY,msZ;

        int width, height;  // keep record of the screen size
    protected:

    private:
};

#endif // _SCENE_H
