#version 330 core
layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 color;
uniform mat4 vp, model, lightSpace;
out vec3 world, surfaceNormal, pigment, sunCoord;
void main(){
    world=(model*vec4(position,1)).xyz;
    surfaceNormal=normalize(transpose(inverse(mat3(model)))*normal);
    pigment=color;
    sunCoord=(lightSpace*vec4(world,1)).xyz*.5+.5;
    gl_Position=vp*vec4(world,1);
}
