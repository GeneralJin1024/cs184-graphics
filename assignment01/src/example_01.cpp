#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <map>

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

const float PI = acos(-1.0);
inline float sqr(float x) { return x*x; }

class Light {
 public:
     Light() = default;
     explicit Light(float x_, float y_, float z_,
                    float r_, float g_, float b_) :
        x(x_), y(y_), z(z_), r(r_), g(g_), b(b_) {}
     ~Light() = default;

     float x, y, z, r, g, b;
};

class Vector {
 public:
     Vector() = default;
     explicit Vector(float x_, float y_, float z_) :
        x(x_), y(y_), z(z_) {}
     Vector(const Vector &other) {
        x = other.x;
        y = other.y;
        z = other.z;
     }
     ~Vector() = default;
     friend Vector operator+(Vector a, const Vector &b) {
        a.x += b.x;
        a.y += b.y;
        a.z += b.z;
        return a;
     }
    friend Vector operator+(Vector a, float b) {
        a.x += b;
        a.y += b;
        a.z += b;
        return a;
     }
     friend Vector operator-(Vector a, const Vector &b) {
        a.x -= b.x;
        a.y -= b.y;
        a.z -= b.z;
        return a;
     }
    friend Vector operator*(float k, Vector a) {
        a.x *= k;
        a.y *= k;
        a.z *= k;
        return a;
    }
    friend Vector operator*(Vector a, float k) {
        a.x *= k;
        a.y *= k;
        a.z *= k;
        return a;
    }
    friend Vector operator/(Vector a, float k) {
        a.x /= k;
        a.y /= k;
        a.z /= k;
        return a;
    }
    friend Vector operator*(Vector a, const Vector &b) {
        a.x *= b.x;
        a.y *= b.y;
        a.z *= b.z;
        return a;
    }
    friend Vector operator-(float a, Vector b) {
        b.x = a - b.x;
        b.y = a - b.y;
        b.z = a - b.z;
        return b;
    } 
    float dot(const Vector &b) const {
        return x * b.x + y * b.y + z * b.z;
    }
    Vector cross(const Vector &b) const {
        return Vector(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
    }
    float norm() const {
        return sqrt(sqr(x) + sqr(y) + sqr(z));
    }
    void normalize() {
        float d = sqrt(sqr(x) + sqr(y) + sqr(z));
        x /= d;
        y /= d;
        z /= d;
    }
    void init(float x_, float y_, float z_) {
        x = x_;
        y = y_;
        z = z_;
    }
    friend ostream& operator<< (ostream &out, const Vector &v) {
        out << "("
            << v.x << ", "
            << v.y << ", "
            << v.z 
            << ")";
        return out;
    }
    friend ostream& operator<< (ostream &out, Vector &v) {
        out << "("
            << v.x << ", "
            << v.y << ", "
            << v.z 
            << ")";
        return out;
    }

     float x, y, z;
};

//****************************************************
// Global Variables
//****************************************************
GLfloat translation[3] = {0.0f, 0.0f, 0.0f};
bool auto_strech = false;
int Width_global = 400;
int Height_global = 400;
float ar, ag, ab;
float dr, dg, db;
float sr, sg, sb;
float power_u, power_v, power_isotropic;
vector<Light> point_lights;
vector<Light> directional_lights;
string output_filename;
bool asms = false, is_toon = false;
Vector ambient_coeffs;
Vector diffuse_coeffs;
Vector specular_coeffs;
const vector<float> toon_diffuse_intervals = {0.0, 0.1, 0.3, 0.5, 0.7, 0.9, 1.0};
const vector<float> toon_specular_intervals = {0.0, 0.5, 1.0};

//****************************************************
// Simple init function
//****************************************************
void initializeRendering()
{
    glfwInit();
}


//****************************************************
// A routine to set a pixel by drawing a GL point.  This is not a
// general purpose routine as it assumes a lot of stuff specific to
// this example.
//****************************************************
void setPixel(float x, float y, Vector v) {
    glColor3f(v.x, v.y, v.z);
    glVertex2f(x+0.5, y+0.5);  // The 0.5 is to target pixel centers
    // Note: Need to check for gap bug on inst machines.
}

//****************************************************
// Keyboard inputs
//****************************************************
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (key) {
            
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
        case GLFW_KEY_Q: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
        case GLFW_KEY_LEFT :
            if (action) translation[0] -= 0.01f * Width_global; break;
        case GLFW_KEY_RIGHT:
            if (action) translation[0] += 0.01f * Width_global; break;
        case GLFW_KEY_UP   :
            if (action) translation[1] += 0.01f * Height_global; break;
        case GLFW_KEY_DOWN :
            if (action) translation[1] -= 0.01f * Height_global; break;
        case GLFW_KEY_F:
            if (action) auto_strech = !auto_strech; break;
        case GLFW_KEY_SPACE: break;
            
        default: break;
    }
    
}

void toon_diffuse_update(float &d) {
    for (auto it : toon_diffuse_intervals) {
        if (d < it) {
            d = it;
            break;
        }
    }
}

void toon_specular_update(float &d) {
    for (auto it : toon_specular_intervals) {
        if (d < it) {
            d = it;
            break;
        }
    }
}

// Updates the ambient, diffuse and specular components given a light and normal.
void update(const Vector &light_dir, const Vector &light_color,
            Vector &ambient, Vector &diffuse, Vector &specular,
            const Vector &view, const Vector &normal) {
    Vector half = (light_dir + view);
    Vector reflected = (2 * normal.dot(light_dir) * normal) - light_dir;
    Vector x(half.x, half.y, 0.0);

    Vector y(0.0, 1.0, 0.0);
    Vector v = y - normal * normal.dot(y);
    v.normalize();
    Vector u = v.cross(normal);
    u.normalize();

    float phi = x.dot(u);
    half.normalize();
    reflected.normalize(); 

    Vector eps(1.0, 0.0, 0.0);
    Vector tangent = normal.cross(eps);
    Vector bitangent = normal.cross(tangent);
    tangent.normalize();
    bitangent.normalize();

    float VdotN = max(view.dot(normal), 0.0f);
    float LdotN = max(light_dir.dot(normal), 0.0f);
    float HdotN = max(half.dot(normal), 0.0f);
    float HdotL = max(half.dot(light_dir), 0.0f);
    float RdotV = max(reflected.dot(view), 0.0f);

    Vector Rd = diffuse_coeffs;
    Vector Rs = specular_coeffs;

    float Nu = power_u;
    float Nv = power_v;

    if (power_u == 0.0 && power_v == 0.0)
        Nu = Nv = power_isotropic;

    float specular_exp = 0.0;
    if (power_isotropic > 0.0)
        specular_exp = power_isotropic;
    else {
        specular_exp = (power_u * pow(half.dot(u), 2.0) + power_v * pow(half.dot(v), 2.0));
        specular_exp = specular_exp / (1 - pow(half.dot(normal), 2.0));
    }

    if (asms) {
        // Ashikhmin Shirley
        Vector Pd = (28.0 * Rd) / (23.0 * PI);
        Pd = Pd * (1.0 - Rs);
        Pd = Pd * (1.0 - pow(1.0 - 0.5 * LdotN, 5.0));
        Pd = Pd * (1.0 - pow(1.0 - 0.5 * VdotN, 5.0));

        float Ps_num = sqrt((Nu + 1.0) * (Nv + 1.0));
        Ps_num *= pow(HdotN, specular_exp);
        float Ps_den = 8.0 * PI * HdotL;
        Ps_den *= max(LdotN, VdotN);
        Vector Ps = Rs * (Ps_num / Ps_den);
        Ps = Ps * (Rs + (1.0 - Rs) * pow(1.0 - HdotL, 5.0));

        Pd = Pd * light_color;
        Ps = Ps * light_color;
      
        if (is_toon) { 
            toon_diffuse_update(Pd.x);
            toon_diffuse_update(Pd.y);
            toon_diffuse_update(Pd.z);

            toon_specular_update(Ps.x);
            toon_specular_update(Ps.y);
            toon_specular_update(Ps.z);
        }
        
        ambient = ambient + ambient_coeffs * light_color; 
        diffuse = diffuse + Pd;
        specular = specular + Ps;
    } else {
        // Phong
        float dF = max(LdotN, 0.0f);
        float sF = pow(RdotV, specular_exp);

        if (is_toon) {
            // Toon shading.
            toon_diffuse_update(dF);
            toon_specular_update(sF); 
        }
        
        ambient = ambient + ambient_coeffs * light_color;
        diffuse = diffuse + diffuse_coeffs * light_color * dF;
        specular = specular + specular_coeffs * light_color * sF;
    }
}

// Helper function to compute all the pixel information to be rendered.
map<pair<int, int>, Vector> get_pixels(float centerX, float centerY, float radius) {
    map<pair<int, int>, Vector> mp;

    const Vector view(0.0, 0.0, 1.0);

    for (int i = 0; i < Width_global; i++) {
        for (int j = 0; j < Height_global; j++) {

            // Location of the center of pixel relative to center of sphere
            float x = (i+0.5-centerX);
            float y = (j+0.5-centerY);

            float dist = sqrt(sqr(x) + sqr(y));

            if (dist <= radius) {
                // This is the front-facing Z coordinate
                float z = sqrt(radius*radius-dist*dist);

                Vector ambient(0.0, 0.0, 0.0);
                Vector diffuse(0.0, 0.0, 0.0);
                Vector specular(0.0, 0.0, 0.0);
                Vector normal(x, y, z);
                normal.normalize();
              
                for (const auto &it : point_lights) {
                    Vector light_pos(it.x, it.y, it.z);
                    Vector light_dir = light_pos - normal;
                    Vector light_color(it.r, it.g, it.b);
                    light_dir.normalize();
                    update(light_dir, light_color, ambient, diffuse, specular, view, normal);
                }

                for (const auto &it : directional_lights) {
                    Vector light_dir(-it.x, -it.y, -it.z);
                    Vector light_color(it.r, it.g, it.b);
                    light_dir.normalize();
                    update(light_dir, light_color, ambient, diffuse, specular, view, normal);
                }

                Vector res = ambient + diffuse + specular;
                mp[make_pair(i, j)] = res;
            }
        }
    }
    return mp;
}

// Helper function to parse command line arguments.
void parse_args(int argc, char *argv[]) {
    int i = 1;
    while (i < argc) {
        string opt(argv[i]);
        if (opt == "-ka") {
            assert(i + 3 < argc);
            float r, g, b;
            r = atof(argv[i + 1]);
            g = atof(argv[i + 2]);
            b = atof(argv[i + 3]);
            i += 4;
            ambient_coeffs.init(r, g, b);
        } else if (opt == "-kd") {
            assert(i + 3 < argc);
            float r, g, b;
            r = atof(argv[i + 1]);
            g = atof(argv[i + 2]);
            b = atof(argv[i + 3]);
            i += 4;
            diffuse_coeffs.init(r, g, b);
        } else if (opt == "-ks") {
            assert(i + 3 < argc);
            float r, g, b;
            r = atof(argv[i + 1]);
            g = atof(argv[i + 2]);
            b = atof(argv[i + 3]);
            i += 4;
            specular_coeffs.init(r, g, b);
        } else if (opt == "-spu") {
            assert(i + 1 < argc);
            float pu = atof(argv[i + 1]);
            i += 2;
            power_u = pu;
        } else if (opt == "-spv") {
            assert(i + 1 < argc);
            float pv = atof(argv[i + 1]);
            i += 2;
            power_v = pv;
        } else if (opt == "-sp") {
            assert(i + 1 < argc);
            float p = atof(argv[i + 1]);
            i += 2;
            power_isotropic = p;
        } else if (opt == "-pl") {
            assert(i + 6 < argc);
            float x, y, z, r, g, b;
            x = atof(argv[i + 1]);
            y = atof(argv[i + 2]);
            z = atof(argv[i + 3]);
            r = atof(argv[i + 4]);
            g = atof(argv[i + 5]);
            b = atof(argv[i + 6]);
            i += 7;
            point_lights.emplace_back(x, y, z, r, g, b);
        } else if (opt == "-dl") {
            assert(i + 6 < argc);
            float x, y, z, r, g, b;
            x = atof(argv[i + 1]);
            y = atof(argv[i + 2]);
            z = atof(argv[i + 3]);
            r = atof(argv[i + 4]);
            g = atof(argv[i + 5]);
            b = atof(argv[i + 6]);
            i += 7;
            directional_lights.emplace_back(x, y, z, r, g, b);
        } else if (opt == "-o") {
            assert(i + 1 < argc);
            output_filename = string(argv[i + 1]);
            i += 2;
        } else if (opt == "-asm") {
            asms = true; 
            i += 1;
        } else if (opt == "-s") {
            assert(i + 4 < argc);
            float x, y, z, r;
            x = atof(argv[i + 1]);
            y = atof(argv[i + 2]);
            z = atof(argv[i + 3]);
            r = atof(argv[i + 4]);
            i += 5;
        } else if (opt == "-toon") {
            is_toon = true;
            i += 1;
        }
    }
}

void drawCircle(const float centerX, const float centerY, const float radius) {
    // Draw inner circle
    glBegin(GL_POINTS);

    auto mp = get_pixels(centerX, centerY, radius);

    for (const auto &it: mp) { 
        setPixel(it.first.first, it.first.second, it.second);
    }

    glEnd();
}

//****************************************************
// function that does the actual drawing of stuff
//***************************************************
void display( GLFWwindow* window )
{
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f ); //clear background screen to black
    
    glClear(GL_COLOR_BUFFER_BIT);                // clear the color buffer (sets everything to black)
    
    glMatrixMode(GL_MODELVIEW);                  // indicate we are specifying camera transformations
    glLoadIdentity();                            // make sure transformation is "zero'd"
    
    //----------------------- code to draw objects --------------------------
    glPushMatrix();
    glTranslatef (translation[0], translation[1], translation[2]);
    drawCircle(Width_global / 2.0, Height_global / 2.0,
               min(Width_global, Height_global) * 0.45);
    glPopMatrix();
    
    glfwSwapBuffers(window);
    
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
    parse_args(argc, argv);

    //This initializes glfw
    initializeRendering();
    
    GLFWwindow* window = glfwCreateWindow( Width_global, Height_global, "CS184", NULL, NULL );
    if ( !window )
    {
        cerr << "Error on window creating" << endl;
        glfwTerminate();
        return -1;
    }
    
    const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if ( !mode )
    {
        cerr << "Error on getting monitor" << endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent( window );
    
    // Get the pixel coordinate of the window
    // it returns the size, in pixels, of the framebuffer of the specified window
    glfwGetFramebufferSize(window, &Width_global, &Height_global);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Width_global, 0, Height_global, 1, -1);
    
    glfwSetWindowTitle(window, "CS184");
    glfwSetWindowSizeCallback(window, size_callback);
    glfwSetKeyCallback(window, key_callback);
   
    while( !glfwWindowShouldClose( window ) ) // infinite loop to draw object again and again
    {   // because once object is draw then window is terminated
        display( window );
        
        if (auto_strech){
            glfwSetWindowSize(window, mode->width, mode->height);
            glfwSetWindowPos(window, 0, 0);
        }
        
        glfwPollEvents();
        
    }

    return 0;
}
