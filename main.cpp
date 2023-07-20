#include "OBJModel.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const std::string PATH = "";
const std::string FILENAME1 = PATH + "pumpkin_tall_10k.obj";
const std::string FILENAME2 = PATH + "teddy.obj";
const std::string FILENAME3 = PATH + "bunny.obj";
const glm::vec3 GRAVITY(0.0f, -0.98f, 0.0f);

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

    glClearColor(0.2, 0.4, 0.6, 0.0);

    OBJModel model1(FILENAME1);
    if (!model1.load()) {
        glfwTerminate();
        return -1;
    }
    model1.Scale(0.005f);
    model1.Rotate(0, 90, 90);
    model1.Translate(-0.5f, -0.1f, 0.0f);

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
    model3.Scale(2.0f);
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
        "gl_Position = projectionMatrix  * modelMatrix * vec4(vp, 1.0);"
        "}";

    const char* fragment_shader =
        "#version 330 core\n"
        "out vec4 frag_color;"
        "void main(){"
        "frag_color = vec4(0.8, 0.4, 0.1, 0.0);"
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


    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float aspect = static_cast<float>(width)/static_cast<float>(height);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), aspect, 0.1f, 1000.0f);
    GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    float rotationAngle = 0.0f;

    float anglemodel2 = 0.f;

    float h = 0.0001f;
    float t = 0.f;
    float angleCam = 0.f;
    
    std::vector<glm::vec3> velocities{ model3.getPoints().size(), {0, 0, 0} };

    int indicefijo = 0;
    glm::vec3 PosicionOriginal = model3.getPoint(indicefijo);
    
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        rotationAngle += 0.01f;

        //SETUP camera
        auto viewMat = glm::translate(glm::mat4(1), {0.f, 0.f, -1.7f});
        auto rotateMat = glm::rotate(glm::mat4(1), angleCam, {0, 1, 0});
        angleCam = (angleCam >= 2*3.1415f)? 0 : angleCam+0.01;
        viewMat*=rotateMat;


        // Calcular nuevas posiciones usando la gravedad (para model 3)
        for (int i = 0; i < model3.getPoints().size(); i++) {
            velocities[i] = velocities[i] + GRAVITY * h;
            model3.setPoint(i, model3.getPoint(i) + velocities[i] * t);
        }

        t += h;

        model3.setPoint(indicefijo, PosicionOriginal); //set fixed
        velocities[indicefijo] = glm::vec3(0.0f,0.0f,0.0f); //set zero velocity


        //(para model 2)
        auto modelMatTeddy  = glm::translate(glm::mat4(1), {0.f, 0.f, 0.f});
        auto rotateMatTeddy = glm::rotate(glm::mat4(1), anglemodel2, {0,0, -1});  //rotation matrix
        anglemodel2 = (anglemodel2 >= 2*3.1415f)? 0 : anglemodel2+0.05;
        auto modelMatrix2 = modelMatTeddy*rotateMatTeddy; //multiply all cube matrices

        glm::mat4 modelMatrix = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 modelMatrix3 = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));

        // Renderizar model1
        render(vao1, ebo1, model1, viewMat* modelMatrix, shaderProgram);

        // Renderizar model2
        render(vao2, ebo2, model2, viewMat*modelMatrix2, shaderProgram);

        // Renderizar model3
        render(vao3, ebo3, model3, viewMat*modelMatrix3, shaderProgram);

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

    //añadido para actualizar buffer con las nuevas posiciones
    glBufferData(GL_ARRAY_BUFFER, 3 * model.getPoints().size() * sizeof(float), &model.getPoints()[0], GL_DYNAMIC_DRAW);

    // Obtener el identificador de la ubicación de la matriz de transformación en el shader
    GLuint modelMatrixLocation = glGetUniformLocation(shaderProgram, "modelMatrix");

    // Establecer el valor de las matrices de transformación en el shader
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    
    // Set the vertex attribute pointer (assuming location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

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
