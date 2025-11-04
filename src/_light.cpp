#include "_light.h"

_light::_light()
{
    //ctor
}

_light::~_light()
{
    //dtor
}

void _light::setLight(GLenum Light)
{
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_LIGHTING);
    glEnable(Light);

    glLightfv(Light, GL_AMBIENT, light_ambient);
    glLightfv(Light, GL_DIFFUSE, light_diffuse);
    glLightfv(Light, GL_SPECULAR,light_specular);
    glLightfv(Light, GL_POSITION,light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS,high_shininess);
}
