#include "rendering.h"
#include "graphics.h"

static void draw_objects(Object *objs, int count, int modelLoc) {
    for (int i = 0; i < count; i++) {
        if (objs[i].vertexCount == 0) continue;     // skipped "0 load" objects
        //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float *)objs[i].model);
        glBindVertexArray(objs[i].vao);
        glDrawArrays(GL_LINES, 0, objs[i].vertexCount);
    }
    glBindVertexArray(0);
}

void updateFrame(World* world,uint32_t prev_flags){
	if((world->render_flags ^ prev_flags) & SHOW_DEFORMED){
		updateStructureObject(world);
	}
	if ((world->render_flags ^ prev_flags) & SHOW_MODE_SHAPE) {
		updateStructureObject(world);
		world->render_flags &= ~SHOW_DEFORMED;
		if (world->render_flags & SHOW_MODE_SHAPE) { 
			world->in_mode_animation = 1;
			world->mode_start_time = glfwGetTime();
		}
		else{
			world->in_mode_animation = 0;
		}
	}
	
}


void drawFrame(World* world,unsigned int modelLoc){
	//first draw the line
	glUseProgram(world->programShader);
	if(world->render_flags & SHOW_ELEMENTS){
		if(world->in_mode_animation){
			double current_time = glfwGetTime();
			updateMode(world,current_time);
		}
		glLineWidth(4.0f);
		draw_objects(&world->structureObject,1,modelLoc); 
	}
	// then draw the tran structure
	glUseProgram(world->programShaderT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	unsigned int alphaLoc = glGetUniformLocation(world->programShaderT, "alpha");
	
	if(world->in_mode_animation){
		glUniform1f(alphaLoc, 0.3f);
	}else{
		glUniform1f(alphaLoc, 1.0f);
	}
	if(world->render_flags & SHOW_DEFORMED){
		for (int i = 0; i < world->deformedTubesCount; i++) {
			Object *object = &world->deformedTubes[i];
			glBindVertexArray(object->vao);
			glDrawArrays(GL_TRIANGLES, 0, object->vertexCount);
		}
	}else{
		for (int i = 0; i < world->structureTubeObjectCount; i++) {
			Object *object = &world->structureTubeObjects[i];
			glBindVertexArray(object->vao);
			glDrawArrays(GL_TRIANGLES, 0, object->vertexCount);
		}
	}
	//then draw the rest
	glUseProgram(world->programShader);
	glDisable(GL_DEPTH_TEST);
	if(world->render_flags & SHOW_SUPPORTS){
		glLineWidth(3.0f);
		draw_objects(world->supportObjects,world->supprtObjectCount,modelLoc);
	}
	if (world->render_flags & SHOW_UNIFORM_LOADS) {
		glLineWidth(2.0f);
		draw_objects(world->loadObjects.uniform, world->loadObjects.uniformCount, modelLoc);
	}
	if (world->render_flags & SHOW_POINT_LOADS) {
		glLineWidth(2.0f);
		draw_objects(world->loadObjects.nodal, world->loadObjects.nodalCount, modelLoc);
		draw_objects(world->loadObjects.nodalMoment,world->loadObjects.nodalCount, modelLoc);
	}
	if (world->render_flags & SHOW_TRAP_LOADS) {
		glLineWidth(2.0f);
		draw_objects(world->loadObjects.trap, world->loadObjects.trapCount, modelLoc);
	}
	glEnable(GL_DEPTH_TEST);
	
}
