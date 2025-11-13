#include "_inputs.h"

_inputs::_inputs()
{
    //ctor
    isRotationActive = false;
    isTranslateActive = false;
}

_inputs::~_inputs()
{
    //dtor
}
void _inputs::keyPressed(_model* mdl)
{
     switch(wParam)
     {
     case VK_LEFT:
        mdl->rotateY +=1.0;
        break;
     case VK_RIGHT:
         mdl->rotateY -=1.0;
        break;

     case VK_UP:
        mdl->rotateX +=1.0;
        break;
     case VK_DOWN:
        mdl->rotateX -=1.0;
        break;

       case VK_ADD:
        mdl->rotateZ +=1.0;
        break;
     case VK_SUBTRACT:
         mdl->rotateZ -=1.0;
        break;
     }
}

void _inputs::keyPressed(_parallax* prlx)
{

     switch(wParam)
     {
     case VK_LEFT:

        break;
     case VK_RIGHT:
          prlx->prlxScrollAuto("left", 0.005);
        break;

     case VK_UP:

        break;
     case VK_DOWN:

        break;

       case VK_ADD:

        break;
     case VK_SUBTRACT:
        break;
     }
}

void _inputs::keyPressed(_3DModelLoader* Ply, _3DModelLoader* W)
{
      switch(wParam)
     {
     case VK_LEFT:
           W->actionTrigger = Ply->actionTrigger = Ply->RUN;
        break;
     case VK_RIGHT:
           W->actionTrigger = Ply->actionTrigger = Ply->ATTACK;
        break;

     case VK_UP:
           W->actionTrigger = Ply->actionTrigger = Ply->JUMP;
        break;
     case VK_DOWN:
          W->actionTrigger = Ply->actionTrigger = Ply->PAIN;
        break;

     default:
        W->actionTrigger = Ply->actionTrigger = Ply->STAND;
        break;
   }

}

void _inputs::keyPressed(_skyBox* sky)
{

     switch(wParam)
     {
     case VK_LEFT:
           sky->rotation.y -=1.0;
        break;
     case VK_RIGHT:
            sky->rotation.y +=1.0;
        break;

     case VK_UP:
        sky->rotation.x +=1.0;
        break;
     case VK_DOWN:
         sky->rotation.x -=1.0;
        break;

       case VK_ADD:

        break;
     case VK_SUBTRACT:

        break;
     }
}

void _inputs::keyPressed(_sprite* mySprite)
{
     switch(wParam)
     {
     case VK_LEFT:
mySprite->actionTrigger = mySprite->WALKLEFT;
        break;
     case VK_RIGHT:
            mySprite->actionTrigger = mySprite->WALKRIGHT;
        break;

     case VK_UP:

        break;
     case VK_DOWN:

        break;

       case VK_ADD:

        break;
     case VK_SUBTRACT:

        break;
     }
}

void _inputs::keyPressed(_camera* cm)
{
   // cout<< wParam << endl;
     switch(wParam)
     {
     case 'A':
     case 'a':
           cm->camMoveLtRt(-1);
        break;
     case 'D':
     case 'd':
            cm->camMoveLtRt(1);
        break;

     case 'W':
     case 'w':
            cm->camMoveFdBd(1);
        break;
     case 'S':
     case 's':
            cm->camMoveFdBd(-1);
        break;

     case 73:    //i
            cm->rotAngle.y +=1.0;
            cm->rotateXY();
        break;

     case 74: //j
            cm->rotAngle.x -=1.0;
              cm->rotateXY();
        break;

      case 75:    //k
             cm->rotAngle.y -=1.0;
               cm->rotateXY();
        break;

     case 76: //l
           cm->rotAngle.x +=1.0;
             cm->rotateXY();
        break;
     case 32:   // space bar
            //cm->camReset();
            cm->isFreeCam = !cm->isFreeCam;
        break;
     }
}

//DEBUG remove this function later
void _inputs::keyPressed(_StaticModelInstance *staticModel){
   switch(wParam){
      case 'W':
         staticModel->Push(0,0,-20);
         break;
      case 'S':
         staticModel->Push(0,0,20);
         break;
      case 'A':
         staticModel->Rotate(0,-45,0);
         break;
      case 'D':
         staticModel->Rotate(0,45,0);
      default:
         break;
   }
}




void _inputs::keyUp(_sprite* mySprite)
{
    mySprite->actionTrigger = mySprite->STAND;
}

void _inputs::keyUp()
{
    switch(wParam)
    {
        default: break;
    }
}

void _inputs::mouseEventDown(_model* mdl, double x, double y)
{
    prev_MouseX =x;
    prev_MouseY =y;

    switch(wParam)
    {
    case MK_LBUTTON:
         isRotationActive =true;  //activate rotation flag
        break;

    case MK_RBUTTON:
        isTranslateActive =true;  //activate translation flag
        break;

    case MK_MBUTTON:
        break;
    }
}

void _inputs::mouseEventUp()
{
    isRotationActive = false;   // deactivate the flags
    isTranslateActive= false;
}

void _inputs::mouseWheel(_model* mdl, double delta)
{
    mdl->posZ +=delta/100.0;    // zoom the model when wheel in action
}

void _inputs::mouseMove(_model* mdl, double x, double y)
{
     if(isRotationActive)                   // if rotation
     {
         mdl->rotateY +=(x-prev_MouseX)/3.0; // around y axis
         mdl->rotateX +=(y-prev_MouseY)/3.0; // around x axis
     }
     if(isTranslateActive)                   //if Translate
     {
         mdl->posX +=(x-prev_MouseX)/100.0;  // change x pos
         mdl->posY -=(y-prev_MouseY)/100.0;  // change y pos
     }

    prev_MouseX = x;                         // reset mouse x
    prev_MouseY = y;                         // reset mouse y
}

