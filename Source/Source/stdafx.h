#pragma once

// debug flag for advanced debugging
#define DE_BUG 1

// OpenGL API stuff
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// GL math libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// imgui stuff
#include "vendor\imgui\imgui.h"
#include "vendor\imgui\imgui_impl_glfw.h"
#include "vendor\imgui\imgui_impl_opengl3.h"
#include "vendor\imgui\imgui_internal.h"

// Lua scripting
#include <lua.hpp>

// common std DS & A
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

// debugging
#include "engine_assert.h"

// other common includes
#include <iostream>
#include <cstdlib>
