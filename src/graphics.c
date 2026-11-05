#include "graphics.h"
#include "../opengl_pipeline/shaders.h"
#include "../opengl_pipeline/buffers.h"
#include "my_io.h"
#include "solver.h"
#include "geometry.h"
#include "path.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define PI 3.14159265358979323846f

float MyRadians(float angle) {
  vec3 vc;
  float result = (angle * M_PI) / 180.0;
  return result;
}

void updateCamera(Camera* camera);
void initCamera(Camera* camera,float yaw,float pitch){
	camera->yaw =  yaw;
	camera->pitch = pitch;
	
	glm_vec3_copy((vec3){0.0,0.0,-3.0},camera->pos);
	glm_vec3_copy((vec3){0.0,1.0,0.0},camera->up);
	glm_vec3_copy((vec3){0.0,0.0,1.0},camera->forward);
	
	vec3 worldUP = {0.0,1.0,0.0};
	glm_vec3_cross(camera->forward,worldUP,camera->right);
	
	vec3 target;
	glm_vec3_add(camera->pos,camera->forward,target);
	
	mat4 camMatrix;
	glm_lookat(camera->pos, target, camera->up,camera->view);
	//updateCamera(camera);
}

void updateCamera(Camera* camera){
	float yaw   = glm_rad(camera->yaw);
    float pitch = glm_rad(camera->pitch);

    camera->forward[0] = cosf(pitch) * cosf(yaw);
    camera->forward[1] = sinf(pitch);
    camera->forward[2] = cosf(pitch) * sinf(yaw);
    glm_vec3_normalize(camera->forward);
    
	vec3 worldUP = {0.0,1.0,0.0};
	glm_vec3_cross(camera->forward,worldUP,camera->right);
	
	vec3 target;
	glm_vec3_add(camera->pos,camera->forward,target);
	
	glm_lookat(camera->pos, target, camera->up,camera->view);
}


void createPointLoadObject(Object* object,structureModel* model,pointLoad* pload,int load_case){
	vec3 color = {1.0,0.0,0.0};
	object->angle = 0.0;
	double fx = pload->fx;
	double fy = pload->fy;
	double fz = pload->fz;
	float len = sqrtf(fx*fx+fy*fy+fz*fz);
	if(fabs(len)<1e-4){
		printf("0 load ! \n");
		object->vertexCount = 0;
		object->vertices = NULL;
		return;
	}
	object->vertexCount = 8;
	object->vertices = (float*)malloc(sizeof(float)*object->vertexCount*6);
	vec3 dir = {fx/len,fy/len,fz/len};
	vec3 up = {0.0,1.0,0.0};
	if (fabsf(dir[1]) > 0.99f) {
		up[0] = 0.0f;
		up[1] = 0.0f;
		up[2] = 1.0f;
	}
	vec3 right;
	glm_vec3_cross(dir,up,right);
	glm_vec3_normalize(right);
	Node* node = &(model->nodes[pload->node]);
	float ratio = len/model->cases[load_case].maxPoint;
	const float S = 0.0028f;          
	float minLength = 0.075f / S;
	float maxLength = 0.25f / S;

	float scale = minLength + ratio * (maxLength - minLength);
	float h     = 0.30f * scale;
	float w     = 0.12f * scale;
	int vidx = 0;
	vec3 pos = {model->nodes[pload->node].x,model->nodes[pload->node].y,model->nodes[pload->node].z};
	createArrowVertices(object->vertices,&vidx,color,scale,w,h,pos,dir,right);
	
	object->scale[0] = object->scale[1] = object->scale[2] = S;   /* 2 / 720 */
	object->pos[0] = -1.0f;
	object->pos[1] = -0.168f;
	object->pos[2] = 0.0f;

	glm_mat4_identity(object->model);
	glm_translate(object->model, object->pos);
	glm_scale(object->model, object->scale);
	object->angle = 0.0;
	BindObjectBuffers(object);
}

void createNodalMomentObject(Object* object,structureModel* model,pointLoad* pload,int load_case){
	vec3 color = {0.7,0.3,0.0};
	object->angle = 0.0;
	double mx = pload->mx;
	double my = pload->my;
	double mz = pload->mz;
	float len = sqrtf(mx*mx+my*my+mz*mz);
	if(fabs(len)<1e-4){
		printf("0 load ! \n");
		object->vertexCount = 0;
		object->vertices = NULL;
		return;
	}
	object->vertexCount = 14;
	object->vertices = (float*)malloc(sizeof(float)*object->vertexCount*6);
	vec3 dir = {mx/len,my/len,mz/len};
	vec3 up = {0.0,1.0,0.0};
	if (fabsf(dir[1]) > 0.99f) {
		up[0] = 0.0f;
		up[1] = 0.0f;
		up[2] = 1.0f;
	}
	vec3 right;
	glm_vec3_cross(dir,up,right);
	glm_vec3_normalize(right);
	Node* node = &(model->nodes[pload->node]);
	float ratio = len/model->cases[load_case].maxMoment;
	const float S = 0.0028f;          // same as the structure
	float minLength = 0.075f / S;
	float maxLength = 0.25f / S;

	float scale = minLength + ratio*(maxLength-minLength);
	float h     = 0.30f * scale;
	float w     = 0.12f * scale;
	int vidx = 0;
	vec3 pos = {model->nodes[pload->node].x,model->nodes[pload->node].y,model->nodes[pload->node].z};
	createDoubleArrowVertices(object->vertices,&vidx,color,scale,w,h,pos,dir,right);
	
	object->scale[0] = object->scale[1] = object->scale[2] = S;   /* 2 / 720 */
	object->pos[0] = -1.0f;
	object->pos[1] = -0.168f;
	object->pos[2] = 0.0f;

	glm_mat4_identity(object->model);
	glm_translate(object->model, object->pos);
	glm_scale(object->model, object->scale);
	object->angle = 0.0;
	BindObjectBuffers(object);
}

void createUniformLoadObject(Object* object,structureModel* model,UniformLoad* uload,int load_case){
	vec3 color = {0.8,0.1,0.8};
	object->angle = 0.0;
	double fx = uload->ux;
	double fy = uload->uy;
	double fz = uload->uz;
	float len = sqrtf(fx*fx+fy*fy+fz*fz);
	if(fabs(len)<1e-4){
		printf("0 load ! \n");
		object->vertexCount = 0;
		object->vertices = NULL;
		return;
	}
	int num_divisions = 10;
	object->vertexCount = 8*num_divisions;
	object->vertices = (float*)malloc(sizeof(float)*object->vertexCount*6);
	vec3 dir = {fx/len,fy/len,fz/len};
	vec3 up = {0.0,1.0,0.0};
	if (fabsf(dir[1]) > 0.99f) {
		up[0] = 0.0f;
		up[1] = 0.0f;
		up[2] = 1.0f;
	}
	vec3 right;
	glm_vec3_cross(dir,up,right);
	glm_vec3_normalize(right);
	frameElement* element = &(model->elements[uload->element]);
	Node* node1 = &(model->nodes[element->node1]);
	Node* node2 = &(model->nodes[element->node2]);
	vec3 element_dir = {node2->x-node1->x,node2->y-node1->y,node2->z-node1->z};
	float Le = sqrtf(element_dir[0]*element_dir[0]+element_dir[1]*element_dir[1]+element_dir[2]*element_dir[2]);
	glm_vec3_normalize(element_dir);
	float step = Le/(float)num_divisions;
	const float S = 0.0028f;  
	float ratio = len/model->cases[load_case].maxUniform;
	float minLength = 0.075f / S;
	float maxLength = 0.25f / S;

	float scale = minLength + ratio*(maxLength-minLength);
	float h     = 0.30f * scale;
	float w     = 0.12f * scale;
	int vidx = 0;
	vec3 pos = {node1->x,node1->y,node1->z};
	for(int i=0;i<num_divisions;i++){
		createArrowVertices(object->vertices,&vidx,color,scale,w,h,pos,dir,right);
		pos[0]+= element_dir[0]*step;
		pos[1]+= element_dir[1]*step;
		pos[2]+= element_dir[2]*step;
	}
	
	object->scale[0] = object->scale[1] = object->scale[2] = S;   /* 2 / 720 */
	object->pos[0] = -1.0f;
	object->pos[1] = -0.168f;
	object->pos[2] = 0.0f;

	glm_mat4_identity(object->model);
	glm_translate(object->model, object->pos);
	glm_scale(object->model, object->scale);
	object->angle = 0.0;
	BindObjectBuffers(object);
}

void createTrapezoidLoadObject(Object* object,structureModel* model,TrapizoidalLoad* tload,int load_case){
	float max_load = 0.0f;
		for (int d = 0; d < 3; d++) {
			max_load = fmaxf(max_load, fabsf(tload->w1[d]));
			max_load = fmaxf(max_load, fabsf(tload->w2[d]));
		}
	max_load = model->cases[load_case].maxTrap;
	if (max_load <= 0.0f) return;
	vec3 color = {0.4,0.2,0.9};
	int num_divisions = 8;
	object->vertexCount = 8*num_divisions*3;
	object->vertices = (float*)malloc(sizeof(float)*object->vertexCount*6);
	int directions = 0;
	frameElement* element = &(model->elements[tload->element]);
	Node* node1 = &(model->nodes[element->node1]);
	Node* node2 = &(model->nodes[element->node2]);
	vec3 element_dir = {node2->x-node1->x,node2->y-node1->y,node2->z-node1->z};
	float Le = sqrtf(element_dir[0]*element_dir[0]+element_dir[1]*element_dir[1]+element_dir[2]*element_dir[2]);
	glm_vec3_normalize(element_dir);
	int vidx = 0;
	const float S = 0.0028f;          // same as the structure
	
	float minLength = 0.075f / S;
	float maxLength = 0.25f / S;
	
	float h = 0.30f * (0.15f / S);
	float w = 0.12f * (0.15f / S);
	int arrow_count = 0;
	for (int d = 0; d < 3; d++) {
		if (fabsf(tload->w1[d]) < 1e-4f && fabsf(tload->w2[d]) < 1e-4f) continue;
		float x1 = tload->x1[d], x2 = tload->x2[d];
		if (x1 < 0.0f || x2 > Le * 1.0001f || x2 <= x1) continue;
		directions++;
		
		vec3 dir = {0, 0, 0};
		dir[d] = 1.0f;
		vec3 up = {0.0,1.0,0.0};
		if (fabsf(dir[1]) > 0.99f) {
			up[0] = 0.0f;
			up[1] = 0.0f;
			up[2] = 1.0f;
		}
		vec3 right;
		glm_vec3_cross(dir,up,right);
		glm_vec3_normalize(right);
		
		vec3 start_pos = {node1->x,node1->y,node1->z};
		start_pos[0]+= element_dir[0]*(tload->x1[d]);start_pos[1]+= element_dir[1]*(tload->x1[d]);start_pos[2]+= element_dir[2]*(tload->x1[d]);
		vec3 end_pos = {node1->x,node1->y,node1->z};
		end_pos[0]+= element_dir[0]*(tload->x2[d]);end_pos[1]+= element_dir[1]*(tload->x2[d]);end_pos[2]+= element_dir[2]*(tload->x2[d]);
		vec3 temp;
		glm_vec3_sub(end_pos,start_pos,temp);
		float L = sqrtf(temp[0]*temp[0]+temp[1]*temp[1]+temp[2]*temp[2]);
		float step = (num_divisions > 1) ? L/(float)(num_divisions-1) : 0.0f;
		for(int i=0;i<num_divisions;i++){
			float t     = (num_divisions > 1) ? (float)i / (num_divisions - 1) : 0.0f;
			float load  = tload->w1[d] + (tload->w2[d] - tload->w1[d]) * t;
			if (fabsf(load) < 1e-6f) continue;
			arrow_count++;
			float ratio = (max_load > 0.0f) ? fabsf(load) / max_load : 0.0f;
			float scale = minLength + ratio * (maxLength - minLength);
			float x = x1 + t * (x2 - x1);                 // distance from node 1
			vec3 pos = { 	node1->x + element_dir[0]*x,
							node1->y + element_dir[1]*x,
							node1->z + element_dir[2]*x 
			};
			vec3 d3 = {0.0,0.0,0.0};			
			d3[d] = (load<0)?-1.0 : 1.0;
			vec3 r3;
			glm_vec3_cross(d3,up,r3);
			glm_vec3_normalize(r3);
			createArrowVertices(object->vertices,&vidx,color,scale,w,h,pos,d3,r3);
			start_pos[0]+= element_dir[0]*step;
			start_pos[1]+= element_dir[1]*step;
			start_pos[2]+= element_dir[2]*step;
			
		}
    }
    
    int actual_count = 8*arrow_count;
    if (actual_count <= 0) {free(object->vertices); object->vertices = NULL; object->vertexCount = 0; return; }

	float* tmp = (float*)realloc(object->vertices, sizeof(float)*actual_count*6);
	if (!tmp) {
		fprintf(stderr, "ERROR: out of memory for load arrow vertices\n");
		exit(41);
	}
	object->vertexCount = actual_count;
	object->vertices = tmp;
    
    object->scale[0] = object->scale[1] = object->scale[2] = S;   /* 2 / 720 */
	object->pos[0] = -1.0f;
	object->pos[1] = -0.168f;
	object->pos[2] = 0.0f;

	glm_mat4_identity(object->model);
	glm_translate(object->model, object->pos);
	glm_scale(object->model, object->scale);
	object->angle = 0.0;
	BindObjectBuffers(object);
}

void freeWorldStructure(World *world) {
    if (!world) return;

    // Structure object
    free_object(&world->structureObject);

    // Load objects
    freeLoadObjects(&world->loadObjects);

    // Support objects
    for (int i = 0; i < world->supprtObjectCount; i++)
        free_object(&world->supportObjects[i]);

    free(world->supportObjects);
    world->supportObjects = NULL;
    world->supprtObjectCount = 0;

    // Structure tube objects
    for (int i = 0; i < world->structureTubeObjectCount; i++)
        free_object(&world->structureTubeObjects[i]);

    free(world->structureTubeObjects);
    world->structureTubeObjects = NULL;
    world->structureTubeObjectCount = 0;
}

void freeLoadObjects(LoadObjects *loads) {
    if (!loads) return;

    for (int i = 0; i < loads->nodalCount; i++)
        free_object(&loads->nodal[i]);

    for (int i = 0; i < loads->uniformCount; i++)
        free_object(&loads->uniform[i]);

    for (int i = 0; i < loads->trapCount; i++)
        free_object(&loads->trap[i]);

    for (int i = 0; i < loads->pointCount; i++)
        free_object(&loads->point[i]);

    free(loads->nodal);
    free(loads->nodalMoment);
    free(loads->uniform);
    free(loads->trap);
    free(loads->point);

    memset(loads, 0, sizeof *loads);
}

void free_object(Object *o) {
    if (!o) return;
    if (o->vao) glDeleteVertexArrays(1, &o->vao);
    if (o->vbo) glDeleteBuffers(1, &o->vbo);
    free(o->vertices);          // free(NULL) is safe
    memset(o, 0, sizeof *o);    // so a second call does nothing
}

void free_object_array(Object **arr, int *count) {
    for (int i = 0; i < *count; i++) free_object(&(*arr)[i]);
    free(*arr);
    *arr = NULL;
    *count = 0;
}

void free_loads(LoadObjects *l) {
    free_object_array(&l->nodal,   &l->nodalCount);
    free_object_array(&l->uniform, &l->uniformCount);
    free_object_array(&l->trap,    &l->trapCount);
    free_object_array(&l->point,   &l->pointCount);
}

void changeLoadCase(World* world, int newCase){
	if (newCase < 0 || newCase > world->model.caseCount - 1) return;

	free_loads(&(world->loadObjects));
	world->load_case = newCase;
	staticLoadCase *lc = &world->model.cases[newCase];

	world->loadObjects.nodalCount = lc->pointLoadCount;
	world->loadObjects.nodal = (Object*)calloc(lc->pointLoadCount, sizeof(Object));
	world->loadObjects.nodalMoment = (Object*)calloc(world->loadObjects.nodalCount, sizeof(Object));
	for (int i = 0; i < lc->pointLoadCount; i++)
		createPointLoadObject(&world->loadObjects.nodal[i], &world->model, &lc->ploads[i],world->load_case);
		
	for (int i = 0; i < world->loadObjects.nodalCount; i++) {
		createNodalMomentObject(&(world->loadObjects.nodalMoment[i]), &(world->model), &(lc->ploads[i]),world->load_case);
	}

	world->loadObjects.uniformCount = lc->uniformLoadCount;
	world->loadObjects.uniform = (Object*)calloc(lc->uniformLoadCount, sizeof(Object));
	for (int i = 0; i < lc->uniformLoadCount; i++)
		createUniformLoadObject(&world->loadObjects.uniform[i], &world->model, &lc->uloads[i],world->load_case);

	world->loadObjects.trapCount = lc->trapLoadCount;
	world->loadObjects.trap = (Object*)calloc(lc->trapLoadCount, sizeof(Object));
	for (int i = 0; i < lc->trapLoadCount; i++)
		createTrapezoidLoadObject(&world->loadObjects.trap[i], &world->model, &lc->tloads[i],world->load_case);
		
}


void OpenGlShaders(unsigned int* V,unsigned int* F,unsigned int* P,const char* VCodeAdr,const char* FCodeAdr){
  int size = 0;
  LinkVShader(V,VCodeAdr);
  LinkFrShader(F,FCodeAdr);
  LinkPShader(V,F,P);
}

void freeLoadCase(staticLoadCase* lc)
{
    free(lc->ploads);     lc->ploads = NULL;     lc->pointLoadCount = 0;
    free(lc->uloads);     lc->uloads = NULL;     lc->uniformLoadCount = 0;
    free(lc->tloads);     lc->tloads = NULL;     lc->trapLoadCount = 0;
    free(lc->iloads);     lc->iloads = NULL;     lc->internalLoadCount = 0;
    free(lc->temp_loads); lc->temp_loads = NULL; lc->tempLoadCount = 0;
    free(lc->predis);     lc->predis = NULL;     lc->preDisCount = 0;
}

void freeModel(structureModel* m)
{
    if (!m) return;

    for (int i = 0; i < m->caseCount; i++)      // guess: loadCaseCount
        freeLoadCase(&m->cases[i]);             // guess: loadCases
    free(m->cases);    m->cases = NULL;    m->caseCount = 0;

    free(m->nodes);        m->nodes = NULL;        m->nodeCount = 0;        // guess: nodes
    free(m->elements);     m->elements = NULL;     m->elementCount = 0;     // guess: elements
    // add free() for any other arrays the model owns (supports, sections, materials...)
}

void createStructureObjects(World* world){
	world->solved = 0;
	vec3 color = {0.1,0.4,1.0};
	world->structureObject.vertexCount = world->model.elementCount*2;
	world->structureObject.vertices = (float*)malloc(sizeof(float)*world->structureObject.vertexCount*6);
	int vidx = 0;
	for(int i=0;i<world->model.elementCount;i++){
		int node1 = world->model.elements[i].node1;
		int node2 = world->model.elements[i].node2;
		world->structureObject.vertices[vidx] = world->model.nodes[node1].x;
		world->structureObject.vertices[vidx+1] = world->model.nodes[node1].y;
		world->structureObject.vertices[vidx+2] = world->model.nodes[node1].z;
		world->structureObject.vertices[vidx+3] = color[0];
		world->structureObject.vertices[vidx+4] = color[1];
		world->structureObject.vertices[vidx+5] = color[2];
		vidx+=6;
		world->structureObject.vertices[vidx] = world->model.nodes[node2].x;
		world->structureObject.vertices[vidx+1] = world->model.nodes[node2].y;
		world->structureObject.vertices[vidx+2] = world->model.nodes[node2].z;
		world->structureObject.vertices[vidx+3] = color[0];
		world->structureObject.vertices[vidx+4] = color[1];
		world->structureObject.vertices[vidx+5] = color[2];
		vidx+=6;
	}
	
	
	BindObjectBuffers(&(world->structureObject));
	
	world->structureTubeObjectCount = world->model.elementCount;
	world->structureTubeObjects = (Object*)malloc(sizeof(Object)*world->structureTubeObjectCount);
	for(int i=0;i<world->structureTubeObjectCount;i++){
		int n1 = world->model.elements[i].node1;
		int n2 = world->model.elements[i].node2;
		Node node1 = world->model.nodes[n1];
		Node node2 = world->model.nodes[n2];
		vec3 start = {node1.x,node1.y,node1.z};
		vec3 end = {node2.x,node2.y,node2.z};
		createElementTube(&(world->model),&(world->structureTubeObjects[i]),start,end,world->model.elements[i].S);
		BindObjectBuffersNormal(&(world->structureTubeObjects[i]));
	}
	
	world->deformedTubes = NULL;
	world->deformedTubesCount = 0;
	
	const staticLoadCase *c = &world->model.cases[world->load_case];

	world->loadObjects.nodalCount = c->pointLoadCount;      // was nodalLoadCount
	world->loadObjects.nodal = (Object*)calloc(world->loadObjects.nodalCount, sizeof(Object));
	world->loadObjects.nodalMoment = (Object*)calloc(world->loadObjects.nodalCount, sizeof(Object));
	for (int i = 0; i < world->loadObjects.nodalCount; i++) {
		createPointLoadObject(&(world->loadObjects.nodal[i]), &(world->model), &(c->ploads[i]),world->load_case);
	}
	for (int i = 0; i < world->loadObjects.nodalCount; i++) {
		createNodalMomentObject(&(world->loadObjects.nodalMoment[i]), &(world->model), &(c->ploads[i]),world->load_case);
	}

	world->loadObjects.uniformCount = c->uniformLoadCount;
	world->loadObjects.uniform = (Object*)calloc(world->loadObjects.uniformCount, sizeof(Object));
	for (int i = 0; i < world->loadObjects.uniformCount; i++) {
		createUniformLoadObject(&(world->loadObjects.uniform[i]), &(world->model), &(c->uloads[i]),world->load_case);
	}

	world->loadObjects.trapCount = c->trapLoadCount;
	world->loadObjects.trap = (Object*)calloc(world->loadObjects.trapCount, sizeof(Object));
	for (int i = 0; i < world->loadObjects.trapCount; i++) {
		createTrapezoidLoadObject(&(world->loadObjects.trap[i]), &(world->model), &(c->tloads[i]),world->load_case);
	}

	world->loadObjects.pointCount = 0;
	world->loadObjects.point = NULL;
	
	world->supprtObjectCount = world->model.supportCount;
	world->supportObjects = (Object*)calloc(world->supprtObjectCount,sizeof(Object));
	float size = fminf(0.04f * world->model.modelSize, 0.5f * world->model.Lmin);
	for(int i=0;i<world->supprtObjectCount;i++){
		createSupportObject(&(world->supportObjects[i]),&world->model,world->model.supports[i].node, world->model.supports[i].r,size) ;
	}
}

void createDeformedTubes(World* world){
	float scale = 0.1f * world->model.modelSize / world->model.cases[world->load_case].maxDisp;
	
	world->deformedTubesCount = world->model.elementCount;
	world->deformedTubes = (Object*)calloc(world->deformedTubesCount,sizeof(Object));
	for(int i=0;i<world->deformedTubesCount;i++){
		int n1 = world->model.elements[i].node1;
		int n2 = world->model.elements[i].node2;
		Node node1 = world->model.nodes[n1];
		Node node2 = world->model.nodes[n2];
		vec3 start = {node1.x,node1.y,node1.z};
		vec3 end = {node2.x,node2.y,node2.z};
		for(int j=0;j<3;j++){
			start[j] += world->result.gdisp[world->load_case*world->model.nodeCount+n1][j]*scale;
			end[j] += world->result.gdisp[world->load_case*world->model.nodeCount+n2][j]*scale;
		}
		createElementTube(&(world->model),&(world->deformedTubes[i]),start,end,world->model.elements[i].S);
		BindObjectBuffersNormal(&(world->deformedTubes[i]));
	}
}

void createSupportObject(Object* object, structureModel* model, int node, const int r[6], float size) {
    int nt = r[0] + r[1] + r[2];
    int nr = r[3] + r[4] + r[5];
    if (nt + nr == 0) {
        object->vertexCount = 0;
        object->vertices = NULL;
        return;
    }
    object->vertices = (float*)malloc(sizeof(float) * 30 * 6);   // 30 vertices is the maximum any shape uses
    object->angle = 0.0;

    Node* nd = &(model->nodes[node]);
    vec3 n = { (float)nd->x, (float)nd->y, (float)nd->z };
    int vi = 0;

    if (nt == 3 && nr == 3) {
        vec3 col = {1.0f, 0.55f, 0.10f};  addBox(object->vertices, &vi, n, size, col);
    } else if (nt == 3 && nr == 0) {
        vec3 col = {0.20f, 0.85f, 0.30f}; addPyramid(object->vertices, &vi, n, size, col);
    } else if (nr == 0) {
        vec3 col = {0.20f, 0.75f, 1.00f};
        addPyramid(object->vertices, &vi, n, size, col);
        addPlate(object->vertices, &vi, n, size, col);
    } else {
        vec3 col = {0.85f, 0.40f, 0.95f}; addGeneral(object->vertices, &vi, n, size, r, col);
    }

    object->vertexCount = vi / 6;
    BindObjectBuffers(object);
}

void initWorld(World* world,int width,int height){
	initPaths();
	if(!find_frame3dd(NULL,world->solver_path,sizeof(world->solver_path))){
		world->solver_found = 0;
	}
	memset(&(world->input),0,sizeof(world->input));
	world->input.first_mouse_press = 1;
	world->solver_found = 1;
	printf("found frame3dd : %s \n",world->solver_path);
	world->view_state |= WASD;
	world->solved = 0;
	world->model_loaded = 0;
	world->width = width;
	world->height = height;
	world->render_flags = SHOW_NODES | SHOW_ELEMENTS | SHOW_SUPPORTS | SHOW_ALL_LOADS;
	world->load_case = 0;

	initCamera(&(world->camera),90.0,0.0);
	glm_perspective(MyRadians(60.0),((float)width/(float)height), 0.01f,100.0f,world->proj);
	
	int size;
	char* vCode = pathJoin(SHADERS,"vertex.txt",strlen("vertex.txt"),&size);
	char* vFragment = pathJoin(SHADERS,"fragment.txt",strlen("fragment.txt"),&size);
	char* vCodeT = pathJoin(SHADERS,"vertexT.txt",strlen("vertexT.txt"),&size);
	char* vFragmentT = pathJoin(SHADERS,"fragmentT.txt",strlen("fragmentT.txt"),&size);
	OpenGlShaders(&world->vertexShader,&world->fragmentShader,&world->programShader,vCode,vFragment);
	OpenGlShaders(&world->vertexShaderT,&world->fragmentShaderT,&world->programShaderT,vCodeT,vFragmentT);
	
	memset(&world->model, 0, sizeof world->model);
	memset(&world->structureObject,0,sizeof(world->structureObject));
	
	glm_vec4_zero(world->structure_quaternion);
	world->structure_quaternion[0] = 1.0;
	
	world->render_flags = SHOW_NODES | SHOW_ELEMENTS | SHOW_SUPPORTS
            | SHOW_POINT_LOADS | SHOW_UNIFORM_LOADS | SHOW_TRAP_LOADS;
}

void updateWorld(World* world){
	if(world->view_state & WASD){
		world->camera.yaw += -world->input.mouseDeltaX;
		world->camera.pitch += -world->input.mouseDeltaY;
		if (world->camera.pitch >  89.0f) world->camera.pitch =  89.0f;
        if (world->camera.pitch < -89.0f) world->camera.pitch = -89.0f;
	}else if(world->view_state & ROTATE_OBJECT){
		float sens = 0.9;
		rotateStructure(world,world->input.mouseDeltaY*sens,world->camera.right);
		rotateStructure(world,world->input.mouseDeltaX*sens,world->camera.up);
	}
	for(int i=0;i<3;i++){
		world->camera.pos[i] += (world->camera.forward[i]*world->input.moveForward)+(world->camera.right[i]*world->input.moveRight); 
	}
	updateCamera(&world->camera);
}

void UpdateObjectVertices(Object *o,float *verts, int vertexCount) {
    glBindBuffer(GL_ARRAY_BUFFER, o->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(sizeof(float) * vertexCount * 6), verts);
}

void updateStructureObject(World* world){
	float scale = 0.1f * world->model.modelSize / world->model.cases[world->load_case].maxDisp;
	printf("!!!!!!!!!!!!!!!!!!!!!!\n LOOK HERE : \n");
	printf("Load case: %d\n", world->load_case);
	printf("Max displacement: %f\n",
       world->model.cases[world->load_case].maxDisp);
    printf("Model Size : %f \n",world->model.modelSize);
	printf("Scale: %f\n", scale);

	int vidx = 0;
	if(world->render_flags & SHOW_DEFORMED){
		vec3 color = {0.6,0.4,1.0};
		for(int i=0;i<world->model.elementCount;i++){
			int node1 = world->model.elements[i].node1;
			int node2 = world->model.elements[i].node2;
			world->structureObject.vertices[vidx]   = world->model.nodes[node1].x + world->result.gdisp[world->load_case*world->model.nodeCount+node1][0]*scale;
			world->structureObject.vertices[vidx+1] = world->model.nodes[node1].y + world->result.gdisp[world->load_case*world->model.nodeCount+node1][1]*scale;
			world->structureObject.vertices[vidx+2] = world->model.nodes[node1].z + world->result.gdisp[world->load_case*world->model.nodeCount+node1][2]*scale;
			world->structureObject.vertices[vidx+3] = color[0];
			world->structureObject.vertices[vidx+4] = color[1];
			world->structureObject.vertices[vidx+5] = color[2];
			vidx+=6;
			world->structureObject.vertices[vidx]   =  world->model.nodes[node2].x + world->result.gdisp[world->load_case*world->model.nodeCount+node2][0]*scale;
			world->structureObject.vertices[vidx+1] =  world->model.nodes[node2].y + world->result.gdisp[world->load_case*world->model.nodeCount+node2][1]*scale;
			world->structureObject.vertices[vidx+2] =  world->model.nodes[node2].z + world->result.gdisp[world->load_case*world->model.nodeCount+node2][2]*scale;
			world->structureObject.vertices[vidx+3] = color[0];
			world->structureObject.vertices[vidx+4] = color[1];
			world->structureObject.vertices[vidx+5] = color[2];
			vidx+=6;
		}
	}else{
		vec3 color = {0.0,0.4,1.0};
		for(int i=0;i<world->model.elementCount;i++){
			int node1 = world->model.elements[i].node1;
			int node2 = world->model.elements[i].node2;
			world->structureObject.vertices[vidx]   = world->model.nodes[node1].x ;
			world->structureObject.vertices[vidx+1] = world->model.nodes[node1].y ;
			world->structureObject.vertices[vidx+2] = world->model.nodes[node1].z ;
			world->structureObject.vertices[vidx+3] = color[0];
			world->structureObject.vertices[vidx+4] = color[1];
			world->structureObject.vertices[vidx+5] = color[2];
			vidx+=6;
			world->structureObject.vertices[vidx]   = world->model.nodes[node2].x ;
			world->structureObject.vertices[vidx+1] = world->model.nodes[node2].y ;
			world->structureObject.vertices[vidx+2] = world->model.nodes[node2].z ;
			world->structureObject.vertices[vidx+3] = color[0];
			world->structureObject.vertices[vidx+4] = color[1];
			world->structureObject.vertices[vidx+5] = color[2];
			vidx+=6;
		}
	}
	Object *o = &world->structureObject;

	o->scale[0] = o->scale[1] = o->scale[2] = 0.0028f;   /* 2 / 720 */
	o->pos[0] = -1.0f;      /* -360 * 0.0028 */
	o->pos[1] = -0.168f;    /* -60 * 0.0028 */
	o->pos[2] = 0.0f;
	o->angle = 0.0f;
	
	glm_mat4_identity(o->model);
	glm_translate(o->model, o->pos);
	glm_rotate(o->model, o->angle, (vec3){0, 1, 0});
	glm_scale(o->model, o->scale);
	o->angle = 0.0;
	UpdateObjectVertices(o,o->vertices,o->vertexCount);
}



void updateMode(World* world, double current_time){
	while(world->load_case >= world->model.caseCount){
		world->load_case--;
	}
	int current_mode = world->current_mode;
	structureModel *m = &world->model;
	if(!m->modes || !m->modes[current_mode].disp) return;

	double dt = current_time - world->mode_start_time;

	// model size (bounding box diagonal)
	float mn[3] = { 1e30f, 1e30f, 1e30f }, mx[3] = { -1e30f, -1e30f, -1e30f };
	for(int i = 0; i < m->nodeCount; i++){
		float p[3] = { m->nodes[i].x, m->nodes[i].y, m->nodes[i].z };
		for(int j = 0; j < 3; j++){
			if(p[j] > mx[j]) mx[j] = p[j];
			if(p[j] < mn[j]) mn[j] = p[j];
		}
	}
	float dx = mx[0]-mn[0], dy = mx[1]-mn[1], dz = mx[2]-mn[2];
	float modelSize = sqrtf(dx*dx + dy*dy + dz*dz);

	// largest displacement in this mode
	float maxDisp = 0.0f;
	for(int i = 0; i < m->nodeCount; i++){
		double *d = m->modes[current_mode].disp[i];
		float len = (float)sqrt(d[0]*d[0] + d[1]*d[1] + d[2]*d[2]);
		if(len > maxDisp) maxDisp = len;
	}

	float baseAmplitude = (maxDisp > 1e-12f) ? 0.1f * modelSize / maxDisp : 0.0f;
	float modeExag = 1.0f;
	float speed = 0.1f;
	float w = 2.0f * PI * (float)m->modes[current_mode].frequency * speed;
	float A = baseAmplitude * modeExag * sinf(w * (float)dt);

	vec3 color = {0.2f, 0.2f, 1.0f};
	int vidx = 0;
	for(int i = 0; i < m->elementCount; i++){
		int ends[2] = { m->elements[i].node1, m->elements[i].node2 };
		for(int k = 0; k < 2; k++){
			int n = ends[k];
			const double *d = m->modes[current_mode].disp[n];
			world->structureObject.vertices[vidx++] = m->nodes[n].x + A * (float)d[0];
			world->structureObject.vertices[vidx++] = m->nodes[n].y + A * (float)d[1];
			world->structureObject.vertices[vidx++] = m->nodes[n].z + A * (float)d[2];
			world->structureObject.vertices[vidx++] = color[0];
			world->structureObject.vertices[vidx++] = color[1];
			world->structureObject.vertices[vidx++] = color[2];
		}
	}

	Object *o = &world->structureObject;
	o->scale[0] = o->scale[1] = o->scale[2] = 0.0028f;
	o->pos[0] = -1.0f;
	o->pos[1] = -0.168f;
	o->pos[2] = 0.0f;
	o->angle = 0.0f;
	glm_mat4_identity(o->model);
	glm_translate(o->model, o->pos);
	glm_rotate(o->model, o->angle, (vec3){0, 1, 0});
	glm_scale(o->model, o->scale);

	UpdateObjectVertices(o, o->vertices, o->vertexCount);
}

