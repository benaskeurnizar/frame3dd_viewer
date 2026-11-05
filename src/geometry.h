#pragma once

#include <cglm/cglm.h>

#include "graphics.h"

typedef struct{
	float u,v;
}point2D;

typedef struct{
	vec3 pos,color,normal;
}Vertex;

void quatenrionRotation(vec4 q, mat4 rot);
void multiply_quaternions(vec4 a,vec4 b,vec4 d);

void createDoubleArrowVertices(float* vertices, int* vidx, vec3 color, float scale, float w, float h, vec3 pos, vec3 dir, vec3 right);
void createArrowVertices(float* vertices, int* vidx, vec3 color, float scale, float w, float h, vec3 pos, vec3 dir, vec3 right) ;
void calculateModelSize(structureModel*m,BoundingBox* box);
void updateModelMatrix(mat4 m,float s,vec3 pos,vec4 quaternion);

void addLine(float* v, int* vi, const vec3 a, const vec3 b, const vec3 c);
void addPyramid(float* v, int* vi, const vec3 n, float s, const vec3 col);
void addPlate(float* v, int* vi, const vec3 n, float s, const vec3 col);
void addBox(float* v, int* vi, const vec3 n, float s, const vec3 col);
void addGeneral(float* v, int* vi, const vec3 n, float s, const int r[6], const vec3 col);


void createElementTube(structureModel* model,Object* object,vec3 start,vec3 end,double S);
void rotateByVector(World* world,vec3 p1,vec3 p2);
void rotateStructure(World* world,float angle_deg,vec3 axis);
