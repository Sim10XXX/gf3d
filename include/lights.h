#ifndef __LIGHTS_H__
#define __LIGHTS_H__

#include "gfc_vector.h"

#define LIGHT_UBO_MAX 16

typedef struct
{
    GFC_Vector4D    lightPos;
    GFC_Vector4D    lightDir;
    GFC_Vector4D    lightColor;

    
    //float           ambientCoefficient; //how strong the ambient
    float           angle;
    float           brightness;
    float           attenuation; //How fast this light falls off
    float           padding;
}Light;

typedef struct
{
    Light lights[LIGHT_UBO_MAX];
}Light_UBO;

#endif