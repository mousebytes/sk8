#include"_StaticModel.h"

_StaticModel::_StaticModel(){
    modelTexID=0;
}

_StaticModel::~_StaticModel(){
    FreeModel();
}

void _StaticModel::FreeModel(){
    // free tex
    glDeleteTextures(1,&modelTexID);
}

bool _StaticModel::LoadModel(const char* filename, char* texpath){
    myModel.init(filename);

    // load tex for the model
    // must be a jpg of same name as model (models/cow.obj) -> (images/cow.jpg)
    //std::string s_filename = filename;
    //std::string texPath = "images/" + s_filename.substr(s_filename.find_last_of("/\\")+1,s_filename.find_last_of(".")-s_filename.find_last_of("/\\")-1)+".jpg";

    std::string s_filename = filename;
    //std::string texPath = s_filename.substr(0, s_filename.find_last_of(".")) + ".jpg";
    char* s_texPath = texpath;

    // DEBUG
    //std::cout << "Loading texture: " << s_texPath << '\n';
    myTex.loadTexture(s_texPath);
    modelTexID=myTex.textID;

    return true;
}

void _StaticModel::Draw(){
    glBindTexture(GL_TEXTURE_2D,modelTexID);
    myModel.Draw();
}

