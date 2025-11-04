#include"_AnimatedModel.h"

_AnimatedModel::_AnimatedModel(){
    modelTexID=0;
}

_AnimatedModel::~_AnimatedModel(){
    FreeModel();
}

int _AnimatedModel::GetFrameCount(){
    return m_Frames.size();
}

bool _AnimatedModel::LoadAnimation(const char* baseName, int frameCount){
    for(int i =0;i<frameCount;++i){
        // build filename (models/sktaer/push_02.obj)
        std::stringstream ss;
        ss << baseName << "_";
        if(i<10) ss << "0"; // add leading 0
        ss << i << ".obj";

        std::string filename = ss.str();

        // create ObjModel & load
        ObjModel* newFrame = new ObjModel();
        newFrame->init(filename);

        if(newFrame->vertices.empty()){
            std::cerr << "Error: could not load: " << filename << '\n';
            delete newFrame;
            return false;
        }
        // add loaded frame to the flipbook
        m_Frames.push_back(newFrame);
    }
    // load tex
    // assume all frames share a texture
    // ex ("models/skater/push.jpg")
    std::string texPath = std::string(baseName)+".jpg";
    std::cout << "Loading texture: " << texPath << '\n';
    myTex.loadTexture(texPath.c_str());
    modelTexID = myTex.textID;

    return true;
}

void _AnimatedModel::FreeModel(){
    // delete all ObjModel we've created
    for(ObjModel* frame : m_Frames){
        delete frame;
    }
    // clean vec
    m_Frames.clear();

    // clean tex
    glDeleteTextures(1,&modelTexID);
}

void _AnimatedModel::RenderInterpolated(int frameA, int frameB, float interp){
    if(m_Frames.empty() || frameA >= m_Frames.size() || frameB >= m_Frames.size()){
        return;
    }

    //get two pages of the flipbook we're gonna blend
    ObjModel* modelA = m_Frames[frameA];
    ObjModel* modelB = m_Frames[frameB];

    // check that they match
    // vert count & face data has to be the same
    if(modelA->vertices.size() != modelB->vertices.size() || modelA->faces.size() != modelB->faces.size()){
        std::cerr<<"Error: Animation frames do not match\n";
        return;
    }
    // make sure to bind tex

    glBindTexture(GL_TEXTURE_2D,modelTexID);

    // draw faces but calc each vertex pos
    glBegin(GL_TRIANGLES);
    for(size_t i =0;i<modelA->faces.size();++i){
        for(size_t j=0;j<3;++j){
            // get vert indices for this corner
            int vertexIndex = modelA->faces[i][j].x;
            int texCoordIndex = modelA->faces[i][j].y;
            int normalIndex = modelA->faces[i][j].z;

            // get vert pos from both frames
            Vector3& posA = modelA->vertices[vertexIndex];
            Vector3& posB = modelB->vertices[vertexIndex];

            // lerp pos
            Vector3 finalPos;
            finalPos.x = posA.x+(posB.x-posA.x)*interp;
            finalPos.y=posA.y+(posB.y-posA.y)*interp;
            finalPos.z=posA.z+(posB.z-posA.z)*interp;

            // lerp normals for smooth lighting
            /*
            Vector3 &normA = modelA->normals[normalIndex];
            Vector3& normB = modelB->normals[normalIndex];
            Vector3 finalNorm;
            finalNorm.x = normA.x+(normB.x-normA.x)*interp;
            finalNorm.y=normA.y+(normB.y-normA.y)*interp;
            finalNorm.z = normA.z + (normB.z-normA.z)*interp;
            */
            // get tex coords (they dont change)
            Vector2& tex = modelA->tex_coords[texCoordIndex];

            // send to opengl
            //glNormal3f(finalNorm.x,finalNorm.y,finalNorm.z);
            glTexCoord2f(tex.x,tex.y);
            glVertex3f(finalPos.x,finalPos.y,finalPos.z);
        }
    }
    glEnd();
}


