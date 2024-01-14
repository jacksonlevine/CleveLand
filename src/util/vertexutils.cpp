#include "vertexutils.h"

std::vector<float> rotateCoordinatesAroundYNegative90(const std::vector<float>& coords, int numRotations) {
    if (coords.size() % 5 != 0) {
        throw std::invalid_argument("Vector must contain sets of 5 elements.");
    }

    std::vector<float> rotatedCoords = coords;  // Copy the original coordinates
    for(int t = 0; t < numRotations; ++t) {
        for (size_t i = 0; i < rotatedCoords.size(); i += 5) {
            float x = rotatedCoords[i];
            float z = rotatedCoords[i + 2];

            // Applying rotation matrix for -90 degrees around Y-axis
            rotatedCoords[i] = -z; // New x is -old z
            rotatedCoords[i + 2] = x; // New z is old x
            // Y and additional floats remain unchanged
        }
    }
    return rotatedCoords;
}