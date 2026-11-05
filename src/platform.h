#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "graphics.h"
#include "ui.h"

GLFWwindow*  createWindow(World* world,int width,int height);
