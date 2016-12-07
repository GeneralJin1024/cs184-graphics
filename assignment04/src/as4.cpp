#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <memory>
#include <sstream>
#include <thread>
#include <chrono>

#include "include.h"
#include "Arm.h"

//include header file for glfw library so that we can use OpenGL
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
static DWORD lastTime;
#else
static struct timeval lastTime;
#endif

using namespace std;

//****************************************************
// Global Variables
//****************************************************
bool auto_strech = false;
int Width_global = 600;
int Height_global = 600;
unique_ptr<Arm> arm, secArm, terArm;
Vector3f goal;
T anglex, angley, anglez;
T zoomf = 1.2, curzoom = 0.3, translatef = 0.2, tx, ty, tz;
T rotatex = 0.0, rotatey = 0.0, rotatef = 10.0, scalef = 8.0;
T cubelen = 0.25;
Vector3f color1, color2, color3, color;

//****************************************************
// Simple init function
//****************************************************
void initializeRendering() {
    glfwInit();
}

//****************************************************
// A routine to set a pixel by drawing a GL point.  This is not a
// general purpose routine as it assumes a lot of stuff specific to
// this example.
//****************************************************
void setPixel(T x, T y, GLfloat r, GLfloat g, GLfloat b) {
    glColor3f(r, g, b);
    glVertex2f(x + 0.5, y + 0.5);  // The 0.5 is to target pixel centers
    // Note: Need to check for gap bug on inst machines.
}

//****************************************************
// Keyboard inputs. Add things to match the spec!
//****************************************************
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    switch (key) {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
        case GLFW_KEY_Q: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
        case GLFW_KEY_EQUAL:
          if (action && mods == GLFW_MOD_SHIFT)
            curzoom *= zoomf; break;
        case GLFW_KEY_MINUS:
          if (action && mods != GLFW_MOD_SHIFT)
            curzoom /= zoomf; break;
        case GLFW_KEY_LEFT:
            if (action && mods == GLFW_MOD_SHIFT) { tx -= translatef; break;}
            if (action) rotatey -= rotatef; break;
        case GLFW_KEY_RIGHT:
            if (action && mods == GLFW_MOD_SHIFT) { tx += translatef; break;}
            if (action) rotatey += rotatef; break;
        case GLFW_KEY_UP:
            if (action && mods == GLFW_MOD_SHIFT) { ty += translatef; break;}
            if (action) rotatex += rotatef; break;
        case GLFW_KEY_DOWN:
            if (action && mods == GLFW_MOD_SHIFT) { ty -= translatef; break;}
            if (action) rotatex -= rotatef; break;
        case GLFW_KEY_SPACE: break;
        default: break;
    }
}

//****************************************************
// function that does the actual drawing of stuff
//***************************************************
void generateCube(Vector3f center) {
  glBegin(GL_QUADS);
    glColor3f(0.8f, 0.8f, 0.8f);
    glVertex3f(center[0] - cubelen, center[1] - cubelen, center[2] - cubelen);
    glVertex3f(center[0] + cubelen, center[1] - cubelen, center[2] - cubelen);
    glVertex3f(center[0] + cubelen, center[1] + cubelen, center[2] - cubelen);
    glVertex3f(center[0] - cubelen, center[1] + cubelen, center[2] - cubelen);

    glVertex3f(center[0] - cubelen, center[1] - cubelen, center[2] - cubelen);
    glVertex3f(center[0] - cubelen, center[1] - cubelen, center[2] + cubelen);
    glVertex3f(center[0] - cubelen, center[1] + cubelen, center[2] + cubelen);
    glVertex3f(center[0] - cubelen, center[1] + cubelen, center[2] - cubelen);

    glVertex3f(center[0] - cubelen, center[1] - cubelen, center[2] - cubelen);
    glVertex3f(center[0] - cubelen, center[1] - cubelen, center[2] + cubelen);
    glVertex3f(center[0] + cubelen, center[1] - cubelen, center[2] + cubelen);
    glVertex3f(center[0] + cubelen, center[1] - cubelen, center[2] - cubelen);

    glVertex3f(center[0] - cubelen, center[1] - cubelen, center[2] + cubelen);
    glVertex3f(center[0] + cubelen, center[1] - cubelen, center[2] + cubelen);
    glVertex3f(center[0] + cubelen, center[1] + cubelen, center[2] + cubelen);
    glVertex3f(center[0] - cubelen, center[1] + cubelen, center[2] + cubelen);

    glVertex3f(center[0] + cubelen, center[1] - cubelen, center[2] - cubelen);
    glVertex3f(center[0] + cubelen, center[1] - cubelen, center[2] + cubelen);
    glVertex3f(center[0] + cubelen, center[1] + cubelen, center[2] + cubelen);
    glVertex3f(center[0] + cubelen, center[1] + cubelen, center[2] - cubelen);

    glVertex3f(center[0] - cubelen, center[1] + cubelen, center[2] - cubelen);
    glVertex3f(center[0] - cubelen, center[1] + cubelen, center[2] + cubelen);
    glVertex3f(center[0] + cubelen, center[1] + cubelen, center[2] + cubelen);
    glVertex3f(center[0] + cubelen, center[1] + cubelen, center[2] - cubelen);
  glEnd();
}

void generateEightFigure() {
  anglex += (360.0 / 120.0);
  angley += (360.0 / 120.0);
  anglez += (360.0 / 120.0);

  glBegin(GL_LINE_LOOP);
    glColor3f(color[0], color[1], color[2]);
    for (int i = 0; i <= 60; i++) {
      Vector3f pt(cos(6.0 * i * PI / 180.0), sin(2.0 * 6.0 * i * PI / 180.0), sin(6.0 * i * PI / 180.0));
      pt *= scalef;
      pt[0] /= 2.0;
      glVertex3f(pt[0], pt[1], pt[2]);
    }
  glEnd();

  goal = Vector3f(cos(anglex * PI / 180.0), sin(2.0 * angley * PI / 180.0), sin(anglez * PI / 180.0));
  goal *= scalef;
  goal[0] /= 2.0;

  generateCube(goal);
}

void generateCircle() {
  anglex += (360.0 / 120.0);
  angley += (360.0 / 120.0);
  anglez += (360.0 / 120.0);

  glBegin(GL_LINE_LOOP);
    glColor3f(color[0], color[1], color[2]);
    for (int i = 0; i <= 60; i++) {
      Vector3f pt(cos(6.0 * i * PI / 180.0), sin(6.0 * i * PI / 180.0), sin(6.0 * i * PI / 180.0));
      pt *= scalef;
      pt[0] /= 2.0;
      glVertex3f(pt[0], pt[1], pt[2]);
    }
  glEnd();

  goal = Vector3f(cos(anglex * PI / 180.0), sin(angley * PI / 180.0), sin(anglez * PI / 180.0));
  goal *= scalef;
  goal[0] /= 2.0;

  generateCube(goal);
}

void generateLine() {
  anglex += (360.0 / 120.0);
  angley += (360.0 / 120.0);
  anglez += (360.0 / 120.0);

  glBegin(GL_LINE_LOOP);
    glColor3f(color[0], color[1], color[2]);
    for (int i = 0; i <= 60; i++) {
      Vector3f pt(cos(6.0 * i * PI / 180.0), sin(2.0 * 6.0 * i * PI / 180.0), 0.0);
      pt *= scalef;
      glVertex3f(pt[0], pt[1], pt[2]);
    }
  glEnd();

  goal = Vector3f(cos(anglez * PI / 180.0), sin(2.0 * angley * PI / 180.0), 0.0);
  goal *= scalef;
  generateCube(goal);
}

void generateLargeEight() {
  anglex += (360.0 / 120.0);
  angley += (360.0 / 120.0);
  anglez += (360.0 / 120.0);

  glBegin(GL_LINE_LOOP);
    glColor3f(color[0], color[1], color[2]);
    for (int i = 0; i <= 120; i++) {
      Vector3f pt(cos(6.0 * i * PI / 180.0), sin(6.0 * i * PI / 180.0) * 0.3, sin(2.0 * 6.0 * i * PI / 180.0));
      pt *= scalef * 1.2;
      glVertex3f(pt[0], pt[1], pt[2]);
    }
  glEnd();

  goal = Vector3f(cos(anglex * PI / 180.0), sin(angley * PI / 180.0) * 0.3, sin(2.0 * anglez * PI / 180.0));
  goal *= scalef * 1.2;
  generateCube(goal);
}

void display(GLFWwindow* window) {
    this_thread::sleep_for(chrono::milliseconds(100));
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f ); //clear background screen to black

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                // clear the color buffer (sets everything to black)
    glMatrixMode(GL_MODELVIEW);                  // indicate we are specifying camera transformations
    glLoadIdentity();                            // make sure transformation is "zero'd"
    glScalef(curzoom, curzoom, curzoom);
    glRotatef(rotatey, 0.0, 1.0, 0.0);
    glRotatef(rotatex, 1.0, 0.0, 0.0);
    glTranslatef(tx, ty, tz);

    //----------------------- code to draw objects --------------------------
    glPushMatrix();
    scalef = 0.0;
    for (int i = 0; i < (int)arm->joints.size(); i++) {
      scalef += arm->joints[i]->length;
    }
    scalef *= 1.2;
    color = color1;
    generateEightFigure();

    arm->solve(goal);
    arm->draw();

    scalef = 0.0;
    for (int i = 0; i < (int)secArm->joints.size(); i++) {
      scalef += secArm->joints[i]->length;
    }
    scalef *= 0.8;
    color = color2;
    generateLargeEight();

    secArm->solve(goal);
    secArm->draw();

    // scalef = 0.0;
    // for (int i = 0; i < (int)terArm->joints.size(); i++) {
    //   scalef += terArm->joints[i]->length;
    // }
    // color = color3;
    // generateEightFigure();

    // terArm->solve(goal);
    // terArm->draw();

    glPopMatrix();

    glfwSwapBuffers(window);

    // note: check out glPolygonMode and glShadeModel
    // for wireframe and shading commands
}

//****************************************************
// function that is called when window is resized
//***************************************************
void size_callback(GLFWwindow* window, int width, int height)
{
    // Get the pixel coordinate of the window
    // it returns the size, in pixels, of the framebuffer of the specified window
    glfwGetFramebufferSize(window, &Width_global, &Height_global);

    glViewport(0, 0, Width_global, Height_global);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Width_global, 0, Height_global, 1, -1);

    display(window);
}

//****************************************************
// the usual stuff, nothing exciting here
//****************************************************
int main(int argc, char *argv[]) {
    //This initializes glfw
    initializeRendering();

    GLFWwindow* window = glfwCreateWindow(Width_global, Height_global, "CS184", NULL, NULL);
    if (!window) {
        cerr << "Error on window creating" << endl;
        glfwTerminate();
        return -1;
    }

    const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if ( !mode ) {
        cerr << "Error on getting monitor" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Get the pixel coordinate of the window
    // it returns the size, in pixels, of the framebuffer of the specified window
    glfwGetFramebufferSize(window, &Width_global, &Height_global);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-6.5, 6.5, -6.5, 6.5, 5, -10);

    glEnable(GL_RESCALE_NORMAL);
    glEnable(GL_DEPTH_TEST);  // enable z-buffering
    glDepthFunc(GL_LESS);

    glfwSetWindowTitle(window, "CS184");
    glfwSetWindowSizeCallback(window, size_callback);
    glfwSetKeyCallback(window, key_callback);

    // glEnable(GL_LIGHTING);
    // GLfloat light_ambient[]={0.0,0.0,0.0,1.0};
    // GLfloat light_diffuse[]={1.0,1.0,1.0,1.0};
    // GLfloat light_specular[]={1.0,1.0,1.0,1.0};
    // GLfloat light_position[]={1.0,1.0,1.0,0.0};
    // GLfloat light_position1[]={-1.0,-1.0,-1.0,0.0};
    // glLightfv(GL_LIGHT0,GL_AMBIENT,light_ambient);
    // glLightfv(GL_LIGHT0,GL_DIFFUSE,light_diffuse);
    // glLightfv(GL_LIGHT0,GL_SPECULAR,light_specular);
    // glLightfv(GL_LIGHT0,GL_POSITION,light_position);
    // glEnable(GL_LIGHT0);
    // glLightfv(GL_LIGHT1,GL_AMBIENT,light_ambient);
    // glLightfv(GL_LIGHT1,GL_DIFFUSE,light_diffuse);
    // glLightfv(GL_LIGHT1,GL_SPECULAR,light_specular);
    // glLightfv(GL_LIGHT1,GL_POSITION,light_position1);
    // glEnable(GL_LIGHT1);

    vector<unique_ptr<BallJoint>> joints, secJoints, terJoints;
    joints.push_back(move(unique_ptr<BallJoint>(new BallJoint(1.0))));
    joints.push_back(move(unique_ptr<BallJoint>(new BallJoint(2.0))));
    joints.push_back(move(unique_ptr<BallJoint>(new BallJoint(3.0))));
    joints.push_back(move(unique_ptr<BallJoint>(new BallJoint(4.0))));

    secJoints.push_back(move(unique_ptr<BallJoint>(new BallJoint(3.0))));
    secJoints.push_back(move(unique_ptr<BallJoint>(new BallJoint(6.0))));
    secJoints.push_back(move(unique_ptr<BallJoint>(new BallJoint(4.0))));
    secJoints.push_back(move(unique_ptr<BallJoint>(new BallJoint(2.0))));

    terJoints.push_back(move(unique_ptr<BallJoint>(new BallJoint(5.0))));
    terJoints.push_back(move(unique_ptr<BallJoint>(new BallJoint(2.0))));
    terJoints.push_back(move(unique_ptr<BallJoint>(new BallJoint(1.0))));
    terJoints.push_back(move(unique_ptr<BallJoint>(new BallJoint(3.0))));

    arm.reset(new Arm());
    arm->set_joints(move(joints));
    secArm.reset(new Arm());
    secArm->set_joints(move(secJoints));
    terArm.reset(new Arm());
    terArm->set_joints(move(terJoints));

    color1[0] = rand()*1.0 / RAND_MAX;
    color1[1] = rand()*1.0 / RAND_MAX;
    color1[2] = rand()*1.0 / RAND_MAX;
    color2[0] = rand()*1.0 / RAND_MAX;
    color2[1] = rand()*1.0 / RAND_MAX;
    color2[2] = rand()*1.0 / RAND_MAX;
    color3[0] = rand()*1.0 / RAND_MAX;
    color3[1] = rand()*1.0 / RAND_MAX;
    color3[2] = rand()*1.0 / RAND_MAX;

    while (!glfwWindowShouldClose(window)) { // infinite loop to draw object again and again
        // because once object is draw then window is terminated
        display(window);

        if (auto_strech) {
            glfwSetWindowSize(window, mode->width, mode->height);
            glfwSetWindowPos(window, 0, 0);
        }

        glfwPollEvents();
    }
    return 0;
}
