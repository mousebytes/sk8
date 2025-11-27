#ifndef _FONTS_H
#define _FONTS_H

#include "_common.h"
#include "_textureloader.h"

class _fonts
{
    public:
        _fonts();
        virtual ~_fonts();

        void initFonts(char* fileName, int fx, int fy); // filename, cols, rows
        void drawText(string text);
        
        void setPosition(float x, float y, float z);
        void setSize(float sx, float sy);

        _textureLoader *tex;

    protected:

    private:
        int framesX;
        int framesY;

        float xMin, yMin, xMax, yMax; // current char UVs

        Vector3 pos;
        Vector2 scale;
        Vector3 vert[4]; // Quad vertices

        // Helper to map ASCII char to grid position
        bool getFontFrame(char c, int& fx, int& fy);
};

#endif // _FONTS_H