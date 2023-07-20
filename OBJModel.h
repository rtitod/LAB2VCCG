#ifndef OBJMODEL_H
#define OBJMODEL_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

class OBJModel {
public:
    OBJModel(const std::string& filename) : filename(filename) {}

    bool load() {
        std::ifstream file(filename);
        std::string line;

        if (!file.is_open()) {
            std::cerr << "Error al abrir el archivo: " << filename << std::endl;
            return false;
        }

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string prefix;
            float x, y, z;
            std::string a;
            if (iss >> prefix) {
                if (prefix == "v") {
                    iss >> x >> y >> z;
                    points.push_back(glm::vec3(x, y, z));
                }
                else if (prefix == "f") {
                    for (int i=0; i<128; i++) {
                        if (iss >> a) {
                            indexes.push_back(intExtractor(a) - 1);
                        }
                        else {
                            if (i < sides) {
                                sides = i;
                            }
                            break;
                        }
                    }
                }
            }
            else {
                std::cerr << "Error al hallar tokens v o f en la línea: " << line << std::endl;
                continue;
            }
        }

        file.close();
        return true;
    }

    unsigned int intExtractor(std::string token) {
        unsigned int number;
        size_t pos = token.find("/");
        if (pos != std::string::npos) {
            std::string numberStr = token.substr(0, pos);
            return std::stoi(numberStr);
        }
        else {
            return std::stoi(token);
        }
        return number;
    }

    void Scale(float scalefactor) {
        for (int i = 0; i < points.size(); i++) {
            points[i] *= scalefactor;
        }
    }

    void Translate(float xtrpos, float ytrpos, float ztrpos) {
        for (int i = 0; i < points.size(); i++) {
            points[i].x += xtrpos;
            points[i].y += ytrpos;
            points[i].z += ztrpos;
        }
    }

    void Rotate(float alpha, float beta, float gamma) {
        alpha = glm::radians(alpha);
        beta = glm::radians(beta);
        gamma = glm::radians(gamma);

        for (int i = 0; i < points.size(); i++) {
            float x,y,z;

            x = points[i].x;
            y = points[i].y;
            z = points[i].z;

            float xPrime = x * std::cos(beta) * std::cos(gamma) + y * (std::cos(alpha) * std::sin(gamma) + std::sin(alpha) * std::sin(beta) * std::cos(gamma)) + z * (std::sin(alpha) * std::sin(gamma) - std::cos(alpha) * std::sin(beta) * std::cos(gamma));
            float yPrime = -x * std::cos(beta) * std::sin(gamma) + y * (std::cos(alpha) * std::cos(gamma) - std::sin(alpha) * std::sin(beta) * std::sin(gamma)) + z * (std::sin(alpha) * std::cos(gamma) + std::cos(alpha) * std::sin(beta) * std::sin(gamma));
            float zPrime = x * std::sin(beta) - y * std::sin(alpha) * std::cos(beta) + z * std::cos(alpha) * std::cos(beta);

            points[i] = glm::vec3(xPrime, yPrime, zPrime);
        }
    }

    const std::vector<glm::vec3>& getPoints() const {
        return points;
    }

    const glm::vec3& getPoint(int index) const {
        return points[index];
    }

    void setPoints(const std::vector<glm::vec3>& newPoints) {
            points = newPoints;
    }

    void setPoint(int index, glm::vec3 value){
        points[index]=value;
    }

    const std::vector<unsigned int>& getIndexes() const {
        return indexes;
    }

    unsigned int getSides() const {
        return sides;
    }
public:

private:
    std::vector<glm::vec3> points;
    std::string filename;
    std::vector<unsigned int> indexes;
    unsigned int sides = std::numeric_limits<unsigned int>::max();
};

#endif // OBJMODEL_H
