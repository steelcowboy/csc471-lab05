/* Lab 5 base code - transforms using local matrix functions
   to be written by students -
   CPE 471 Cal Poly Z. Wood + S. Sueda
   */
#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "WindowManager.h"

// used for helper in perspective
#include "glm/glm.hpp"

using namespace std;
using namespace glm;

class Matrix
{

    public:

        static void printMat(float *A, const char *name = 0)
        {
            // OpenGL uses col-major ordering:
            // [ 0  4  8 12]
            // [ 1  5  9 13]
            // [ 2  6 10 14]
            // [ 3  7 11 15]
            // The (i, j)th element is A[i+4*j].

            if (name)
            {
                printf("%s=[\n", name);
            }

            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    printf("%- 5.2f ", A[i + 4*j]);
                }
                printf("\n");
            }

            if (name)
            {
                printf("];");
            }
            printf("\n");
        }

        static void createIdentityMat(float *M)
        {
            // set all values to zero
            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    M[i + 4*j] = 0;
                }
            }

            // overwrite diagonal with 1s
            M[0] = M[5] = M[10] = M[15] = 1;
        }

        static void createTranslateMat(float *T, float x, float y, float z)
        {
            // set all values to 0, but it's array so let's not waste time
            for (size_t i=0; i < 16; i++) {
                T[i] = 0;
            }

            T[0 + 4*3] = x;
            T[1 + 4*3] = y;
            T[2 + 4*3] = z;

            T[0] = T[5] = T[10] = T[15] = 1;
        }

        static void createScaleMat(float *S, float x, float y, float z)
        {
            for (size_t i=0; i < 16; i++) {
                S[i] = 0;
            }

            S[0 + 4*0] = x;
            S[1 + 4*1] = y;
            S[2 + 4*2] = z;
            S[3 + 4*3] = 1;
        }

        static void createRotateMatX(float *R, float radians)
        {
            for (size_t i=0; i < 16; i++) {
                R[i] = 0;
            }

            R[0] = 1;
            R[1 + 4*1] = cos(radians);
            R[1 + 4*2] = -sin(radians);
            R[2 + 4*1] = sin(radians);
            R[2 + 4*2] = cos(radians);

            R[15] = 1;
        }

        static void createRotateMatY(float *R, float radians)
        {
            for (size_t i=0; i < 16; i++) {
                R[i] = 0;
            }

            R[0 + 4*0] = cos(radians);
            R[0 + 4*2] = sin(radians);
            R[1 + 4*1] = 1;
            R[2 + 4*0] = -sin(radians);
            R[2 + 4*2] = cos(radians);

            R[15] = 1;
        }

        static void createRotateMatZ(float *R, float radians)
        {
            for (size_t i=0; i < 16; i++) {
                R[i] = 0;
            }

            R[0 + 4*0] = cos(radians);
            R[0 + 4*1] = -sin(radians);
            R[1 + 4*0] = sin(radians);
            R[1 + 4*1] = cos(radians);
            R[2 + 4*2] = 1;

            R[15] = 1;
        }

        static void multMat(float *C, const float *A, const float *B)
        {
            float c = 0;

            for (size_t crow = 0; crow < 4; crow++)
            {
                // Process kth column of C
                for (int ccol = 0; ccol < 4; ++ccol)
                {
                    // Process ith row of C.
                    // The (i,k)th element of C is the dot product
                    // of the ith row of A and kth col of B.
                    c = 0;

                    // vector dot product
                    for (int j = 0; j < 4; ++j)
                    {
                        c += A[crow + 4*j] * B[j + 4*ccol];
                    }
                    C[crow + 4*ccol] = c;
                }
            }
        }

        static void createPerspectiveMat(float *m, float fovy, float aspect, float zNear, float zFar)
        {
            // http://www-01.ibm.com/support/knowledgecenter/ssw_aix_61/com.ibm.aix.opengl/doc/openglrf/gluPerspective.htm%23b5c8872587rree
            float f = 1.0f / glm::tan(0.5f * fovy);

            m[ 0] = f / aspect;
            m[ 1] = 0.0f;
            m[ 2] = 0.0f;
            m[ 3] = 0.0f;
            m[ 4] = 0;

            m[ 5] = f;
            m[ 6] = 0.0f;
            m[ 7] = 0.0f;
            m[ 8] = 0.0f;

            m[ 9] = 0.0f;
            m[10] = (zFar + zNear) / (zNear - zFar);
            m[11] = -1.0f;
            m[12] = 0.0f;

            m[13] = 0.0f;
            m[14] = 2.0f * zFar * zNear / (zNear - zFar);
            m[15] = 0.0f;
        }

};

class Application : public EventCallbacks
{

    public:

        WindowManager * windowManager = nullptr;

        // Our shader program
        std::shared_ptr<Program> prog;

        // Shape to be used (from obj file)
        shared_ptr<Shape> h_l;
        shared_ptr<Shape> h_cross;
        shared_ptr<Shape> h_r;
        shared_ptr<Shape> i_cube;


        // Contains vertex information for OpenGL
        GLuint VertexArrayID;

        // Data necessary to give our triangle to OpenGL
        GLuint VertexBufferID;

        void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(window, GL_TRUE);
            }
        }

        void mouseCallback(GLFWwindow *window, int button, int action, int mods)
        {
            double posX, posY;

            if (action == GLFW_PRESS)
            {
                glfwGetCursorPos(window, &posX, &posY);
                cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
            }
        }

        void resizeCallback(GLFWwindow *window, int width, int height)
        {
            glViewport(0, 0, width, height);
        }

        void init(const std::string& resourceDirectory)
        {
            GLSL::checkVersion();

            // Set background color.
            glClearColor(0.12f, 0.34f, 0.56f, 1.0f);

            // Enable z-buffer test.
            glEnable(GL_DEPTH_TEST);

            // Initialize the GLSL program.
            prog = make_shared<Program>();
            prog->setVerbose(true);
            prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
            if (! prog->init())
            {
                std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
                exit(1);
            }
            prog->init();
            prog->addUniform("P");
            prog->addUniform("MV");
            prog->addAttribute("vertPos");
            prog->addAttribute("vertNor");
        }

        void initGeom(const std::string& resourceDirectory)
        {
            // Initialize mesh.
            h_l = make_shared<Shape>();
            h_l->loadMesh(resourceDirectory + "/cube.obj");
            h_l->resize();
            h_l->init();

            h_r = make_shared<Shape>();
            h_r->loadMesh(resourceDirectory + "/cube.obj");
            h_r->resize();
            h_r->init();

            i_cube = make_shared<Shape>();
            i_cube->loadMesh(resourceDirectory + "/cube.obj");
            i_cube->resize();
            i_cube->init();

            h_cross = make_shared<Shape>();
            h_cross->loadMesh(resourceDirectory + "/cube.obj");
            h_cross->resize();
            h_cross->init();
        }

        void render()
        {
            static const float OUT_Z = -8.0;
            static const float VERT_SCALE = 4.0;
            
            // Local modelview matrix use this for lab 5
            float MV[16] = {0};
            float P[16] = {0};
            float TRANS[16] = {0};
            float SCALE[16] = {0};
            float ROTAT[16] = {0};
            float I[16] = {0};
            float TMP[16] = {0};
            float GLOB_ROT[16] = {0};
            float GLOB_TRANS[16] = {0};
            
            Matrix::createRotateMatY(GLOB_ROT, glfwGetTime());
            //Matrix::createRotateMatY(GLOB_ROT, 5.75);
            Matrix::createTranslateMat(GLOB_TRANS, 0, 0, OUT_Z);

            // Get current frame buffer size.
            int width, height;
            glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
            glViewport(0, 0, width, height);

            // Clear framebuffer.
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Use the local matrices for lab 5
            float aspect = width/(float)height;
            Matrix::createPerspectiveMat(P, 70.0f, aspect, 0.1f, 100.0f);
            
            // Draw mesh using GLSL
            prog->bind();
            glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, P);

            // Left part of H
            Matrix::createTranslateMat(TRANS, -6, 0, OUT_Z);
            Matrix::createScaleMat(SCALE, 1, VERT_SCALE, 1);
            Matrix::multMat(I, GLOB_TRANS, GLOB_ROT);
            Matrix::multMat(TMP, I, TRANS);
            Matrix::multMat(MV, TMP, SCALE);

            glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV);
            h_l->draw(prog);

            // Right part of the H
            Matrix::createTranslateMat(TRANS, -2, 0, OUT_Z);
            Matrix::createScaleMat(SCALE, 1, VERT_SCALE, 1);
            Matrix::multMat(I, GLOB_TRANS, GLOB_ROT);
            Matrix::multMat(TMP, I, TRANS);
            Matrix::multMat(MV, TMP, SCALE);
            glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV);
            h_r->draw(prog);
            
            // Letter I 
            Matrix::createTranslateMat(TRANS, 2, 0, OUT_Z);
            Matrix::createScaleMat(SCALE, 1, VERT_SCALE, 1);
            //Matrix::multMat(I, GLOB_TRANS, GLOB_ROT);
            //Matrix::multMat(TMP, I, TRANS);
            //Matrix::multMat(MV, TMP, SCALE);
            Matrix::multMat(I, GLOB_TRANS, TRANS);
            Matrix::multMat(TMP, I, SCALE);
            Matrix::multMat(MV, TMP, GLOB_ROT);
            glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV);
            i_cube->draw(prog);

            // Cross of H
            Matrix::createTranslateMat(TRANS, -4, 0, OUT_Z);
            Matrix::createScaleMat(SCALE, 0.5, 4, 1);
            Matrix::createRotateMatZ(ROTAT, 1.0);

            // Translate * Rotate * Scale
            Matrix::multMat(I, GLOB_TRANS, GLOB_ROT);
            Matrix::multMat(TMP, I, TRANS);
            Matrix::multMat(I, TMP, ROTAT);
            Matrix::multMat(MV, I, SCALE);
            glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV);
            h_cross->draw(prog);

            prog->unbind();
        }
};

int main(int argc, char **argv)
{
    // Where the resources are loaded from
    std::string resourceDir = "../resources";

    if (argc >= 2)
    {
        resourceDir = argv[1];
    }

    Application *application = new Application();

    // Your main will always include a similar set up to establish your window
    // and GL context, etc.

    WindowManager *windowManager = new WindowManager();
    windowManager->init(640, 480);
    windowManager->setEventCallbacks(application);
    application->windowManager = windowManager;

    // This is the code that will likely change program to program as you
    // may need to initialize or set up different data and state

    application->init(resourceDir);
    application->initGeom(resourceDir);

    // Loop until the user closes the window.
    while (! glfwWindowShouldClose(windowManager->getHandle()))
    {
        // Render scene.
        application->render();

        // Swap front and back buffers.
        glfwSwapBuffers(windowManager->getHandle());
        // Poll for and process events.
        glfwPollEvents();
    }

    // Quit program.
    windowManager->shutdown();
    return 0;
}
