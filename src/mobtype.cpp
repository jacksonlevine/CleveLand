#include "mobtype.h"

GLuint mobVBOs[2] = {
  0,
  0
};

GLuint mobPosVBOs[2] = {
  0,
  0
};


std::vector<std::vector<float>> mobVerts = {
  {    // Positions        // Corner IDs
    // Front face
    -0.4f, -0.4f,  0.4f, 0.0f,  // Bottom-left
     0.4f,  0.4f,  0.4f, 2.0f,  // Top-right
     0.4f, -0.4f,  0.4f, 1.0f,  // Bottom-right

    -0.4f, -0.4f,  0.4f, 0.0f,  // Bottom-left
    -0.4f,  0.4f,  0.4f, 3.0f,  // Top-left
     0.4f,  0.4f,  0.4f, 2.0f,  // Top-right

    // Back face
    -0.4f, -0.4f, -0.4f, 0.0f,  // Bottom-left
     0.4f, -0.4f, -0.4f, 1.0f,  // Bottom-right
     0.4f,  0.4f, -0.4f, 2.0f,  // Top-right

    -0.4f, -0.4f, -0.4f, 0.0f,  // Bottom-left
     0.4f,  0.4f, -0.4f, 2.0f,  // Top-right
    -0.4f,  0.4f, -0.4f, 3.0f,  // Top-left

    // Top face
    -0.4f,  0.4f,  0.4f, 3.0f,  // Front-left
     0.4f,  0.4f, -0.4f, 2.0f,  // Back-right
     0.4f,  0.4f,  0.4f, 1.0f,  // Front-right

    -0.4f,  0.4f,  0.4f, 3.0f,  // Front-left
    -0.4f,  0.4f, -0.4f, 0.0f,  // Back-left
     0.4f,  0.4f, -0.4f, 2.0f,  // Back-right

    // Bottom face
    -0.4f, -0.4f,  0.4f, 0.0f,  // Front-left
     0.4f, -0.4f,  0.4f, 1.0f,  // Front-right
     0.4f, -0.4f, -0.4f, 2.0f,  // Back-right

    -0.4f, -0.4f,  0.4f, 0.0f,  // Front-left
     0.4f, -0.4f, -0.4f, 2.0f,  // Back-right
    -0.4f, -0.4f, -0.4f, 3.0f,  // Back-left

    // Right face
     0.4f, -0.4f,  0.4f, 0.0f,  // Bottom-front
     0.4f,  0.4f, -0.4f, 2.0f,  // Top-back
     0.4f, -0.4f, -0.4f, 1.0f,  // Bottom-back

     0.4f, -0.4f,  0.4f, 0.0f,  // Bottom-front
     0.4f,  0.4f,  0.4f, 3.0f,  // Top-front
     0.4f,  0.4f, -0.4f, 2.0f,  // Top-back

    // Left face
    -0.4f, -0.4f,  0.4f, 0.0f,  // Bottom-front
    -0.4f, -0.4f, -0.4f, 1.0f,  // Bottom-back
    -0.4f,  0.4f, -0.4f, 2.0f,  // Top-back

    -0.4f, -0.4f,  0.4f, 0.0f,  // Bottom-front
    -0.4f,  0.4f, -0.4f, 2.0f,  // Top-back
    -0.4f,  0.4f,  0.4f, 3.0f   // Top-front
  },
  {    // Front face
    -0.4f, -0.4f,  0.4f, 0.0f,  // Bottom-left
     0.4f,  0.4f,  0.4f, 2.0f,  // Top-right
     0.4f, -0.4f,  0.4f, 1.0f,  // Bottom-right

    -0.4f, -0.4f,  0.4f, 0.0f,  // Bottom-left
    -0.4f,  0.4f,  0.4f, 3.0f,  // Top-left
     0.4f,  0.4f,  0.4f, 2.0f,  // Top-right

    // Back face
    -0.4f, -0.4f, -0.4f, 0.0f,  // Bottom-left
     0.4f, -0.4f, -0.4f, 1.0f,  // Bottom-right
     0.4f,  0.4f, -0.4f, 2.0f,  // Top-right

    -0.4f, -0.4f, -0.4f, 0.0f,  // Bottom-left
     0.4f,  0.4f, -0.4f, 2.0f,  // Top-right
    -0.4f,  0.4f, -0.4f, 3.0f,  // Top-left

    // Top face
    -0.4f,  0.4f,  0.4f, 3.0f,  // Front-left
     0.4f,  0.4f, -0.4f, 2.0f,  // Back-right
     0.4f,  0.4f,  0.4f, 1.0f,  // Front-right

    -0.4f,  0.4f,  0.4f, 3.0f,  // Front-left
    -0.4f,  0.4f, -0.4f, 0.0f,  // Back-left
     0.4f,  0.4f, -0.4f, 2.0f,  // Back-right

    // Bottom face
    -0.4f, -0.4f,  0.4f, 0.0f,  // Front-left
     0.4f, -0.4f,  0.4f, 1.0f,  // Front-right
     0.4f, -0.4f, -0.4f, 2.0f,  // Back-right

    -0.4f, -0.4f,  0.4f, 0.0f,  // Front-left
     0.4f, -0.4f, -0.4f, 2.0f,  // Back-right
    -0.4f, -0.4f, -0.4f, 3.0f,  // Back-left

    // Right face
     0.4f, -0.4f,  0.4f, 0.0f,  // Bottom-front
     0.4f,  0.4f, -0.4f, 2.0f,  // Top-back
     0.4f, -0.4f, -0.4f, 1.0f,  // Bottom-back

     0.4f, -0.4f,  0.4f, 0.0f,  // Bottom-front
     0.4f,  0.4f,  0.4f, 3.0f,  // Top-front
     0.4f,  0.4f, -0.4f, 2.0f,  // Top-back

    // Left face
    -0.4f, -0.4f,  0.4f, 0.0f,  // Bottom-front
    -0.4f, -0.4f, -0.4f, 1.0f,  // Bottom-back
    -0.4f,  0.4f, -0.4f, 2.0f,  // Top-back

    -0.4f, -0.4f,  0.4f, 0.0f,  // Bottom-front
    -0.4f,  0.4f, -0.4f, 2.0f,  // Top-back
    -0.4f,  0.4f,  0.4f, 3.0f   // Top-front
}
};