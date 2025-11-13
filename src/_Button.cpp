#include"_Button.h"

_Button::_Button(){
    m_tex = new _textureLoader();
}

_Button::~_Button(){
    delete m_tex;
}

void _Button::Init(char* filename, int width, int height, int posX, int posY, int posZ, int framesX, int framesY){
    this->width = width;
    this->height = height;
    this->filename = filename;

    frames.x = framesX;
    frames.y = framesY;

    pos.x = posX;
    pos.y = posY;
    pos.z = posZ;

    xMin = 0.0f;
    yMin = 0.0f;
    xMax = 1.0f;
    yMax = 1.0f;

    vertices[0].x = -0.5; vertices[0].y = -0.5; vertices[0].z=1;
    vertices[1].x =  0.5; vertices[1].y = -0.5; vertices[1].z=1;
    vertices[2].x =  0.5; vertices[2].y =  0.5; vertices[2].z=1;
    vertices[3].x = -0.5; vertices[3].y =  0.5; vertices[3].z=1;

    m_tex->loadTexture(filename);
}

void _Button::Draw(){
    glPushMatrix();
     glTranslatef(pos.x,pos.y,pos.z);

     if (width > 0 && height > 0) {
        glScalef(width, height, 1.0);
     }

     m_tex->bindTexture();

     glBegin(GL_QUADS);
       // bottom left v
       glTexCoord2f(xMin, yMin); 
       glVertex2f(vertices[0].x,vertices[0].y);

       // bottom right v
       glTexCoord2f(xMax, yMin); 
       glVertex2f(vertices[1].x,vertices[1].y);

       // top right v
       glTexCoord2f(xMax, yMax); 
       glVertex2f(vertices[2].x,vertices[2].y);

       // top left v
       glTexCoord2f(xMin, yMax); 
       glVertex2f(vertices[3].x,vertices[3].y);

     glEnd();

    glPopMatrix();
}

bool _Button::isClicked(int mouseX, int mouseY) {
    // get the button's screen boundaries
    float left = pos.x - (width / 2.0f);
    float right = pos.x + (width / 2.0f);
    float top = pos.y - (height / 2.0f);
    float bottom = pos.y + (height / 2.0f);

    // check if the mouse click is inside the button
    if (mouseX >= left && mouseX <= right && mouseY >= top && mouseY <= bottom) {
        return true;
    }
    return false;
}

Vector3 _Button::GetPos(){
    return pos;
}