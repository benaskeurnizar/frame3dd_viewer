#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "graphics.h"
#include "my_io.h"
#include "ui.h"
#include "solver.h"
#include "analysis_results.h"
#include "rendering.h"
#include "geometry.h"
#include "platform.h"

#include <string.h>


static void print_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { printf("(cannot open %s)\n", path); return; }
    char line[512];
    while (fgets(line, sizeof line, f)) fputs(line, stdout);
    fclose(f);
}

void reset_result_flags(World* world)
{
    world->render_flags &= ~(unsigned int)(SHOW_DEFORMED | SHOW_UNDEFORMED | SHOW_REACTIONS |
                                               SHOW_MODE_SHAPE | SHOW_ALL_DIAGRAMS |
                                               COLOR_BY_AXIAL | COLOR_BY_MOMENT | COLOR_BY_DISPLACEMENT);
}

int main(){
	int firstFrame = 1;
	World world;
	memset(&world, 0, sizeof world);
	int width = 800,height = 600;

	GLFWwindow* window = createWindow(&world,width,height);
	initWorld(&world,width,height);
	
	ui_init(window);
    
    printf("GL version: %s\n", glGetString(GL_VERSION));
    printf("Renderer:   %s\n", glGetString(GL_RENDERER));
    
    unsigned int modelLoc = glGetUniformLocation(world.programShader, "model");
	unsigned int viewLoc = glGetUniformLocation(world.programShader, "view");
	unsigned int projectionLoc = glGetUniformLocation(world.programShader, "proj");
	
	unsigned int modelLocT = glGetUniformLocation(world.programShaderT, "model");
	unsigned int viewLocT = glGetUniformLocation(world.programShaderT, "view");
	unsigned int projectionLocT = glGetUniformLocation(world.programShaderT, "proj");
	unsigned int camPosLoc = glGetUniformLocation(world.programShaderT, "camera_pos");
	if (modelLoc == -1 || viewLoc == -1 || projectionLoc == -1){
		fprintf(stderr, "A shader uniform was not found\n");
	}
	double last = glfwGetTime();
	uint32_t prev_flags = 0;
    while (!glfwWindowShouldClose(window)) {
		double now = glfwGetTime();
        float dt = (float) (last-now);
        last = now;
        updateWorld(&world);
         
		world.input.mouseDeltaX = 0.0;
		world.input.mouseDeltaY = 0.0;

		world.input.moveForward = 0.0f;
		world.input.moveRight = 0.0f;
        glClearColor(0.4f, 0.5f, 0.6f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        prev_flags = world.render_flags;
        ui_begin_frame();
		ui_draw(&world);
		
		
		unsigned int req = ui_take_requests();
		if (req & UI_REQ_LC_CHANGED){ 
			changeLoadCase(&world, world.load_case);
			updateStructureObject(&world);
			if(world.deformedTubesCount){
				for(int i=0;i<world.deformedTubesCount;i++){
					free_object(&(world.deformedTubes[i]));
				}
				free(world.deformedTubes);
			}
			createDeformedTubes(&world);
		}
		if (req & UI_REQ_MODE_CHANGED) {
			world.current_mode = ui_mode();   // use your real field name, 0-based
			// then update whatever depends on it, e.g. your mode shape function
		}
	
		if (req & UI_REQ_SOLVE) {
			char outDir[1024];
			int code = run_solver(world.solver_path, world.model_path, outDir, sizeof outDir);
			char path[1100];
			if (code == 0) {
				printf("solved, results in: %s\n", outDir);
				snprintf(path, sizeof path, "%s/model.out", outDir);
				printf("%s\n",path);
				print_file(path);
				if(loadResults(&(world.model),&(world.result),path)){
					printf("SOLVED !!!!\n");
					createDeformedTubes(&world);
					world.solved = 1;
					ui_set_solved(1);
					if(world.model.mode_count > 0){
						if(loadModes(&(world.model),path)){
							printf("MODES LOADED!! \n");
							world.modes_loaded = 1;
						}
					}
				}
			} else {
				printf("solver failed, code %d\n", code);
				snprintf(path, sizeof path, "%s/stderr.txt", outDir);
				print_file(path);                       // why it failed
			}
		}
		if (req & UI_REQ_OPEN) {
			ui_set_solved(0);
			reset_result_flags(&world);
			world.in_mode_animation = 0;
			if(world.model_loaded){
				freeModel(&(world.model));
				world.model_loaded = 0;
				freeWorldStructure(&world);
				if(world.deformedTubesCount){
					for(int i=0;i<world.deformedTubesCount;i++){
						free_object(&(world.deformedTubes[i]));
					}
					free(world.deformedTubes);
					world.deformedTubesCount = 0;
					world.deformedTubes = NULL;
				}
				world.result.nCases = 0;
				world.result.nNodes = 0;
				free(world.result.gdisp);
				world.result.gdisp = NULL;
			}
			world.load_case = 0;
			loadModel(world.model_path,&(world.model)) ;
			glm_vec4_zero(world.structure_quaternion);
			world.structure_quaternion[0] = 1.0;
			updateModelMatrix(world.structure_model,world.model.viewScale,(vec3){0.0,0.0,0.0},world.structure_quaternion);
			world.model_loaded = 1;   // use your function name and return convention
			createStructureObjects(&world);
		}
		glUseProgram(world.programShader);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (float *)world.camera.view);
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, (float *)world.proj);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float *)world.structure_model);
		
		glUseProgram(world.programShaderT);
		glUniformMatrix4fv(viewLocT, 1, GL_FALSE, (float *)world.camera.view);
		glUniformMatrix4fv(projectionLocT, 1, GL_FALSE, (float *)world.proj);
		glUniformMatrix4fv(modelLocT ,1, GL_FALSE, (float *)world.structure_model);
		glUniform3fv(camPosLoc, 1, world.camera.pos);
		
		updateFrame(&world,prev_flags);
		drawFrame(&world,modelLoc);
	
		
        ui_end_frame();               // after the scene, before swap
		glfwSwapBuffers(window);
        glfwPollEvents();
    }
	// at exit
	ui_shutdown();
    glfwTerminate();
    return 0;
}
