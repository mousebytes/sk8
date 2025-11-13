#include"_AnimatedModelInstance.h"



_AnimatedModelInstance::_AnimatedModelInstance(_AnimatedModel* modelAsset){
    blueprint = modelAsset;

    pos.x=pos.y=0;
    pos.z=-10.0;
    rotation = Vector3();
    scale.x=scale.y=scale.z=1.0f;

    m_currentFrame=0;
    m_interp=0.0f;
    m_animationSpeed=1.0f;
    // set default animation to play
    PlayAnimation("idle",1.0f);

    // start with no velocity
    velocity = Vector3(0,0,0);
    // start in the air
    isGrounded=false;

    // change to 0 for no gravity (duh)
    gravity = 9.8f;
    isHit=false;
}

_AnimatedModelInstance::~_AnimatedModelInstance(){
    for(_Collider* collider : colliders){
        delete collider;
    }
    colliders.clear();
}
// play animation by name
void _AnimatedModelInstance::PlayAnimation(string name, float speed){
    // don't restart animation if it's currently playing
    if(m_currentAnimationName == name){
        return;
    }

    // check if the animation exists before trying to play it
    if(blueprint->GetFrameCount(name)>0){
        m_currentAnimationName=name;
        m_animationSpeed=speed;
        //reset to beginning of new animation
        m_currentFrame=0;
        m_interp=0.0f;
    }else{
        // if name doesn't exists, try to play idle
        if(m_currentAnimationName!="idle" && blueprint->GetFrameCount("idle")>0){
            m_currentAnimationName="idle";
            m_animationSpeed=1.0f;
            m_currentFrame=0;
            m_interp=0.0f;
        }
    }
}

void _AnimatedModelInstance::Update(){

    // FIZZICS
    if(!isGrounded){
        velocity.y-=gravity*_Time::deltaTime;
    }

    pos.x+=velocity.x*_Time::deltaTime;
    pos.y+=velocity.y*_Time::deltaTime;
    pos.z+=velocity.z*_Time::deltaTime;

    // animation
    int frameCount = blueprint->GetFrameCount(m_currentAnimationName);
    if(frameCount==0){
        return; // no animation to play
    }

    m_interp += m_animationSpeed*_Time::deltaTime;

    if(m_interp>=1.0f){
        m_interp = fmod(m_interp,1.0f); // wrap interp
        m_currentFrame++;//next frame
        
        if(m_currentFrame >= frameCount){
            m_currentFrame=0; //loop animation
        }
    }
}

void _AnimatedModelInstance::Draw(){
    glPushMatrix();
        glTranslatef(pos.x,pos.y,pos.z);
        //DEBUG ROT
        glRotatef(rotation.x,1.0f,0.0f,0.0f);
        glRotatef(rotation.y,0.0f,1.0f,0.0f);
        glRotatef(rotation.z,0.0f,0.0f,1.0f);

        glScalef(scale.x,scale.y,scale.z);

        
        int frameCount = blueprint->GetFrameCount(m_currentAnimationName);
        if(frameCount>0){
            // calc next frame -- handle wraparound
            int nextFrame = m_currentFrame + 1;
            if(nextFrame >= frameCount){
                nextFrame=0; //loop back
            }

            blueprint->Draw(
                m_currentAnimationName,
                m_currentFrame,
                nextFrame,
                m_interp
            );
        }

        DrawColliders();
    glPopMatrix();
}

void _AnimatedModelInstance::AddCollider(_Collider* collider) {
    colliders.push_back(collider);
}

void _AnimatedModelInstance::DrawColliders() {
    if (!isDebug) return;

    // this is called inside Draw(), so the matrix
    // is already transformed by the instance's pos/rot
    for (_Collider* collider : colliders) {
        collider->Draw();
    }
}

bool _AnimatedModelInstance::CheckCollision(_StaticModelInstance* other) {
    // checks all of our colliders against all of the other's colliders

    for (_Collider* myModelCol : this->colliders) {
        // create a temporary world space collider for ourselves
        _Collider* myWorldCol = myModelCol->GetWorldSpaceCollider(this->pos, Vector3(1,1,1), this->rotation);

        for (_Collider* otherModelCol : other->colliders) {
            // create a temporary world space collider for them
            _Collider* otherWorldCol = otherModelCol->GetWorldSpaceCollider(other->pos, other->scale, other->rotation);

            // perform the collision check
            if (myWorldCol->CheckCollision(otherWorldCol)) {
                // delete temporary colliders
                delete myWorldCol;
                delete otherWorldCol;
                return true; // found a collision
            }

            delete otherWorldCol; // clean up inner
        }

        delete myWorldCol; // clean up outer
    }

    return false; // no collisions found
}