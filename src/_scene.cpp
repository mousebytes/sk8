#include "_scene.h"

_Scene::_Scene()
{
    //ctor
    myTime->startTime = clock();

    //playerModel = nullptr;
    //weaponModel = nullptr;
    //player = nullptr;
    //weapon = nullptr;
    skaterModel = nullptr;
    skater = nullptr;
}

_Scene::~_Scene()
{
    //dtor
    delete myLight;
    delete myModel;
    delete myInput;
    delete myTexture;
    delete myPrlx;
    delete mySkyBox;
    delete mySprite;
    delete myTime;
    delete myCam;

    delete skater;
    delete skaterModel;
}

void _Scene::reSizeScene(int width, int height)
{
    float aspectRatio = (float)width/(float)height;// keep track of the ratio
    glViewport(0,0,width,height); // adjust my viewport

    glMatrixMode(GL_PROJECTION);  // To setup ptrojection
    glLoadIdentity();             // calling identity matrix
    gluPerspective(45, aspectRatio,0.1,1000.0); // setting perspective projection

    this->width = GetSystemMetrics(SM_CXSCREEN);
    this->height= GetSystemMetrics(SM_CYSCREEN);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();             // calling identity matrix
}

void _Scene::initGL()
{
    glShadeModel(GL_SMOOTH); // to handle GPU shaders
    glClearColor(0.0f,0.0f,0.0f,0.0f); // black background color
    glClearDepth(2.0f);         //depth test for layers
    glEnable(GL_DEPTH_TEST);    //activate depth test
    glDepthFunc(GL_LEQUAL);     // depth function type

    glEnable(GL_TEXTURE_2D);

    myLight->setLight(GL_LIGHT0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);


    myTexture->loadTexture("images/tex.jpg");
    myPrlx->parallaxInit("images/prlx.jpg");

    mySkyBox->skyBoxInit();
    mySkyBox->tex[0] = mySkyBox->textures->loadTexture("images/front.jpg");
    mySkyBox->tex[1] = mySkyBox->textures->loadTexture("images/back.jpg");
    mySkyBox->tex[2] = mySkyBox->textures->loadTexture("images/top.jpg");
    mySkyBox->tex[3] = mySkyBox->textures->loadTexture("images/bottom.jpg");
    mySkyBox->tex[4] = mySkyBox->textures->loadTexture("images/right.jpg");
    mySkyBox->tex[5] = mySkyBox->textures->loadTexture("images/left.jpg");
    mySkyBox->tex[6] = mySkyBox->textures->loadTexture("images/Stairs.jpg");

    //mySprite->spriteInit("images/eg.png",6,4);
    //mdl3D->initModel("models/Tekk/tris.md2");
    //mdl3DW->initModel("models/Tekk/weapon.md2");

    // Load Blueprints
    //playerModel = new _MD2Model();
    //playerModel->LoadModel("models/Tekk/tris.md2");

    //weaponModel = new _MD2Model();
    //weaponModel->LoadModel("models/Tekk/weapon.md2");

    // Create Instances from Blueprints
    //player = new _ModelInstance(playerModel);
    //weapon = new _ModelInstance(weaponModel);

    // Set default states
    //player->actionTrigger = player->STAND;
    //weapon->actionTrigger = weapon->STAND;

    myCam->camInit();

    _Time::Init();

    // skater init
    skaterModel = new _AnimatedModel();
    // load push_00.obj, push_01.obj, ... push_(n-1).obj
    skaterModel->LoadAnimation("models/skater/idle",3);
    skater = new _ModelInstance(skaterModel);
    skater->actionTrigger = skater->STAND;
}

void _Scene::drawScene()
{
    _Time::Update();
    //cout << '\n' << _Time::deltaTime;
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);//clear bits in each itteration
    glLoadIdentity();             // calling identity matrix

     //gluLookAt(0,0,10,0,0,0,0,1,0);
     myCam->setUpCam();


     glPushMatrix();
    myTexture->bindTexture();
    //myModel->drawModel();
     glPopMatrix();
    glPushMatrix();
    glScalef(4.33,4.33,1);
 //   myPrlx->drawParallax(width,height);
  //  myPrlx->prlxScrollAuto("left", 0.0005);
    mySkyBox->drawSkyBox();
    glPopMatrix();

/*      glPushMatrix();
       mySprite->drawSprite();
      // mySprite->actionTrigger = mySprite->WALKRIGHT;

    if(myTime->getTicks()>70)
    {
       mySprite->spriteActions();
       myTime->reset();
    }
    glPopMatrix();

*/

    /*
   glPushMatrix();
        glTranslatef(mdl3D->pos.x,mdl3D->pos.y,mdl3D->pos.z);
        glRotatef(-90,1,0,0);
        glRotatef(180,0,0,1);
        glScalef(0.1,0.1,0.1);
        mdl3D->Actions();
        mdl3DW->Actions();
        mdl3DW->Draw();
        mdl3D->Draw();
    glPopMatrix();
    */

    //weapon->pos = player->pos;

    //player->Actions();
    //weapon->Actions();

    //player->Update();
    //weapon->Update();

    /*
    glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        glRotatef(180, 0, 0, 1);
        glScalef(0.1, 0.1, 0.1);

        player->Draw();
        weapon->Draw();
    glPopMatrix();
    */

    // draw skater
    //skater->actionTrigger = skater->STAND;
    //skater->Actions();
    skater->Update();
    glDisable(GL_LIGHTING);
    skater->Draw();
    glEnable(GL_LIGHTING);
}


int _Scene::winMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_KEYDOWN:
            myInput->wParam = wParam;
            myInput->keyPressed(myModel);
            myInput->keyPressed(myPrlx);
            myInput->keyPressed(mySkyBox);
            myInput->keyPressed(mySprite);
            myInput->keyPressed(myCam);
            //myInput->keyPressed(mdl3D,mdl3DW);
        break;

        case WM_KEYUP:
            myInput->wParam = wParam;
            myInput->keyUp(mySprite);
            //mdl3D->actionTrigger=mdl3D->STAND;
            //mdl3DW->actionTrigger=mdl3DW->STAND;
        break;

        case WM_LBUTTONDOWN:
            myInput->wParam = wParam;
            myInput->mouseEventDown(myModel,LOWORD(lParam),HIWORD(lParam));
        break;

        case WM_RBUTTONDOWN:
            myInput->wParam = wParam;
            myInput->mouseEventDown(myModel,LOWORD(lParam),HIWORD(lParam));
        break;

         case WM_MBUTTONDOWN:
             myInput->wParam = wParam;
             myInput->mouseEventDown(myModel,LOWORD(lParam),HIWORD(lParam));
        break;


        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
            myInput->wParam = wParam;
            myInput->mouseEventUp();
            break;

        case WM_MOUSEMOVE:
              myInput->wParam = wParam;
              myInput->mouseMove(myModel,LOWORD(lParam),HIWORD(lParam));
            break;
        case WM_MOUSEWHEEL:
              myInput->wParam = wParam;
              myInput->mouseWheel(myModel,(double)GET_WHEEL_DELTA_WPARAM(wParam));
            break;

        default:
            break;

    }
}

void _Scene::mouseMapping(int x, int y)
{
    GLint viewPort[4];
    GLdouble modelViewM[16];
    GLdouble projectionM[16];
    GLfloat winX,winY,winZ;

    glGetDoublev(GL_MODELVIEW_MATRIX,modelViewM);
    glGetDoublev(GL_PROJECTION_MATRIX,projectionM);
    glGetIntegerv(GL_VIEWPORT,viewPort);

    winX = (GLfloat)x;
    winY=(GLfloat)y;

    glReadPixels(x,(int)winY,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&winZ);
    gluUnProject(winX,winY,winZ,modelViewM,projectionM,viewPort,&msX,&msY,&msZ);
}
