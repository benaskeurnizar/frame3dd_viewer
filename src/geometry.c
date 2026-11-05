#include "geometry.h"
#include <string.h>
#include <math.h>

#ifndef PI
#define PI 3.14159265358979323846f
#endif


void createDoubleArrowVertices(float* vertices, int* vidx, vec3 color, float scale, float w, float h, vec3 pos, vec3 dir, vec3 right){
	// 1st Vertex
    vertices[*vidx]     = pos[0] - (dir[0] * scale);
    vertices[*vidx + 1] = pos[1] - (dir[1] * scale);
    vertices[*vidx + 2] = pos[2] - (dir[2] * scale) - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;

    // 2nd Vertex
    vertices[*vidx]     = pos[0];
    vertices[*vidx + 1] = pos[1];
    vertices[*vidx + 2] = pos[2] - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;

    // 3rd Vertex
    vertices[*vidx]     = pos[0] - (dir[0] * h) + (right[0] * w);
    vertices[*vidx + 1] = pos[1] - (dir[1] * h) + (right[1] * w);
    vertices[*vidx + 2] = pos[2] - (dir[2] * h) + (right[2] * w) - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;

    // 4th Vertex
    vertices[*vidx]     = pos[0];
    vertices[*vidx + 1] = pos[1];
    vertices[*vidx + 2] = pos[2] - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;

    // 5th Vertex
    vertices[*vidx]     = pos[0] - (dir[0] * h) - (right[0] * w);
    vertices[*vidx + 1] = pos[1] - (dir[1] * h) - (right[1] * w);
    vertices[*vidx + 2] = pos[2] - (dir[2] * h) - (right[2] * w) - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;

    // 6th Vertex
    vertices[*vidx]     = pos[0];
    vertices[*vidx + 1] = pos[1];
    vertices[*vidx + 2] = pos[2] - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;

    // 7th Vertex
    vertices[*vidx]     = pos[0] - (dir[0] * h) + (right[0] * w);
    vertices[*vidx + 1] = pos[1] - (dir[1] * h) + (right[1] * w);
    vertices[*vidx + 2] = pos[2] - (dir[2] * h) + (right[2] * w) - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;

    // 8th Vertex
    vertices[*vidx]     = pos[0] - (dir[0] * h) - (right[0] * w);
    vertices[*vidx + 1] = pos[1] - (dir[1] * h) - (right[1] * w);
    vertices[*vidx + 2] = pos[2] - (dir[2] * h) - (right[2] * w) - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;
    
    // 9rd Vertex
    vertices[*vidx]     = pos[0] - (dir[0] * 2*h) + (right[0] * w);
    vertices[*vidx + 1] = pos[1] - (dir[1] * 2*h) + (right[1] * w);
    vertices[*vidx + 2] = pos[2] - (dir[2] * 2*h) + (right[2] * w) - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;

    // 10th Vertex
    vertices[*vidx]     = pos[0] - (dir[0]*h);
    vertices[*vidx + 1] = pos[1] - (dir[1]*h);
    vertices[*vidx + 2] = pos[2] - (dir[2]*h)- 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;

    // 11th Vertex
    vertices[*vidx]     = pos[0] - (dir[0] * 2*h) - (right[0] * w);
    vertices[*vidx + 1] = pos[1] - (dir[1] * 2*h) - (right[1] * w);
    vertices[*vidx + 2] = pos[2] - (dir[2] * 2*h) - (right[2] * w) - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;

    // 12th Vertex
    vertices[*vidx]     = pos[0] - (dir[0]*h);
    vertices[*vidx + 1] = pos[1] - (dir[1]*h);
    vertices[*vidx + 2] = pos[2] - (dir[2]*h)- 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;

    // 13th Vertex
    vertices[*vidx]     = pos[0] - (dir[0] * 2*h) + (right[0] * w);
    vertices[*vidx + 1] = pos[1] - (dir[1] * 2*h) + (right[1] * w);
    vertices[*vidx + 2] = pos[2] - (dir[2] * 2*h) + (right[2] * w) - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;

    // 14th Vertex
    vertices[*vidx]     = pos[0] - (dir[0] * 2*h) - (right[0] * w);
    vertices[*vidx + 1] = pos[1] - (dir[1] * 2*h) - (right[1] * w);
    vertices[*vidx + 2] = pos[2] - (dir[2] * 2*h) - (right[2] * w) - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;
    
}
void createArrowVertices(float* vertices, int* vidx, vec3 color, float scale, float w, float h, vec3 pos, vec3 dir, vec3 right) {
    // 1st Vertex
    vertices[*vidx]     = pos[0] - (dir[0] * scale);
    vertices[*vidx + 1] = pos[1] - (dir[1] * scale);
    vertices[*vidx + 2] = pos[2] - (dir[2] * scale) - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;

    // 2nd Vertex
    vertices[*vidx]     = pos[0];
    vertices[*vidx + 1] = pos[1];
    vertices[*vidx + 2] = pos[2] - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;

    // 3rd Vertex
    vertices[*vidx]     = pos[0] - (dir[0] * h) + (right[0] * w);
    vertices[*vidx + 1] = pos[1] - (dir[1] * h) + (right[1] * w);
    vertices[*vidx + 2] = pos[2] - (dir[2] * h) + (right[2] * w) - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;

    // 4th Vertex
    vertices[*vidx]     = pos[0];
    vertices[*vidx + 1] = pos[1];
    vertices[*vidx + 2] = pos[2] - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;

    // 5th Vertex
    vertices[*vidx]     = pos[0] - (dir[0] * h) - (right[0] * w);
    vertices[*vidx + 1] = pos[1] - (dir[1] * h) - (right[1] * w);
    vertices[*vidx + 2] = pos[2] - (dir[2] * h) - (right[2] * w) - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;

    // 6th Vertex
    vertices[*vidx]     = pos[0];
    vertices[*vidx + 1] = pos[1];
    vertices[*vidx + 2] = pos[2] - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;

    // 7th Vertex
    vertices[*vidx]     = pos[0] - (dir[0] * h) + (right[0] * w);
    vertices[*vidx + 1] = pos[1] - (dir[1] * h) + (right[1] * w);
    vertices[*vidx + 2] = pos[2] - (dir[2] * h) + (right[2] * w) - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;

    // 8th Vertex
    vertices[*vidx]     = pos[0] - (dir[0] * h) - (right[0] * w);
    vertices[*vidx + 1] = pos[1] - (dir[1] * h) - (right[1] * w);
    vertices[*vidx + 2] = pos[2] - (dir[2] * h) - (right[2] * w) - 0.01f;
    vertices[*vidx + 3] = color[0];
    vertices[*vidx + 4] = color[1];
    vertices[*vidx + 5] = color[2];
    *vidx += 6;
}

void addLine(float* v, int* vi, const vec3 a, const vec3 b, const vec3 c) {
    const float* p[2] = { a, b };
    for (int k = 0; k < 2; k++) {
        v[*vi] = p[k][0]; v[*vi+1] = p[k][1]; v[*vi+2] = p[k][2];
        v[*vi+3] = c[0];  v[*vi+4] = c[1];    v[*vi+5] = c[2];
        *vi += 6;
    }
}

void addPyramid(float* v, int* vi, const vec3 n, float s, const vec3 col) {
    float w = 0.5f * s;
    vec3 c[4] = { {n[0]-w, n[1]-s, n[2]-w}, {n[0]+w, n[1]-s, n[2]-w},
                  {n[0]+w, n[1]-s, n[2]+w}, {n[0]-w, n[1]-s, n[2]+w} };
    for (int i = 0; i < 4; i++) {
        addLine(v, vi, n, c[i], col);                 // apex to corner
        addLine(v, vi, c[i], c[(i+1)%4], col);        // base edge
    }
}

void addPlate(float* v, int* vi, const vec3 n, float s, const vec3 col) {
    float w = 0.5f * s, y = n[1] - 1.2f * s;
    vec3 c[4] = { {n[0]-w, y, n[2]-w}, {n[0]+w, y, n[2]-w},
                  {n[0]+w, y, n[2]+w}, {n[0]-w, y, n[2]+w} };
    for (int i = 0; i < 4; i++) addLine(v, vi, c[i], c[(i+1)%4], col);
}

void addBox(float* v, int* vi, const vec3 n, float s, const vec3 col) {
    float w = 0.5f * s;
    vec3 t[4] = { {n[0]-w, n[1],   n[2]-w}, {n[0]+w, n[1],   n[2]-w},
                  {n[0]+w, n[1],   n[2]+w}, {n[0]-w, n[1],   n[2]+w} };
    vec3 b[4] = { {n[0]-w, n[1]-s, n[2]-w}, {n[0]+w, n[1]-s, n[2]-w},
                  {n[0]+w, n[1]-s, n[2]+w}, {n[0]-w, n[1]-s, n[2]+w} };
    for (int i = 0; i < 4; i++) {
        addLine(v, vi, t[i], t[(i+1)%4], col);
        addLine(v, vi, b[i], b[(i+1)%4], col);
        addLine(v, vi, t[i], b[i], col);
    }
}

void addGeneral(float* v, int* vi, const vec3 n, float s, const int r[6], const vec3 col) {
    for (int a = 0; a < 3; a++) {
        if (r[a]) {                                    // restrained translation: line through the node
            vec3 p0 = { n[0], n[1], n[2] }, p1 = { n[0], n[1], n[2] };
            p0[a] -= 0.6f * s;  p1[a] += 0.6f * s;
            addLine(v, vi, p0, p1, col);
        }
        if (r[3 + a]) {                                // restrained rotation: square ring around the axis
            int u = (a + 1) % 3, w = (a + 2) % 3;
            float q = 0.3f * s;
            float su[4] = { -q,  q,  q, -q }, sw[4] = { -q, -q,  q,  q };
            vec3 c[4];
            for (int k = 0; k < 4; k++) {
                c[k][0] = n[0]; c[k][1] = n[1]; c[k][2] = n[2];
                c[k][u] += su[k];  c[k][w] += sw[k];
            }
            for (int k = 0; k < 4; k++) addLine(v, vi, c[k], c[(k+1)%4], col);
        }
    }
}


void calculateModelSize(structureModel*m,BoundingBox* box){
	for(int i = 0; i < m->nodeCount; i++){
		float p[3] = { m->nodes[i].x, m->nodes[i].y, m->nodes[i].z };
		for(int j = 0; j < 3; j++){
			if(p[j] > box->max[j]) box->max[j] = p[j];
			if(p[j] < box->min[j]) box->min[j] = p[j];
		}
	}
}

void updateModelMatrix(mat4 m,float s,vec3 pos,vec4 quaternion){
	mat4 R;
	quatenrionRotation(quaternion,R);
 
	mat4 T,S,RT;
	glm_mat4_identity(T);
	glm_translate(T,pos);

	vec3 scale = {s,s,s};
	glm_mat4_identity(S);
	glm_scale(S,scale);

	glm_mat4_mul(R,T,RT);
	glm_mat4_mul(RT,S,m);
}

point2D* createCircleOutline(float raduis,float deltaDeg,int* count){
	int N = (int)(360.0 / deltaDeg + 0.5);
	*count = N;
	point2D* points = (point2D*)calloc(N,sizeof(point2D));
	for(int i=0;i<N;i++){
		float theta = glm_rad(i*deltaDeg);
		points[i].u = raduis*cosf(theta);
		points[i].v = raduis*sinf(theta);
	}
	return points;
}

void createElementTube(structureModel* model,Object* object,vec3 start,vec3 end,double S){
	float raduis = sqrtf(S/PI)*2.0;
	float deltaDeg = 10.0;
	vec3 color = {0.3,0.3,0.3};
	//int n1 = element->node1;
	//int n2 = element->node2;
	//Node node1 = model->nodes[n1];
	//Node node2 = model->nodes[n2];
	//vec3 start = {node1.x,node1.y,node1.z};
	//vec3 end = {node2.x,node2.y,node2.z};
	vec3 dir;
	glm_vec3_sub(end,start,dir);
	float Le = sqrtf(dir[0]*dir[0]+dir[1]*dir[1]+dir[2]*dir[2]);
	float maxR = 0.15f * Le;               // radius is never more than 15% of the length
	if (raduis > maxR) raduis = maxR;
	glm_vec3_normalize(dir);
	vec3 up = {0.0,1.0,0.0};
	if (fabsf(dir[1]) > 0.99f) {
		up[0] = 0.0f;
		up[1] = 0.0f;
		up[2] = 1.0f;
	}
	vec3 right;
	glm_vec3_cross(dir,up,right);
	glm_vec3_normalize(right);
	
	glm_vec3_cross(right, dir, up);
	glm_vec3_normalize(up);
	
	vec3 ndir;
	ndir[0] = -dir[0];ndir[1] = -dir[1];ndir[2] = -dir[2];
	
	int outline_count = 0;
	point2D* outline_points = createCircleOutline(raduis,deltaDeg,&outline_count);

	int ring_count = 5;
	Vertex** rings = (Vertex**)malloc(sizeof(Vertex*)*ring_count);
	float step = Le/(float)(ring_count-1);
	for(int i=0;i<ring_count;i++){
		vec3 pos;
		pos[0] = start[0]+(i*step)*dir[0];
		pos[1] = start[1]+(i*step)*dir[1];
		pos[2] = start[2]+(i*step)*dir[2];
		rings[i] = (Vertex*)calloc(outline_count,sizeof(Vertex));
		for(int j=0;j<outline_count;j++){
			rings[i][j].pos[0] = pos[0]+outline_points[j].u*right[0]+outline_points[j].v*up[0];
			rings[i][j].pos[1] = pos[1]+outline_points[j].u*right[1]+outline_points[j].v*up[1];
			rings[i][j].pos[2] = pos[2]+outline_points[j].u*right[2]+outline_points[j].v*up[2];
			//if(i!=0 && i!=ring_count-1){
			vec3 normal;
			glm_vec3_sub(rings[i][j].pos,pos,normal);
			glm_vec3_normalize(normal);
			//}
			glm_vec3_copy(normal,rings[i][j].normal);
			glm_vec3_copy(color,rings[i][j].color);
		}
	}
	
	// create the triangles for the first outline
	int section1_count = outline_count * 3;
	Vertex* section1 = calloc(section1_count,sizeof(Vertex));
	int idx = 0;
	for(int i=0;i<outline_count;i++){
		int next = (i+1)%outline_count;
		glm_vec3_copy(start,section1[idx].pos);
		glm_vec3_copy(ndir,section1[idx].normal);
		glm_vec3_copy(color,section1[idx].color);
		idx++;
		
		glm_vec3_copy(rings[0][i].pos,section1[idx].pos);
		glm_vec3_copy(ndir,section1[idx].normal);
		glm_vec3_copy(color,section1[idx].color);
		idx++;
		
		glm_vec3_copy(rings[0][next].pos,section1[idx].pos);
		glm_vec3_copy(ndir,section1[idx].normal);
		glm_vec3_copy(color,section1[idx].color);
		idx++;
	}
	
	// create the triangles for the second outline
	int section2_count = outline_count * 3;
	Vertex* section2 = calloc(section2_count,sizeof(Vertex));
	idx = 0;
	for(int i=0;i<outline_count;i++){
		int next = (i+1)%outline_count;
		glm_vec3_copy(end,section2[idx].pos);
		glm_vec3_copy(dir,section2[idx].normal);
		glm_vec3_copy(color,section2[idx].color);
		idx++;
		
		glm_vec3_copy(rings[ring_count-1][i].pos,section2[idx].pos);
		glm_vec3_copy(dir,section2[idx].normal);
		glm_vec3_copy(color,section2[idx].color);
		idx++;
		
		glm_vec3_copy(rings[ring_count-1][next].pos,section2[idx].pos);
		glm_vec3_copy(dir,section2[idx].normal);
		glm_vec3_copy(color,section2[idx].color);
		idx++;
	}
	
	// connect the rings
	int section3_count = (ring_count-1)*outline_count*6;
	Vertex* section3 = calloc(section3_count,sizeof(Vertex));
	idx = 0;
	for(int i=0;i<ring_count-1;i++){
		int next_ring = i+1;
		for(int j=0;j<outline_count;j++){
			int next_j = (j+1)%outline_count;
			memcpy(&(section3[idx]),&(rings[i][j]),sizeof(Vertex));
			idx++;
			memcpy(&(section3[idx]),&(rings[i][next_j]),sizeof(Vertex));
			idx++;
			memcpy(&(section3[idx]),&(rings[next_ring][j]),sizeof(Vertex));
			idx++;
			memcpy(&(section3[idx]),&(rings[next_ring][j]),sizeof(Vertex));
			idx++;
			memcpy(&(section3[idx]),&(rings[next_ring][next_j]),sizeof(Vertex));
			idx++;
			memcpy(&(section3[idx]),&(rings[i][next_j]),sizeof(Vertex));
			idx++;
		}
	}
	
	object->vertexCount = section1_count+section2_count+section3_count;
	object->vertices = (float*)calloc(object->vertexCount*9,sizeof(float));
	int vidx = 0;
	for(int i=0;i<section1_count;i++){
		object->vertices[vidx]   = section1[i].pos[0];
		object->vertices[vidx+1] = section1[i].pos[1];
		object->vertices[vidx+2] = section1[i].pos[2];
		object->vertices[vidx+3] = section1[i].color[0];
		object->vertices[vidx+4] = section1[i].color[1];
		object->vertices[vidx+5] = section1[i].color[2];
		object->vertices[vidx+6] = section1[i].normal[0];
		object->vertices[vidx+7] = section1[i].normal[1];
		object->vertices[vidx+8] = section1[i].normal[2];
		vidx+=9;
	}
	for(int i=0;i<section2_count;i++){
		object->vertices[vidx]   = section2[i].pos[0];
		object->vertices[vidx+1] = section2[i].pos[1];
		object->vertices[vidx+2] = section2[i].pos[2];
		object->vertices[vidx+3] = section2[i].color[0];
		object->vertices[vidx+4] = section2[i].color[1];
		object->vertices[vidx+5] = section2[i].color[2];
		object->vertices[vidx+6] = section2[i].normal[0];
		object->vertices[vidx+7] = section2[i].normal[1];
		object->vertices[vidx+8] = section2[i].normal[2];
		vidx+=9;
	}
	for(int i=0;i<section3_count;i++){
		object->vertices[vidx]   = section3[i].pos[0];
		object->vertices[vidx+1] = section3[i].pos[1];
		object->vertices[vidx+2] = section3[i].pos[2];
		object->vertices[vidx+3] = section3[i].color[0];
		object->vertices[vidx+4] = section3[i].color[1];
		object->vertices[vidx+5] = section3[i].color[2];
		object->vertices[vidx+6] = section3[i].normal[0];
		object->vertices[vidx+7] = section3[i].normal[1];
		object->vertices[vidx+8] = section3[i].normal[2];
		vidx+=9;
	}
	
	
	free(outline_points);
	
	// Free all rings
	for (int i = 0; i < ring_count; i++) {
		free(rings[i]);
	}
	free(rings);

	// Free sections
	free(section1);
	free(section2);
	free(section3);
}

void multiply_quaternions(vec4 a,vec4 b,vec4 d){
  d[0] = a[0]*b[0] - a[1]*b[1] - a[2]*b[2] - a[3]*b[3];
  d[1] = a[0]*b[1] + a[1]*b[0] + a[2]*b[3] - a[3]*b[2];
  d[2] = a[0]*b[2] - a[1]*b[3] + a[2]*b[0] + a[3]*b[1];
  d[3] = a[0]*b[3] + a[1]*b[2] - a[2]*b[1] + a[3]*b[0];
}

void quatenrionRotation(vec4 q, mat4 rot){
  glm_mat4_identity(rot);
  float w=q[0], x=q[1], y=q[2], z=q[3];

  rot[0][0] = 1.0f-2.0f*(y*y+z*z);
  rot[1][0] = 2.0f*(x*y-w*z);
  rot[2][0] = 2.0f*(x*z+w*y);

  rot[0][1] = 2.0f*(x*y+w*z);
  rot[1][1] = 1.0f-2.0f*(x*x+z*z);
  rot[2][1] = 2.0f*(y*z-w*x);

  rot[0][2] = 2.0f*(x*z-w*y);
  rot[1][2] = 2.0f*(y*z+w*x);
  rot[2][2] = 1.0f-2.0f*(x*x+y*y);
}

void rotateByVector(World* world,vec3 p1,vec3 p2){
  vec3 axis;
  glm_vec3_cross(p1,p2,axis);
  glm_vec3_normalize(axis);

  float dot = glm_vec3_dot(p2,p1);
  dot = fmaxf(-1.0f, fminf(1.0f, dot));
  float angle = acos(dot);

  
  vec4 q;
  q[0] = cosf(angle/2);           // w
  q[1] = axis[0] * sinf(angle/2); // x
  q[2] = axis[1] * sinf(angle/2); // y
  q[3] = axis[2] * sinf(angle/2); // z

  vec4 result;
  multiply_quaternions(q, world->structure_quaternion, result);
  glm_vec4_copy(result, world->structure_quaternion);
  
  vec4 quaternion;
  glm_vec4_copy(world->structure_quaternion,quaternion);
  float len = sqrt(quaternion[0]*quaternion[0] + quaternion[1]*quaternion[1] + quaternion[2]*quaternion[2] + quaternion[3]*quaternion[3]);
  world->structure_quaternion,quaternion[0]/=len; world->structure_quaternion,quaternion[1]/=len; world->structure_quaternion,quaternion[2]/=len; world->structure_quaternion,quaternion[3]/=len;

  mat4 R;
  quatenrionRotation(world->structure_quaternion,R);
 

  mat4 T,S,RT;
  glm_mat4_identity(T);
  glm_translate(T,world->pos);

  vec3 scale = {world->model.viewScale,world->model.viewScale,world->model.viewScale};
  glm_mat4_identity(S);
  glm_scale(S,scale);

  glm_mat4_mul(R,T,RT);
  glm_mat4_mul(RT,S,world->structure_model);
  
}


void rotateStructure(World* world,float angle_deg,vec3 axis){
  float angle_rad = angle_deg * (PI / 180.0);
  vec4 delta_q;
  delta_q[0] = cosf(angle_rad/2.0);delta_q[1] = axis[0]*sinf(angle_rad/2.0);
  delta_q[2] = axis[1]*sinf(angle_rad/2.0);
  delta_q[3] = axis[2]*sinf(angle_rad/2.0);

  vec4 temp;
  multiply_quaternions(delta_q,world->structure_quaternion, temp);
  glm_vec4_copy(temp,world->structure_quaternion);

  //NOTE: normalize it
  float n = sqrtf(temp[0]*temp[0]+temp[1]*temp[1]+temp[2]*temp[2]+temp[3]*temp[3]);
  world->structure_quaternion[0]/=n;
  world->structure_quaternion[1]/=n;
  world->structure_quaternion[2]/=n;
  world->structure_quaternion[3]/=n;

  mat4 R;
  quatenrionRotation(world->structure_quaternion,R);

  mat4 T,S,RT;
  glm_mat4_identity(T);
  glm_translate(T,world->pos);

  vec3 scale = {world->model.viewScale,world->model.viewScale,world->model.viewScale};
  glm_mat4_identity(S);
  glm_scale(S,scale);

  glm_mat4_mul(R,T,RT);
  glm_mat4_mul(RT,S,world->structure_model);
  
}






