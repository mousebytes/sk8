#include "_bullets.h"

_bullets::_bullets()
{
    //ctor
    pos.x=pos.y=pos.z=0;
    t=0;
    des.x=0;
    des.y=20;
    des.z= -50;
    src.x=src.y=src.z=0;

    isLive=false;
}

_bullets::~_bullets()
{
    //dtor
}

void _bullets::iniBullet(char*)
{

}

void _bullets::drawBullet()
{
    // only if you are using sphere
    glDisable(GL_TEXTURE_2D);

    glPushMatrix();
        if(isLive)
        {
            glTranslatef(pos.x,pos.y,pos.z);
            glutSolidSphere(0.5,20,20);
        }
    glPopMatrix();
    glEnable(GL_TEXTURE_2D); // only if you are using glut sphere
}

void _bullets::bulletActions()
{

}
