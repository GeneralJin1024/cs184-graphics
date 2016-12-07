#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <memory>
#include <sstream>
#include <thread>
#include <chrono>

#include "BezierObject.h"
#include "Polygon.h"

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

#define ANIMATION 1
#define BRASS 0

const float PI = acos(-1.0);

using namespace std;

enum class transform_t {
  ROTATE, SCALE, TRANSLATE
};

void draw_uniform_subdivision();
void draw_adaptive_tessellation();
void parse_obj_file(const string&, const Transform<T, 3, Affine>&);
void parse_bez_file(const string&, const Transform<T, 3, Affine>&);
void parse_scene_file(const string&);
void draw_polygons();
void apply_global_transformation(const transform_t&, const Vector3f&);

/*
For UC Berkeley's CS184 Fall 2016 counter-clockwiserse, assignment 3 (Bezier surfaces)
*/

//****************************************************
// Global Variables
//****************************************************
GLfloat zoomf = 1.2, rotatef = PI * 0.1, translatef = 0.2;
Transform<T, 3, Affine> TRANS = Transform<T, 3, Affine>::Identity();
const Transform<T, 3, Affine> Identity_Trans = Transform<T, 3, Affine>::Identity();
bool auto_strech = false;
bool output_to_file = false;
int Width_global = 400;
int Height_global = 400;
int Z_buffer_bit_depth = 128;
int selected_item = 0, obj_iter = 0;

T subdivision_param = 0.01;
bool is_adaptive = false;
bool is_wireframe_mode = false;
bool is_smooth_shading = false;
string output_filename;
vector<unique_ptr<BezierObject>> bezier_objects;
vector<unique_ptr<Polygon>> polygons;

inline T sqr(T x) { return x*x; }

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
void inc_selected_item(int val) {
  selected_item += val;
  int N = (int)polygons.size() + (int)bezier_objects.size();
  if (selected_item < 0) selected_item = N - 1;
  if (selected_item == N) selected_item = 0;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    switch (key) {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
        case GLFW_KEY_Q: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
        case GLFW_KEY_S:
            if (action) is_smooth_shading = !is_smooth_shading; break;
        case GLFW_KEY_W:
            if (action) is_wireframe_mode = !is_wireframe_mode; break;
        case GLFW_KEY_EQUAL:
            if (action && mods == GLFW_MOD_SHIFT)
              apply_global_transformation(transform_t::SCALE, Vector3f(zoomf, zoomf, zoomf)); break;
        case GLFW_KEY_MINUS:
            if (action && mods != GLFW_MOD_SHIFT)
              apply_global_transformation(transform_t::SCALE, Vector3f(1.0/zoomf, 1.0/zoomf, 1.0/zoomf)); break;
        case GLFW_KEY_LEFT:
            if (action && mods == GLFW_MOD_SHIFT) { apply_global_transformation(transform_t::TRANSLATE, -translatef * Vector3f::UnitX()); break;}
            if (action) apply_global_transformation(transform_t::ROTATE, -Vector3f::UnitY()); break;
        case GLFW_KEY_RIGHT:
            if (action && mods == GLFW_MOD_SHIFT) { apply_global_transformation(transform_t::TRANSLATE, translatef * Vector3f::UnitX()); break;}
            if (action) apply_global_transformation(transform_t::ROTATE, Vector3f::UnitY()); break;
        case GLFW_KEY_UP:
            if (action && mods == GLFW_MOD_SHIFT) { apply_global_transformation(transform_t::TRANSLATE, translatef * Vector3f::UnitY()); break;}
            if (action) apply_global_transformation(transform_t::ROTATE, Vector3f::UnitX()); break;
        case GLFW_KEY_DOWN:
            if (action && mods == GLFW_MOD_SHIFT) { apply_global_transformation(transform_t::TRANSLATE, -translatef * Vector3f::UnitY()); break;}
            if (action) apply_global_transformation(transform_t::ROTATE, -Vector3f::UnitX()); break;
        case GLFW_KEY_F:
            if (action) auto_strech = !auto_strech; break;
        case GLFW_KEY_D:
            if (action) inc_selected_item(1); break;
        case GLFW_KEY_A:
            if (action) inc_selected_item(-1); break;
        case GLFW_KEY_SPACE: break;

        default: break;
    }
}

//****************************************************
// function that does the actual drawing of stuff
//***************************************************
int display_iter = -1;
void display(GLFWwindow* window) {
#if ANIMATION==1
    this_thread::sleep_for(std::chrono::milliseconds(40));
    ++display_iter;
    if (display_iter == 1)
      this_thread::sleep_for(std::chrono::seconds(10));
    if (display_iter > 1) {
      if (display_iter <= 20)
        apply_global_transformation(transform_t::ROTATE, Vector3f::UnitX());
      else if (display_iter <= 40)
        apply_global_transformation(transform_t::ROTATE, Vector3f::UnitY());
      else if (display_iter <= 60)
        apply_global_transformation(transform_t::ROTATE, Vector3f::UnitZ());
      else if (display_iter <= 80)
        apply_global_transformation(transform_t::ROTATE, (Vector3f::UnitX() + Vector3f::UnitY() + Vector3f::UnitZ()).normalized());
      else if (display_iter <= 85)
        apply_global_transformation(transform_t::SCALE, Vector3f(zoomf, zoomf, zoomf));
      else if (display_iter <= 90)
        apply_global_transformation(transform_t::SCALE, Vector3f(1.0/zoomf, 1.0/zoomf, 1.0/zoomf));
      else if (display_iter <= 95)
        apply_global_transformation(transform_t::TRANSLATE, -translatef * Vector3f::UnitX());
      else if (display_iter <= 105)
        apply_global_transformation(transform_t::TRANSLATE, -translatef * Vector3f::UnitY());
      else if (display_iter <= 115)
        apply_global_transformation(transform_t::TRANSLATE, translatef * Vector3f::UnitX());
      else if (display_iter <= 125)
        apply_global_transformation(transform_t::TRANSLATE, translatef * Vector3f::UnitY());
      else if (display_iter <= 130)
        apply_global_transformation(transform_t::TRANSLATE, -translatef * Vector3f::UnitX());
    }
#endif
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f ); //clear background screen to black

    if (is_wireframe_mode) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (is_smooth_shading) {
      glShadeModel(GL_SMOOTH);
    } else {
      glShadeModel(GL_FLAT);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                // clear the color buffer (sets everything to black)
    glMatrixMode(GL_MODELVIEW);                  // indicate we are specifying camera transformations
    glLoadIdentity();                            // make sure transformation is "zero'd"

    //----------------------- code to draw objects --------------------------
    glPushMatrix();

    if (is_adaptive) {
      draw_adaptive_tessellation();
    } else {
      draw_uniform_subdivision();
    }
    draw_polygons();

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
    // glfwGetFramebufferSize(window, &Width_global, &Height_global);

    // glViewport(0, 0, Width_global, Height_global);
    // glMatrixMode(GL_PROJECTION);
    // glLoadIdentity();
    // glOrtho(0, Width_global, 0, Height_global, 1, -1);

    display(window);
}

void parse_command_line_options(int argc, char *argv[]) {
    if (argc > 1) {
        string input_filename = string(argv[1]);
        int len = (int)input_filename.length();
        if (len >= 4) {
          if (input_filename[len-3] == 'b' &&
              input_filename[len-2] == 'e' &&
              input_filename[len-1] == 'z') {
            parse_bez_file(input_filename, Identity_Trans);
          } else if (input_filename[len-3] == 'o' &&
                     input_filename[len-2] == 'b' &&
                     input_filename[len-1] == 'j') {
            parse_obj_file(input_filename, Identity_Trans);
          }
        }
        if (len >= 6) {
          if (input_filename[len-5] == 's' &&
              input_filename[len-4] == 'c' &&
              input_filename[len-3] == 'e' &&
              input_filename[len-2] == 'n' &&
              input_filename[len-1] == 'e') {
            parse_scene_file(input_filename);
          }
        }
    }

    if (argc > 2) {
        subdivision_param = stof(argv[2]);
    }

    int i = 3;
    while (i < argc) {
      if (argv[i][0] == '-') {
        if (argv[i][1] == 'o') {
          i += 1;
          output_filename = string(argv[i]);
          output_to_file = true;
          i += 1;
        } else if (argv[i][1] == 'a') {
          is_adaptive = true;
          i += 1;
        }
      }
    }
}

void apply_transformation_helper(Transform<T, 3, Affine> &TRANS,
                                 const transform_t &trans_type,
                                 const Vector3f &values) {
  auto back_trans(Translation<T, 3>(-TRANS.translation()));
  auto fwd_trans(Translation<T, 3>(TRANS.translation()));
  TRANS = back_trans * TRANS;
  if (trans_type == transform_t::SCALE) {
    TRANS = Scaling(values) * TRANS;
  } else if (trans_type == transform_t::ROTATE) {
    TRANS = AngleAxis<T>(rotatef, values) * TRANS;
  } else if (trans_type == transform_t::TRANSLATE) {
    TRANS = Translation<T, 3>(values) * TRANS;
  }
  TRANS = fwd_trans * TRANS;
}

void apply_global_transformation(const transform_t &trans_type, const Vector3f &values) {
  int N = (int)bezier_objects.size();
  if (selected_item < N) {
    apply_transformation_helper(bezier_objects[selected_item]->transformation, trans_type, values);
  } else {
    apply_transformation_helper(polygons[selected_item - N]->transformation, trans_type, values);
  }
}

void parse_scene_file(const string &input_filename) {
  ifstream fin(input_filename);
  string line, type;
  Transform<T, 3, Affine> transformation = Transform<T, 3, Affine>::Identity();

  while (getline(fin, line)) {
    stringstream ss;
    ss << line;
    ss >> type;

    string spec_filename;
    if (type == "obj") {
      ss >> spec_filename;
      parse_obj_file(spec_filename, transformation);
    } else if (type == "bez") {
      ss >> spec_filename;
      parse_bez_file(spec_filename, transformation);
    } else if ((int)type.length() == 3 && type[0] == 'x' && type[1] == 'f') {
      // transformation
      if (type[2] == 't') {
        T x, y, z;
        ss >> x >> y >> z;
        transformation = Translation<T, 3>(x, y, z) * transformation;
      } else if (type[2] == 'r') {
        T x, y, z, rad;
        ss >> x >> y >> z >> rad;

        transformation = AngleAxis<T>(rad, Vector3f(x, y, z).normalized());
      } else if (type[2] == 's') {
        T x, y, z;
        ss >> x >> y >> z;
        transformation = Scaling(x, y, z) * transformation;
      } else if (type[2] == 'z') {
        transformation = Transform<T, 3, Affine>::Identity();
      }
    }
  }
}

void parse_obj_file(const string &input_filename,
                    const Transform<T, 3, Affine> &transformation) {
  T max_coord = -1e-10;
  ifstream fin(input_filename);
  string line;
  char type;
  unique_ptr<Polygon> polygon(new Polygon(++obj_iter));

  while (getline(fin, line)) {
    stringstream ss;
    ss << line;
    ss >> type;

    if (type == 'v') {
      T x, y, z;
      ss >> x >> y >> z;
      max_coord = max(max_coord, max(fabs(x), max(fabs(y), fabs(z))));
      polygon->vertices.push_back(move(unique_ptr<Vector3f>(new Vector3f(x, y, z))));
    } else {
      vector<int> faces;
      int x;
      while (ss >> x) {
        faces.push_back(x-1);
      }
      polygon->faces.push_back(move(faces));
    }
  }

  T scale_factor = 1.0f / (max_coord * 1.25f); // scale to [-0.5, 0.5]
  auto scale_vector = Vector3f(scale_factor, scale_factor, scale_factor);
  polygon->transformation = transformation * Scaling(scale_vector) * polygon->transformation;
  polygons.push_back(move(polygon));
}

void parse_bez_file(const string &input_filename,
                    const Transform<T, 3, Affine> &transformation) {
    ifstream fin(input_filename);
    int num_patches;

    fin >> num_patches;

    unique_ptr<BezierObject> bezier_object(new BezierObject(++obj_iter));
    while (num_patches--) {
        vector<vector<shared_ptr<Vector3f>>> points;
        for (int i = 0; i < 4; i++) {
            vector<shared_ptr<Vector3f>> rowvec;
            for (int j = 0; j < 4; j++) {
                T x, y, z;
                fin >> x >> y >> z;
                x += 1e-10;
                y += 1e-10;
                z += 1e-10;
                shared_ptr<Vector3f> v_pt(new Vector3f(x, y, z));
                rowvec.push_back(move(v_pt));
            }
            points.push_back(move(rowvec));
        }
        unique_ptr<BezierPatch> bezier_pt(new BezierPatch(move(points)));
        bezier_object->patches.push_back(move(bezier_pt));
    }
    bezier_object->transformation = transformation;
    bezier_objects.push_back(move(bezier_object));;
}

void compute_uniform_subdivisions() {
  for (const auto &bezier_object : bezier_objects) {
    for (const auto &patch : bezier_object->patches) {
      patch->uniform_subdivision(subdivision_param);
    }
  }
}

const GLfloat WHITE[] = {1.0f, 1.0f, 0.0f};
void brass_material() {
  GLfloat mat_ambient[]={0.329412f,0.223529f,0.027451f};
  GLfloat mat_diffuse[]={0.780392f,0.568627f,0.113725f };
  GLfloat mat_specular[]={0.992157f, 0.941176f, 0.807843f };
  glMaterialfv (GL_FRONT,GL_AMBIENT,mat_ambient);
  glMaterialfv (GL_FRONT,GL_DIFFUSE,mat_diffuse);
  glMaterialfv (GL_FRONT,GL_SPECULAR,mat_specular);
  glMaterialf (GL_FRONT,GL_SHININESS,27.89743616);
}

void draw_uniform_subdivision() {
  compute_uniform_subdivisions();

  int index = 0;
  for (const auto &bezier_object : bezier_objects) {
    for (const auto &patch : bezier_object->patches) {
      if (patch->uniform_points.empty()) break;
      int N = (int)patch->uniform_points.size();
      int M = (int)patch->uniform_points[0].size();
      for (int i = 0; i < N - 1; i++) {
        for (int j = 0; j < M - 1; j++) {
          auto p0 = bezier_object->transformation * (*patch->uniform_points[i][j]);
          auto p1 = bezier_object->transformation * (*patch->uniform_points[i+1][j]);
          auto p2 = bezier_object->transformation * (*patch->uniform_points[i][j+1]);
          auto p3 = bezier_object->transformation * (*patch->uniform_points[i+1][j+1]);

          auto n0 = bezier_object->transformation.linear() * (*patch->uniform_normals[i][j]);
          auto n1 = bezier_object->transformation.linear() * (*patch->uniform_normals[i+1][j]);
          auto n2 = bezier_object->transformation.linear() * (*patch->uniform_normals[i][j+1]);
          auto n3 = bezier_object->transformation.linear() * (*patch->uniform_normals[i+1][j+1]);

          glBegin(GL_QUADS);
            if ((int)bezier_objects.size() + (int)polygons.size() > 1 && selected_item == index) {
              glMaterialfv (GL_FRONT,GL_AMBIENT,WHITE);
              glMaterialfv (GL_FRONT,GL_DIFFUSE,WHITE);
              glMaterialfv (GL_FRONT,GL_SPECULAR,WHITE);
              glMaterialf (GL_FRONT,GL_SHININESS,1.0f);
            } else {
              #if BRASS == 0
              glMaterialfv (GL_FRONT,GL_AMBIENT,bezier_object->mat_ambient);
              glMaterialfv (GL_FRONT,GL_DIFFUSE,bezier_object->mat_diffuse);
              glMaterialfv (GL_FRONT,GL_SPECULAR,bezier_object->mat_specular);
              glMaterialf (GL_FRONT,GL_SHININESS,bezier_object->shininess);
              #else
              brass_material();
              #endif
            }
            glNormal3f(n0[0], n0[1], n0[2]);
            glVertex3f(p0[0], p0[1], p0[2]);
            glNormal3f(n1[0], n1[1], n1[2]);
            glVertex3f(p1[0], p1[1], p1[2]);
            glNormal3f(n2[0], n2[1], n2[2]);
            glVertex3f(p2[0], p2[1], p2[2]);
            glNormal3f(n3[0], n3[1], n3[2]);
            glVertex3f(p3[0], p3[1], p3[2]);
          glEnd();
        }
      }
    }
    ++index;
  }
}

void compute_adaptive_tessellations() {
  for (const auto &bezier_object : bezier_objects) {
    for (const auto &patch : bezier_object->patches) {
      patch->adaptive_tessellation(subdivision_param);
    }
  }
}

void draw_adaptive_tessellation() {
  compute_adaptive_tessellations();

  int index = 0;
  for (const auto &bezier_object : bezier_objects) {
    for (const auto &patch : bezier_object->patches) {
      for (int i = 0; i < (int)patch->adaptive_points.size(); i++) {
        auto p0 = bezier_object->transformation * (*patch->adaptive_points[i][0]);
        auto p1 = bezier_object->transformation * (*patch->adaptive_points[i][1]);
        auto p2 = bezier_object->transformation * (*patch->adaptive_points[i][2]);

        auto n0 = bezier_object->transformation.linear() * (*patch->adaptive_normals[i][0]);
        auto n1 = bezier_object->transformation.linear() * (*patch->adaptive_normals[i][1]);
        auto n2 = bezier_object->transformation.linear() * (*patch->adaptive_normals[i][2]);

        glBegin(GL_TRIANGLES);
          if ((int)bezier_objects.size() + (int)polygons.size() > 1 && selected_item == index) {
            glMaterialfv (GL_FRONT,GL_AMBIENT,WHITE);
            glMaterialfv (GL_FRONT,GL_DIFFUSE,WHITE);
            glMaterialfv (GL_FRONT,GL_SPECULAR,WHITE);
            glMaterialf (GL_FRONT,GL_SHININESS,1.0f);
          } else {
            #if BRASS == 0
            glMaterialfv (GL_FRONT,GL_AMBIENT,bezier_object->mat_ambient);
            glMaterialfv (GL_FRONT,GL_DIFFUSE,bezier_object->mat_diffuse);
            glMaterialfv (GL_FRONT,GL_SPECULAR,bezier_object->mat_specular);
            glMaterialf (GL_FRONT,GL_SHININESS,bezier_object->shininess);
            #else
            brass_material();
            #endif
          }
          glNormal3f(n0[0], n0[1], n0[2]);
          glVertex3f(p0[0], p0[1], p0[2]);
          glNormal3f(n1[0], n1[1], n1[2]);
          glVertex3f(p1[0], p1[1], p1[2]);
          glNormal3f(n2[0], n2[1], n2[2]);
          glVertex3f(p2[0], p2[1], p2[2]);
        glEnd();
      }
    }
    ++index;
  }
}

void draw_polygons() {
  int index = 0;
  for (const auto &polygon : polygons) {
    for (const auto &face : polygon->faces) {
      glBegin(GL_POLYGON);
        if ((int)bezier_objects.size() + index == selected_item) {
          glMaterialfv (GL_FRONT,GL_AMBIENT,WHITE);
          glMaterialfv (GL_FRONT,GL_DIFFUSE,WHITE);
          glMaterialfv (GL_FRONT,GL_SPECULAR,WHITE);
          glMaterialf (GL_FRONT,GL_SHININESS,1.0f);
        } else {
          #if BRASS == 0
          glMaterialfv (GL_FRONT,GL_AMBIENT,polygon->mat_ambient);
          glMaterialfv (GL_FRONT,GL_DIFFUSE,polygon->mat_diffuse);
          glMaterialfv (GL_FRONT,GL_SPECULAR,polygon->mat_specular);
          glMaterialf (GL_FRONT,GL_SHININESS,polygon->shininess);
          #else
          brass_material();
          #endif
        }
        for (const int idx : face) {
          auto pt = polygon->transformation * (*polygon->vertices[idx]);
          glVertex3f(pt[0], pt[1], pt[2]);
        }
      glEnd();
    }
    ++index;
  }
}

inline void init_objects() {
  right_mat << 1,  0,  0, 0,
              -3,  3,  0, 0,
               3, -6,  3, 0,
              -1,  3, -3, 1;
  left_mat = right_mat.transpose();
}

void generate_obj_output() {
  cout << "Computing tessellations.." << endl;
  ofstream fout(output_filename);
  if (is_adaptive) {
    compute_adaptive_tessellations();
  } else {
    compute_uniform_subdivisions();
  }
  cout << "Generating .obj file.." << endl;
  int count = 1;
  for (const auto &bezier_object : bezier_objects) {
    for (const auto &patch : bezier_object->patches) {
      for (int i = 0; i < (int)patch->adaptive_points.size(); i++) {
        auto p0 = bezier_object->transformation * (*patch->adaptive_points[i][0]);
        auto p1 = bezier_object->transformation * (*patch->adaptive_points[i][1]);
        auto p2 = bezier_object->transformation * (*patch->adaptive_points[i][2]);

        fout << "v " << p0[0] << " " << p0[1] << " " << p0[2] << endl;
        fout << "v " << p1[0] << " " << p1[1] << " " << p1[2] << endl;
        fout << "v " << p2[0] << " " << p2[1] << " " << p2[2] << endl;
        fout << "f " << count++;
        fout << " " << count++;
        fout << " " << count++ << endl;
      }
    }
  }
  cout << "Done!" << endl;
}

//****************************************************
// the usual stuff, nothing exciting here
//****************************************************
int main(int argc, char *argv[]) {
    parse_command_line_options(argc, argv);
    init_objects();

    if (output_to_file) {
      generate_obj_output();
      return 0;
    }

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
    glOrtho(-3.5, 3.5, -3.5, 3.5, 5, -5);

    glEnable(GL_RESCALE_NORMAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);  // enable z-buffering
    glDepthFunc(GL_LESS);

    glfwSetWindowTitle(window, "CS184");
    glfwSetWindowSizeCallback(window, size_callback);
    glfwSetKeyCallback(window, key_callback);

    glEnable(GL_LIGHTING);
    GLfloat light_ambient[]={0.0,0.0,0.0,1.0};
    GLfloat light_diffuse[]={1.0,1.0,1.0,1.0};
    GLfloat light_specular[]={1.0,1.0,1.0,1.0};
    GLfloat light_position[]={1.0,1.0,1.0,0.0};
    GLfloat light_position1[]={-1.0,-1.0,-1.0,0.0};
    glLightfv(GL_LIGHT0,GL_AMBIENT,light_ambient);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,light_diffuse);
    glLightfv(GL_LIGHT0,GL_SPECULAR,light_specular);
    glLightfv(GL_LIGHT0,GL_POSITION,light_position);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT1,GL_AMBIENT,light_ambient);
    glLightfv(GL_LIGHT1,GL_DIFFUSE,light_diffuse);
    glLightfv(GL_LIGHT1,GL_SPECULAR,light_specular);
    glLightfv(GL_LIGHT1,GL_POSITION,light_position1);
    glEnable(GL_LIGHT1);

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
