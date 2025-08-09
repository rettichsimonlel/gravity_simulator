#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp> // optional, for vector rotation
#include <glm/gtc/constants.hpp>     // optional, for pi, etc.

const int screenWidth = 800;
const int screenHeight = 600;
const int pie = 3.141592653589;
const float G = 6.67430 * pow(10, -11);
//const float G = 0.00000001f;

GLFWwindow* StartGLFW();

GLuint LoadShader(const char* vertexPath, const char* fragmentPath) {
    std::ifstream vShaderFile(vertexPath);
    std::ifstream fShaderFile(fragmentPath);

    std::stringstream vShaderStream, fShaderStream;
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();

    std::string vertexCode = vShaderStream.str();
    std::string fragmentCode = fShaderStream.str();

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    GLuint vertex, fragment;
    GLint success;
    char infoLog[512];

    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << "ERROR::VERTEX_SHADER\n" << infoLog << std::endl;
    }

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cout << "ERROR::FRAGMENT_SHADER\n" << infoLog << std::endl;
    }

    // Shader Program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER_PROGRAM\n" << infoLog << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}


struct Camera {
    float posX = 0.0f, posY = 150.0f, posZ = 300.0f;
    float yaw = -90.0f;
    float pitch = 0.0f;

    float speed = 50.0f;
    float sensitivity = 0.1f;

    void ProccessKeyboard(GLFWwindow* window, float deltaTime) {
        float velocity = speed * deltaTime;
        float frontX = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        float frontY = sin(glm::radians(pitch));
        float frontZ = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        glm::vec3 front(frontX, frontY, frontZ);
        front = glm::normalize(front);
        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            posX += front.x * velocity, posY += front.y * velocity, posZ += front.z * velocity;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            posX -= front.x * velocity, posY -= front.y * velocity, posZ -= front.z * velocity;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            posX -= right.x * velocity, posY -= right.y * velocity, posZ -= right.z * velocity;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            posX += right.x * velocity, posY += right.y * velocity, posZ += right.z * velocity;
    }

    void ProcessMouse(float offsetX, float offsetY) {
        yaw += offsetX * sensitivity;
        pitch += offsetY * sensitivity;

        if (pitch > 89.0f) pitch = 89.0f;
        if (yaw < -89.0f) yaw = -89.0f;
    }
    
    void ApplyView() {
        float frontX = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        float frontY = sin(glm::radians(pitch));
        float frontZ = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        glm::vec3 front(frontX, frontY, frontZ);
        glm::vec3 center = glm::vec3(posX, posY, posZ) + glm::normalize(front);
        
        gluLookAt(posX, posY, posZ,
                  center.x, center.y, center.z,
                  0.0f, 1.0f, 0.0f);
    }
};

class Object {
public:
    std::vector<float> position;
    std::vector<float> velocity;
    float radius;
    float mass;

    Object(std::vector<float> position, std::vector<float> velocity, float mass, float radius) {
        this->position = position;
        this->velocity = velocity;
        this->radius = radius;
        this->mass = mass;
    }

    void accelerate(float x, float y, float z, float delta) {
        this->velocity[0] += (x*delta) / 94;
        this->velocity[1] += (y*delta) / 94;
        this->velocity[2] += (z*delta) / 94;
    }

    void updatePos(float delta) {
        this->position[0] += (this->velocity[0]*delta) / 94;
        this->position[1] += (this->velocity[1]*delta) / 94;
        this->position[2] += (this->velocity[2]*delta) / 94;
    }

    void DrawSphere(GLuint shader) {
        glPushMatrix();
        // glColor3f(0.5f, 0.8f, 0.9f);
        GLint colorLoc = glGetUniformLocation(shader, "objectColor");

        if (mass > 5e23)
            glUniform3f(colorLoc, 0.2f, 0.6f, 0.3f); // Earth-like
        else if (mass > 1e22)
            glUniform3f(colorLoc, 0.6f, 0.6f, 0.6f); // Moon-like
        else
            glUniform3f(colorLoc, 0.9f, 0.7f, 0.2f); // Small body
        glTranslatef(position[0], position[1], position[2]);
        GLUquadric* quad = gluNewQuadric();
        gluSphere(quad, radius, 16, 16);
        gluDeleteQuadric(quad);
        glPopMatrix();
    }

    void CheckCollision(std::vector<Object>& objs, float delta) {
        for (auto& obj : objs) {
            if (&obj == this) continue;
            float dx = obj.position[0] - this->position[0];
            float dy = obj.position[1] - this->position[1];
            float dz = obj.position[2] - this->position[2];
            float distance = sqrt(dx*dx + dy*dy);
            if ((distance < obj.radius + this->radius)) {
                obj.velocity[0] -= 0.1 * delta;
                obj.velocity[1] -= 0.1 * delta;
                obj.velocity[2] -= 0.1 * delta;
            }
        }
    }
};

bool firstMouse = true;
float lastX = screenWidth / 2.0f;
float lastY = screenHeight / 2.0f;

void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    static Camera* cam = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window));

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float offsetX = xpos - lastX;
    float offsetY = lastY - ypos; // Inverted Y
    lastX = xpos;
    lastY = ypos;

    cam->ProcessMouse(offsetX, offsetY);
}


int main() {
    GLFWwindow* window = StartGLFW();
    glfwMakeContextCurrent(window); // You must make the context current before OpenGL calls.

    glewInit();

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)screenWidth / (double)screenHeight, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);

    Camera cam;
    glfwSetWindowUserPointer(window, &cam);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    double lastTime = glfwGetTime();
    float speedFactor = 0.5f;
    
    std::vector<Object> objects = {
        Object({-50.0f, 0.0f, 0.0f}, {0.0f, 2050.0f, 0.0f}, 7.35e22, 5.0f),
        Object({50.0f, 0.0f, 0.0f}, {0.0f, -2150.0f, 0.0f}, 7.35e22, 5.0f),
        Object({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 5.97e24, 10.0f)
    };

    glDisable(GL_LIGHTING);

    GLuint shader = LoadShader("./shader/basic.vert", "./shader/basic.frag");

    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);

    GLint colorLoc = glGetUniformLocation(shader, "objectColor");
    glUniform3f(colorLoc, 0.2f, 0.6f, 0.3f); // example color

    while(!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime) * speedFactor;
        lastTime = currentTime;
        if (deltaTime > 0.02f) deltaTime = 0.02f;

        cam.ProccessKeyboard(window, deltaTime);
        cam.ApplyView();

        glUseProgram(shader);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        gluLookAt(0.0, 150.0, 300.0,
                  0.0, 0.0, 0.0,
                  0.0, 1.0, 0.0);

        for (auto& obj : objects) {
            for (auto& obj2 : objects) {
                if (&obj2 == &obj) continue;
                float dx = obj2.position[0] - obj.position[0];
                float dy = obj2.position[1] - obj.position[1];
                float dz = obj2.position[2] - obj.position[2];
                float distance = sqrt(dx*dx + dy*dy + dz*dz);
                if (distance < 1e-5) continue;

                std::vector<float> direction = {dx / distance, dy / distance, dz / distance};
                distance *= 1000;

                float Gforce = (G * obj.mass * obj2.mass) / (distance * distance);
                float acc = Gforce / obj.mass;
                
                obj.accelerate(acc * direction[0], acc * direction[1], acc * direction[2], deltaTime);
            }
        }

        for (auto& obj : objects) {
            obj.updatePos(deltaTime);
        }

        for (auto& obj : objects) {
            obj.CheckCollision(objects, deltaTime);
        }

        for (auto& obj : objects) {
            obj.DrawSphere(shader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

GLFWwindow* StartGLFW() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize glfw, panic!\n";
        return nullptr;
    }
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "SS-Simulator", NULL, NULL);
    glfwMakeContextCurrent(window);

    return window;
}
