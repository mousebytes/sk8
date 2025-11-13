#include"_ObjModel.h"


void ObjModel::init(std::string filename){
    this->filename = filename;
    readObj();
}

void ObjModel::readObj(){
    ifstream obj_file(filename);
    string line;
    if(!obj_file.is_open()) {cout << "\nfile unable to be opened: " << filename << '\n'; return;}
    Vector3 minCoords(numeric_limits<float>::max(), std::numeric_limits<float>::max(),numeric_limits<float>::max());
    Vector3 maxCoords(-numeric_limits<float>::max(),-numeric_limits<float>::max(),-numeric_limits<float>::max());
    while(getline(obj_file,line)){
        stringstream ss(line);
        string lineHeader;
        ss >> lineHeader;
        if(lineHeader == "v"){
            Vector3 vertex_pos;
            ss >> vertex_pos.x >> vertex_pos.y >> vertex_pos.z;
            if(vertex_pos.x < minCoords.x) minCoords.x = vertex_pos.x;
            if(vertex_pos.y < minCoords.y) minCoords.y = vertex_pos.y;
            if(vertex_pos.z < minCoords.z) minCoords.z = vertex_pos.z;

            if(vertex_pos.x > maxCoords.x) maxCoords.x = vertex_pos.x;
            if(vertex_pos.y > maxCoords.y) maxCoords.y = vertex_pos.y;
            if(vertex_pos.z > maxCoords.z) maxCoords.z = vertex_pos.z;
            vertices.push_back(vertex_pos);
        } else if(lineHeader == "vn"){
            Vector3 vertex_normal;
            ss >> vertex_normal.x >> vertex_normal.y >> vertex_normal.z;
            normals.push_back(vertex_normal);
        } else if(lineHeader == "vt"){
            Vector2 t_tex;
            ss >> t_tex.x >> t_tex.y;
            tex_coords.push_back(t_tex);
        } else if(lineHeader == "f"){
            vector<Vector3> t_faces;
            string face_piece;
            while(ss>>face_piece){
                if(face_piece.empty()) continue;
                //debug
                //cout << "\nface_piece: " << face_piece;
                string piece1, piece2,piece3;
                piece1=piece2=piece3="";
                bool changeToPiece2=false;
                bool changeToPiece3 = false;
                for(char& c : face_piece){
                    if(c == '/' && !changeToPiece2) {changeToPiece2 = true; continue;}
                    if(c== '/' && !changeToPiece3) {changeToPiece3=true; continue;}
                    //cout << "\nmade it past";
                    if(!changeToPiece2){
                        piece1+=c;
                    } else if (changeToPiece2 && !changeToPiece3){
                        piece2+=c;
                    } else{
                        piece3+=c;
                    }
                }
                // debug tools
                /*cout << "\npiece1: " << piece1;
                cout << "\npiece2: " << piece2;
                cout << "\npiece3: " << piece3;
                */
                Vector3 pieces;
                if(!piece2.empty())
                {
                    pieces.x = stoi(piece1)-1;
                    pieces.y=stoi(piece2)-1;
                    pieces.z=stoi(piece3)-1;
                }
                else
                {
                    pieces.x = stoi(piece1)-1;
                    pieces.y=-1;
                    pieces.z=stoi(piece3)-1;
                }
                t_faces.push_back(pieces);
            }
            faces.push_back(t_faces);
        }
    }
    // normalize v coords
    Vector3 center = (minCoords + maxCoords) / 2.0f;
    Vector3 size = maxCoords - minCoords;
    float maxDistance = std::max(size.x, std::max(size.y, size.z));
    float scaleFactor = 2.0f / maxDistance;
    for(auto& vertex : vertices){
        vertex = (vertex - center) * scaleFactor;
    }
}

void ObjModel::Draw(){
    glBegin(GL_TRIANGLES);
        for(auto &face : faces){
            for(auto &v3 : face){
                Vector3 &t_pos = vertices[v3.x];
                Vector3 &t_normals = normals[v3.z];
                glNormal3f(t_normals.x,t_normals.y,t_normals.z);
                /*if(v3.y!=-1)
                {
                    Vector2 &t_uv = tex_coords[v3.y];
                    glTexCoord2f(t_uv.x,t_uv.y);
                }*/
                if(v3.y!=-1){
                    Vector2 &t_uv = tex_coords[v3.y];
                    // obj files store (0,0) bottom left
                    // png stores (0,0) top left
                    glTexCoord2f(t_uv.x,1.0f-t_uv.y);
                }
                else{
                    // if no UV just use default (0,0)
                    // anything broken will be solid color (hopefully)
                    glTexCoord2f(0.0f,0.0f);
                }
                glVertex3f(t_pos.x,t_pos.y,t_pos.z);
            }
        }
    glEnd();
}
