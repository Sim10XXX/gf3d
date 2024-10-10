#version 450
#extension GL_ARB_separate_shader_objects : enable

const uint MAX_LIGHTS = 16;

struct MeshUBO
{
    mat4    model;
    mat4    view;
    mat4    proj;
    vec4    color;             
    vec4    camera;            //needed for many light calculations
};

struct MaterialUBO
{
    vec4    ambient;        //how much ambient light affects this material
    vec4    diffuse;        //how much diffuse light affects this material - primary influcen for material color
    vec4    specular;       //color of the shine on the materials
    vec4    emission;       //color that shows regardless of light
    float   transparency;   //how translucent the material should be overall
    float   shininess;      //how shiny the materials is.  // how pronounced the specular is
    vec2    padding;        //for alignment
};

layout(binding = 0) uniform UniformBufferObject
{
    MeshUBO         mesh;
    MaterialUBO     material;   //this may become an array
    vec4            lightPos;
    vec4            lightDir;
    vec4            lightColor;
    float           angle;
    float           brightness;
    float           falloff;
    float           padding;
}ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 position;

layout(location = 0) out vec4 outColor;

void main()
{   
    int i;
    vec4 surfaceColor = texture(texSampler, fragTexCoord);
    vec3 normal = fragNormal;
    vec3 surfaceToCamera = normalize(ubo.mesh.camera.xyz - position);
    vec3 surfaceToLight = -normalize(ubo.lightDir.xyz);

    surfaceColor.xyz *= ubo.material.diffuse.xyz;
    surfaceColor.w *= ubo.material.diffuse.w * ubo.material.transparency;

    float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
    vec3 diffuse = diffuseCoefficient * surfaceColor.rgb * ubo.lightColor.xyz;

    outColor = surfaceColor;
    outColor.xyz += diffuse.xyz;
}

