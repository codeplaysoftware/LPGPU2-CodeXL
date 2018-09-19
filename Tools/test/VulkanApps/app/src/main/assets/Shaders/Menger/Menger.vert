#version 400

#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 0) uniform vert_buf {
 float frame;

 int   xres;
 int   yres;
} uni;

layout (location = 0) in  vec4  vPosition;

layout (location = 0) out vec3   position;


const float distance = 2.0/sqrt(3.0);


void main() {
 vec2 resolution = vec2(uni.xres,uni.yres);

 position = (vPosition+0.5).xyz;

 float p = 0.001 * uni.frame;

 float sy = sin(p);
 float cy = cos(p);

 mat3 ry = mat3(1.0);

 ry[0].x= cy; ry[0].z=-sy;
 ry[2].x= sy; ry[2].z= cy;


 float sz = sin(2.0*p);
 float cz = cos(2.0*p);

 mat3 rz = mat3(1.0);

 rz[0].x= cz; rz[0].y=-sz;
 rz[1].x= sz; rz[1].y= cz;


 float sx = sin(3.0*p);
 float cx = cos(3.0*p);

 mat3 rx = mat3(1.0);

 rx[1].y= cx; rx[1].z=-sx;
 rx[2].y= sx; rx[2].z= cx;

 vec3 pos = distance * (rx * ry * rz * vPosition.xyz);

 gl_Position = vec4(pos.x * resolution.y / resolution.x,pos.y,pos.z,1.0);

 // The following line remaps the Z distance to what Vulkan expects
 // It's a quick fix for porting GL to Vulkan. Saw it here:
 // anki3d.org/vulkan-coordinate-system

 gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}
