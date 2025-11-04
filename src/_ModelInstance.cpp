#include"_ModelInstance.h"



_ModelInstance::_ModelInstance(_AnimatedModel* modelAsset){
    blueprint = modelAsset;

    pos.x=pos.y=0;
    pos.z=-10.0;
    dirAngleZ=180;

    currentFrame=0;
    interp=0.0;
    startFrame=0;
    // set default to full animation
    endFrame=blueprint->GetFrameCount()-1;
}

_ModelInstance::~_ModelInstance(){

}

void _ModelInstance::SetAnimation(int start, int end){
    startFrame = start;
    endFrame = end;
    // reset to beginning of new animation
    currentFrame=start;
    interp=0.0f;
}

void _ModelInstance::Update(){
    Animate(startFrame,endFrame,&currentFrame,&interp);
}

void _ModelInstance::Draw(){
    glPushMatrix();
        
        glTranslatef(pos.x,pos.y,pos.z);
        //DEBUG ROT
        glRotatef(180.0f,0.0f,1.0f,0.0f);
        glRotatef(dirAngleZ,0.0,0.0,1.0);

        // calc next frame -- handle wraparound
        int nextFrame = currentFrame+1;
        if(nextFrame>endFrame){
            nextFrame=startFrame; // loop back
        }
        
        blueprint->RenderInterpolated(
            currentFrame,
            nextFrame,
            interp
        );

    glPopMatrix();
}

void _ModelInstance::Animate(int start, int end, int* frame, float* interp){
    // animation speed (fps)
    float animSpeed = 10.0f;

    // +interp based on deltaTime
    *interp+=animSpeed*_Time::deltaTime;

    // check if interp is done
    if(*interp>=1.0f){
        // reset interp
        *interp = fmod(*interp, 1.0f);

        // next frame
        (*frame)++;

        // loop anim if passed end frame
        if(*frame>end){
            *frame=start;
        }
    }
}

void _ModelInstance::Actions()
{
    // Check the trigger and set the correct animation
    switch(actionTrigger)
    {
        case STAND:
            SetAnimation(0, 2); // "stand" animation frames
            break;
        case RUN:
            SetAnimation(40, 45); // "run" animation frames
            break;
        case ATTACK:
            SetAnimation(46, 53); // "attack" animation frames
            break;
        case JUMP:
            SetAnimation(66, 71); // "jump" animation frames
            break;
        // add other cases like WALKLEFT etc
    }
}