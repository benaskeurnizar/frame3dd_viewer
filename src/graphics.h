#pragma once

#include <cglm/cglm.h>

#define MAX_PATH 256

typedef enum {
    // Structure
    SHOW_NODES           = 1 << 0,
    SHOW_ELEMENTS        = 1 << 1,
    SHOW_SUPPORTS        = 1 << 2,
    SHOW_LOCAL_AXES      = 1 << 3,
    SHOW_GLOBAL_AXES     = 1 << 4,
    SHOW_NODE_LABELS     = 1 << 5,
    SHOW_ELEMENT_LABELS  = 1 << 6,
    SHOW_RELEASES        = 1 << 7,

    // Loads
    SHOW_POINT_LOADS     = 1 << 8,
    SHOW_UNIFORM_LOADS   = 1 << 9,
    SHOW_TRAP_LOADS      = 1 << 10,
    SHOW_INTERNAL_LOADS  = 1 << 11,
    SHOW_TEMP_LOADS      = 1 << 12,
    SHOW_PRESCRIBED_DISP = 1 << 13,
    SHOW_GRAVITY         = 1 << 14,

    // Results
    SHOW_DEFORMED        = 1 << 15,
    SHOW_UNDEFORMED      = 1 << 16,   // overlay under the deformed shape
    SHOW_REACTIONS       = 1 << 17,
    SHOW_MODE_SHAPE      = 1 << 18,   // animated mode

    // Force diagrams
    SHOW_AXIAL_FORCE     = 1 << 19,
    SHOW_SHEAR_VY        = 1 << 20,
    SHOW_SHEAR_VZ        = 1 << 21,
    SHOW_TORQUE_MX       = 1 << 22,
    SHOW_MOMENT_MY       = 1 << 23,
    SHOW_MOMENT_MZ       = 1 << 24,

    // Color maps
    COLOR_BY_AXIAL       = 1 << 25,
    COLOR_BY_MOMENT      = 1 << 26,
    COLOR_BY_DISPLACEMENT= 1 << 27,

    // Handy groups
    SHOW_ALL_LOADS    = SHOW_POINT_LOADS | SHOW_UNIFORM_LOADS | SHOW_TRAP_LOADS |
                        SHOW_INTERNAL_LOADS | SHOW_TEMP_LOADS | SHOW_PRESCRIBED_DISP |
                        SHOW_GRAVITY,
    SHOW_ALL_DIAGRAMS = SHOW_AXIAL_FORCE | SHOW_SHEAR_VY | SHOW_SHEAR_VZ |
                        SHOW_TORQUE_MX | SHOW_MOMENT_MY | SHOW_MOMENT_MZ,
} RenderFlags;



typedef struct{
	vec3 pos;
	vec3 forward;
	vec3 up;
	vec3 right;
	mat4 view;
	float yaw,pitch;
}Camera;

typedef struct{
	float* vertices;
	int vertexCount;
	unsigned int vao,vbo;
	mat4 model;
	vec3 pos,scale;
	float angle;
}Object;

typedef struct{
	int id;
	double x,y,z,r;
}Node;

typedef struct{
	int id;
	int node1;
	int node2;
	double S;
}frameElement;

typedef struct{
	int node;
	double fx,fy,fz;
	double mx,my,mz;
}pointLoad;

typedef struct{
	int element;
	double ux,uy,uz;
}UniformLoad;

typedef struct{
	int element;
	double x1[3],x2[3],w1[3],w2[3];
}TrapizoidalLoad;

typedef struct{
    int   element;   // element number from the file
    double  px,py,pz;         // load in local x, y, z (N)
    double x;         // distance from node1 along the element
}InternalLoad;

typedef struct{
	int element;
	double a;
	double hy,hz;
	double typ,tym,tzp,tzm;
}TempLoad;

typedef struct{
	int node;
	double dx,dy,dz,dxx,dyy,dzz;
}PreDisplacement;

typedef struct{
	double gx,gy,gz;
	pointLoad* ploads;
	int pointLoadCount;
	UniformLoad* uloads;
	int uniformLoadCount;
	TrapizoidalLoad* tloads;
	int trapLoadCount;
	InternalLoad* iloads;
	int internalLoadCount;
	TempLoad* temp_loads;
	int tempLoadCount;
	PreDisplacement* predis;
	int preDisCount;
	double maxDisp;
	double maxPoint;
	double maxMoment;
	double maxUniform;
	double maxTrap;
	double maxInternal;
}staticLoadCase;

typedef struct{
	int id;
	double frequency;
	double (*disp)[6];
}Mode;


typedef struct{
	float min[3];
	float max[3];
}BoundingBox;

typedef enum {
    SUPPORT_NONE = 0,   // nothing restrained
    SUPPORT_FIXED,      // 3 translations + 3 rotations   (box, orange)
    SUPPORT_PINNED,     // 3 translations, no rotations   (pyramid, green)
    SUPPORT_ROLLER,     // 1 or 2 translations, no rotations (pyramid + plate, cyan)
    SUPPORT_GENERAL     // any other mix                  (lines and rings, magenta)
} SupportType;

typedef struct {
    int         node;       // 0-based index into model->nodes
    int         r[6];       // Rx Ry Rz Rxx Ryy Rzz, 1 = restrained
    SupportType type;       // derived from r[]
} Support;

typedef struct{
	Node* nodes;
	int nodeCount;
	frameElement* elements;
	int elementCount;
	staticLoadCase* cases;
	int caseCount;
	int mode_count;
	Mode* modes;
	Support* supports;
	int supportCount;
	
	BoundingBox box;
	float modelSize;
	float Lmin;
	float Lmax;
	float Lavg;
	float viewScale;
}structureModel;

typedef struct {
    Object *nodal,*nodalMoment, *uniform, *trap, *point;   // each has its own VAO/VBO and vertexCount
    int nodalCount,uniformCount,trapCount,pointCount;
} LoadObjects;

typedef struct{
	int nCases,nNodes;
	double (*gdisp)[6];
}Results;

typedef enum{
  INPUT_W           = 1u << 0,
  INPUT_A           = 1u << 1,
  INPUT_S           = 1u << 2,
  INPUT_D           = 1u << 3,
  INPUT_SPACE       = 1u << 4,
  INPUT_SHIFT       = 1u << 5,
  INPUT_LEFT_MOUSE  = 1u << 6,
  INPUT_RIGHT_MOUSE = 1u << 7,
}InputKeys;


typedef struct Input {
  // Button/key state
  uint32_t keys;

  // Current mouse positions
  double mouse_xpos,mouse_ypos;
  int first_mouse_press;
  
  // Mouse movement this frame
  double mouseDeltaX;
  double mouseDeltaY;

  // Movement input this frame (-1..1)
  float moveForward;
  float moveRight;
} InputState;

typedef enum{
	WASD = 1u<<0,
	ROTATE_OBJECT = 1u<<1,
}ViewState;

typedef struct{
	float width,height;
	mat4 proj;
	Camera camera;
	vec3 scale;
	vec3 pos;
	mat4 structure_model;
	vec4 structure_quaternion;
	
	unsigned int vertexShader,fragmentShader,programShader;
	unsigned int vertexShaderT,fragmentShaderT,programShaderT;
	Object structureObject;
	LoadObjects loadObjects;
	Object* supportObjects;
	int supprtObjectCount;
	
	Object* structureTubeObjects;
	int structureTubeObjectCount;
	
	Object* deformedTubes;
	int deformedTubesCount;
	
	structureModel model;
	
	InputState input;
	
	
	unsigned int view_state;
	unsigned int render_flags;
	int load_case;
	
	const char model_path[MAX_PATH];
	int model_loaded;
	const char solver_path[MAX_PATH];
	int solver_found;
	
	int solved;
	Results result;
	int modes_loaded;
	
	double mode_start_time;
	int in_mode_animation;
	
	int current_mode;
}World;

#ifdef __cplusplus
extern "C"{
#endif
float MyRadians(float angle) ;
void updateCamera(Camera* camera);
void initWorld(World* world,int width,int height);
void updateWorld(World* world);
void changeLoadCase(World* world,int newCase);
void freeModel(structureModel* m);
void free_object(Object *o);
void freeLoadObjects(LoadObjects *loads) ;
void freeWorldStructure(World *world);
void createStructureObjects(World* world);
void updateStructureObject(World* world);
void updateMode(World* world, double current_time);
void createSupportObject(Object* object, structureModel* model, int node, const int r[6], float size) ;
void createDeformedTubes(World* world);
#ifdef __cplusplus
}
#endif

