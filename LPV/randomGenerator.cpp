#include "randomGenerator.h"
using namespace std;
using namespace glm;
float RandomGenerator::uniform0To1() {
    dis = uniform_int_distribution<int>(0, DEFAULT_ACCURACY);
    return (float)dis(gen) / DEFAULT_ACCURACY;
}
// [n, m]
int RandomGenerator::uniformNToM(int n, int m) {
    if (n > m)   swap(n, m);
    dis = uniform_int_distribution<int>(n, m);
    return dis(gen);
}

vector<vec2> RandomGenerator::uniform0To1By2D(int size)
{
    vector<vec2> randomArray;
    if (size == 0) return randomArray;
    for (int i = 0; i < size; ++i) {
        randomArray.push_back({ uniform0To1(), uniform0To1() });
    }
    return randomArray;
}

std::vector<glm::vec3> RandomGenerator::uniform0To1By3D(int size)
{
    vector<vec3> randomArray;
    if (size == 0) return randomArray;
    for (int i = 0; i < size; ++i) {
        randomArray.push_back({ uniform0To1(), uniform0To1(), uniform0To1() });
    }
    return randomArray;
}

vector<int> RandomGenerator::shuffleN(int size)
{
    vector<int> shuffleArray;
    for (int i = 0; i < size; ++i) {
        shuffleArray.push_back(i);
    }
    for (int i = 0; i < size; ++i) {
        int index = uniformNToM(0, size - 1);
        int tmp = shuffleArray[i];
        shuffleArray[i] = shuffleArray[index];
        shuffleArray[index] = tmp;
    }
    return shuffleArray;
}