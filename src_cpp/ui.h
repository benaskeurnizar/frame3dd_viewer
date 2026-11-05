/* ui.h */
#ifndef UI_H
#define UI_H

#include <GLFW/glfw3.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "graphics.h"   /* World, RenderFlags */


enum { UI_REQ_OPEN = 1, UI_REQ_RELOAD = 2, UI_REQ_SOLVE = 4, UI_REQ_LC_CHANGED = 8 ,UI_REQ_MODE_CHANGED = 16};
typedef enum { UI_CAM_NONE = 0, UI_CAM_RESET, UI_CAM_FIT, UI_CAM_TOP, UI_CAM_FRONT, UI_CAM_SIDE, UI_CAM_ISO } UiCameraRequest;

void  ui_init(GLFWwindow* window);    /* register your GLFW callbacks BEFORE this call */
void  ui_begin_frame(void);
void  ui_draw(World* world);
void  ui_end_frame(void);
int   ui_wants_mouse(void);
int   ui_wants_keyboard(void);
void  ui_shutdown(void);

void  ui_status(const char* msg);     /* left side of the status bar */

/* Actions: read once per frame; each call returns what was requested and clears it. */
unsigned int ui_take_requests(void);  /* UI_REQ_* bits; the new load case is already in world->load_case */
int   ui_take_camera(void);           /* UiCameraRequest */

/* State your code pushes into the UI. */
void  ui_set_solved(int solved);                       /* enables Results and Modes */
void  ui_set_modes(int count, const double* freq_hz);  /* pointer must stay valid */
void  ui_set_selection(int kind, int id);              /* 0 none, 1 node, 2 element; id 0-based */

/* Values your renderer reads. */
float ui_line_width(void);
float ui_arrow_scale(void);
float ui_deform_scale(void);
float ui_diagram_scale(void);
int   ui_mode(void);
int   ui_mode_animate(void);
float ui_mode_speed(void);

float ui_panel_width(void);           /* left panel width, framebuffer pixels (0 when hidden) */

/* Free 3D rectangle, framebuffer pixels, bottom-left origin. Call after ui_draw() in the same frame. */
void  ui_get_viewport(int* x, int* y, int* w, int* h);

#ifdef __cplusplus
}
#endif
#endif
