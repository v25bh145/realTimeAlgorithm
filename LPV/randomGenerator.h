#pragma once
#include <random>
// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define DEFAULT_ACCURACY 10000.f
class RandomGenerator {
public:
    std::random_device rd;
    std::default_random_engine gen;
    std::uniform_int_distribution<int> dis;

    RandomGenerator() {
        gen = std::default_random_engine(rd());
    }

    float uniform0To1();
    int uniformNToM(int n, int m);
    std::vector<glm::vec2> uniform0To1By2D(int size);
    std::vector<glm::vec3> uniform0To1By3D(int size);
    // e.g: size==5: 54213 13254 13245
    std::vector<int> shuffleN(int size);
};