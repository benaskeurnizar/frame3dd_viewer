#pragma once


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "graphics.h"

void drawFrame(World* world,unsigned int modelLoc);
void updateFrame(World* world,uint32_t prev_flags);
