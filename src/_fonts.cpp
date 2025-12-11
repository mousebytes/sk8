#include "_fonts.h"

_fonts::_fonts()
{
    tex = new _textureLoader();
    framesX = 1;
    framesY = 1;
    pos = Vector3(0, 0, -2);
    scale = Vector2(0.2f, 0.2f);

    // Standard Quad centered at 0,0
    vert[0] = Vector3(-1.0, -1.0, 0.0);
    vert[1] = Vector3( 1.0, -1.0, 0.0);
    vert[2] = Vector3( 1.0,  1.0, 0.0);
    vert[3] = Vector3(-1.0,  1.0, 0.0);
}

_fonts::~_fonts()
{
    if(tex) delete tex;
}

void _fonts::initFonts(char* fileName, int fx, int fy)
{
    framesX = fx;
    framesY = fy;
    tex->loadTexture(fileName);
}

void _fonts::setPosition(float x, float y, float z)
{
    pos.x = x;
    pos.y = y;
    pos.z = z;
}

void _fonts::setSize(float sx, float sy)
{
    scale.x = sx;
    scale.y = sy;
}

void _fonts::drawText(string text)
{
    tex->bindTexture();
    glColor3f(1.0f, 1.0f, 1.0f); // Default white

    float charWidthUV = 1.0f / (float)framesX;
    float charHeightUV = 1.0f / (float)framesY;
    
    // Spacing between characters (visual gap)
    float spacing = scale.x * 1.5f; 

    for (size_t i = 0; i < text.length(); i++)
    {
        char c = text[i];
        
        int fx = 0, fy = 0;
        if (!getFontFrame(c, fx, fy)) continue; 

        // Calculate UVs for this character
        float uMin = fx * charWidthUV;
        float uMax = uMin + charWidthUV;
        
        // --- FIX FOR UPSIDE DOWN TEXT ---
        // In OpenGL, V=0 is usually bottom, V=1 is top.
        // But typical sprite sheets have row 0 at the top.
        // We calculate standard top-down UVs:
        float vTop = 1.0f - (fy * charHeightUV);       // The visual "top" of the char
        float vBottom = vTop - charHeightUV;           // The visual "bottom" of the char

        glPushMatrix();
            // Move to start pos + offset for this specific character
            glTranslatef(pos.x + (i * spacing), pos.y, pos.z);
            glScalef(scale.x, scale.y, 1.0f);

            glBegin(GL_QUADS);
                // We map the "bottom" UV to the bottom vertex, etc.
                // BUT since your image is appearing upside down, it means your texture loader
                // or previous logic was inverted.
                // The fix is to SWAP the V coordinates applied to the Top vs Bottom vertices.

                // Bottom Left Vertex -> Map to vTop (because texture is currently inverted)
                glTexCoord2f(uMin, vTop); 
                glVertex3f(vert[0].x, vert[0].y, vert[0].z);

                // Bottom Right Vertex -> Map to vTop
                glTexCoord2f(uMax, vTop); 
                glVertex3f(vert[1].x, vert[1].y, vert[1].z);

                // Top Right Vertex -> Map to vBottom
                glTexCoord2f(uMax, vBottom); 
                glVertex3f(vert[2].x, vert[2].y, vert[2].z);

                // Top Left Vertex -> Map to vBottom
                glTexCoord2f(uMin, vBottom); 
                glVertex3f(vert[3].x, vert[3].y, vert[3].z);
            glEnd();

        glPopMatrix();
    }
}

bool _fonts::getFontFrame(char c, int& fx, int& fy)
{
    // Convert generic text to spritesheet grid coordinates
    // Based on standard 15x8 grid logic found in your previous uploads
    
    //Handle Uppercase conversion
    if (c >= 'a' && c <= 'z') c -= 32; 

    //Numbers '0' - '9'
    if(c >= '0' && c <= '9') {
        fx = 1 + (c - '0');
        fy = 6;
    }
    //Letters 'A' - 'L'
    else if(c >= 'A' && c <= 'L') {
        fx = 3 + (c - 'A');
        fy = 5;
    }
    //Letters 'M' - 'Z'
    else if(c >= 'M' && c <= 'Z') {
        fx = (c - 'M');
        fy = 4;
    }
    //Special Chars
    else if(c == ' ') {
        fx = 0; fy = 7; // Blank space
    }
    else if(c == '+') {
        fx = 11; fy = 7;
    }
    else if(c == ':') {
        fx = 11; fy = 6;
    }
    else if(c == '-') {
        fx = 12; fy = 7; // Assuming '-' is here
    }
    else {
        // Unknown char, default to space or question mark
        return false;
    }

    // Safety Clamp
    if (fx < 0 || fy < 0 || fx >= framesX || fy >= framesY) return false;

    return true;
}