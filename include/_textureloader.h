#ifndef _TEXTURELOADER_H
#define _TEXTURELOADER_H

#include<_common.h>
#include<SOIL2.h>

class _textureLoader
{
    public:
        _textureLoader();
        virtual ~_textureLoader();

        GLuint loadTexture(char *);    //To read the image file
        void bindTexture();          //To bind image to a model

        unsigned char *image;        // To handle image data
        int width,height;            // image width & height

        GLuint textID;               // Image data buffer handler


    protected:

    private:
};

#endif // _TEXTURELOADER_H
