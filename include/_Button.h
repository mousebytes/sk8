#ifndef _MYBUTTON_H
#define _MYBUTTON_H

#include"_common.h"
#include"_textureloader.h"

class _Button{
public:
    _Button();
    ~_Button();

    void Init(char* filename, int width, int height, int posX, int posY, int posZ, int framesX, int framesY);
    void Draw();
    bool isClicked(int mouseX, int mouseY);
    Vector3 GetPos();
private:
    _textureLoader *m_tex;
    char* filename;
    int width, height;
    float xMin,yMin,xMax,yMax;
    Vector2 frames;
    Vector3 pos;
    Vector3 vertices[4];

protected:

};


#endif // _MYBUTTON_H
