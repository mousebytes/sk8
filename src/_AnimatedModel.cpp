#include "_AnimatedModel.h"

_AnimatedModel::_AnimatedModel() {
    modelTexID = 0;
}

_AnimatedModel::~_AnimatedModel() {
    FreeModel();
}

bool _AnimatedModel::LoadTexture(char* texpath) {
    cout << "Loading texture: " << texpath << '\n';
    myTex.loadTexture(texpath);
    modelTexID = myTex.textID;
    return modelTexID != 0;
}

// gets the frame count for a specific named animation
int _AnimatedModel::GetFrameCount(string animName) {
    // check if the animation name exists in our map
    if (m_Animations.find(animName) == m_Animations.end()) {
        return 0; // Not found
    }
    return m_Animations[animName].size();
}

// loads an animation from a sequence of OBJs and stores it by name
bool _AnimatedModel::RegisterAnimation(string name, const char* baseName, int frameCount) {
    
    // create a new vector to hold this animation's frames
    vector<ObjModel*> frames;

    for (int i = 0; i < frameCount; ++i) {
        // build filename (models/player/idle_00.obj)
        stringstream ss;
        ss << baseName << "_";
        if (i < 10) ss << "0"; // add leading 0
        ss << i << ".obj";

        string filename = ss.str();

        // create ObjModel & load
        ObjModel* newFrame = new ObjModel();
        newFrame->init(filename);

        if (newFrame->vertices.empty()) {
            cerr << "Error: could not load: " << filename << '\n';
            delete newFrame;
            // clean up already loaded frames for this animation
            for (ObjModel* frame : frames) {
                delete frame;
            }
            return false;
        }
        // add loaded frame to this animation's vector
        frames.push_back(newFrame);
    }

    // if this is the first animation being loaded,
    // initialize the scratch frame structure
    if (m_Animations.empty() && !frames.empty()) {
        ObjModel* firstFrame = frames[0];
        m_ScratchFrame.faces = firstFrame->faces;
        m_ScratchFrame.tex_coords = firstFrame->tex_coords;
        m_ScratchFrame.normals.resize(firstFrame->normals.size());
        m_ScratchFrame.vertices.resize(firstFrame->vertices.size());
    }

    // store the newly loaded frame vector in our map
    m_Animations[name] = frames;

    return true;
}

void _AnimatedModel::FreeModel() {
    for (auto& pair : m_Animations) {
        // pair.first is the string name (e.g., "idle")
        // pair.second is the vector<ObjModel*>
        for (ObjModel* frame : pair.second) {
            delete frame;
        }
        pair.second.clear();
    }
    // clean the map itself
    m_Animations.clear();

    // clean tex
    glDeleteTextures(1, &modelTexID);
}

void _AnimatedModel::Draw(string animName, int frameA, int frameB, float interp) {
    
    // find the requested animation in our map
    if (m_Animations.find(animName) == m_Animations.end()) {
        return; // this animation doesn't exist
    }

    vector<ObjModel*>& frames = m_Animations[animName];

    if (frames.empty() || frameA >= frames.size() || frameB >= frames.size()) {
        return;
    }

    //get two pages of the flipbook we're gonna blend
    ObjModel* modelA = frames[frameA];
    ObjModel* modelB = frames[frameB];

    // check that they match
    if (modelA->vertices.size() != modelB->vertices.size() ||
        modelA->normals.size() != modelB->normals.size())
    {
        cerr << "Error: Animation frames do not match\n";
        return;
    }

    // calc the final vertex positions and normals,
    // and store them in m_ScratchFrame.
    
    for (size_t i = 0; i < modelA->vertices.size(); ++i) {
        // get vert pos from both frames
        Vector3& posA = modelA->vertices[i];
        Vector3& posB = modelB->vertices[i];

        // lerp pos
        m_ScratchFrame.vertices[i].x = posA.x + (posB.x - posA.x) * interp;
        m_ScratchFrame.vertices[i].y = posA.y + (posB.y - posA.y) * interp;
        m_ScratchFrame.vertices[i].z = posA.z + (posB.z - posA.z) * interp;
    }
    
    // also lerp normals for smooth lighting
    for (size_t i = 0; i < modelA->normals.size(); ++i) {
        Vector3 &normA = modelA->normals[i];
        Vector3& normB = modelB->normals[i];

        m_ScratchFrame.normals[i].x = normA.x + (normB.x - normA.x) * interp;
        m_ScratchFrame.normals[i].y = normA.y + (normB.y - normA.y) * interp;
        m_ScratchFrame.normals[i].z = normA.z + (normB.z - normA.z) * interp;
    }


    // make sure to bind tex
    glBindTexture(GL_TEXTURE_2D, modelTexID);

    // m_ScratchFrame contains the final interpolated geometry
    // (and its .faces and .tex_coords were already copied)
    m_ScratchFrame.Draw();
}