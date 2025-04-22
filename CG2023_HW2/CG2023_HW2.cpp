#include <Windows.h>

#include "headers.h"
#include "trianglemesh.h"
#include "camera.h"
#include "shaderprog.h"
#include "light.h"

// Global variables.
int screenWidth = 800;
int screenHeight = 800;
GLuint vbo;
GLuint ibo;
float scale = 0, theta = 0.0f, phi = 0.0f, tho = 0.0f, move_x = 0.0f, move_y = 0.0f, move_z = 0.0f;
int open = 0;
std::string filepath = "";
glm::vec3 centertransform;

// Triangle mesh.
TriangleMesh* mesh = nullptr;
// Lights.
DirectionalLight* dirLight = nullptr;
PointLight* pointLight = nullptr;
SpotLight* spotLight = nullptr;
glm::vec3 dirLightDirection = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 dirLightRadiance = glm::vec3(0.6f, 0.6f, 0.6f);
glm::vec3 pointLightPosition = glm::vec3(0.8f, 0.0f, 0.8f);
glm::vec3 pointLightIntensity = glm::vec3(0.5f, 0.1f, 0.1f);
glm::vec3 spotLightPosition = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 spotLightDirection = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 spotLightIntensity = glm::vec3(0.5f, 0.5f, 0.1f);
float spotLightCutoffStartInDegree = 30.0f;
float spotLightTotalWidthInDegree = 45.0f;
glm::vec3 ambientLight = glm::vec3(0.2f, 0.2f, 0.2f);
// Camera.
Camera* camera = nullptr;
glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 5.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float fovy = 30.0f;
float zNear = 0.1f;
float zFar = 1000.0f;

// Shaders.
FillColorShaderProg* fillColorShader = nullptr;
PhongShadingDemoShaderProg* phongShadingShader = nullptr;
// UI.
const float lightMoveSpeed = 0.2f;


// SceneObject.
struct SceneObject
{
    SceneObject() {
        mesh = nullptr;
        worldMatrix = glm::mat4x4(1.0f);
    }
    TriangleMesh* mesh;
    glm::mat4x4 worldMatrix;
};
SceneObject sceneObj;

// ScenePointLight (for visualization of a point light).
struct ScenePointLight
{
    ScenePointLight() {
        light = nullptr;
        worldMatrix = glm::mat4x4(1.0f);
        visColor = glm::vec3(1.0f, 1.0f, 1.0f);
    }
    PointLight* light;
    glm::mat4x4 worldMatrix;
    glm::vec3 visColor;
};
ScenePointLight pointLightObj;
ScenePointLight spotLightObj;

// Function prototypes.
void ReleaseResources();
// Callback functions.
void RenderSceneCB();
void ReshapeCB(int, int);
void ProcessSpecialKeysCB(int, int, int);
void ProcessKeysCB(unsigned char, int, int);
void SetupRenderState();
void LoadObjects();
void CreateCamera();
void CreateShaderLib();
void CreateLights();

std::string openfilename() {

    OPENFILENAME opfn;
    WCHAR file_name[MAX_PATH];
    ZeroMemory(&opfn, sizeof(OPENFILENAME));
    opfn.lStructSize = sizeof(OPENFILENAME);
    opfn.lpstrFilter = L"所有文件\0*.*\0";
    opfn.nFilterIndex = 1;
    opfn.lpstrFile = file_name;
    opfn.lpstrFile[0] = '\0';
    opfn.nMaxFile = sizeof(file_name);
    opfn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileName(&opfn))
    {
        char ch[260];
        char DefChar = ' ';
        WideCharToMultiByte(CP_ACP, 0, file_name, -1, ch, 260, &DefChar, NULL);

        std::string ss(ch);
        return ss;
    }
    else {
        return "";
    }
}

glm::mat4x4 finaltransform() {
    glm::mat4x4 final(1.0f);
    glm::mat4x4 T = glm::translate(final, glm::vec3(move_x, move_y, move_z));//Transformation
    glm::mat4x4 S;
    if (mesh == nullptr || mesh->scale() == 0) {
        S = glm::scale(final, glm::vec3(1 + scale, 1 + scale, 1 + scale));//scale
    }
    else {
        S = glm::scale(final, glm::vec3(1 / mesh->scale() + scale, 1 / mesh->scale() + scale, 1 / mesh->scale() + scale));//scale
    }

    glm::mat4x4 RY = glm::rotate(final, glm::radians(theta), glm::vec3(0.0f, 1.0f, 0.0f));//RotateY
    glm::mat4x4 RZ = glm::rotate(final, glm::radians(phi), glm::vec3(0.0f, 0.0f, 1.0f));//RotateZ
    glm::mat4x4 RX = glm::rotate(final, glm::radians(tho), glm::vec3(1.0f, 0.0f, 0.0f));//RotateX

    final = T * S * RY * RZ * RX;
    return final;
}

void reset() {
    scale = 0, theta = 0.0f, phi = 0.0f, tho = 0.0f, move_x = 0.0f, move_y = 0.0f, move_z = 0.0f;
}
void processMenuEvents(int option) {
    switch (option) {
    case 1:
        open = 1;
        system("cls");
        reset();
        CreateLights();
        CreateCamera();
        LoadObjects();
        break;
    }
}
void createGLUTMenus() {

    int menu;
    menu = glutCreateMenu(processMenuEvents);
    glutAddMenuEntry("openfile", 1);
    glutAttachMenu(GLUT_LEFT_BUTTON);
}

//--------------------------------------------------------------------------

void ReleaseResources()
{
    // ----------------------------------------------------
    // You do not need to change the code.
    // ----------------------------------------------------
    // Delete scene objects and lights.

    if (mesh != nullptr) {
        delete mesh;
        mesh = nullptr;
    }
    if (pointLight != nullptr) {
        delete pointLight;
        pointLight = nullptr;
    }
    if (spotLight != nullptr) {
        delete spotLight;
        spotLight = nullptr;
    }
    if (dirLight != nullptr) {
        delete dirLight;
        dirLight = nullptr;
    }
    // Delete camera.
    if (camera != nullptr) {
        delete camera;
        camera = nullptr;
    }
    // Delete shaders.
    if (fillColorShader != nullptr) {
        delete fillColorShader;
        fillColorShader = nullptr;
    }
    if (phongShadingShader != nullptr) {
        delete phongShadingShader;
        phongShadingShader = nullptr;
    }
    filepath = "";
    SetupRenderState();
}

static float curRotationY = 0.0f;
const float rotStep = 0.025f;
void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render a triangle mesh with Phong shading. ------------------------------------------------
    TriangleMesh* mesh = sceneObj.mesh;
    if (sceneObj.mesh != nullptr) {
        // Update transform.
        //curRotationY += rotStep;
        glm::mat4x4 S = glm::scale(glm::mat4x4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f));
        glm::mat4x4 R = glm::rotate(glm::mat4x4(1.0f), glm::radians(curRotationY), glm::vec3(0, 1, 0));
        sceneObj.worldMatrix = S * R * finaltransform();
        // -------------------------------------------------------
        // Note: if you want to compute lighting in the View Space, 
        //       you might need to change the code below.
        // -------------------------------------------------------
        glm::mat4x4 normalMatrix = glm::transpose(glm::inverse(sceneObj.worldMatrix));
        glm::mat4x4 MVP = camera->GetProjMatrix() * camera->GetViewMatrix() * sceneObj.worldMatrix;

        // -------------------------------------------------------


        phongShadingShader->Bind();
        glUniformMatrix4fv(phongShadingShader->GetLocM(), 1, GL_FALSE, glm::value_ptr(sceneObj.worldMatrix));
        glUniformMatrix4fv(phongShadingShader->GetLocNM(), 1, GL_FALSE, glm::value_ptr(normalMatrix));
        glUniformMatrix4fv(phongShadingShader->GetLocMVP(), 1, GL_FALSE, glm::value_ptr(MVP));
        // Light data.
        if (dirLight != nullptr) {
            glUniform3fv(phongShadingShader->GetLocDirLightDir(), 1, glm::value_ptr(dirLight->GetDirection()));
            glUniform3fv(phongShadingShader->GetLocDirLightRadiance(), 1, glm::value_ptr(dirLight->GetRadiance()));
        }
        if (spotLight != nullptr) {
            glUniform3fv(phongShadingShader->GetLocSpotLightPos(), 1, glm::value_ptr(spotLight->GetPosition()));
            glUniform3fv(phongShadingShader->GetLocSpotLightIntensity(), 1, glm::value_ptr(spotLight->GetIntensity()));
            glUniform3fv(phongShadingShader->GetLocspotLightDirection(), 1, glm::value_ptr(spotLight->GetDirection()));
            glUniform1f(phongShadingShader->GetLocspotLightCutoffStartInDegree(), spotLight->GetspotLightCutoffStartInDegree());
            glUniform1f(phongShadingShader->GetLocspotLightTotalWidthInDegree(), spotLight->GetspotLightTotalWidthInDegree());

        }
        if (pointLight != nullptr) {
            glUniform3fv(phongShadingShader->GetLocPointLightPos(), 1, glm::value_ptr(pointLight->GetPosition()));
            glUniform3fv(phongShadingShader->GetLocPointLightIntensity(), 1, glm::value_ptr(pointLight->GetIntensity()));
        }
        glUniform3fv(phongShadingShader->GetLocAmbientLight(), 1, glm::value_ptr(ambientLight));

        mesh->Draw(phongShadingShader);

        phongShadingShader->UnBind();

        // -------------------------------------------------------
    }

    // Visualize the light with fill color. ------------------------------------------------------
    // ----------------------------------------------------
    // You do not need to change the code.
    // ----------------------------------------------------
    PointLight* pointLight = pointLightObj.light;
    if (pointLight != nullptr) {
        glm::mat4x4 T = glm::translate(glm::mat4x4(1.0f), pointLight->GetPosition());
        pointLightObj.worldMatrix = T;
        glm::mat4x4 MVP = camera->GetProjMatrix() * camera->GetViewMatrix() * pointLightObj.worldMatrix;
        fillColorShader->Bind();
        glUniformMatrix4fv(fillColorShader->GetLocMVP(), 1, GL_FALSE, glm::value_ptr(MVP));
        glUniform3fv(fillColorShader->GetLocFillColor(), 1, glm::value_ptr(pointLightObj.visColor));
        // Render the point light.
        pointLight->Draw();
        fillColorShader->UnBind();
    }

    SpotLight* spotLight = (SpotLight*)(spotLightObj.light);
    if (spotLight != nullptr) {
        glm::mat4x4 T = glm::translate(glm::mat4x4(1.0f), spotLight->GetPosition());
        spotLightObj.worldMatrix = T;
        glm::mat4x4 MVP = camera->GetProjMatrix() * camera->GetViewMatrix() * spotLightObj.worldMatrix;
        fillColorShader->Bind();
        glUniformMatrix4fv(fillColorShader->GetLocMVP(), 1, GL_FALSE, glm::value_ptr(MVP));
        glUniform3fv(fillColorShader->GetLocFillColor(), 1, glm::value_ptr(spotLightObj.visColor));
        // Render the point light.
        spotLight->Draw();
        fillColorShader->UnBind();
    }
    // -------------------------------------------------------------------------------------------

    glutSwapBuffers();
}

void ReshapeCB(int w, int h)
{
    // ----------------------------------------------------
    // You do not need to change the code.
    // ----------------------------------------------------

    // Update viewport.
    screenWidth = w;
    screenHeight = h;
    glViewport(0, 0, screenWidth, screenHeight);
    // Adjust camera and projection.
    float aspectRatio = (float)screenWidth / (float)screenHeight;
    camera->UpdateProjection(fovy, aspectRatio, zNear, zFar);
}

void ProcessSpecialKeysCB(int key, int x, int y)
{
    // ----------------------------------------------------
    // You do not need to change the code.
    // ----------------------------------------------------

    // Handle special (functional) keyboard inputs such as F1, spacebar, page up, etc. 
    switch (key) {
    case GLUT_KEY_F1:
        // Render with point mode.
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        break;
    case GLUT_KEY_F2:
        // Render with line mode.
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
    case GLUT_KEY_F3:
        // Render with fill mode.
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;

        // Point light control.
    case GLUT_KEY_LEFT:
        if (pointLight != nullptr)
            pointLight->MoveLeft(lightMoveSpeed);
        break;
    case GLUT_KEY_RIGHT:
        if (pointLight != nullptr)
            pointLight->MoveRight(lightMoveSpeed);
        break;
    case GLUT_KEY_UP:
        if (pointLight != nullptr)
            pointLight->MoveUp(lightMoveSpeed);
        break;
    case GLUT_KEY_DOWN:
        if (pointLight != nullptr)
            pointLight->MoveDown(lightMoveSpeed);
        break;

    default:
        break;
    }
}

void ProcessKeysCB(unsigned char key, int x, int y)
{
    // ----------------------------------------------------
    // You do not need to change the code.
    // ----------------------------------------------------

    // Handle other keyboard inputs those are not defined as special keys.
    if (key == 27) {
        // Release memory allocation if needed.
        ReleaseResources();
        exit(0);
    }
    // Spot light control.
    if (spotLight != nullptr) {
        if (key == 'a')
            spotLight->MoveLeft(lightMoveSpeed);
        if (key == 'd')
            spotLight->MoveRight(lightMoveSpeed);
        if (key == 'w')
            spotLight->MoveUp(lightMoveSpeed);
        if (key == 's')
            spotLight->MoveDown(lightMoveSpeed);
    }

}

void SetupRenderState()
{
    // ----------------------------------------------------
    // You do not need to change the code.
    // ----------------------------------------------------

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);

    glm::vec4 clearColor = glm::vec4(0.44f, 0.57f, 0.75f, 1.00f);
    glClearColor(
        (GLclampf)(clearColor.r),
        (GLclampf)(clearColor.g),
        (GLclampf)(clearColor.b),
        (GLclampf)(clearColor.a)
    );
}

void LoadObjects()
{
    // -------------------------------------------------------
    // Note: you can change the code below if you want to load
    //       the model dynamically.
    // -------------------------------------------------------
    mesh = new TriangleMesh();
    //mesh->LoadFromFile(".obj", true);
    //mesh->ShowInfo();
    //sceneObj.mesh = mesh;
    //Dianamic loading
    if (open) {
        filepath = openfilename();
        mesh->LoadFromFile(filepath, true);
        mesh->ShowInfo();
        sceneObj.mesh = mesh;
    }
}

void CreateLights()
{
    // ----------------------------------------------------
    // You do not need to change the code.
    // ----------------------------------------------------

    // Create a directional light.
    dirLight = new DirectionalLight(dirLightDirection, dirLightRadiance);
    // Create a point light.
    pointLight = new PointLight(pointLightPosition, pointLightIntensity);
    pointLightObj.light = pointLight;
    pointLightObj.visColor = glm::normalize((pointLightObj.light)->GetIntensity());
    // Create a spot light.
    spotLight = new SpotLight(spotLightPosition, spotLightIntensity, spotLightDirection,
        spotLightCutoffStartInDegree, spotLightTotalWidthInDegree);
    spotLightObj.light = spotLight;
    spotLightObj.visColor = glm::normalize((spotLightObj.light)->GetIntensity());
}

void CreateCamera()
{
    // ----------------------------------------------------
    // You do not need to change the code.
    // ----------------------------------------------------

    // Create a camera and update view and proj matrices.
    camera = new Camera((float)screenWidth / (float)screenHeight);
    camera->UpdateView(cameraPos, cameraTarget, cameraUp);
    float aspectRatio = (float)screenWidth / (float)screenHeight;
    camera->UpdateProjection(fovy, aspectRatio, zNear, zFar);
}

void CreateShaderLib()
{
    // ----------------------------------------------------
    // You do not need to change the code.
    // ----------------------------------------------------

    fillColorShader = new FillColorShaderProg();
    if (!fillColorShader->LoadFromFiles("shaders/fixed_color.vs", "shaders/fixed_color.fs"))
        exit(1);

    phongShadingShader = new PhongShadingDemoShaderProg();
    if (!phongShadingShader->LoadFromFiles("shaders/phong_shading_demo.vs", "shaders/phong_shading_demo.fs"))
        exit(1);
}

int main(int argc, char** argv)
{
    // Setting window properties.
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(screenWidth, screenHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("HW2: Lighting and Shading");

    // Initialize GLEW.
    // Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        std::cerr << "GLEW initialization error: "
            << glewGetErrorString(res) << std::endl;
        return 1;
    }

    // Initialization.
    SetupRenderState();
    CreateLights();
    CreateCamera();
    CreateShaderLib();
    LoadObjects();

    // Register callback functions.
    glutDisplayFunc(RenderSceneCB);
    glutIdleFunc(RenderSceneCB);
    glutReshapeFunc(ReshapeCB);
    glutSpecialFunc(ProcessSpecialKeysCB);
    glutKeyboardFunc(ProcessKeysCB);
    createGLUTMenus();

    // Start rendering loop.
    glutMainLoop();

    return 0;
}
