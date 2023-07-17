#include "OBJModel.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const std::string PATH = "/home/user/CGVC/laboratorio_2/";
const std::string FILENAME1 = PATH + "bunny.obj";
const std::string FILENAME2 = PATH +  "teddy.obj";
const std::string FILENAME3 = PATH + "pumpkin_tall_10k.obj";


void createBufferObjects(GLuint& vbo, GLuint& ebo, const std::vector<glm::vec3>& points, const std::vector<unsigned int>& indexes);
void configureVertexArray(GLuint vao, GLuint vbo);
void render(GLuint vao, GLuint ebo, OBJModel model, const glm::mat4& modelMatrix,  GLuint shaderProgram);

int main() {
    GLFWwindow* window;
    if (!glfwInit()) {
        std::cerr << "fallo GLFW\n";
        return -1;
    }

    window = glfwCreateWindow(800, 800, "Visualizador OBJ Simple", nullptr, nullptr);

    if (window == nullptr) {
        std::cerr << "Fallo en la creacion de la ventana GLFW\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE; //mac

    if (glewInit() != GLEW_OK) {
        std::cerr << "fallo GLEW\n";
        glfwTerminate();
        return -1;
    }

    glClearColor(0.2, 0.2, 0.2, 0.0);

    OBJModel model1(FILENAME1);
    if (!model1.load()) {
        glfwTerminate();
        return -1;
    }
    model1.Scale(2.0f);
    model1.Translate(-0.5f, -0.7f, 0.0f);

    OBJModel model2(FILENAME2);
    if (!model2.load()) {
        glfwTerminate();
        return -1;
    }
    model2.Scale(0.0125f);
    model2.Translate(0.5f, -0.5f, 0.0f);

    OBJModel model3(FILENAME3);
    if (!model3.load()) {
        glfwTerminate();
        return -1;
    }
    model3.Scale(0.005f);
    model3.Rotate(0, 90, 90);
    model3.Translate(0.0f, 0.7f, 0.0f);

    GLuint vbo1, vbo2, vbo3, ebo1, ebo2, ebo3;
    createBufferObjects(vbo1, ebo1, model1.getPoints(), model1.getIndexes());
    createBufferObjects(vbo2, ebo2, model2.getPoints(), model2.getIndexes());
    createBufferObjects(vbo3, ebo3, model3.getPoints(), model3.getIndexes());

    GLuint vao1, vao2, vao3;
    glGenVertexArrays(1, &vao1);
    glGenVertexArrays(1, &vao2);
    glGenVertexArrays(1, &vao3);

    configureVertexArray(vao1, vbo1);
    configureVertexArray(vao2, vbo2);
    configureVertexArray(vao3, vbo3);

    const char* vertex_shader =
        "#version 330 core\n"
        "in vec3 vp;"
        "uniform mat4 modelMatrix;"
        "uniform mat4 viewMatrix;"
        "uniform mat4 projectionMatrix;"
        "void main(){"
        "gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vp, 1.0);"
        "}";

    const char* fragment_shader =
        "#version 330 core\n"
        "out vec4 frag_color;"
        "void main(){"
        "frag_color = vec4(1.0, 1.0, 1.0, 0.0);"
        "}";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, nullptr);
    glCompileShader(fs);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, fs);
    glAttachShader(shaderProgram, vs);
    glLinkProgram(shaderProgram);

    glValidateProgram(shaderProgram);
    glUseProgram(shaderProgram);

    std::vector<glm::vec3> velocities{model3.getPoints().size(), {0,0,0}};

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float aspect = static_cast<float>(width)/static_cast<float>(height);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(40.0f), aspect, 0.1f, 100.0f);
    GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    float rotationAngle = 0.0f;


    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        rotationAngle += 0.01f;

        glm::mat4 modelMatrix = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 modelMatrix2 = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));

        // Renderizar model1
        render(vao1, ebo1, model1, modelMatrix, shaderProgram);

        // Renderizar model2
        render(vao2, ebo2, model2, modelMatrix, shaderProgram);

        // Renderizar model3
        render(vao3, ebo3, model3, modelMatrix2, shaderProgram);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}

// Función para renderizar
void render(GLuint vao, GLuint ebo, OBJModel model, const glm::mat4& modelMatrix,  GLuint shaderProgram) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    // Obtener el identificador de la ubicación de la matriz de transformación en el shader
    GLuint modelMatrixLocation = glGetUniformLocation(shaderProgram, "modelMatrix");

    // Establecer el valor de las matrices de transformación en el shader
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    if (model.getSides() == 3) {
        glDrawElements(GL_TRIANGLES, model.getIndexes().size(), GL_UNSIGNED_INT, 0);
    }
    else {
        glDrawElements(GL_QUADS, model.getIndexes().size(), GL_UNSIGNED_INT, 0);
    }
}

// Función para generar VBO y EBO
void createBufferObjects(GLuint& vbo, GLuint& ebo, const std::vector<glm::vec3>& points, const std::vector<unsigned int>& indexes) {
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * points.size() * sizeof(float), &points[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * sizeof(unsigned int), &indexes[0], GL_STATIC_DRAW);
}

// Función para configurar VAO
void configureVertexArray(GLuint vao, GLuint vbo) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
}
