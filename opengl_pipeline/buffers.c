#include <stdlib.h>
#include <stdio.h>

#include <glad/glad.h>

#include "buffers.h"

void BindObjectBuffers(Object* object){
	
  glGenVertexArrays(1, &(object->vao));
  glGenBuffers(1, &(object->vbo));

  glBindVertexArray(object->vao);
  glBindBuffer(GL_ARRAY_BUFFER, object->vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*6*object->vertexCount, object->vertices, GL_STATIC_DRAW);

  int stride = 6*sizeof(float);
  // Set attribute pointers (for position only here)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,stride, (void*)0);
  glEnableVertexAttribArray(0);
  
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,stride, (void*)(3*sizeof(float)));
  glEnableVertexAttribArray(1);
  
  glBindVertexArray(0);

}

void BindObjectBuffersNormal(Object* object){
	
  glGenVertexArrays(1, &(object->vao));
  glGenBuffers(1, &(object->vbo));

  glBindVertexArray(object->vao);
  glBindBuffer(GL_ARRAY_BUFFER, object->vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*9*object->vertexCount, object->vertices, GL_STATIC_DRAW);

  int stride = 9*sizeof(float);
  // Set attribute pointers (for position only here)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,stride, (void*)0);
  glEnableVertexAttribArray(0);
  
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,stride, (void*)(3*sizeof(float)));
  glEnableVertexAttribArray(1);
  
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,stride, (void*)(6*sizeof(float)));
  glEnableVertexAttribArray(2);
  
  glBindVertexArray(0);

}
