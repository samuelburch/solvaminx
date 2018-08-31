#ifndef CONTROLS_HPP
#define CONTROLS_HPP

void computeMatricesFromInputs(int *currentface, vec3 *camerapos);
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

#endif