#include "main.h"

string cubeMapDirs[6] ={
    "textures/right.png",
    "textures/left.png",
    "textures/top.png",
    "textures/bottom.png",
    "textures/front.png",
    "textures/back.png"
};

Sprite skyBoxSprite("objects/cube.obj",cubeMapDirs);;
Sprite groundSprite = Sprite("objects/ground.obj",
                             "textures/water.jpeg");
Sprite characterSprite = Sprite("objects/Yatch_ps.obj",
                              "textures/Yatch_DIF.png");
Sprite buoySprite = Sprite("objects/buoy_ps.obj",
                              "textures/buoy.png");

int gWidth = 800, gHeight = 450;
glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::vec3 eyePos   = glm::vec3(0.0f, 0.0f,  0.0f);
glm::vec3 eyeFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 eyeUp    = glm::vec3(0.0f, 1.0f,  0.0f);

glm::vec3 movementOffset(0.0f, -1.0f, 0.0f);
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float eyeSpeed = 1.0f;

float mouseLastX=gWidth/2;
float mouseLastY=gHeight/2;
const float sensitivity = 0.1f;
float yaw = -90.0f;
float pitch = 0.0f;

void initWindowShape(){
    glViewport(0, 0, gWidth, gHeight);
    float fovyRad = (float)(45.0 / 180.0) * M_PI;
    projectionMatrix = glm::perspective(fovyRad, gWidth/(float) gHeight, 1.0f, 100.0f);
}

void renderSkyBox(){
    glDisable(GL_DEPTH_TEST);
    
    glm::mat4 matS = glm::scale(glm::mat4(1.f), glm::vec3(8.0f ,8.0f ,8.0f));
    glm::mat4 modelingMatrix = matS;
    
    glUseProgram(skyBoxSprite.gProgram);
    glUniform1i(glGetUniformLocation(skyBoxSprite.gProgram, "skybox"), 0);
    glUniformMatrix4fv(glGetUniformLocation(skyBoxSprite.gProgram, "viewingMatrix"), 1, GL_FALSE, glm::value_ptr(viewingMatrix));
    glUniformMatrix4fv(glGetUniformLocation(skyBoxSprite.gProgram, "modelingMatrix"), 1, GL_FALSE, glm::value_ptr(modelingMatrix));
    
    glBindVertexArray(skyBoxSprite.VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxSprite.textureID);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    glEnable(GL_DEPTH_TEST);    // Switch the depth function back on
}

void initShaders(){
    initShader("shaders/skyboxVert.glsl","shaders/skyboxFrag.glsl",skyBoxSprite,projectionMatrix);
    groundSprite.initShader("shaders/groundVert.glsl","shaders/groundFrag.glsl");
    characterSprite.initShader("shaders/statueVert.glsl","shaders/statueFrag.glsl");
    buoySprite.initShader("shaders/statueVert.glsl","shaders/statueFrag.glsl");
}

void init(){
    glEnable(GL_DEPTH_TEST);
    initWindowShape();
    initShaders();
    skyBoxSprite.initSkyBoxBuffer();
    groundSprite.initBuffer();
    characterSprite.initBuffer();
    buoySprite.initBuffer();
}

void display(){
    viewingMatrix = glm::lookAt(eyePos, eyePos + eyeFront, eyeUp);
    renderSkyBox();
    groundSprite.render(300.0f, movementOffset, projectionMatrix, viewingMatrix);
    characterSprite.render(3.0f, movementOffset, projectionMatrix, viewingMatrix);
    buoySprite.render(0.05f, movementOffset + glm::vec3(10.0f,-0.9f,10.0f), projectionMatrix, viewingMatrix);
}

void movementKeys(GLFWwindow* window){
    eyeSpeed = 5.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        movementOffset -= eyeSpeed * eyeFront;
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        movementOffset += eyeSpeed * eyeFront;
    }
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        movementOffset += glm::normalize(glm::cross(eyeFront, eyeUp)) * eyeSpeed;
    }
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        movementOffset -= glm::normalize(glm::cross(eyeFront, eyeUp)) * eyeSpeed;
    }
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    else if(key == GLFW_KEY_L && action == GLFW_PRESS){
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }
    else if(key == GLFW_KEY_O && action == GLFW_PRESS){
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }
}

void mouse(GLFWwindow* window, double xpos, double ypos){
    float xoffset = xpos - mouseLastX;
    float yoffset = mouseLastY - ypos; // reversed since y-coordinates range from bottom to top
    mouseLastX = xpos;
    mouseLastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;
    yaw   += xoffset;
    pitch += yoffset;
    
    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    eyeFront = glm::normalize(direction);
}

void calculateFrameTime(){
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void cleanBuffers(){
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void mainLoop(GLFWwindow* window){
    while (!glfwWindowShouldClose(window)){
        movementKeys(window);
        calculateFrameTime();
        cleanBuffers();
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int main(int argc, char** argv){
    
    GLFWwindow* window;
    if (!glfwInit()){
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    window = glfwCreateWindow(gWidth, gHeight, "CENG469_HW2", NULL, NULL);

    if (!window){
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLEW to setup the OpenGL Function pointers
    if (GLEW_OK != glewInit()){
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    char rendererInfo[512] = { 0 };
    strcpy(rendererInfo, (const char*)glGetString(GL_RENDERER));
    strcat(rendererInfo, " - ");
    strcat(rendererInfo, (const char*)glGetString(GL_VERSION));
    glfwSetWindowTitle(window, rendererInfo);

    init();
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, keyboard);
    glfwSetCursorPosCallback(window, mouse);
    
    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
