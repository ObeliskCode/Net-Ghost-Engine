#ifndef SCENE_H
#define SCENE_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include <glm/gtx/string_cast.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <math.h>
#include <cmath>

#include "stb_image.h"

#include "btBulletDynamicsCommon.h"

//#include "Daemon.h"
#include "Globals.h"
#include "Shader.h"
#include "Audio.h"
#include "GUI.h"
#include "ECS.h"
#include "Entity.h"
#include "Camera.h"
#include "Model.h"
#include "Wire.h"
#include "Particle.h"
#include "Physics.h"
#include "Skybox.h"
//#include "SkeletalModel.h"
//#include "Skeleton.h"
//#include "Light.h"
//#include "Animator.h"
#include "Input.h"
#include "ParticleRenderer.h"
//#include "LightSystem.h"
#include "ParticleSystem.h"
#include "Transform.h"
#include "QuadRenderer.h"

const double PI = 3.1415926535897932384626433832795028841971693993751058209;

// todo move to particle class?
struct
{
    bool operator()(Particle a, Particle b) const { return glm::length(Globals::get().camera->getPosition() - a.getTranslation()) > glm::length(Globals::get().camera->getPosition() - b.getTranslation()); }
} Less;

const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

// stackoverflow!
glm::quat quat_axis_angle(const double &xx, const double &yy, const double &zz, const double &a)
{
    // Here we calculate the sin( theta / 2) once for optimization
    double factor = sin(a / 2.0);

    // Calculate the x, y and z of the quaternion
    double x = xx * factor;
    double y = yy * factor;
    double z = zz * factor;

    // Calcualte the w value by cos( theta / 2 )
    double w = cos(a / 2.0);

    return glm::quat(w, x, y, z);
}

void linkModelShaderUniforms(Shader &shader)
{
    Globals &globals = Globals::get();

    shader.Activate();

    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(Globals::get().lightSpaceMatrix));

    glActiveTexture(GL_TEXTURE0 + 6);
    glBindTexture(GL_TEXTURE_2D, globals.depthMap);
    glUniform1i(glGetUniformLocation(shader.ID, "shadowMap"), 6);

    glActiveTexture(GL_TEXTURE0 + 5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, globals.depthCubeMap);
    glUniform1i(glGetUniformLocation(shader.ID, "shadowCubeMap"), 5);

    glUniform1f(glGetUniformLocation(shader.ID, "far_plane"), globals.far_plane);
}

class Scene
{
public:
    virtual int loadResources(GLFWwindow *window) = 0;
    virtual int tick(GLFWwindow *window, double delta) = 0;
    virtual int drawFrame(GLFWwindow *window, double frameTime) = 0;
    //virtual int cleanup() = 0;

    int cleanup() {
        // careful with these! not well written![BROKEN ATM]
        //LightSystem::destruct();
        ParticleSystem::destruct();
        Audio::destruct();
        GUI::destruct();
        ECS::destruct();
        Physics::destruct();
        Globals::destruct();
        Input::destruct();
        //Daemon::destruct();
        return 1;
    }

    // [TODO] fix interopt problem with these members!
    ECS &ecs = ECS::get();
    //Daemon &dae = Daemon::get();
    GUI &gui = GUI::get();
    Globals &globals = Globals::get();
    //LightSystem &lightsys = LightSystem::get();
    ParticleSystem &particlesys = ParticleSystem::get();
    Physics &physics = Physics::get();
    Input &input = Input::get();
    Audio &audio = Audio::get();

    static void renderScene()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void setupCallbacks(GLFWwindow *window)
    {
        glfwSetFramebufferSizeCallback(window, winFun);

        glfwSetKeyCallback(window, keyFun);

        glfwSetCursorPosCallback(window, curFun);
    }

    void (*winFun)(GLFWwindow *, int, int);
    void (*keyFun)(GLFWwindow *, int, int, int, int);
    void (*curFun)(GLFWwindow *, double, double);

private:
};

class MainMenu : public Scene
{
public:
    Shader textProgram;

    MainMenu()
    {
        winFun = [](GLFWwindow *window, int width, int height)
        {
            // Define the portion of the window used for OpenGL rendering.
            glViewport(0, 0, width, height);

            Globals &globals = Globals::get();

            globals.screenWidth = width == 0 ? 1 : width;
            globals.screenHeight = height == 0 ? 1 : height;

            globals.camera->setDims(globals.screenWidth, globals.screenHeight);

            globals.handCam->setDims(globals.screenWidth, globals.screenHeight);
        };

        keyFun = [](GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            Globals &globals = Globals::get();
            Input &input = Input::get();

            if (action == GLFW_RELEASE)
            {
                input.setValue(key, false);
                return;
            }

            input.setValue(key, true);
            switch (key)
            {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, true);
                break;
            case GLFW_KEY_F10:
                glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, globals.screenWidth, globals.screenHeight, GLFW_DONT_CARE);
                break;
            case GLFW_KEY_F9:
                glfwSetWindowMonitor(window, NULL, 100, 100, globals.screenWidth, globals.screenHeight, GLFW_DONT_CARE);
                break;
            }
        };

        curFun = [](GLFWwindow *window, double xpos, double ypos)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        };
    }

    int loadResources(GLFWwindow *window) override
    {

        textProgram = Shader("textVert.glsl", "textFrag.glsl");

        return 1;
    }

    int tick(GLFWwindow *window, double delta) override
    {
        return 1;
    }

    int drawFrame(GLFWwindow *window, double frameTime) override
    {
        renderScene();

        glEnable(GL_BLEND);
        gui.RenderText(textProgram, "Obelisk Engine", (globals.screenWidth / 2) - 150.0f, globals.screenHeight - (globals.screenHeight / 10), 0.75f, glm::vec3(1.f, 1.f, 1.f));
        gui.RenderText(textProgram, "Test Room", (globals.screenWidth / 2) - 150.0f, globals.screenHeight - (globals.screenHeight / 6), 0.75f, glm::vec3(1.f, 1.f, 1.f));
        glDisable(GL_BLEND);

        glfwSwapBuffers(window);
        return 1;
    }

private:
};

class FoldAnim : public Scene
{
public:
    QuadRenderer *quadSys = new QuadRenderer();

    Shader quadProgram;

    FoldAnim()
    {
        winFun = [](GLFWwindow *window, int width, int height)
        {
            Globals &globals = Globals::get();

            // Define the portion of the window used for OpenGL rendering.
            glViewport(0, 0, width, height);
            globals.screenWidth = width == 0 ? 1 : width;
            globals.screenHeight = height == 0 ? 1 : height;

            globals.camera->setDims(globals.screenWidth, globals.screenHeight);

            globals.handCam->setDims(globals.screenWidth, globals.screenHeight);
        };

        keyFun = [](GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            Globals &globals = Globals::get();
            Input &input = Input::get();

            if (action == GLFW_RELEASE)
            {
                input.setValue(key, false);
                return;
            }

            input.setValue(key, true);
            switch (key)
            {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, true);
                break;
            case GLFW_KEY_X:
                globals.cursorLocked = !globals.cursorLocked;
                break;
            case GLFW_KEY_F10:
                glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, globals.screenWidth, globals.screenHeight, GLFW_DONT_CARE);
                break;
            case GLFW_KEY_F9:
                glfwSetWindowMonitor(window, NULL, 100, 100, globals.screenWidth, globals.screenHeight, GLFW_DONT_CARE);
                break;
            }
        };

        curFun = [](GLFWwindow *window, double xpos, double ypos)
        {
            Globals &globals = Globals::get();

            if (globals.cursorLocked == true)
            {
                float sensitivity = 1.0f; // mouse sens

                double deltaAngle = 0.0005; // base angle we move in

                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

                double xMiddle = (double)globals.screenWidth / 2.0;
                double yMiddle = (double)globals.screenHeight / 2.0;

                double xOffset = xMiddle - xpos;
                double yOffset = yMiddle - ypos;

                globals.rotX += xOffset * sensitivity * deltaAngle;
                globals.rotY += yOffset * sensitivity * deltaAngle;

                glfwSetCursorPos(window, xMiddle, yMiddle);
            }
            else
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        };
    }

    int loadResources(GLFWwindow *window) override
    {

        quadProgram = Shader("quadVert.glsl", "quadFrag.glsl");

        Quad q = Quad();
        // multiply first rot last when multiplying quats!
        glm::quat rot1 = quat_axis_angle(1.0, 0.0, 0.0, PI / 2);
        glm::quat rot2 = quat_axis_angle(0.0, 1.0, 0.0, PI / 4);
        glm::quat originQuat = rot2 * rot1;

        q.t.setRotation(originQuat);
        quadSys->quads.push_back(q);

        globals.camera->setPosition(glm::vec3(0.0f, 10.0f, 20.0f));
        globals.camera->setOrientation(glm::normalize(glm::vec3(0.0f, -10.0f, -20.0f)));

        glClearColor(0.08f, 0.11f, 0.5f, 1.0f);

        return 1;
    }

    int tick(GLFWwindow *window, double delta) override
    {
        {
            globals.camera->setOrientation(glm::rotate(globals.camera->getOrientation(), (float)globals.rotX, globals.camera->getUp()));

            glm::vec3 perpendicular = glm::normalize(glm::cross(globals.camera->getOrientation(), globals.camera->getUp()));
            // Clamps rotY so it doesn't glitch when looking directly up or down
            if (!((globals.rotY > 0 && globals.camera->getOrientation().y > 0.99f) || (globals.rotY < 0 && globals.camera->getOrientation().y < -0.99f)))
                globals.camera->setOrientation(glm::rotate(globals.camera->getOrientation(), (float)globals.rotY, perpendicular));

            globals.rotX = 0.0;
            globals.rotY = 0.0;
        }
        { // update cam pos
            if (!globals.camLock)
            {
                glm::vec3 proj = glm::rotate(globals.camera->getOrientation(), glm::radians(90.0f), globals.camera->getUp());
                proj.y = 0.0f;
                proj = glm::normalize(proj);

                float moveSpeed = 1.0f; // position of camera move speed
                float order = 10.0f;

                glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);

                if (input.getValue(GLFW_KEY_W))
                {
                    velocity += globals.camera->getOrientation();
                }
                if (input.getValue(GLFW_KEY_A))
                {
                    velocity += proj;
                }
                if (input.getValue(GLFW_KEY_S))
                {
                    velocity -= globals.camera->getOrientation();
                }
                if (input.getValue(GLFW_KEY_D))
                {
                    velocity -= proj;
                }

                if (glm::length(velocity) > 0)
                {
                    velocity = glm::normalize(velocity);

                    velocity.x *= delta * order * moveSpeed;
                    velocity.y *= delta * order * moveSpeed;
                    velocity.z *= delta * order * moveSpeed;
                }

                globals.camera->setPosition(globals.camera->getPosition() + velocity);
            }
        }
        return 1;
    }

    int drawFrame(GLFWwindow *window, double frameTime) override
    {
        renderScene();

        glDisable(GL_CULL_FACE);
        quadSys->DrawQuads(quadProgram, *globals.camera);
        glEnable(GL_CULL_FACE);

        glfwSwapBuffers(window);
        return 1;
    }

private:
};

#ifndef NOASS
class TestRoom : public Scene
{
public:
    double batRot = 0.0f;
    float bf = 0.0f;

    unsigned int cigCt = 1;

    unsigned int prevID = 0;

    bool cig_anim = false;
    bool is_smoking = false;
    float cig_anim_time = 0.0f;

    int cd = 0;

    unsigned int batEntID;
    unsigned int cigEntID;
    unsigned int characterID;
    unsigned int panelID;
    unsigned int dumpID;
    unsigned int lampID;
    unsigned int benchID;
    unsigned int tentID;
    unsigned int quadID;
    //unsigned int girlID;
    unsigned int cartID;
    //unsigned int wolfID;
    //unsigned int sitID;
    //unsigned int light1ID;
    //unsigned int light2ID;
    unsigned int cigEnts[5];
    unsigned int axisID, sphere1ID, sphere2ID, capsuleID;
    unsigned int chID;

    //unsigned int depthMapFBO;
    //unsigned int depthMap;

    Skybox *sky;
    Shader skyProgram;

    ParticleRenderer particleEmitter;

    QuadRenderer *quadSys = new QuadRenderer();

    Shader textProgram;

    Shader partProgram;

    Shader quadProgram;

    //Animator *mator;

    TestRoom()
    {
        winFun = [](GLFWwindow *window, int width, int height)
        {
            Globals &globals = Globals::get();

            // Define the portion of the window used for OpenGL rendering.
            glViewport(0, 0, width, height);
            globals.screenWidth = width == 0 ? 1 : width;
            globals.screenHeight = height == 0 ? 1 : height;

            globals.camera->setDims(globals.screenWidth, globals.screenHeight);

            globals.handCam->setDims(globals.screenWidth, globals.screenHeight);
        };

        keyFun = [](GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            Globals &globals = Globals::get();
            Input &input = Input::get();

            if (action == GLFW_RELEASE)
            {
                input.setValue(key, false);
                return;
            }

            input.setValue(key, true);
            switch (key)
            {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, true);
                break;
            case GLFW_KEY_X:
                globals.cursorLocked = !globals.cursorLocked;
                break;
            case GLFW_KEY_Z:
                globals.camLock = !globals.camLock;
                break;
            case GLFW_KEY_O:
                globals.drawWires = !globals.drawWires;
                break;
            case GLFW_KEY_F10:
                glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, globals.screenWidth, globals.screenHeight, GLFW_DONT_CARE);
                break;
            case GLFW_KEY_F9:
                glfwSetWindowMonitor(window, NULL, 100, 100, globals.screenWidth, globals.screenHeight, GLFW_DONT_CARE);
                break;
            }
        };

        curFun = [](GLFWwindow *window, double xpos, double ypos)
        {
            Globals &globals = Globals::get();

            if (globals.cursorLocked == true)
            {

                float sensitivity = 1.0f; // mouse sens

                double deltaAngle = 0.0005; // base angle we move in

                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

                double xMiddle = (double)globals.screenWidth / 2.0;
                double yMiddle = (double)globals.screenHeight / 2.0;

                double xOffset = xMiddle - xpos;
                double yOffset = yMiddle - ypos;

                globals.rotX += xOffset * sensitivity * deltaAngle;
                globals.rotY += yOffset * sensitivity * deltaAngle;

                glfwSetCursorPos(window, xMiddle, yMiddle);
            }
            else
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        };
    };

    int loadResources(GLFWwindow *window) override
    {

        /*
        int* data = new int[20];
        for(int i = 0; i < 20; i++){
            data[i] = i;
            std::cout << std::to_string(data[i]) << std::endl;
        }
        void* ret = dae.blockingProcess(TestFunc(20), (void*)data);
        int* retList = (int*)ret;
        for(int i = 0; i < 20; i++){
            std::cout << std::to_string(retList[i]) << std::endl;
        }*/

        textProgram = Shader("textVert.glsl", "textFrag.glsl");

        renderScene();

        glEnable(GL_BLEND);
        gui.RenderText(textProgram, "Loading...", (globals.screenWidth / 2) - 100.0f, globals.screenHeight / 2, 1.0f, glm::vec3(1.f, 1.f, 1.f));
        glDisable(GL_BLEND);

        glfwSwapBuffers(window);

        quadProgram = Shader("quadVert.glsl", "quadFrag.glsl");

        Quad q = Quad();
        quadSys->quads.push_back(q);

        // warning these need to be deleted!
        globals.rigProgram = Shader("rigVert.glsl", "mdlFrag.glsl");
        //globals.lightProgram = Shader("rigVert.glsl", "lightFrag.glsl");
        //globals.animProgram = Shader("animVert.glsl", "mdlFrag.glsl");
        //globals.noTexAnimProgram = Shader("animVert.glsl", "noTexFrag.glsl");
        globals.wireShader = Shader("wireVert.glsl", "wireFrag.glsl");
        //globals.shadowShader = Shader("shadowVert.glsl", "shadowFrag.glsl");
        //globals.animShadowShader = Shader("animShadowVert.glsl", "shadowFrag.glsl");
        globals.cellShader = Shader("rigVert.glsl", "cellFrag.glsl");
        //globals.animCellShader = Shader("animVert.glsl", "cellFrag.glsl");
        //globals.pointShadowShader = Shader("pointShadowVert.glsl", "pointShadowFrag.glsl", "pointShadowGeom.glsl");
        //globals.animPointShadowShader = Shader("animPointShadowVert.glsl", "pointShadowFrag.glsl", "pointShadowGeom.glsl");

        unsigned int entID;
        Entity e;
        Transform *trf;

        trf = new Transform();
        Model *panel = new Model("floor/floor.dae");
        entID = ecs.createEntity();
        panelID = entID;
        trf->setTranslation(glm::vec3(0.0f, 5.0f, 0.0f));
        trf->setScale(glm::vec3(20.0f));
        ecs.addModel(entID, panel);
        ecs.addShader(entID, globals.rigProgram);
        ecs.addCamera(entID, globals.camera);
        ecs.addTransform(entID, trf);
        e = ecs.getEntity(entID);
        e.surface_flag = 1;
        ecs.updateEntity(e);

        trf = new Transform();
        Model *lamp = new Model("lamp/lamp.dae");
        entID = ecs.createEntity();
        lampID = entID;
        trf->setScale(glm::vec3(0.05f));
        trf->setTranslation(glm::vec3(0.0f, 5.0f, 0.0f));
        ecs.addModel(entID, lamp);
        ecs.addShader(entID, globals.lightProgram);
        ecs.addCamera(entID, globals.camera);
        ecs.addTransform(entID, trf);
        ecs.addPhysBody(entID, physics.addUnitBoxStaticBody(entID, 2.0f, 5.0f, 2.0f, -20.0f, 10.0f - 0.1f, 28.0f));
        ecs.addPhysTransform(entID, new Transform());
        ecs.addWireFrame(entID, 2.0f, 5.0f, 2.0f);
        e = ecs.getEntity(entID);
        e.light_flag = 1;
        ecs.updateEntity(e);

        trf = new Transform();
        Model *bench = new Model("bench/bench.dae");
        float CosHalfPi = sqrt(2.f) / 2.f;
        entID = ecs.createEntity();
        benchID = entID;
        trf->setScale(glm::vec3(4.5f));
        trf->setTranslation(glm::vec3(0.0f, -2.5f, 0.0f));
        trf->setRotation(glm::quat(CosHalfPi, 0.f, -CosHalfPi, 0.f));
        ecs.addModel(entID, bench);
        ecs.addShader(entID, globals.rigProgram);
        ecs.addCamera(entID, globals.camera);
        ecs.addTransform(entID, trf);
        ecs.addPhysBody(entID, physics.addUnitBoxStaticBody(entID, 2.0f, 2.5f, 5.0f, 5.0f, 5.0f + 2.5f, 28.f));
        ecs.addPhysTransform(entID, new Transform());
        ecs.addWireFrame(entID, 2.0f, 2.5f, 5.0f);

        trf = new Transform();
        Model *handBat = new Model("bat/bat.dae");
        entID = ecs.createEntity();
        batEntID = entID;
        trf->setRotation(glm::quat(0.0f, 0.0f, 1.0f, 0.0f));
        trf->setTranslation(glm::vec3(2.0f, -2.0f, 0.0f));
        ecs.addModel(entID, handBat);
        ecs.addShader(entID, globals.rigProgram);
        ecs.addCamera(entID, globals.handCam);
        ecs.addTransform(entID, trf);

        trf = new Transform();
        Model *cig = new Model("cig/cig.dae");
        entID = ecs.createEntity();
        cigEntID = entID;
        trf->setTranslation(glm::vec3(0.0f, -2.0f, -1.0f));
        trf->setRotation(glm::quat(0.0f, 0.0f, 1.0f, 0.0f));
        ecs.addModel(entID, cig);
        ecs.addShader(entID, globals.rigProgram);
        ecs.addCamera(entID, globals.handCam);
        ecs.addTransform(entID, trf);

        trf = new Transform();
        Model *dumpster = new Model("dumpster/dumpster.dae");
        entID = ecs.createEntity();
        dumpID = entID;
        trf->setScale(glm::vec3(6.5f));
        trf->setTranslation(glm::vec3(0.0f, -4.0f, 0.0f));
        trf->setRotation(glm::quat(CosHalfPi, 0.f, -CosHalfPi, 0.f));
        ecs.addModel(entID, dumpster);
        ecs.addShader(entID, globals.rigProgram);
        ecs.addCamera(entID, globals.camera);
        ecs.addTransform(entID, trf);
        ecs.addPhysBody(entID, physics.addUnitBoxStaticBody(entID, 3.0f, 4.0f, 6.0f, 25.0f, 5.0f + 4.0f, 35.f));
        ecs.addPhysTransform(entID, new Transform());
        ecs.addWireFrame(entID, 3.0f, 4.0f, 6.0f);

        trf = new Transform();
        entID = ecs.createEntity();
        cartID = entID;
        Model *cart = new Model("cart/ShoppingCart.dae");
        trf->setTranslation(glm::vec3(20.0f, 5.0f, 20.0f));
        trf->setScale(glm::vec3(7.f));
        ecs.addModel(entID, cart);
        ecs.addShader(entID, globals.rigProgram);
        ecs.addCamera(entID, globals.camera);
        ecs.addTransform(entID, trf);

        trf = new Transform();
        entID = ecs.createEntity();
        tentID = entID;
        Model *tent = new Model("tent/tent.dae");
        trf->setScale(glm::vec3(7.f));
        trf->setTranslation(glm::vec3(0.0f, -4.0f, 0.0f));
        ecs.addModel(entID, tent);
        ecs.addShader(entID, globals.rigProgram);
        ecs.addCamera(entID, globals.camera);
        ecs.addTransform(entID, trf);
        ecs.addPhysBody(entID, physics.addUnitBoxStaticBody(entID, 6.0f, 4.0f, 6.0f, -15.0f, 5.0f + 4.0f, 10.0f));
        ecs.addPhysTransform(entID, new Transform());
        ecs.addWireFrame(entID, 6.0f, 4.0f, 6.0f);

        /*
        trf = new Transform();
        entID = ecs.createEntity();
        wolfID = entID;
        SkeletalModel *wolf = new SkeletalModel("wolf/Wolf_One_dae.dae");
        Skeleton *wolfAnimation = new Skeleton("wolf/Wolf_One_dae.dae", wolf);
        Animator *animator = new Animator(wolfAnimation);
        trf->setScale(glm::vec3(10.0f, 10.0f, 10.0f));
        trf->setTranslation(glm::vec3(10.0f, 5.0f, 10.0f));
        ecs.addSkModel(entID, wolf);
        ecs.addAnimator(entID, animator);
        ecs.addShader(entID, globals.noTexAnimProgram);
        ecs.addCamera(entID, globals.camera);
        ecs.addTransform(entID, trf);

        trf = new Transform();
        entID = ecs.createEntity();
        sitID = entID;
        SkeletalModel *sit = new SkeletalModel("sit/Sitting Clap.dae");
        Skeleton *sitAnimation = new Skeleton("sit/Sitting Clap.dae", sit);
        Animator *sitMator = new Animator(sitAnimation);
        trf->setScale(glm::vec3(5.0f, 5.0f, 5.0f));
        trf->setTranslation(glm::vec3(5.0f, 5.0f, 28.f));
        trf->setRotation(glm::quat(CosHalfPi, 0.0f, -CosHalfPi, 0.0f));
        ecs.addSkModel(entID, sit);
        ecs.addAnimator(entID, sitMator);
        ecs.addShader(entID, globals.animProgram);
        ecs.addCamera(entID, globals.camera);
        ecs.addTransform(entID, trf);

        trf = new Transform();
        entID = ecs.createEntity();
        girlID = entID;
        SkeletalModel *walk = new SkeletalModel("jjong/Idle.dae");
        Skeleton *walkAnimation = new Skeleton("jjong/Idle.dae", walk);
        walkAnimation->addAnimation("jjong/Walking.dae", walk);
        mator = new Animator(walkAnimation);
        mator->QueueAnimation(1);
        trf->setScale(glm::vec3(5.0f, 5.0f, 5.0f));
        trf->setTranslation(glm::vec3(15.0f, 5.0f, -5.0f));
        ecs.addSkModel(entID, walk);
        ecs.addAnimator(entID, mator);
        ecs.addShader(entID, globals.animProgram);
        ecs.addCamera(entID, globals.camera);
        ecs.addTransform(entID, trf);*/

        //Light *lampLight = new Light(glm::vec4(0.6f, 0.6f, 0.6f, 1.0f), glm::vec3(-20.0f, 13.2f, 28.0f));
        //lightsys.lights.push_back(lampLight);

        //Light *light2 = new Light(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), glm::vec3(-20.0f, 10.0f, -20.0f));
        //lightsys.lights.push_back(light2);

        /*
        trf = new Transform();
        entID = ecs.createEntity();
        light1ID = entID;
        Model *light = new Model("bulb/scene.gltf");
        trf->setScale(glm::vec3(5.0f, 5.0f, 5.0f));
        trf->setTranslation(glm::vec3(-20.0f, 13.2f, 28.0f));
        ecs.addModel(entID, light);
        ecs.addShader(entID, globals.lightProgram);
        ecs.addCamera(entID, globals.camera);
        ecs.addTransform(entID, trf);
        e = ecs.getEntity(entID);
        e.light_flag = 1;
        ecs.updateEntity(e);

        trf = new Transform();
        entID = ecs.createEntity();
        light2ID = entID;
        Model *lightSecond = new Model("bulb/scene.gltf");
        trf->setScale(glm::vec3(5.0f, 5.0f, 5.0f));
        trf->setTranslation(glm::vec3(-20.0f, 10.0f, -20.0f));
        ecs.addModel(entID, lightSecond);
        ecs.addShader(entID, globals.lightProgram);
        ecs.addCamera(entID, globals.camera);
        ecs.addTransform(entID, trf);
        e = ecs.getEntity(entID);
        e.light_flag = 1;
        ecs.updateEntity(e);*/

        //lampLight->linkShader(globals.rigProgram);
        //lampLight->linkShader(globals.animProgram);
        //lampLight->linkShader(globals.noTexAnimProgram);
        //lampLight->linkShader(globals.lightProgram);

        //lightsys.linkShader(globals.rigProgram);
        //lightsys.linkShader(globals.animProgram);
        //lightsys.linkShader(globals.noTexAnimProgram);
        //lightsys.linkShader(globals.lightProgram);

        // QUAD
        entID = ecs.createEntity();
        quadID = entID;
        ecs.addTransform(entID, new Transform());
        ecs.addCamera(entID, globals.camera);
        ecs.addWire(entID, new Wire(glm::vec3(60.0f, 50.0f, -60.0f), glm::vec3(-60.0f, 50.0f, -60.0f)));
        ecs.addWire(entID, new Wire(glm::vec3(-60.0f, 50.0f, -60.0f), glm::vec3(-60.0f, 50.0f, 60.0f)));
        ecs.addWire(entID, new Wire(glm::vec3(-60.0f, 50.0f, 60.0f), glm::vec3(60.0f, 50.0f, 60.0f)));
        ecs.addWire(entID, new Wire(glm::vec3(60.0f, 50.0f, 60.0f), glm::vec3(60.0f, 50.0f, -60.0f)));
        ecs.addPhysBody(entID, physics.addShape1(entID));
        ecs.addPhysTransform(entID, new Transform());

        // STAGING AXIS
        entID = ecs.createEntity();
        axisID = entID;
        ecs.addTransform(entID, new Transform());
        ecs.addCamera(entID, globals.camera);
        ecs.addWire(entID, new Wire(glm::vec3(-5.0f, 0.0f, 0.0f), glm::vec3(5.0f, 0.0f, 0.0f)));
        ecs.addWire(entID, new Wire(glm::vec3(0.0f, -5.0f, 0.0f), glm::vec3(0.0f, 5.0f, 0.0f)));
        ecs.addWire(entID, new Wire(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f, 0.0f, 5.0f)));

        // RIGID BODY 1
        entID = ecs.createEntity();
        sphere1ID = entID;
        ecs.addTransform(entID, new Transform());
        ecs.addCamera(entID, globals.camera);
        ecs.addWire(entID, new Wire(glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
        ecs.addWire(entID, new Wire(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
        ecs.addWire(entID, new Wire(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        ecs.addPhysBody(entID, physics.addShape2(entID));
        ecs.addPhysTransform(entID, new Transform());

        // RIGID BODY 2
        entID = ecs.createEntity();
        sphere2ID = entID;
        ecs.addTransform(entID, new Transform());
        ecs.addCamera(entID, globals.camera);
        ecs.addWire(entID, new Wire(glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
        ecs.addWire(entID, new Wire(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
        ecs.addWire(entID, new Wire(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        ecs.addPhysBody(entID, physics.addShape3(entID));
        ecs.addPhysTransform(entID, new Transform());

        // CONTROLLABLE BODY
        entID = ecs.createEntity();
        characterID = entID;
        ecs.addTransform(entID, new Transform());
        ecs.addCamera(entID, globals.camera);
        ecs.addWire(entID, new Wire(glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
        ecs.addWire(entID, new Wire(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
        ecs.addWire(entID, new Wire(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        ecs.addPhysBody(entID, physics.addShape4(entID));
        ecs.addPhysTransform(entID, new Transform());

        // CAPSULE
        entID = ecs.createEntity();
        capsuleID = entID;
        ecs.addTransform(entID, new Transform());
        ecs.addCamera(entID, globals.camera);
        ecs.addWire(entID, new Wire(glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
        ecs.addWire(entID, new Wire(glm::vec3(0.0f, -2.0f, 0.0f), glm::vec3(0.0f, 2.0f, 0.0f)));
        ecs.addWire(entID, new Wire(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        ecs.addPhysBody(entID, physics.addShape5(entID));
        ecs.addPhysTransform(entID, new Transform());

        // TEST CIGS
        for (int i = 0; i < 5; i++)
        {
            entID = ecs.createEntity();
            cigEnts[i] = entID;
            ecs.addTransform(entID, new Transform());
            ecs.addModel(entID, new Model("cig/cig.dae"));
            ecs.addShader(entID, globals.rigProgram);
            ecs.addCamera(entID, globals.camera);
            ecs.addWire(entID, new Wire(glm::vec3(-0.06f, 0.0f, 0.0f), glm::vec3(0.06f, 0.0f, 0.0f)));
            ecs.addWire(entID, new Wire(glm::vec3(0.0f, -0.06f, 0.0f), glm::vec3(0.0f, 0.06f, 0.0f)));
            ecs.addWire(entID, new Wire(glm::vec3(0.0f, 0.0f, -0.685f), glm::vec3(0.0f, 0.0f, 0.685f)));
            ecs.addPhysBody(entID, physics.addShape6(entID));
            ecs.addPhysTransform(entID, new Transform());
            e = ecs.getEntity(entID);
            e.pickup_flag = 1;
            ecs.updateEntity(e);
        }

        // CROSSHAIR HACK (class required)
        entID = ecs.createEntity();
        chID = entID;
        ecs.addTransform(entID, new Transform());
        ecs.addCamera(entID, globals.handCam);
        ecs.addWire(entID, new Wire(glm::vec3(-0.04f, 0.0f, 0.0f), glm::vec3(0.04f, 0.0f, 0.0f)));
        ecs.addWire(entID, new Wire(glm::vec3(0.0f, -0.04f, 0.0f), glm::vec3(0.0f, 0.04f, 0.0f)));

        partProgram = Shader("partVert.glsl", "partFrag.glsl");

        std::vector<std::string> faces =
            {
                "models/skybox/right.jpg",
                "models/skybox/left.jpg",
                "models/skybox/top.jpg",
                "models/skybox/bottom.jpg",
                "models/skybox/front.jpg",
                "models/skybox/back.jpg"};
        sky = new Skybox(faces);

        skyProgram = Shader("skyVert.glsl", "skyFrag.glsl");

        /*
        glGenFramebuffers(1, &depthMapFBO);

        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float clampColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        globals.depthMap = depthMap;

        float near_plane = 0.1f, far_plane = 200.0f;
        glm::mat4 lightProjection = glm::ortho(-60.0f, 60.0f, -60.0f, 60.0f, near_plane, far_plane);

        globals.far_plane = far_plane;

        glm::mat4 lightView = glm::lookAt(glm::vec3(-4.0f, 10.0f, -6.0f),
                                          glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
        globals.lightSpaceMatrix = lightSpaceMatrix;

        globals.shadowShader.Activate();
        glUniformMatrix4fv(glGetUniformLocation(globals.shadowShader.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        //globals.animShadowShader.Activate();
        //glUniformMatrix4fv(glGetUniformLocation(globals.animShadowShader.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        globals.depthCubeMap = lampLight->lightShadow->getMap();
        */

        return 1;
    }

    int tick(GLFWwindow *window, double delta) override
    {
        // double ts = glfwGetTime();
        { // mouse panning
            globals.camera->setOrientation(glm::rotate(globals.camera->getOrientation(), (float)globals.rotX, globals.camera->getUp()));

            glm::vec3 perpendicular = glm::normalize(glm::cross(globals.camera->getOrientation(), globals.camera->getUp()));
            // Clamps rotY so it doesn't glitch when looking directly up or down
            if (!((globals.rotY > 0 && globals.camera->getOrientation().y > 0.99f) || (globals.rotY < 0 && globals.camera->getOrientation().y < -0.99f)))
                globals.camera->setOrientation(glm::rotate(globals.camera->getOrientation(), (float)globals.rotY, perpendicular));

            globals.rotX = 0.0;
            globals.rotY = 0.0;
        }
        { // character, delta
            if (globals.camLock)
            {
                btRigidBody *body = ecs.cset_body.getMem(characterID);

                glm::vec2 camOri = glm::vec2(globals.camera->getOrientation().x, globals.camera->getOrientation().z);

                glm::vec2 proj = glm::rotate(camOri, glm::radians(-90.0f));

                glm::vec2 vel = glm::vec2(0.0f);

                if (input.getValue(GLFW_KEY_W))
                {
                    vel += camOri;
                }
                if (input.getValue(GLFW_KEY_A))
                {
                    vel += proj;
                }
                if (input.getValue(GLFW_KEY_S))
                {
                    vel -= camOri;
                }
                if (input.getValue(GLFW_KEY_D))
                {
                    vel -= proj;
                }

                if (cd > 0)
                    cd--;
                // code character->canJump()
                if (input.getValue(GLFW_KEY_SPACE))
                {
                    if (cd == 0)
                    {
                        cd = 192;
                        body->applyCentralImpulse(btVector3(0.0f, 9.0f, 0.0f));
                    }
                }

                float accel;
                accel = delta * 60.0f;

                if (glm::length(vel) > 0)
                {
                    vel = glm::normalize(vel);
                    vel *= accel;
                }

                btVector3 linVel = body->getLinearVelocity();
                btVector3 horizVel = btVector3(linVel.getX() + vel.x, 0.0f, linVel.getZ() + vel.y);
                if (horizVel.length() > 12.0f)
                {
                    horizVel = horizVel * (12.0f / horizVel.length());
                }
                linVel = btVector3(horizVel.getX(), linVel.getY(), horizVel.getZ());
                body->setLinearVelocity(linVel);
            }
        } // character.physicsProcess(delta)

        physics.updateSim(delta); // regular time advance

        // move sim forward by delta
        ecs.syncPhysics(); // update entities with physics state
        {
            if (globals.camLock)
            {
                btTransform trans;
                btRigidBody *body = ecs.cset_body.getMem(characterID);
                body->getMotionState()->getWorldTransform(trans);
                glm::vec3 pos = glm::vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()) + 7.0f, float(trans.getOrigin().getZ()));
                globals.camera->setPosition(pos);
            }
        }
        /*{
            if (input.getValue(GLFW_KEY_RIGHT))
                bf += 0.02f;
            if (input.getValue(GLFW_KEY_LEFT))
                bf -= 0.02f;
            if (bf < 0.0f)
                bf = 0.0f;
            if (bf > 1.0f)
                bf = 1.0f;
            mator->SetBlendFactor(bf);
        }*/
        { // update cam pos
            if (!globals.camLock)
            {
                glm::vec3 proj = glm::rotate(globals.camera->getOrientation(), glm::radians(90.0f), globals.camera->getUp());
                proj.y = 0.0f;
                proj = glm::normalize(proj);

                float moveSpeed = 1.0f; // position of camera move speed
                float order = 10.0f;

                glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);

                if (input.getValue(GLFW_KEY_W))
                {
                    velocity += globals.camera->getOrientation();
                }
                if (input.getValue(GLFW_KEY_A))
                {
                    velocity += proj;
                }
                if (input.getValue(GLFW_KEY_S))
                {
                    velocity -= globals.camera->getOrientation();
                }
                if (input.getValue(GLFW_KEY_D))
                {
                    velocity -= proj;
                }

                if (glm::length(velocity) > 0)
                {
                    velocity = glm::normalize(velocity);

                    velocity.x *= delta * order * moveSpeed;
                    velocity.y *= delta * order * moveSpeed;
                    velocity.z *= delta * order * moveSpeed;
                }

                globals.camera->setPosition(globals.camera->getPosition() + velocity);
            }
        }

        { // do rayCast // raycast selection causes big hitching! how do we optimize?

            if (input.getValue(GLFW_KEY_E))
            {
                glm::vec3 ppp = globals.camera->getPosition() + (globals.camera->getOrientation() * 12.5f);

                btVector3 from = btVector3(globals.camera->getPosition().x, globals.camera->getPosition().y, globals.camera->getPosition().z);
                btVector3 to = btVector3(ppp.x, ppp.y, ppp.z);
                btCollisionWorld::ClosestRayResultCallback rrc = btCollisionWorld::ClosestRayResultCallback(from, to);
                physics.getDynamicsWorld()->rayTest(from, to, rrc);

                if (rrc.hasHit())
                {
                    btRigidBody *sel = btRigidBody::upcast(const_cast<btCollisionObject *>(rrc.m_collisionObject));
                    unsigned int entID = physics.m_EntityMap[sel];
                    if (entID != 0)
                    {
                        if (prevID != 0 && prevID != entID)
                        {
                            Entity prevEnt = ecs.getEntity(prevID);
                            if (prevEnt.m_id != NULL)
                            {
                                prevEnt.stencil_flag = 0;
                                ecs.stencilSet.erase(prevEnt.m_id);
                                ecs.updateEntity(prevEnt);
                            }
                            prevID = 0;
                        }
                        if (ecs.getEntity(entID).pickup_flag)
                        {
                            cigCt++;
                            ecs.deleteEntity(entID);
                            prevID = 0;
                        }
                        else if (!ecs.getEntity(entID).surface_flag)
                        {
                            Entity selEnt = ecs.getEntity(entID);
                            if (selEnt.m_id != NULL)
                            {
                                selEnt.stencil_flag = 1;
                                ecs.stencilSet.insert(selEnt.m_id);
                                ecs.updateEntity(selEnt);
                                prevID = entID;
                            }
                        }
                    }
                }
                else if (prevID != 0)
                {
                    Entity prevEnt = ecs.getEntity(prevID);
                    if (prevEnt.m_id != NULL)
                    {
                        prevEnt.stencil_flag = 0;
                        ecs.stencilSet.erase(prevEnt.m_id);
                        ecs.updateEntity(prevEnt);
                    }
                    prevID = 0;
                }
            }
        }
        {
            if (input.getValue(GLFW_KEY_Y))
                audio.playAudio(1);
            if (input.getValue(GLFW_KEY_U))
                audio.playAudio(0);
        }

        { // particles
            if (input.getValue(GLFW_KEY_P) && !is_smoking && cigCt > 0)
            {
                is_smoking = true;
                cig_anim = true;
                cig_anim_time = 0.0f;
                cigCt--;
            }

            if (is_smoking)
            {
                cig_anim_time += delta;
                if (cig_anim_time >= 2.0f)
                    cig_anim = false;
                if (cig_anim_time >= 3.0f)
                    is_smoking = false;
            }

            Entity cigEnt = ecs.getEntity(cigEntID);

            if (cig_anim)
            {
                cigEnt.visible_flag = 1;
                ecs.updateEntity(cigEnt);
            }
            else
            {
                cigEnt.visible_flag = 0;
                ecs.updateEntity(cigEnt);
            }

            if (cig_anim == false && is_smoking == true)
            {
                Particle p = Particle();
                p.setTranslation(globals.camera->getPosition() - glm::vec3(0.f, 0.25f, 0.f));
                p.setScale(0.1f);
                p.vel = glm::normalize(globals.camera->getOrientation());
                particleEmitter.particles.push_back(p);

                Particle p2 = Particle();
                p2.setTranslation(globals.camera->getPosition() - glm::vec3(0.f, 0.25f, 0.f));
                p2.setScale(0.1f);
                p2.vel = glm::normalize(globals.camera->getOrientation());
                particleEmitter.particles.push_back(p2);
            }

            // how slow is this? (update/cleanup particles)
            particleEmitter.updateParticles(delta);

            for (int i = 0; i < particleEmitter.particles.size(); i++)
            {
                if (particleEmitter.particles[i].life >= particleEmitter.particles[i].expire)
                {
                    particleEmitter.particles.erase(particleEmitter.particles.begin() + i);
                    i--;
                }
            }
        }
        // double te = glfwGetTime();
        // double elap = te - ts;
        // std::cerr << "tick time: " << elap << std::endl;
        return 1;
    }

    // TODO : fix drawFrame hitching!
    int drawFrame(GLFWwindow *window, double frameTime) override
    {
        // double ts = glfwGetTime();

        { // pls do not make this a function
            batRot += frameTime;
            if (batRot > 2 * PI)
            {
                batRot = batRot - (2 * PI);
            }
            float q1, q3;
            q1 = cos(batRot / 2);
            q3 = sin(batRot / 2);
            Transform *batTRF = ecs.cset_transform.getMem(batEntID);
            batTRF->setRotation(glm::quat(q1, 0.0f, q3, 0.0f));
        }

        renderScene();

        //ecs.advanceEntityAnimations(frameTime);

        /*
        { // directional shadow draw pass
            glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT); // clears this framebuffers depth bit!

            ecs.DrawEntityShadows();

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glViewport(0, 0, globals.screenWidth, globals.screenHeight);
        }

        lightsys.RenderPointShadows();

        linkModelShaderUniforms(globals.rigProgram);
        linkModelShaderUniforms(globals.lightProgram);
        //linkModelShaderUniforms(globals.animProgram);
        //linkModelShaderUniforms(globals.noTexAnimProgram);*/

        ecs.DrawEntities(); // crashing here!

        /* for debugging
        ecs.DrawEntity(panelID);
        ecs.DrawEntity(dumpID);
        ecs.DrawEntity(lampID);
        ecs.DrawEntity(benchID);
        ecs.DrawEntity(tentID);
        ecs.DrawEntity(characterID);
        ecs.DrawEntity(quadID);
        ecs.DrawEntity(girlID);
        ecs.DrawEntity(cartID);
        ecs.DrawEntity(wolfID);
        ecs.DrawEntity(sitID);
        ecs.DrawEntity(light1ID);
        ecs.DrawEntity(light2ID);
        for (int i = 0; i < 5; i++) {
            ecs.DrawEntity(cigEnts[i]);
        }
        ecs.DrawEntity(axisID);
        ecs.DrawEntity(sphere1ID);
        ecs.DrawEntity(sphere2ID);
        ecs.DrawEntity(capsuleID);
        ecs.DrawEntity(chID);
        ecs.DrawScreenEntity(batEntID);
        ecs.DrawScreenEntity(cigEntID);
        */

        quadSys->DrawQuads(quadProgram, *globals.camera);

        sky->Draw(skyProgram, *globals.camera);

        ecs.DrawEntityStencils();

        ecs.DrawScreenEntity(batEntID);
        ecs.DrawScreenEntity(cigEntID);

        glEnable(GL_BLEND);
        std::sort(particleEmitter.particles.begin(), particleEmitter.particles.end(), Less);
        particleEmitter.DrawParticles(partProgram, *globals.camera);
        glDisable(GL_BLEND);

        glEnable(GL_BLEND);
        gui.RenderText(textProgram, "Cigarettes: " + std::to_string(cigCt), 25.0f, 25.0f, 1.0f, glm::vec3(1.f, 1.f, 1.f));
        glDisable(GL_BLEND);

        glfwSwapBuffers(window);
        // double te = glfwGetTime();
        // double elap = te - ts;
        // if (elap >= 0.03)
        // std::cerr << "frame time: " << elap << std::endl;
        return 1;
    }

private:
};
#endif

#endif
