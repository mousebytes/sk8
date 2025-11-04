#include<_MD2Model.h>

vec3_t _MD2Model::anorms_table[162]={
    #include "Anorms.h"
};

_MD2Model::_MD2Model(){
    md2file.skins = NULL;
    md2file.texcoords = NULL;
    md2file.triangles = NULL;
    md2file.frames = NULL;
    md2file.glcmds = NULL;
    numFrames = 0;
}

_MD2Model::~_MD2Model(){
    delete myTex;
    FreeModel();
}

bool _MD2Model::LoadModel(const char* filename){
    if(!ReadMD2Model(filename)){
        return false;
    }
    numFrames=md2file.header.num_frames;
    return true;
}

int _MD2Model::ReadMD2Model(const char* filename){
    FILE *fp;

    fp=fopen(filename,"rb");
    if(!fp){
        fprintf(stderr,"Error: couldn't open \"%s\"!\n", filename);
        return 0;
    }

    // read header
    fread(&md2file.header, 1, sizeof(struct md2_header_t), fp);

    if ((md2file.header.ident != 844121161) || (md2file.header.version != 8))
    {
      fprintf(stderr, "Error: bad version or identifier\n");
      fclose(fp);
      return 0;
    }

    // Memory allocations
    md2file.skins = (struct md2_skin_t *)malloc(sizeof(struct md2_skin_t) * md2file.header.num_skins);
    md2file.texcoords = (struct md2_texCoord_t *)malloc(sizeof(struct md2_texCoord_t) * md2file.header.num_st);
    md2file.triangles = (struct md2_triangle_t *)malloc(sizeof(struct md2_triangle_t) * md2file.header.num_tris);
    md2file.frames = (struct md2_frame_t *)malloc(sizeof(struct md2_frame_t) * md2file.header.num_frames);
    md2file.glcmds = (int *)malloc(sizeof(int) * md2file.header.num_glcmds);

    // Read model data
    fseek(fp, md2file.header.offset_skins, SEEK_SET);
    fread(md2file.skins, sizeof(struct md2_skin_t), md2file.header.num_skins, fp);

    fseek(fp, md2file.header.offset_st, SEEK_SET);
    fread(md2file.texcoords, sizeof(struct md2_texCoord_t), md2file.header.num_st, fp);

    fseek(fp, md2file.header.offset_tris, SEEK_SET);
    fread(md2file.triangles, sizeof(struct md2_triangle_t), md2file.header.num_tris, fp);

    fseek(fp, md2file.header.offset_glcmds, SEEK_SET);
    fread(md2file.glcmds, sizeof(int), md2file.header.num_glcmds, fp);

    // Read frames
    fseek(fp, md2file.header.offset_frames, SEEK_SET);
    for (int i = 0; i < md2file.header.num_frames; ++i)
    {
      // Memory allocation for vertices of this frame
      md2file.frames[i].verts = (struct md2_vertex_t *)malloc(sizeof(struct md2_vertex_t) * md2file.header.num_vertices);

      // Read frame data
      fread(md2file.frames[i].scale, sizeof(vec3_t), 1, fp);
      fread(md2file.frames[i].translate, sizeof(vec3_t), 1, fp);
      fread(md2file.frames[i].name, sizeof(char), 16, fp);
      fread(md2file.frames[i].verts, sizeof(struct md2_vertex_t), md2file.header.num_vertices, fp);
    }

    // --- Load Texture ---
    if (md2file.header.num_skins > 0)
    {
        // This logic assumes the texture is in a "models/Tekk/"-like folder
        
        // First, find the path of the model file
        std::string modelPath = filename;
        size_t lastSlash = modelPath.find_last_of("/\\");
        if (lastSlash != std::string::npos)
        {
            modelPath = modelPath.substr(0, lastSlash + 1); // Get "models/Tekk/"
        }
        else
        {
            modelPath = ""; // Model is in the same folder
        }

        // Get the skin filename from the model data
        std::string skinFilename = md2file.skins[0].name;
        // The skin name might have its own path, so find its filename
        size_t lastSkinSlash = skinFilename.find_last_of("/\\");
        if (lastSkinSlash != std::string::npos)
        {
            skinFilename = skinFilename.substr(lastSkinSlash + 1);
        }

        //DEBUG testing this
        size_t dotPos = skinFilename.find_last_of(".");
        if (dotPos != std::string::npos) {
            // Replaces ".pcx" with ".jpg"
            skinFilename = skinFilename.substr(0, dotPos) + ".jpg"; 
        }

        // Combine the model's path with the skin's filename
        std::string texturePath = modelPath + skinFilename;
        
        cout << "Loading texture: " << texturePath << endl;
        
        // Use the member object
        myTex->loadTexture(texturePath.c_str());
        md2file.tex_id = myTex->textID;
    }

    fclose(fp);
    return 1;
}

void _MD2Model::RenderFrameItpWithGLCmds(int n, float interp, int startFrame, int endFrame)
{
   // Check if n is in a valid range
   if ((n < 0) || (n > md2file.header.num_frames - 1))
    return;

   // Enable model's texture
   glBindTexture(GL_TEXTURE_2D, md2file.tex_id);

   // Calculate the next frame, handling wrap-around for smooth animation
   int n_next = n + 1;
   if (n_next > endFrame)
   {
       n_next = startFrame;
   }

   // Temporary variables
   int i, *pglcmds;
   float s, t;
   vec3_t v_curr, v_next, v, norm;
   float *p_curr_norm, *p_next_norm;
   struct md2_frame_t *pframe1, *pframe2;
   struct md2_vertex_t *pvert1, *pvert2;
   struct md2_glcmd_t *packet;

   pglcmds = md2file.glcmds;

   // Draw the model
   while ((i = *(pglcmds++)) != 0)
   {
      if (i < 0)
      {
	    glBegin(GL_TRIANGLE_FAN);
	    i = -i;
      }
      else
      {
	    glBegin(GL_TRIANGLE_STRIP);
      }

      // Draw each vertex of this group
      for (/* Nothing */; i > 0; --i, pglcmds += 3)
      {
	    packet = (struct md2_glcmd_t *)pglcmds;
	    pframe1 = &md2file.frames[n];      // Current frame
	    pframe2 = &md2file.frames[n_next]; // Next frame (handles loop)
	    pvert1 = &pframe1->verts[packet->index];
	    pvert2 = &pframe2->verts[packet->index];

	    // Pass texture coordinates to OpenGL
	    glTexCoord2f(packet->s, packet->t);

	    // Interpolate normals
	    p_curr_norm = anorms_table[pvert1->normalIndex];
        p_next_norm = anorms_table[pvert2->normalIndex];

	    norm[0] = p_curr_norm[0] + interp * (p_next_norm[0] - p_curr_norm[0]);
        norm[1] = p_curr_norm[1] + interp * (p_next_norm[1] - p_curr_norm[1]);
        norm[2] = p_curr_norm[2] + interp * (p_next_norm[2] - p_curr_norm[2]);

	    glNormal3fv(norm);

	    // Interpolate vertices
	    v_curr[0] = pframe1->scale[0] * pvert1->v[0] + pframe1->translate[0];
	    v_curr[1] = pframe1->scale[1] * pvert1->v[1] + pframe1->translate[1];
	    v_curr[2] = pframe1->scale[2] * pvert1->v[2] + pframe1->translate[2];

	    v_next[0] = pframe2->scale[0] * pvert2->v[0] + pframe2->translate[0];
	    v_next[1] = pframe2->scale[1] * pvert2->v[1] + pframe2->translate[1];
	    v_next[2] = pframe2->scale[2] * pvert2->v[2] + pframe2->translate[2];

	    v[0] = v_curr[0] + interp * (v_next[0] - v_curr[0]);
	    v[1] = v_curr[1] + interp * (v_next[1] - v_curr[1]);
	    v[2] = v_curr[2] + interp * (v_next[2] - v_curr[2]);

	    glVertex3fv(v);
      }
      glEnd();
    }
}

void _MD2Model::FreeModel()
{
    // Check if data was ever allocated before freeing
    if (md2file.skins)
    {
      free(md2file.skins);
      md2file.skins = NULL;
    }

    if (md2file.texcoords)
    {
      free(md2file.texcoords);
      md2file.texcoords = NULL;
    }

    if (md2file.triangles)
    {
      free(md2file.triangles);
      md2file.triangles = NULL;
    }

    if (md2file.glcmds)
    {
      free(md2file.glcmds);
      md2file.glcmds = NULL;
    }

    if (md2file.frames)
    {
      for (int i = 0; i < md2file.header.num_frames; ++i)
      {
        if (md2file.frames[i].verts)
        {
	        free(md2file.frames[i].verts);
	        md2file.frames[i].verts = NULL;
        }
      }

      free(md2file.frames);
      md2file.frames = NULL;
    }
}