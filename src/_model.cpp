#include "_model.h"

_model::_model()
{
    //ctor
        rotateX =0;
        rotateY =0;
        rotateZ =0;

        posX =0;
        posY = 0;
        posZ =-8.0;

        p.x = posX;
        p.y = posY;
        p.z = posZ;

        scale =1;
}

_model::~_model()
{
    //dtor
}
void _model::drawModel()
{
    glPushMatrix();          // start group
    glColor3f(1,1,1);    //set colors

        p.x = posX;
        p.y = posY;
        p.z = posZ;

    glTranslated(posX,posY,posZ); //translation
    glRotated(rotateX,1,0,0);     //rotate around X-Axis
    glRotated(rotateY,0,1,0);     //rotate around Y-Axis
    glRotated(rotateZ,0,0,1);     //rotate around Z-Axis

    glScaled(scale,scale,scale);  // Scale your model
   // glutSolidTorus(0.5,1.5,40,40); //Draw model

    glutSolidTeapot(1.5);
    glPopMatrix();                 //End group
}
