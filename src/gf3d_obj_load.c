#include <stdio.h>

#include "simple_logger.h"

#include "gf3d_obj_load.h"

#define collisions_max 8

int gf3d_obj_edge_test(ObjData* obj, GFC_Matrix4 offset, GFC_Edge3D e, GFC_Vector3D* contact)
{
    int i;
    GFC_Vector4D out;
    GFC_Triangle3D t;
    if ((!obj)||(!obj->outFace))return 0;
    for (i = 0;i < obj->face_count;i++)
    {
        t.a = obj->faceVertices[obj->outFace[i].verts[0]].vertex;
        t.b = obj->faceVertices[obj->outFace[i].verts[1]].vertex;
        t.c = obj->faceVertices[obj->outFace[i].verts[2]].vertex;
        //apply offset
        gfc_matrix4_multiply_v(&out,offset,gfc_vector3dw(t.a,0));
        t.a = gfc_vector4dxyz(out);
        gfc_matrix4_multiply_v(&out,offset,gfc_vector3dw(t.b,0));
        t.b = gfc_vector4dxyz(out);
        gfc_matrix4_multiply_v(&out,offset,gfc_vector3dw(t.c,0));
        t.c = gfc_vector4dxyz(out);
        if (gfc_trigfc_angle_edge_test(e,t,contact))return 1;
    }
    return 0;
}

/*void applymatrix(GFC_Matrix4 matrix, GFC_Vector3D* vec) {
    GFC_Matrix4 vecmatrix;
    gfc_matrix4_from_vectors(
        vecmatrix,
        *vec,
        gfc_vector3d(0, 0, 0),
        gfc_vector3d(1, 1, 1));
    gfc_matrix4_multiply(
        vecmatrix,
        vecmatrix,
        matrix);
    gfc_matrix4_to_vectors(
        vecmatrix,
        vec,
        NULL,
        NULL);
}*/

Uint8 gf3d_obj_sphere_test(ObjData* obj, GFC_Matrix4 offset, GFC_Sphere s, GFC_Vector4D* vlist, int *vlistc)
{
    int i, j;
    Uint8 f = 0; //return flag
    GFC_Vector3D contact;
    GFC_Vector4D out;
    GFC_Triangle3D t;
    GFC_Edge3D e1, e2;
    GFC_Vector3D ab, //vector from a->b, 
        ac, //vector from a->c, 
        abxac; //cross product between ab and ac
    float mult;

    //GFC_Vector3D offtran, offrot, offscale;
    //gfc_matrix4_to_vectors(offset, &offtran, &offrot, &offscale);

    //slog("Offtra: x: %f, y: %f, z: %f", offtran.x, offtran.y, offtran.z);
    //slog("Offrot: x: %f, y: %f, z: %f", offrot.x, offrot.y, offrot.z);
    //slog("Offsca: x: %f, y: %f, z: %f", offscale.x, offscale.y, offscale.z);
    if ((!obj) || (!obj->outFace))return 0;
    if (!vlist) return 0;
    for (i = 0; i < obj->face_count; i++)
    {
        t.a = obj->faceVertices[obj->outFace[i].verts[0]].vertex;
        t.b = obj->faceVertices[obj->outFace[i].verts[1]].vertex;
        t.c = obj->faceVertices[obj->outFace[i].verts[2]].vertex;
        
        //apply offset
        if (i == 0) {
        //    slog("ta bef: x: %f, y: %f, z: %f", t.a.x, t.a.y, t.a.z);
        }
        //int ii, jj;
        //for (ii = 0; ii < 4; ii++) {
        //    for (jj = 0; jj < 4; jj++) {
                //slog("mat[%i][%i]= %f", ii, jj, offset[ii][jj]);
        //    }
        //}
        
        //gfc_matrix4_multiply_v(&out, offset, gfc_vector3dw(t.a, 0));
        //t.a = gfc_vector4dxyz(out);
        //gfc_matrix4_multiply_v(&out, offset, gfc_vector3dw(t.b, 0));
        //t.b = gfc_vector4dxyz(out);
        //gfc_matrix4_multiply_v(&out, offset, gfc_vector3dw(t.c, 0));
        //t.c = gfc_vector4dxyz(out);
        apply_matrix(offset, &t.a);
        apply_matrix(offset, &t.b);
        apply_matrix(offset, &t.c);

        if (i == 0)
        {
        //    slog("ta aft: x: %f, y: %f, z: %f", t.a.x, t.a.y, t.a.z);
        }
            

        //calculate vector perpendicular to triangle
        
        
        gfc_vector3d_sub(ab, t.b, t.a);
        gfc_vector3d_sub(ac, t.c, t.a);
        gfc_vector3d_cross_product(&abxac, ab, ac);

        //gfc_matrix4_multiply_v(&out, offset, gfc_vector3dw(abxac, 0));
        //abxac = gfc_vector4dxyz(out);

        //scale vector to the radius of the sphere
        mult = s.r / gfc_vector3d_magnitude(abxac);
        gfc_vector3d_scale(abxac, abxac, mult);

        //slog("Offset: x: %f, y: %f, z: %f", abxac.x, abxac.y, abxac.z);
        //gfc_matrix4_multiply_v(&out, offset, gfc_vector4d(s.x,s.y,s.z, 0));
        //s.x = out.x;
        //s.y = out.y;
        //s.z = out.z;


        //create edges based on the vector and the sphere's position
        gfc_vector3d_add(e1.a, s, abxac);
        gfc_vector3d_sub(e2.a, s, abxac);

        gfc_vector3d_copy(e1.b, s);
        gfc_vector3d_copy(e2.b, s);

        /*gfc_matrix4_multiply_v(&out, offset, gfc_vector3dw(e1.a, 0));
        e1.a = gfc_vector4dxyz(out);
        gfc_matrix4_multiply_v(&out, offset, gfc_vector3dw(e1.b, 0));
        e1.b = gfc_vector4dxyz(out);

        gfc_matrix4_multiply_v(&out, offset, gfc_vector3dw(e2.a, 0));
        e2.a = gfc_vector4dxyz(out);
        gfc_matrix4_multiply_v(&out, offset, gfc_vector3dw(e2.b, 0));
        e2.b = gfc_vector4dxyz(out);*/


        if (gfc_trigfc_angle_edge_test(e1, t, &contact)) { //swap e1 and e2 for performance
                                                  //e1 means clipping from inside a face, e2 is the typical collision
            f = 1;
            gfc_vector3d_set(vlist[*vlistc], abxac.x, abxac.y, abxac.z);
            *vlistc += 1;
            if (*vlistc == collisions_max) {
                *vlistc = 0;
            }
            //slog("e1 hit");
        }
        else if (gfc_trigfc_angle_edge_test(e2, t, &contact)) {
            f = 1;
            gfc_vector3d_set(vlist[*vlistc], abxac.x, abxac.y, abxac.z);
            *vlistc += 1;
            if (*vlistc == collisions_max) {
                *vlistc = 0;
            }
            //slog("e2 hit");
        }

        
    }
    return f;
}

void gf3d_obj_get_counts_from_file(ObjData *obj, FILE *file);
void gf3d_obj_load_get_data_from_file(ObjData *obj, FILE *file);

void gf3d_obj_free(ObjData *obj)
{
    if (!obj)return;
    
    if (obj->vertices != NULL)
    {
        free(obj->vertices);
    }
    if (obj->normals != NULL)
    {
        free(obj->normals);
    }
    if (obj->texels != NULL)
    {
        free(obj->texels);
    }
    if (obj->boneIndices!= NULL)
    {
        free(obj->boneIndices);
    }
    if (obj->boneWeights != NULL)
    {
        free(obj->boneWeights);
    }
    
    if (obj->faceVerts != NULL)
    {
        free(obj->faceVerts);
    }
    if (obj->faceNormals != NULL)
    {
        free(obj->faceNormals);
    }
    if (obj->faceTexels != NULL)
    {
        free(obj->faceTexels);
    }
    if (obj->faceBones != NULL)
    {
        free(obj->faceBones);
    }
    
    if (obj->faceWeights != NULL)
    {
        free(obj->faceWeights);
    }
    
    if (obj->outFace != NULL)
    {
        free(obj->outFace);
    }
    
    free(obj);
}

//while normal obj files don't support bones, the obj structure is used as a staging area for gltf loading.

void gf3d_obj_load_reorg(ObjData *obj)
{
    int i,f;
    int vert = 0;
    int vertexIndex,normalIndex,texelIndex,boneIndex,weightIndex;
    
    if (!obj)return;
    
    obj->face_vert_count = obj->face_count*3;
    obj->faceVertices = (Vertex *)gfc_allocate_array(sizeof(Vertex),obj->face_vert_count);
    obj->outFace = (Face *)gfc_allocate_array(sizeof(Face),obj->face_count);
    
    for (i = 0; i < obj->face_count;i++)
    {
        for (f = 0; f < 3;f++,vert++)
        {
            vertexIndex = obj->faceVerts[i].verts[f];
            gfc_vector3d_copy(obj->faceVertices[vert].vertex,obj->vertices[vertexIndex]);

            if (obj->faceNormals)
            {
                normalIndex = obj->faceNormals[i].verts[f];
                gfc_vector3d_copy(obj->faceVertices[vert].normal,obj->normals[normalIndex]);
            }
            if (obj->faceTexels)
            {
                texelIndex = obj->faceTexels[i].verts[f];
                gfc_vector2d_copy(obj->faceVertices[vert].texel,obj->texels[texelIndex]);
            }
            if (obj->faceBones)
            {
                boneIndex = obj->faceBones[i].verts[f];
                gfc_vector4d_copy(obj->faceVertices[vert].bones,obj->boneIndices[boneIndex]);
            }
            if (obj->faceWeights)
            {
                weightIndex = obj->faceWeights[i].verts[f];
                gfc_vector4d_copy(obj->faceVertices[vert].weights,obj->boneWeights[weightIndex]);
            }
            
            obj->outFace[i].verts[f] = vert;
        }
    }
}

void gf3d_obj_get_bounds(ObjData *obj)
{
    int i;
    if (!obj)return;
    for (i = 0; i < obj->vertex_count; i++)
    {
        if (obj->vertices[i].x < obj->bounds.x)obj->bounds.x = obj->vertices[i].x;
        if (obj->vertices[i].y < obj->bounds.y)obj->bounds.y = obj->vertices[i].y;
        if (obj->vertices[i].z < obj->bounds.z)obj->bounds.z = obj->vertices[i].z;
        if (obj->vertices[i].x > obj->bounds.w)obj->bounds.w = obj->vertices[i].x;
        if (obj->vertices[i].y > obj->bounds.h)obj->bounds.h = obj->vertices[i].y;
        if (obj->vertices[i].z > obj->bounds.d)obj->bounds.d = obj->vertices[i].z;
    }
    obj->bounds.w -= obj->bounds.x;
    obj->bounds.h -= obj->bounds.y;
    obj->bounds.d -= obj->bounds.z;
}

ObjData *gf3d_obj_load_from_file(const char *filename)
{
    ObjData *obj;
    FILE* file;
    
    if (!filename)return NULL;
    file = fopen(filename, "r");
    if (!file)
    {
        slog("failed to open material file %s", filename);
        return NULL;
    }        
    obj = (ObjData*)gfc_allocate_array(sizeof(ObjData),1);
    if (!obj)
    {
        fclose(file);
        return NULL;
    }
    
    gf3d_obj_get_counts_from_file(obj, file);
    
    obj->vertices = (GFC_Vector3D *)gfc_allocate_array(sizeof(GFC_Vector3D),obj->vertex_count);
    obj->normals = (GFC_Vector3D *)gfc_allocate_array(sizeof(GFC_Vector3D),obj->normal_count);
    obj->texels = (GFC_Vector2D *)gfc_allocate_array(sizeof(GFC_Vector2D),obj->texel_count);
    
    obj->faceVerts = (Face *)gfc_allocate_array(sizeof(Face),obj->face_count);
    obj->faceNormals = (Face *)gfc_allocate_array(sizeof(Face),obj->face_count);
    obj->faceTexels = (Face *)gfc_allocate_array(sizeof(Face),obj->face_count);
    
    gf3d_obj_load_get_data_from_file(obj, file);
    
    
    gf3d_obj_get_bounds(obj);
    gf3d_obj_load_reorg(obj);
    fclose(file);
    return obj;
}

void gf3d_obj_get_counts_from_file(ObjData *obj, FILE *file)
{
    char buf[256];
    int  numvertices = 0;
    int  numtexels = 0;
    int  numnormals = 0;
    int  numfaces = 0;

    if ((!obj)||(!file))
    {
        return;
    }
    rewind(file);
    while(fscanf(file, "%s", buf) != EOF)
    {
        switch(buf[0])
        {
            case 'v':
              switch(buf[1])
              {
                case '\0':
                  numvertices++;
                  break;
                case 'n':
                  numnormals++;
                  break;
                case 't':
                  numtexels++;
                  break;
                default:
                  break;
              }
              break;
            case 'f':
              numfaces++;
              break;
            default:
              break;
        }
    }
    obj->vertex_count  = numvertices;
    obj->texel_count  = numtexels;
    obj->normal_count  = numnormals;
    obj->face_count = numfaces;
}

void gf3d_obj_load_get_data_from_file(ObjData *obj, FILE *file)
{
    int  numvertices = 0;
    int  numnormals = 0;
    int  numtexcoords = 0;
    int  numfaces = 0;
    char buf[128];
    float x,y,z;
    int f[3][3];

    if ((!obj)||(!file))return;

    rewind(file);
    while(fscanf(file, "%s", buf) != EOF)
    {
        switch(buf[0])
        {
            case 'v':
              switch(buf[1])
              {
                case '\0':
                  fscanf(
                      file,
                      "%f %f %f",
                      &x,
                      &y,
                      &z
                    );
                  obj->vertices[numvertices].x = x;
                  obj->vertices[numvertices].y = y;
                  obj->vertices[numvertices].z = z;
                  numvertices++;
                  break;
                case 'n':
                  fscanf(
                      file,
                      "%f %f %f",
                      &x,
                      &y,
                      &z
                  );
                  obj->normals[numnormals].x = x;
                  obj->normals[numnormals].y = y;
                  obj->normals[numnormals].z = z;
                  numnormals++;
                  break;
                case 't':
                  fscanf(
                      file,
                      "%f %f",
                      &x,
                      &y
                    );
                  obj->texels[numtexcoords].x = x;
                  obj->texels[numtexcoords].y = 1 - y;
                  numtexcoords++;
                  break;
                default:
                  break;
              }
              break;
            case 'f':
              fscanf(
                  file,
                  "%d/%d/%d %d/%d/%d %d/%d/%d",
                  &f[0][0],
                  &f[0][1],
                  &f[0][2],
                  
                  &f[1][0],
                  &f[1][1],
                  &f[1][2],
                  
                  &f[2][0],
                  &f[2][1],
                  &f[2][2]);
              
              obj->faceVerts[numfaces].verts[0]   = f[0][0] - 1;
              obj->faceTexels[numfaces].verts[0]  = f[0][1] - 1;
              obj->faceNormals[numfaces].verts[0] = f[0][2] - 1;
              
              obj->faceVerts[numfaces].verts[1]   = f[1][0] - 1;
              obj->faceTexels[numfaces].verts[1]  = f[1][1] - 1;
              obj->faceNormals[numfaces].verts[1] = f[1][2] - 1;
              
              obj->faceVerts[numfaces].verts[2]   = f[2][0] - 1;
              obj->faceTexels[numfaces].verts[2]  = f[2][1] - 1;
              obj->faceNormals[numfaces].verts[2] = f[2][2] - 1;
              numfaces++;
              break;
            default:
              break;
        }
    }
}

void gf3d_obj_move(ObjData *obj,GFC_Vector3D offset,GFC_Vector3D rotation)
{
    GFC_Vector4D outV = {0};
    GFC_Matrix4 matrix = {0};
    int i;
    if (!obj)return;
    for (i = 0; i < obj->face_vert_count;i++)
    {
        //update the vertices
        gfc_matrix4_from_vectors(
            matrix,
            offset,
            rotation,
            gfc_vector3d(1,1,1));//TODO add the scale too
        gfc_matrix4_v_multiply(&outV,gfc_vector3dw(obj->faceVertices[i].vertex,1.0),matrix);
        gfc_vector3d_copy(obj->faceVertices[i].vertex,outV);
        //update the normal, without the translation
        gfc_matrix4_from_vectors(
            matrix,
            gfc_vector3d(0,0,0),
            rotation,
            gfc_vector3d(1,1,1));
        gfc_matrix4_v_multiply(&outV,gfc_vector3dw(obj->faceVertices[i].normal,1.0),matrix);
        gfc_vector3d_copy(obj->faceVertices[i].normal,outV);

    }
}

ObjData *gf3d_obj_new()
{
    ObjData *out = NULL;
    out = (ObjData*)gfc_allocate_array(sizeof(ObjData),1);
    return out;
}

ObjData *gf3d_obj_duplicate(ObjData *in)
{
    ObjData *out = NULL;
    out = gf3d_obj_new();
    if (!out)
    {
        slog("failed to duplicate obj data");
        return NULL;
    }
    
    if ((in->vertices)&&(in->vertex_count))
    {
        out->vertices = gfc_allocate_array(sizeof(GFC_Vector3D),in->vertex_count);
        if (out->vertices)
        {
            memcpy(out->vertices,in->vertices,sizeof(GFC_Vector3D)*in->vertex_count);
            out->vertex_count = in->vertex_count;
        }
    }
    if ((in->normals)&&(in->normal_count))
    {
        out->normals = gfc_allocate_array(sizeof(GFC_Vector3D),in->normal_count);
        if (out->normals)
        {
            memcpy(out->normals,in->normals,sizeof(GFC_Vector3D)*in->normal_count);
            out->normal_count = in->normal_count;
        }
    }
    if ((in->texels)&&(in->texel_count))
    {
        out->texels = gfc_allocate_array(sizeof(GFC_Vector2D),in->texel_count);
        if (out->texels)
        {
            memcpy(out->texels,in->texels,sizeof(GFC_Vector2D)*in->texel_count);
            out->texel_count = in->texel_count;
        }
    }
    if ((in->boneIndices)&&(in->bone_count))
    {
        out->boneIndices = gfc_allocate_array(sizeof(GFC_Vector4UI8),in->bone_count);
        if (out->boneIndices)
        {
            memcpy(out->boneIndices,in->boneIndices,sizeof(GFC_Vector4UI8)*in->bone_count);
            out->bone_count = in->bone_count;
        }
    }
    if ((in->boneWeights)&&(in->weight_count))
    {
        out->boneWeights = gfc_allocate_array(sizeof(GFC_Vector4D),in->weight_count);
        if (out->boneWeights)
        {
            memcpy(out->boneWeights,in->boneWeights,sizeof(GFC_Vector4D)*in->weight_count);
            out->weight_count = in->weight_count;
        }
    }
    if (in->face_count)
    {
        if (in->faceVerts)
        {
            out->faceVerts = gfc_allocate_array(sizeof(Face),in->face_count);
            if (out->faceVerts)
            {
                memcpy(out->faceVerts,in->faceVerts,sizeof(Face)*in->face_count);
            }
        }
        if (in->faceNormals)
        {
            out->faceNormals = gfc_allocate_array(sizeof(Face),in->face_count);
            if (out->faceNormals)
            {
                memcpy(out->faceNormals,in->faceNormals,sizeof(Face)*in->face_count);
            }
        }
        if (in->faceTexels)
        {
            out->faceTexels = gfc_allocate_array(sizeof(Face),in->face_count);
            if (out->faceTexels)
            {
                memcpy(out->faceTexels,in->faceTexels,sizeof(Face)*in->face_count);
            }
        }
        if (in->faceBones)
        {
            out->faceBones = gfc_allocate_array(sizeof(Face),in->face_count);
            if (out->faceBones)
            {
                memcpy(out->faceBones,in->faceBones,sizeof(Face)*in->face_count);
            }
        }
        if (in->faceWeights)
        {
            out->faceWeights = gfc_allocate_array(sizeof(Face),in->face_count);
            if (out->faceWeights)
            {
                memcpy(out->faceWeights,in->faceWeights,sizeof(Face)*in->face_count);
            }
        }
        if (in->outFace)
        {
            out->outFace = gfc_allocate_array(sizeof(Face),in->face_count);
            if (out->outFace)
            {
                memcpy(out->outFace,in->outFace,sizeof(Face)*in->face_count);
            }
        }
        out->face_count = in->face_count;
    }
    if ((in->faceVertices)&&(in->face_vert_count))
    {
        out->faceVertices = gfc_allocate_array(sizeof(Vertex),in->face_vert_count);
        if (out->faceVertices)
        {
            memcpy(out->faceVertices,in->faceVertices,sizeof(Vertex)*in->face_vert_count);
            out->face_vert_count = in->face_vert_count;
        }
    }
    memcpy(&out->bounds,&in->bounds,sizeof(GFC_Box));
    return out;
}

ObjData *gf3d_obj_merge(ObjData *ObjA,GFC_Vector3D offsetA,ObjData *ObjB,GFC_Vector3D offsetB,GFC_Vector3D rotation)
{
    int i;
    GFC_Vector4D outV = {0};
    GFC_Matrix4 matrix;
    ObjData *ObjNew;
    if ((!ObjA)||(!ObjB))return NULL;
    if ((!ObjA->faceVertices)||(!ObjB->faceVertices))
    {
        slog("must reorg for memory buffer before calling");
        return NULL;
    }
    ObjNew = gf3d_obj_new();
    if (!ObjNew)return NULL;
    //allocate space for new verices
    ObjNew->faceVertices = gfc_allocate_array(sizeof(Vertex),ObjA->face_vert_count + ObjB->face_vert_count);
    if (!ObjNew->faceVertices)
    {
        gf3d_obj_free(ObjNew);
        return NULL;
    }
    ObjNew->face_vert_count = ObjA->face_vert_count + ObjB->face_vert_count;
    ObjNew->outFace = gfc_allocate_array(sizeof(Face),ObjA->face_count + ObjB->face_count);
    if (!ObjNew->outFace)
    {
        gf3d_obj_free(ObjNew);
        return NULL;
    }
    ObjNew->face_count = ObjA->face_count + ObjB->face_count;
    //copy the old data into the ObjNew
    for (i = 0; i < ObjA->face_count;i++)
    {
        memcpy(&ObjNew->outFace[i],&ObjA->outFace[i],sizeof(Face));
    }
    for (i = 0; i < ObjB->face_count;i++)
    {
        memcpy(&ObjNew->outFace[i + ObjA->face_count],&ObjB->outFace[i],sizeof(Face));
        //the face indices need to be updated as well;
        ObjNew->outFace[i + ObjA->face_count].verts[0]+= ObjA->face_vert_count;
        ObjNew->outFace[i + ObjA->face_count].verts[1]+= ObjA->face_vert_count;
        ObjNew->outFace[i + ObjA->face_count].verts[2]+= ObjA->face_vert_count;
    }
    for (i = 0; i < ObjA->face_vert_count;i++)
    {
        memcpy(&ObjNew->faceVertices[i],&ObjA->faceVertices[i],sizeof(Vertex));
        gfc_vector3d_add(ObjNew->faceVertices[i].vertex,ObjNew->faceVertices[i].vertex,offsetA);
    }
    for (i = 0; i < ObjB->face_vert_count;i++)
    {
        //update the vertices
        memcpy(&ObjNew->faceVertices[i + ObjA->face_vert_count],&ObjB->faceVertices[i],sizeof(Vertex));
        gfc_matrix4_from_vectors(
            matrix,
            offsetB,
            rotation,
            gfc_vector3d(1,1,1));
        gfc_matrix4_v_multiply(&outV,gfc_vector3dw(ObjNew->faceVertices[i + ObjA->face_vert_count].vertex,1.0),matrix);
        gfc_vector3d_copy(ObjNew->faceVertices[i + ObjA->face_vert_count].vertex,outV);
        //update the normal, without the translation
        gfc_matrix4_from_vectors(
            matrix,
            gfc_vector3d(0,0,0),
            rotation,
            gfc_vector3d(1,1,1));
        gfc_matrix4_v_multiply(&outV,gfc_vector3dw(ObjNew->faceVertices[i + ObjA->face_vert_count].normal,1.0),matrix);
        gfc_vector3d_copy(ObjNew->faceVertices[i + ObjA->face_vert_count].normal,outV);
    }
    return ObjNew;
}


Uint8 gf3d_obj_line_test(ObjData *obj, GFC_Edge3D e, GFC_Vector3D *contact) {
    int i;
    Uint32 index;
    GFC_Triangle3D t;
    if (!obj) return 0;
    if (!obj->outFace) return 0;
    if (!obj->faceVertices) return 0;

    for (i = 0; i < obj->face_count; i++) {
        index = obj->outFace[i].verts[0];
        t.a = obj->faceVertices[index].vertex;
        index = obj->outFace[i].verts[1];
        t.b = obj->faceVertices[index].vertex;
        index = obj->outFace[i].verts[2];
        t.c = obj->faceVertices[index].vertex;
        if (gfc_trigfc_angle_edge_test(e, t, contact)) return 1;
    }
    return 0;
}

/*eol@eof*/
