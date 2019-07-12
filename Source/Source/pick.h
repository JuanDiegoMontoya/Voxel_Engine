#pragma once

// index 3D elements in 1D array


void raycast(glm::vec3 origin, glm::vec3 direction, float radius, std::function<bool(float, float, float, BlockPtr, glm::vec3)> callback);
