
#include "platform.h"


#define MOVE_SPEED 0.1
#define KEYBOARD_SENSITIVITY 0.5
#define MOUSE_SENSITIVITY 0.3

void framebuffer_size_callback(GLFWwindow* window, int w, int h) {
	World* world = glfwGetWindowUserPointer(window);
    (void)window;
    /*int vx, vy, vw, vh;
	ui_get_viewport(&vx, &vy, &vw, &vh);
	glViewport(vx, vy, vw, vh);

	static int last_w = 0, last_h = 0;
	if (vw != last_w || vh != last_h) {                  // only rebuild when the size changes
		last_w = vw;  last_h = vh;
		glm_perspective(MyRadians(60.0), (float)vw / (float)vh, 0.1f, 100.0f, world->proj);
	}*/
	
    world->width  = (float)w;
    world->height = (float)h;
    if (h == 0) return;
    glm_perspective(MyRadians(60.0),((float)w/(float)h), 0.01f,100.0f,world->proj);
    glViewport(0, 0, w, h);
}

void keyboardUpdate(World* world){
	 world->input.moveForward = 0.0f;
	 world->input.moveRight = 0.0f;
	 if(world->view_state & WASD){
		if(world->input.keys & INPUT_W){
			world->input.moveForward = MOVE_SPEED*KEYBOARD_SENSITIVITY;
		}
		if(world->input.keys & INPUT_S){
			world->input.moveForward = -MOVE_SPEED*KEYBOARD_SENSITIVITY;
		}
		if(world->input.keys & INPUT_D){
			world->input.moveRight = MOVE_SPEED*KEYBOARD_SENSITIVITY;
		}
		if(world->input.keys & INPUT_A){
			world->input.moveRight = -MOVE_SPEED*KEYBOARD_SENSITIVITY;
		}
	 }
}

static void keyCallBack(GLFWwindow* window, int button,int scanecode, int action, int mods){
	if(ui_wants_keyboard()){
		return;
	}
	int press = (action==GLFW_PRESS);
	int release = (action==GLFW_RELEASE);
	World* world = (World*)glfwGetWindowUserPointer(window);
	switch(button){
		case GLFW_KEY_W:
		{
			if(press){
				world->input.keys |= INPUT_W;
			}else if(release){
				world->input.keys &= ~INPUT_W;
			}
		}break;
		case GLFW_KEY_A:
		{
			if(press){
				world->input.keys |= INPUT_A;
			}else if(release){
				world->input.keys &= ~INPUT_A;
			}
		}break;
		case GLFW_KEY_S:
		{
			if(press){
				world->input.keys |= INPUT_S;
			}else if(release){
				world->input.keys &= ~INPUT_S;
			}
		}break;
		case GLFW_KEY_D:
		{
			if(press){
				world->input.keys |= INPUT_D;
			}else if(release){
				world->input.keys &= ~INPUT_D;
			}
		}break;
	}
	keyboardUpdate(world);
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos){
	World* world = (World*)glfwGetWindowUserPointer(window);
	world->input.mouseDeltaX = 0.0f;
	world->input.mouseDeltaY = 0.0f;
	if(world->input.keys & INPUT_LEFT_MOUSE){
		if(world->input.first_mouse_press){
			world->input.mouse_xpos = xpos;
			world->input.mouse_ypos = ypos;
			world->input.first_mouse_press = 0;
			return;
		}
		world->input.mouseDeltaX = xpos - world->input.mouse_xpos;
		world->input.mouseDeltaY = ypos - world->input.mouse_ypos;
		world->input.mouseDeltaX *= MOUSE_SENSITIVITY;
		world->input.mouseDeltaY *= MOUSE_SENSITIVITY;
		world->input.mouse_xpos = xpos;
		world->input.mouse_ypos = ypos;
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if(ui_wants_mouse()){
		return;
	}
  World* world = (World*)glfwGetWindowUserPointer(window);
  if (ui_wants_mouse()) return;
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
	world->input.keys |= INPUT_LEFT_MOUSE;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    world->input.mouse_xpos = xpos;
    world->input.mouse_ypos = ypos;
    world->input.first_mouse_press = 0;
  }
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
    world->input.keys &= ~INPUT_LEFT_MOUSE;
    world->input.first_mouse_press = 1;
  }
}


GLFWwindow*  createWindow(World* world,int width,int height){
	#if __linux__ && defined(GLFW_PLATFORM_X11)
	if (getenv("WSL_DISTRO_NAME")) glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);   
	#endif
	if (!glfwInit()) {
        fprintf(stderr, "Failed to init GLFW\n");
        exit(41);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	glfwWindowHint(GLFW_SAMPLES, 4); 
	GLFWwindow* window = glfwCreateWindow(width, height, "Frame3dd Renderer", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        glfwTerminate();
        exit(41);
    }
    glfwMakeContextCurrent(window);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to init GLAD\n");
        exit(41);
    }
    glEnable(GL_MULTISAMPLE);
    
    glfwSetWindowUserPointer(window, world);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetKeyCallback(window,keyCallBack);
	glfwSetCursorPosCallback(window,cursor_pos_callback);
	
	return window;
}
