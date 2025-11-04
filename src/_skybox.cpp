#include "_skybox.h"

_skyBox::_skyBox()
{
    //ctor
}

_skyBox::~_skyBox()
{
    //dtor
}
void _skyBox::skyBoxInit()
{
    xMin = yMin =0;
    xMax = yMax =1;

    glGenTextures(7,tex);

    rotation.x= rotation.y=rotation.z = 0;
    pos.x = pos.y =0;
    pos.z = -9.0;

    boxSize.x = boxSize.z = 30.0;
    boxSize.y = 5.0;

}

void _skyBox::drawSkyBox()
{
   glDisable(GL_LIGHTING); // if you are using room comment this line
   glPushMatrix();

   glColor3f(1.0,1.0,1.0);// set environment to white
   glScalef(boxSize.x,boxSize.y,boxSize.z);

   glRotatef(rotation.x, 1,0,0);
   glRotatef(rotation.y, 0,1,0);
   glRotatef(rotation.z, 0,0,1);

   //front Wall

   glBindTexture(GL_TEXTURE_2D,tex[0]);
   //glNormal3f()  use this only if you are setting room with lighting
   glBegin(GL_QUADS);
    glTexCoord2f(xMin,yMin); glVertex3f(-1.0,1.0,1.0);
    glTexCoord2f(xMax,yMin); glVertex3f(1.0,1.0,1.0);
    glTexCoord2f(xMax,yMax); glVertex3f(1.0,-1.0,1.0);
    glTexCoord2f(xMin,yMax); glVertex3f(-1.0,-1.0,1.0);
   glEnd();

    //Back Wall

   glBindTexture(GL_TEXTURE_2D,tex[1]);
   //glNormal3f()  use this only if you are setting room with lighting
   glBegin(GL_QUADS);
    glTexCoord2f(xMin,yMax); glVertex3f(1.0,-1.0,-1.0);
    glTexCoord2f(xMax,yMax); glVertex3f(-1.0,-1.0,-1.0);
    glTexCoord2f(xMax,yMin); glVertex3f(-1.0, 1.0,-1.0);
    glTexCoord2f(xMin,yMin); glVertex3f(1.0,1.0,-1.0);
   glEnd();

   //Top Wall

   glBindTexture(GL_TEXTURE_2D,tex[2]);
   //glNormal3f()  use this only if you are setting room with lighting
   glBegin(GL_QUADS);
    glTexCoord2f(xMin,yMax); glVertex3f(1.0,1.0,-1.0);
    glTexCoord2f(xMin,yMin); glVertex3f(1.0,1.0,1.0);
    glTexCoord2f(xMax,yMin); glVertex3f(-1.0,1.0,1.0);
    glTexCoord2f(xMax,yMax); glVertex3f(-1.0,1.0,-1.0);
   glEnd();

   //Bottom Wall

   glBindTexture(GL_TEXTURE_2D,tex[3]);
   //glNormal3f()  use this only if you are setting room with lighting
   glBegin(GL_QUADS);
    glTexCoord2f(xMax,yMax); glVertex3f(-1.0,-1.0,1.0);
    glTexCoord2f(xMin,yMax); glVertex3f(1.0,-1.0,1.0);
    glTexCoord2f(xMin,yMin); glVertex3f(1.0,-1.0,-1.0);
    glTexCoord2f(xMax,yMin); glVertex3f(-1.0,-1.0,-1.0);
   glEnd();

   //Right Wall

   glBindTexture(GL_TEXTURE_2D,tex[5]);
   //glNormal3f()  use this only if you are setting room with lighting
   glBegin(GL_QUADS);
    glTexCoord2f(xMax,yMin); glVertex3f(1.0,1.0,-1.0);
    glTexCoord2f(xMax,yMax); glVertex3f(1.0,-1.0,-1.0);
    glTexCoord2f(xMin,yMax); glVertex3f(1.0,-1.0,1.0);
    glTexCoord2f(xMin,yMin); glVertex3f(1.0,1.0,1.0);
   glEnd();

      //Left Wall

   glBindTexture(GL_TEXTURE_2D,tex[4]);
   //glNormal3f()  use this only if you are setting room with lighting
   glBegin(GL_QUADS);
    glTexCoord2f(xMin,yMin); glVertex3f(-1.0,1.0,-1.0);
    glTexCoord2f(xMax,yMin); glVertex3f(-1.0,1.0,1.0);
    glTexCoord2f(xMax,yMax); glVertex3f(-1.0,-1.0,1.0);
    glTexCoord2f(xMin,yMax); glVertex3f(-1.0,-1.0,-1.0);
   glEnd();

   glPopMatrix();

   glEnable(GL_LIGHTING);

}

void _skyBox::drawSkyBoxSingle()
{

}
