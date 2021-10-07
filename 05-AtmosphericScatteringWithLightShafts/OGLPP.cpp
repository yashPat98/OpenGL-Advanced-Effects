//headers
#include <windows.h>               //standard windows header
#include <stdio.h>                 //C header 
#include <gl/glew.h>               //OpenGL extension wrangler (must be included before gl.h)
#include <gl/gl.h>                 //OpenGL header
#include <vector>                  //vector header
#include "vmath.h"                 //Maths header
#include "RESOURCES.h"             //Resources header

//import libraries
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "OpenGL32.lib")

//symbolic constants
#define WIN_WIDTH  800             //initial width of window  
#define WIN_HEIGHT 600             //initial height of window

#define VK_F       0x46            //virtual key code of F key
#define VK_f       0x60            //virtual key code of f key 

//namespaces
using namespace vmath;

//type declarations
enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOORD
};

//callback procedure declaration
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

//global variables
HWND   ghwnd  = NULL;              //handle to a window
HDC    ghdc   = NULL;              //handle to a device context
HGLRC  ghrc   = NULL;              //handle to a rendering context

DWORD dwStyle = NULL;              //window style
WINDOWPLACEMENT wpPrev;            //structure for holding previous window position

bool gbActiveWindow = false;       //flag indicating whether window is active or not
bool gbFullscreen = false;         //flag indicating whether window is fullscreen or not

FILE*  gpFile = NULL;              //log file

GLuint ground_vertexShaderObject;          
GLuint ground_fragmentShaderObject;  
GLuint ground_shaderProgramObject;                

GLuint ground_modelMatrixUniform;
GLuint ground_viewMatrixUniform;
GLuint ground_projectionMatrixUniform;

GLuint ground_v3CameraPosUniform;
GLuint ground_v3LightPos;
GLuint ground_v3InvWavelength;
GLuint ground_fCameraHeight;
GLuint ground_fCameraHeight2;
GLuint ground_fOuterRadius;
GLuint ground_fOuterRadius2;
GLuint ground_fInnerRadius;
GLuint ground_fInnerRadius2;
GLuint ground_fKrESun;
GLuint ground_fKmESun;
GLuint ground_fKr4PI;
GLuint ground_fKm4PI;
GLuint ground_fScale;
GLuint ground_fScaleDepth;
GLuint ground_fScaleOverScaleDepth;

GLuint sky_vertexShaderObject;          
GLuint sky_fragmentShaderObject;  
GLuint sky_shaderProgramObject;                

GLuint sky_modelMatrixUniform;
GLuint sky_viewMatrixUniform;
GLuint sky_projectionMatrixUniform;

GLuint sky_v3CameraPosUniform;
GLuint sky_v3LightPos;
GLuint sky_v3InvWavelength;
GLuint sky_fCameraHeight;
GLuint sky_fCameraHeight2;
GLuint sky_fOuterRadius;
GLuint sky_fOuterRadius2;
GLuint sky_fInnerRadius;
GLuint sky_fInnerRadius2;
GLuint sky_fKrESun;
GLuint sky_fKmESun;
GLuint sky_fKr4PI;
GLuint sky_fKm4PI;
GLuint sky_fScale;
GLuint sky_fScaleDepth;
GLuint sky_fScaleOverScaleDepth;
GLuint sky_g;
GLuint sky_g2;   

GLuint occluded_vertexShaderObject;
GLuint occluded_fragmentShaderObject;
GLuint occluded_shaderProgramObject;

GLuint occluded_mvpMatrixUniform;     
GLuint occluded_colorUniform;          

GLuint godRays_vertexShaderObject;
GLuint godRays_fragmentShaderObject;
GLuint godRays_shaderProgramObject;

GLuint godRays_sceneSamplerUniform;
GLuint godRays_occludedSamplerUniform;
GLuint godRays_lightPositionOnScreenUniform;

GLuint fbo_occluded;
GLuint rbo_occluded;
GLuint occluded_texture;

GLuint fbo_scene;
GLuint rbo_scene;
GLuint scene_texture;

GLuint vao_quad;
GLuint vbo_quad_vertices;
GLuint vbo_quad_texcoords;

GLuint vao_inner_sphere;
GLuint vbo_inner_sphere_position;
GLuint vbo_inner_sphere_indices;

GLuint vao_outer_sphere;
GLuint vbo_outer_sphere_position;
GLuint vbo_outer_sphere_indices;    

GLuint vao_sphere;
GLuint vbo_sphere_vertices;
GLuint vbo_sphere_indices;  

GLuint vao_pyramid;
GLuint vbo_pyramid_vertices;

mat4 perspectiveProjectionMatrix;  

vec3 lightPos;
vec3 wavelength;
GLfloat cameraHeight;
GLfloat fscale;
GLfloat rayleigh_constant = 0.0025f;
GLfloat mie_constant = 0.0015f;
GLfloat scaleDepth = 0.25f;
GLfloat innerRadius = 800.0f;
GLfloat outerRadius = innerRadius * 1.025f;       
GLfloat eSun = 10.0f;
GLfloat g = -0.99f;

vec4 godRays_lightPosition;

std::vector<float> vertices_inner_sphere;
std::vector<int> indices_inner_sphere;
std::vector<float> vertices_outer_sphere;
std::vector<int> indices_outer_sphere;
std::vector<float> vertices_small_sphere;
std::vector<int> indices_small_sphere;

vec3 camera_pos;
vec3 camera_front;
vec3 camera_up;
float camera_speed = 0.05f;
float sensitivity = 0.1f;
bool first_mouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float last_x = 800.0f / 2.0f;
float last_y = 600.0f / 2.0f;
float fov = 45.0f;

int scr_width;
int scr_height;

//windows entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    //function declarations
    void Initialize(void);                                          //initialize OpenGL state machine
    void Display(void);                                             //render scene

    //variable declarations
    WNDCLASSEX wndclass;                                            //structure holding window class attributes
    MSG msg;                                                        //structure holding message attributes
    HWND hwnd;                                                      //handle to a window
    TCHAR szAppName[] = TEXT("OpenGL : Atmospheric Scattering");    //name of window class

    int cxScreen, cyScreen;                                         //screen width and height for centering window
    int init_x, init_y;                                             //top-left coordinates of centered window
    bool bDone = false;                                             //flag indicating whether or not to exit from game loop

    //code
    //create/open  'log.txt' file
    if(fopen_s(&gpFile, "log.txt", "w") != 0)
    {
        MessageBox(NULL, TEXT("Failed to open log.txt file"), TEXT("Error"), MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }
    else
    {
        fprintf(gpFile, "-> Program Started Successfully\n\n");
    }
    
    //initialization of WNDCLASSEX
    wndclass.cbSize         = sizeof(WNDCLASSEX);                            //size of structure
    wndclass.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;            //window style
    wndclass.lpfnWndProc    = WndProc;                                       //address of callback procedure
    wndclass.cbClsExtra     = 0;                                             //extra class bytes
    wndclass.cbWndExtra     = 0;                                             //extra window bytes
    wndclass.hInstance      = hInstance;                                     //handle to a program
    wndclass.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));  //handle to an icon
    wndclass.hCursor        = LoadCursor((HINSTANCE)NULL, IDC_ARROW);        //handle to a cursor
    wndclass.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);           //handle to a background brush
    wndclass.lpszClassName  = szAppName;                                     //name of a custom class
    wndclass.lpszMenuName   = NULL;                                          //name of a custom menu
    wndclass.hIconSm        = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));  //handle to a small icon

    //register above class
    RegisterClassEx(&wndclass);

    //get screen width and height
    cxScreen = GetSystemMetrics(SM_CXSCREEN);
    cyScreen = GetSystemMetrics(SM_CYSCREEN);

    scr_width = cxScreen;
    scr_height = cyScreen;

    //calculate top-left coordinates for a centered window
    init_x = (cxScreen / 2) - (WIN_WIDTH / 2);
    init_y = (cyScreen / 2) - (WIN_HEIGHT / 2);

    //create window
    hwnd = CreateWindowEx(WS_EX_APPWINDOW,                //extended window style          
            szAppName,                                    //class name
            szAppName,                                    //window caption
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN |       //window style
            WS_CLIPSIBLINGS | WS_VISIBLE,   
            init_x,                                       //X-coordinate of top left corner of window 
            init_y,                                       //Y-coordinate of top left corner of window
            WIN_WIDTH,                                    //initial window width                 
            WIN_HEIGHT,                                   //initial window height
            (HWND)NULL,                                   //handle to a parent window  : NULL desktop
            (HMENU)NULL,                                  //handle to a menu : NULL no menu
            hInstance,                                    //handle to a program instance
            (LPVOID)NULL);                                //data to be sent to window callback : NULL no data to send      

    //store handle to a window in global handle
    ghwnd = hwnd;                                         

    //initialize OpenGL rendering context
    Initialize();

    ShowWindow(hwnd, iCmdShow);                 //set specified window's show state
    SetForegroundWindow(hwnd);                  //brings the thread that created the specified window to foreground
    SetFocus(hwnd);                             //set the keyboard focus to specified window 

    //game loop
    while(bDone == false)
    {   
        //1 : pointer to structure for window message
        //2 : handle to window : NULL do not process child window's messages 
        //3 : message filter min range : 0 no range filtering
        //4 : message filter max range : 0 no range filtering
        //5 : remove message from queue after processing from PeekMessage
        if(PeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_QUIT)           //if current message is WM_QUIT then exit from game loop
            {
                bDone = true;
            }
            else
            {
                TranslateMessage(&msg);          //translate virtual-key message into character message
                DispatchMessage(&msg);           //dispatch message  to window procedure
            }
        }
        else
        {
            if(gbActiveWindow == true)           //if window has keyboard focus 
            {
                Display();                       //render the scene
            }
        }
    }

    return ((int)msg.wParam);                    //exit code given by PostQuitMessage 
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    //function declarations
    void ToggleFullscreen(void);                 //toggle window between fullscreen and previous position 
    void Resize(int, int);                       //handle window resize event
    void UnInitialize(void);                     //release resources  
    void mouse_move(double xPos, double yPos);

    //code
    switch(iMsg)
    {
        case WM_SETFOCUS:                        //event : window has keyboard focus
            gbActiveWindow = true;
            break;
        
        case WM_KILLFOCUS:                       //event : window dosen't have keyboard focus
            gbActiveWindow = false;
            break;

        case WM_ERASEBKGND:                      //event : window background must be erased 
            return (0);                          //dont let DefWindowProc handle this event
        
        case WM_SIZE:                            //event : window is resized
            Resize(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_KEYDOWN:                         //event : a key has been pressed
            switch(wParam)
            {
                case VK_ESCAPE:
                    DestroyWindow(hwnd);
                    break;

                case VK_F:
                case VK_f:
                    ToggleFullscreen();
                    break;

                case VK_UP:
                    camera_pos += camera_speed * camera_front;
                    break;

                case VK_DOWN:
                    camera_pos -= camera_speed * camera_front;
                    break;

                case VK_LEFT:
                    camera_pos -= normalize(cross(camera_front, camera_up)) * camera_speed;
                    break;
                
                case VK_RIGHT:
                    camera_pos += normalize(cross(camera_front, camera_up)) * camera_speed;
                    break;

                default:
                    break;
            }
            break;

        case WM_CHAR:
            switch(wParam)
            {
                case 'R':
                    rayleigh_constant += 0.0001f;
                    break;
                
                case 'r':
                    rayleigh_constant -= 0.0001f;
                    break;
                
                case 'M':
                    mie_constant += 0.0001f;
                    break;
                
                case 'm':
                    mie_constant -= 0.0001f;
                    break;

                case 'I':
                    eSun += 0.1f;
                    break;

                case 'i':
                    eSun -= 0.1f;
                    break;
                
                case 'g':
                    g -= 0.001f;
                    break;

                case 'G':
                    g += 0.001f;
                    break;

                case 'K':
                    lightPos[1] += 0.002f;
                    godRays_lightPosition[1] += 2.0f;
                    break;

                case 'k':
                    lightPos[1] -= 0.002f;
                    godRays_lightPosition[1] -= 2.0f;
                    break;

                case 'L':
                    lightPos[0] += 0.002f;
                    break;
                
                case 'l':
                    lightPos[0] -= 0.002f;
                    break;

                default:
                    break;
            }
            break;

        case WM_MOUSEMOVE:
            mouse_move(LOWORD(lParam), HIWORD(lParam));
            break;
        
        case WM_CLOSE:                           //event : window is closed from sysmenu or close button
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            UnInitialize();
            PostQuitMessage(0);
            break;
        
        default:
            break;
    }

    //call default window procedure for unhandled messages
    return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullscreen(void)
{
    //variable declarations
    MONITORINFO mi = { sizeof(MONITORINFO) };            //structure holding monitor information

    //code
    if(gbFullscreen == false)                            //if screen is not in fulscreen mode 
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);       //get window style
        if(dwStyle & WS_OVERLAPPEDWINDOW)                //if current window style has WS_OVERLAPPEDWINDOW
        {
            if(GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
            {
                // if wpPrev is successfully filled with current window placement
                // and mi is successfully filled with primary monitor info then
                // 1 -> Remove WS_OVERLAPPEDWINDOW style
                // 2 -> Set window position by aligning left-top corner of window 
                //     to left-top corner of monitor and setting width and height 
                //     to monitor's width and height (effectively making window 
                //     fullscreen)
                // SWP_NOZORDER : Don't change Z-order
                // SWP_FRAMECHANGED: Forces recalculation of New Client area (WM_NCCALCSIZE)
                SetWindowLong(ghwnd, GWL_STYLE, (dwStyle & ~WS_OVERLAPPEDWINDOW));
                SetWindowPos(ghwnd,                                     //     top 
                    HWND_TOP,                                           //left +--------------+ right
                    mi.rcMonitor.left,                                  //     |              |
                    mi.rcMonitor.top,                                   //     |              |
                    mi.rcMonitor.right - mi.rcMonitor.left,             //     |              |
                    mi.rcMonitor.bottom - mi.rcMonitor.top,             //     |              |
                    SWP_NOZORDER | SWP_FRAMECHANGED);                   //     +--------------+
            }                                                           //     bottom
        }

        ShowCursor(false);                                 //hide the cursor
        gbFullscreen = true;                          
    }
    else                                                   //if screen is in fullscreen mode
    {
        // Toggle the window to previously saved dimension
        // 1 -> Add WS_OVERLAPPEDWINDOW to window style 
        // 2 -> Set window placement to stored previous placement
        // 3 -> Force the effects of SetWindowPlacement by call to 
        //      SetWindowPos with
        // SWP_NOMOVE : Don't change left top position of window 
        //              i.e ignore third and forth parameters
        // SWP_NOSIZE : Don't change dimensions of window
        //              i.e ignore fifth and sixth parameters
        // SWP_NOZORDER : Don't change Z-order of the window and
        //              its child windows
        // SWP_NOOWNERZORDER : Don't change Z-order of owner of the 
        //              window (reffered by ghwnd)
        // SWP_FRAMECHANGED : Forces recalculation of New Client area (WM_NCCALCSIZE)
        SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowPos(ghwnd,
            HWND_TOP,
            0,
            0,
            0, 
            0,
            SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
        
        ShowCursor(true);            //show cursor
        gbFullscreen = false;
    }
}

void mouse_move(double xPos, double yPos)
{
    if(first_mouse)
    {
        last_x = xPos;
        last_y = yPos;
        first_mouse = false;
    }

    float xoffset = xPos - last_x;
    float yoffset = last_y - yPos;
    last_x = xPos;
    last_y = yPos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
    {
        pitch = 89.0f;
    }

    if(pitch < -89.0f)
    {
        pitch = -89.0f;
    }

    vec3 front;
    front[0] = cos(radians(yaw)) * cos(radians(pitch));
    front[1] = sin(radians(pitch));
    front[2] = sin(radians(yaw)) * cos(radians(pitch));

    camera_front = normalize(front);
}

void Initialize(void)
{
    //function declarations
    void Resize(int, int);          //warm-up call
    void UnInitialize(void);        //release resources
    void GenerateSphere(float radius, float sectorCount, float stackCount, std::vector<float> &vertices, std::vector<int> &indicess);

    //variable declarations
    PIXELFORMATDESCRIPTOR pfd;      //structure describing the pixel format
    int iPixelFormatIndex;          //index of the pixel format structure in HDC

    //code
    //zero out the memory
    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR)); 

    //initialization of PIXELFORMATDESCRIPTOR
    pfd.nSize       = sizeof(PIXELFORMATDESCRIPTOR);                                //size of structure
    pfd.nVersion    = 1;                                                            //version information
    pfd.dwFlags     = PFD_DRAW_TO_WINDOW| PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;    //pixel format properties
    pfd.iPixelType  = PFD_TYPE_RGBA;                                                //type of pixel format to chosen
    pfd.cColorBits  = 32;                                                           //color depth in bits (32 = True Color)
    pfd.cRedBits    = 8;                                                            //red color bits
    pfd.cGreenBits  = 8;                                                            //green color bits
    pfd.cBlueBits   = 8;                                                            //blue color bits
    pfd.cAlphaBits  = 8;                                                            //alpha bits
    pfd.cDepthBits  = 32;                                                           //depth bits

    //obtain a device context
    ghdc = GetDC(ghwnd);                    

    //choose required pixel format from device context
    //which matches pfd structure and get the index of 
    //that pixel format (1 based index)
    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if(iPixelFormatIndex == 0)
    {
        fprintf(gpFile, "ChoosePixelFormat() failed.\n");
        DestroyWindow(ghwnd);
    }

    //set the current pixel format of the device context (ghdc) to
    //pixel format specified by index
    if(SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
    {
        fprintf(gpFile, "SetPixelFormat() failed.\n");
        DestroyWindow(ghwnd);
    }

    //create rendering context 
    ghrc = wglCreateContext(ghdc);
    if(ghrc == NULL)
    {
        fprintf(gpFile, "wglCreateContext() failed.\n");
        DestroyWindow(ghwnd);
    }

    //set rendering context as current context
    if(wglMakeCurrent(ghdc, ghrc) == FALSE)
    {
        fprintf(gpFile, "wglMakeCurrent() failed.\n");
        DestroyWindow(ghwnd);
    }

    //initialize glew (enable extensions)
    GLenum glew_error = glewInit();
    if(glew_error != GLEW_OK)
    {
        fprintf(gpFile, "glewInit() failed.\n");
        DestroyWindow(ghwnd);
    }

    //opengl related log
    fprintf(gpFile, "OpenGL Information\n");
    fprintf(gpFile, "OpenGL Vendor     : %s\n", glGetString(GL_VENDOR));
    fprintf(gpFile, "OpenGL Renderer   : %s\n", glGetString(GL_RENDERER));
    fprintf(gpFile, "OpenGL Version    : %s\n", glGetString(GL_VERSION));
    fprintf(gpFile, "GLSL Version      : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    //opengl enabled extensions
    GLint numExt;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);

    fprintf(gpFile, "OpenGL Extensions : \n");
    for(int i = 0; i < numExt; i++)
    {
        fprintf(gpFile, "%s\n", glGetStringi(GL_EXTENSIONS, i));
    }

    //setup render scene

    /****************** Atmospheric Scattering ********************/

    //Ground From Atmosphere
    fprintf(gpFile, "\n-> Ground From Atmosphere \n");

    //vertex shader
    ground_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* ground_vertexShaderObjectSource = 
        "#version 450 core"                                                                             \
        "\n"                                                                                            \

        "in vec3 aPosition;"                                                                            \

        "uniform mat4 projectionMatrix;"                                                                \
        "uniform mat4 viewMatrix;"                                                                      \
        "uniform mat4 modelMatrix;"                                                                     \

        "uniform vec3 v3CameraPos;"                                                                     \
        "uniform vec3 v3LightPos;"                                                                      \
        "uniform vec3 v3InvWavelength;"                                                                 \
        "uniform float fCameraHeight;"                                                                  \
        "uniform float fCameraHeight2;"                                                                 \
        "uniform float fOuterRadius;"                                                                   \
        "uniform float fOuterRadius2;"                                                                  \
        "uniform float fInnerRadius;"                                                                   \
        "uniform float fInnerRadius2;"                                                                  \
        "uniform float fKrESun;"                                                                        \
        "uniform float fKmESun;"                                                                        \
        "uniform float fKr4PI;"                                                                         \
        "uniform float fKm4PI;"                                                                         \
        "uniform float fScale;"                                                                         \
        "uniform float fScaleDepth;"                                                                    \
        "uniform float fScaleOverScaleDepth;"                                                           \

        "out vec3 out_color1;"                                                                          \
        "out vec3 out_color2;"                                                                          \

        "const int nSamples = 3;"                                                                       \
        "const float fSamples = 3.0f;"                                                                  \

        "float scale(float fCos)"                                                                       \
        "{"                                                                                             \
        "   float x = 1.0f - fCos;"                                                                     \
        "   return (fScaleDepth * exp(-0.00287f + x*(0.459f + x*(3.83f + x*(-6.8f + x*5.25f)))));"      \
        "}"                                                                                             \

        "void main(void)"                                                                               \
        "{"                                                                                             \
            //get the ray from the camera to the vertex(far point) and its length
        "   vec3 v3Ray = aPosition - v3CameraPos;"                                                      \
        "   float fFar = length(v3Ray);"                                                                \
        "   v3Ray = v3Ray / fFar;"                                                                      \

            //calculate the ray's starting position, then its scattering offset
        "   vec3 v3Start = v3CameraPos;"                                                                \
        "   float fDepth = exp((fInnerRadius - fCameraHeight) / fScaleDepth);"                          \
        "   float fCameraAngle = dot(-v3Ray, aPosition) / length(aPosition);"                           \
        "   float fLightAngle = dot(v3LightPos, aPosition) / length(aPosition);"                        \
        "   float fCameraScale = scale(fCameraAngle);"                                                  \
        "   float fLightScale = scale(fLightAngle);"                                                    \
        "   float fCameraOffset = fDepth * fCameraScale;"                                               \
        "   float fTemp = (fLightScale + fCameraScale);"                                                \

            //initialize the scattering loop variables
        "   float fSampleLength = fFar / fSamples;"                                                     \
        "   float fScaledLength = fSampleLength * fScale;"                                              \
        "   vec3 v3SampleRay = v3Ray * fSampleLength;"                                                  \
        "   vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5f;"                                         \
        
            //now loop through the sample rays
        "   vec3 v3FrontColor = vec3(0.0f, 0.0f, 0.0f);"                                                \
        "   vec3 v3Attenuate;"                                                                          \
        "   for(int i = 0; i < nSamples; i++)"                                                          \
        "   {"                                                                                          \
        "       float fHeight = length(v3SamplePoint);"                                                 \
        "       float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));"                   \
        "       float fScatter = fDepth * fTemp - fCameraOffset;"                                       \
        "       v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));"                    \
        "       v3FrontColor += v3Attenuate * (fDepth * fScaledLength);"                                \
        "       v3SamplePoint += v3SampleRay;"                                                          \
        "   }"                                                                                          \

        "   out_color1 = v3FrontColor * (v3InvWavelength * fKrESun + fKmESun);"                         \
        "   out_color2 = v3Attenuate;"                                                                  \
        "   gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPosition, 1.0f);"         \
        "}";

    //provide source code to shader object
    glShaderSource(ground_vertexShaderObject, 1, (const GLchar**)&ground_vertexShaderObjectSource, NULL);

    //compile shader 
    glCompileShader(ground_vertexShaderObject);

    //shader compilation error checking
    GLint infoLogLength = 0;
    GLint shaderCompiledStatus = 0;
    GLchar* szInfoLog = NULL;

    glGetShaderiv(ground_vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(ground_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(ground_vertexShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> vertex shader compilation log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    } 
    fprintf(gpFile, "-> vertex shader compiled successfully\n");

    //fragment shader
    ground_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* ground_fragmentShaderObjectSource = 
        "#version 450 core"                                                     \
        "\n"                                                                    \

        "in vec3 out_color1;"                                                   \
        "in vec3 out_color2;"                                                   \
        "out vec4 FragColor;"                                                   \
        
        "void main(void)"                                                       \
        "{"                                                                     \
        "   FragColor = vec4(out_color1 * 0.5f + 0.25f * out_color2, 1.0f);"    \
        "}";

    //provide source code to shader object 
    glShaderSource(ground_fragmentShaderObject, 1, (const GLchar**)&ground_fragmentShaderObjectSource, NULL);

    //compile shader
    glCompileShader(ground_fragmentShaderObject);

    //shader compilation error checking
    glGetShaderiv(ground_fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(ground_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(ground_fragmentShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> fragment shader compilation log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }
    fprintf(gpFile, "-> fragment shader compiled successfully\n");

    //shader program
    ground_shaderProgramObject = glCreateProgram();

    //attach shaders to shader program
    glAttachShader(ground_shaderProgramObject, ground_vertexShaderObject);
    glAttachShader(ground_shaderProgramObject, ground_fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(ground_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "aPosition");

    //link shader program 
    glLinkProgram(ground_shaderProgramObject);

    //shader linking error checking
    GLint shaderProgramLinkStatus = 0;
    glGetProgramiv(ground_shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(ground_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(ground_shaderProgramObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> shader program link log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }
    fprintf(gpFile, "-> shader program linked successfully\n");

    //get uniform locations
    ground_modelMatrixUniform = glGetUniformLocation(ground_shaderProgramObject, "modelMatrix");
    ground_viewMatrixUniform = glGetUniformLocation(ground_shaderProgramObject, "viewMatrix");
    ground_projectionMatrixUniform = glGetUniformLocation(ground_shaderProgramObject, "projectionMatrix");

    ground_v3CameraPosUniform = glGetUniformLocation(ground_shaderProgramObject, "v3CameraPos");
    ground_v3LightPos = glGetUniformLocation(ground_shaderProgramObject, "v3LightPos");
    ground_v3InvWavelength = glGetUniformLocation(ground_shaderProgramObject, "v3InvWavelength");
    ground_fCameraHeight = glGetUniformLocation(ground_shaderProgramObject, "fCameraHeight");
    ground_fCameraHeight2 = glGetUniformLocation(ground_shaderProgramObject, "fCameraHeight2");
    ground_fOuterRadius = glGetUniformLocation(ground_shaderProgramObject, "fOuterRadius");
    ground_fOuterRadius2 = glGetUniformLocation(ground_shaderProgramObject, "fOuterRadius2");
    ground_fInnerRadius = glGetUniformLocation(ground_shaderProgramObject, "fInnerRadius");
    ground_fInnerRadius2 = glGetUniformLocation(ground_shaderProgramObject, "fInnerRadius2");
    ground_fKrESun = glGetUniformLocation(ground_shaderProgramObject, "fKrESun");
    ground_fKmESun = glGetUniformLocation(ground_shaderProgramObject, "fKmESun");
    ground_fKr4PI = glGetUniformLocation(ground_shaderProgramObject, "fKr4PI");
    ground_fKm4PI = glGetUniformLocation(ground_shaderProgramObject, "fKm4PI");
    ground_fScale = glGetUniformLocation(ground_shaderProgramObject, "fScale");
    ground_fScaleDepth = glGetUniformLocation(ground_shaderProgramObject, "fScaleDepth");
    ground_fScaleOverScaleDepth = glGetUniformLocation(ground_shaderProgramObject, "fScaleOverScaleDepth");

    //Sky From Atmosphere
    fprintf(gpFile, "\n-> Sky From Atmosphere \n");

    //vertex shader
    sky_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* sky_vertexShaderObjectSource = 
         "#version 450 core"                                                                             \
        "\n"                                                                                            \

        "in vec3 aPosition;"                                                                            \

        "uniform mat4 projectionMatrix;"                                                                \
        "uniform mat4 viewMatrix;"                                                                      \
        "uniform mat4 modelMatrix;"                                                                     \

        "uniform vec3 v3CameraPos;"                                                                     \
        "uniform vec3 v3LightPos;"                                                                      \
        "uniform vec3 v3InvWavelength;"                                                                 \
        "uniform float fCameraHeight;"                                                                  \
        "uniform float fCameraHeight2;"                                                                 \
        "uniform float fOuterRadius;"                                                                   \
        "uniform float fOuterRadius2;"                                                                  \
        "uniform float fInnerRadius;"                                                                   \
        "uniform float fInnerRadius2;"                                                                  \
        "uniform float fKrESun;"                                                                        \
        "uniform float fKmESun;"                                                                        \
        "uniform float fKr4PI;"                                                                         \
        "uniform float fKm4PI;"                                                                         \
        "uniform float fScale;"                                                                         \
        "uniform float fScaleDepth;"                                                                    \
        "uniform float fScaleOverScaleDepth;"                                                           \

        "out vec3 out_color1;"                                                                          \
        "out vec3 out_color2;"                                                                          \
        "out vec3 out_direction;"                                                                       \

        "const int nSamples = 3;"                                                                       \
        "const float fSamples = 3.0f;"                                                                  \

        "float scale(float fCos)"                                                                       \
        "{"                                                                                             \
        "   float x = 1.0f - fCos;"                                                                     \
        "   return (fScaleDepth * exp(-0.00287f + x*(0.459f + x*(3.83f + x*(-6.8f + x*5.25f)))));"      \
        "}"                                                                                             \

        "void main(void)"                                                                               \
        "{"                                                                                             \
            //get the ray from the camera to the vertex(far point) and its length
        "   vec3 v3Ray = aPosition - v3CameraPos;"                                                      \
        "   float fFar = length(v3Ray);"                                                                \
        "   v3Ray = v3Ray / fFar;"                                                                      \

            //calculate the ray's starting position, then its scattering offset
        "   vec3 v3Start = v3CameraPos;"                                                                \
        "   float fHeight = length(v3Start);"                                                           \
        "   float fStartDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fCameraHeight));"            \
        "   float fStartAngle = dot(v3Ray, v3Start) / fHeight;"                                         \
        "   float fStartOffset = fStartDepth * scale(fStartAngle);"                                     \

            //initialize the scattering loop variables
        "   float fSampleLength = fFar / fSamples;"                                                     \
        "   float fScaledLength = fSampleLength * fScale;"                                              \
        "   vec3 v3SampleRay = v3Ray * fSampleLength;"                                                  \
        "   vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5f;"                                         \
        
            //now loop through the sample rays
        "   vec3 v3FrontColor = vec3(0.0f, 0.0f, 0.0f);"                                                \
        "   for(int i = 0; i < nSamples; i++)"                                                          \
        "   {"                                                                                          \
        "       float fHeight = length(v3SamplePoint);"                                                 \
        "       float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));"                   \
        "       float fLightAngle = dot(v3LightPos, v3SamplePoint) / fHeight;"                          \
        "       float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;"                              \
        "       float fScatter = (fStartOffset + fDepth * (scale(fLightAngle) - scale(fCameraAngle)));" \
        "       vec3 v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));"               \
        "       v3FrontColor += v3Attenuate * (fDepth * fScaledLength);"                                \
        "       v3SamplePoint += v3SampleRay;"                                                          \
        "   }"                                                                                          \

        "   out_color1 = v3FrontColor * (v3InvWavelength * fKrESun);"                                   \
        "   out_color2 = v3FrontColor * fKmESun;"                                                       \
        "   out_direction = v3CameraPos - aPosition;"                                                   \
        "   gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPosition, 1.0f);"         \
        "}";

    //provide source code to shader object
    glShaderSource(sky_vertexShaderObject, 1, (const GLchar**)&sky_vertexShaderObjectSource, NULL);

    //compile shader 
    glCompileShader(sky_vertexShaderObject);

    //shader compilation error checking
    glGetShaderiv(sky_vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(sky_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(sky_vertexShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> vertex shader compilation log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    } 
    fprintf(gpFile, "-> vertex shader compiled successfully\n");

    //fragment shader
    sky_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* sky_fragmentShaderObjectSource = 
        "#version 450 core"                                                                                             \
        "\n"                                                                                                            \

        "in vec3 out_color1;"                                                                                           \
        "in vec3 out_color2;"                                                                                           \
        "in vec3 out_direction;"                                                                                        \
        
        "uniform vec3 v3LightPos;"                                                                                      \
        "uniform float g;"                                                                                              \
        "uniform float g2;"                                                                                             \
        
        "out vec4 FragColor;"                                                                                           \
        
        "const float gamma = 2.2f;"                                                                                     \
        "const float exposure = 1.0f;"                                                                                  \

        "void main(void)"                                                                                               \
        "{"                                                                                                             \
        "   float fCos = dot(v3LightPos, out_direction) / length(out_direction);"                                       \
        "   float fMiePhase = 1.5f * ((1.0f-g2) / (2.0f+g2)) * (1.0f+fCos*fCos) / pow(1.0f + g2 - 2.0f*g*fCos, 1.5f);"  \
        "   float fRayleightPhase = 0.75f + 0.75 * fCos * fCos;"                                                        \
        
        "   vec4 color;"
        "   color.rgb = out_color1 * fRayleightPhase + out_color2 * fMiePhase;"                                         \
        "   color.a = color.b;"                                                                                         \
        
            //gamma correction and hdr
        "   color.rgb = vec3(1.0f) - exp(-color.rgb * exposure);"                                                       \
        "   color.rgb = pow(color.rgb, vec3(1.0f / gamma));"                                                            \
        "   FragColor = color;"                                                                                         \
        "}";

    //provide source code to shader object 
    glShaderSource(sky_fragmentShaderObject, 1, (const GLchar**)&sky_fragmentShaderObjectSource, NULL);

    //compile shader
    glCompileShader(sky_fragmentShaderObject);

    //shader compilation error checking
    glGetShaderiv(sky_fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(sky_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(sky_fragmentShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> fragment shader compilation log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }
    fprintf(gpFile, "-> fragment shader compiled successfully\n");

    //shader program
    sky_shaderProgramObject = glCreateProgram();

    //attach shaders to shader program
    glAttachShader(sky_shaderProgramObject, sky_vertexShaderObject);
    glAttachShader(sky_shaderProgramObject, sky_fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(sky_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "aPosition");

    //link shader program 
    glLinkProgram(sky_shaderProgramObject);

    //shader linking error checking
    glGetProgramiv(sky_shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(sky_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(sky_shaderProgramObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> shader program link log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }
    fprintf(gpFile, "-> shader program linked successfully\n");

    //get uniform locations
    sky_modelMatrixUniform = glGetUniformLocation(sky_shaderProgramObject, "modelMatrix");
    sky_viewMatrixUniform = glGetUniformLocation(sky_shaderProgramObject, "viewMatrix");
    sky_projectionMatrixUniform = glGetUniformLocation(sky_shaderProgramObject, "projectionMatrix");

    sky_v3CameraPosUniform = glGetUniformLocation(sky_shaderProgramObject, "v3CameraPos");
    sky_v3LightPos = glGetUniformLocation(sky_shaderProgramObject, "v3LightPos");
    sky_v3InvWavelength = glGetUniformLocation(sky_shaderProgramObject, "v3InvWavelength");
    sky_fCameraHeight = glGetUniformLocation(sky_shaderProgramObject, "fCameraHeight");
    sky_fCameraHeight2 = glGetUniformLocation(sky_shaderProgramObject, "fCameraHeight2");
    sky_fOuterRadius = glGetUniformLocation(sky_shaderProgramObject, "fOuterRadius");
    sky_fOuterRadius2 = glGetUniformLocation(sky_shaderProgramObject, "fOuterRadius2");
    sky_fInnerRadius = glGetUniformLocation(sky_shaderProgramObject, "fInnerRadius");
    sky_fInnerRadius2 = glGetUniformLocation(sky_shaderProgramObject, "fInnerRadius2");
    sky_fKrESun = glGetUniformLocation(sky_shaderProgramObject, "fKrESun");
    sky_fKmESun = glGetUniformLocation(sky_shaderProgramObject, "fKmESun");
    sky_fKr4PI = glGetUniformLocation(sky_shaderProgramObject, "fKr4PI");
    sky_fKm4PI = glGetUniformLocation(sky_shaderProgramObject, "fKm4PI");
    sky_fScale = glGetUniformLocation(sky_shaderProgramObject, "fScale");
    sky_fScaleDepth = glGetUniformLocation(sky_shaderProgramObject, "fScaleDepth");
    sky_fScaleOverScaleDepth = glGetUniformLocation(sky_shaderProgramObject, "fScaleOverScaleDepth");
    sky_g = glGetUniformLocation(sky_shaderProgramObject, "g"); 
    sky_g2 = glGetUniformLocation(sky_shaderProgramObject, "g2"); 

    //****************** God Rays ********************/

    //Occluded Shader Program
    fprintf(gpFile, "\n-> Occluded Shader Program \n");

    //vertex shader
    occluded_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* occluded_vertexShaderSource = 
        "#version 450 core"                                         \
        "\n"                                                        \
        "in vec4 vPosition;"                                        \
        "uniform mat4 u_mvpMatrix;"                                 \
        "void main(void)"                                           \
        "{"                                                         \
        "   gl_Position = u_mvpMatrix * vPosition;"                 \
        "}";

    //provide source code to shader object
    glShaderSource(occluded_vertexShaderObject, 1, (const GLchar**)&occluded_vertexShaderSource, NULL);

    //compile shader 
    glCompileShader(occluded_vertexShaderObject);

    //shader compilation error checking
    glGetShaderiv(occluded_vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(occluded_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(occluded_vertexShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> vertex shader compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    } 

    fprintf(gpFile, "-> vertex shader compiled successfully\n");

    //fragment shader
    occluded_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* occluded_fragmentShaderSource = 
        "#version 450 core"                             \
        "\n"                                            \
        "out vec4 FragColor;"                           \
        "uniform vec3 u_color;"
        "void main(void)"                               \
        "{"                                             \
            //ray color fragment
        "   FragColor = vec4(u_color, 1.0f);"           \
        "}";

    //provide source code to shader object 
    glShaderSource(occluded_fragmentShaderObject, 1, (const GLchar**)&occluded_fragmentShaderSource, NULL);

    //compile shader
    glCompileShader(occluded_fragmentShaderObject);

    //shader compilation error checking
    glGetShaderiv(occluded_fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(occluded_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(occluded_fragmentShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> fragment shader compilation log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "-> fragment shader compiled successfully \n");

    //shader program
    occluded_shaderProgramObject = glCreateProgram();

    //attach vertex shader to shader program
    glAttachShader(occluded_shaderProgramObject, occluded_vertexShaderObject);
    glAttachShader(occluded_shaderProgramObject, occluded_fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(occluded_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPositon");

    //link shader program 
    glLinkProgram(occluded_shaderProgramObject);

    //shader linking error checking
    glGetProgramiv(occluded_shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(occluded_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(occluded_shaderProgramObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> shader program link log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "-> shader program linked successfully \n");

    //get uniform locations
    occluded_mvpMatrixUniform = glGetUniformLocation(occluded_shaderProgramObject, "u_mvpMatrix"); 
    occluded_colorUniform = glGetUniformLocation(occluded_shaderProgramObject, "u_color"); 

    //Light Shaft Shader Program
    fprintf(gpFile, "\n Light Shaft Shader Program \n");
    
    //vertex shader
    godRays_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* godRays_vertexShaderSource = 
        "#version 450 core"                                                             \
        "\n"                                                                            \

        "in vec2 vPosition;"                                                            \
        "in vec2 vTexCoord;"                                                            \
        "out vec2 out_texcoord;"                                                        \

        "void main(void)"                                                               \
        "{"                                                                             \
        "   out_texcoord = vTexCoord;"                                                  \
        "   gl_Position = vec4(vPosition.x, vPosition.y, 0.0f, 1.0f);"                  \
        "}";

    //provide source code to shader object
    glShaderSource(godRays_vertexShaderObject, 1, (const GLchar**)&godRays_vertexShaderSource, NULL);

    //compile shader 
    glCompileShader(godRays_vertexShaderObject);

    //shader compilation error checking
    glGetShaderiv(godRays_vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(godRays_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(godRays_vertexShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> vertex shader compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    } 

    fprintf(gpFile, "\n-> vertex shader compiled successfully \n");

    //fragment shader
    godRays_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* godRays_fragmentShaderSource = 
        "#version 450 core"                                                                     \
        "\n"                                                                                    \

        "in vec2 out_texcoord;"                                                                 \
        
        "uniform sampler2D sceneSampler;"                                                       \
        "uniform sampler2D occludedSampler;"                                                    \
        "uniform vec2 lightPositionOnScreen;"                                                   \

        "out vec4 FragColor;"                                                                   \

        "const float decay = 0.96815f;"                                                         \
        "const float exposure = 0.3f;"                                                          \
        "const float density = 1.5f;"                                                           \
        "const float weight = 0.88767f;"                                                        \

        "void main(void)"                                                                       \
        "{"                                                                                     \
        "   int NUM_SAMPLES = 150;"                                                             \
        
        "   vec2 tc = out_texcoord.xy;"                                                         \
        "   vec2 deltaTexCoord = (tc - lightPositionOnScreen);"                                 \
        "   deltaTexCoord *= 1.0f / float(NUM_SAMPLES) * density;"                              \
        "   float illuminationDecay = 1.0f;"                                                    \

        "   vec4 base_color = texture2D(sceneSampler, tc);"                                     \
        "   vec4 occluded_color = texture2D(occludedSampler, tc);"                              \

        "   for(int i = 0; i < NUM_SAMPLES; i++)"                                               \
        "   {"                                                                                  \
        "       tc -= deltaTexCoord;"                                                           \
        "       vec4 tex_sample = texture2D(occludedSampler, tc);"                              \
        "       tex_sample *= illuminationDecay * weight;"                                      \
        "       occluded_color += tex_sample;"                                                  \
        "       illuminationDecay *= decay;"                                                    \
        "   }"                                                                                  \

        "   vec4 final_color = base_color * 0.7f + occluded_color * exposure * 0.3f;"           \
        "   FragColor = clamp(final_color, vec4(0.0f), vec4(1.0f));"                            \
        "}";

    //provide source code to shader object 
    glShaderSource(godRays_fragmentShaderObject, 1, (const GLchar**)&godRays_fragmentShaderSource, NULL);

    //compile shader
    glCompileShader(godRays_fragmentShaderObject);

    //shader compilation error checking
    glGetShaderiv(godRays_fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(godRays_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(godRays_fragmentShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> fragment shader compilation log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "-> fragment shader compiled successfully\n");

    //shader program
    godRays_shaderProgramObject = glCreateProgram();

    //attach vertex shader to shader program
    glAttachShader(godRays_shaderProgramObject, godRays_vertexShaderObject);
    glAttachShader(godRays_shaderProgramObject, godRays_fragmentShaderObject);

    //binding of shader program object with vertex shader attributes
    glBindAttribLocation(godRays_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPositon");
    glBindAttribLocation(godRays_shaderProgramObject, AMC_ATTRIBUTE_TEXCOORD, "vTexCoord");

    //link shader program 
    glLinkProgram(godRays_shaderProgramObject);

    //shader linking error checking
    glGetProgramiv(godRays_shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(godRays_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(godRays_shaderProgramObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> shader program link log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "-> shader program linked successfully\n");

    //get uniform locations
    godRays_sceneSamplerUniform = glGetUniformLocation(godRays_shaderProgramObject, "sceneSampler");
    godRays_occludedSamplerUniform = glGetUniformLocation(godRays_shaderProgramObject, "occludedSampler");
    godRays_lightPositionOnScreenUniform = glGetUniformLocation(godRays_shaderProgramObject, "lightPositionOnScreen");

    //sphere data
    GenerateSphere(innerRadius, 200, 200, vertices_inner_sphere, indices_inner_sphere);
    GenerateSphere(outerRadius, 200, 200, vertices_outer_sphere, indices_outer_sphere);
    GenerateSphere(5.0f, 100, 100, vertices_small_sphere, indices_small_sphere);

    //quad data 
    const GLfloat quad_vertices[] = 
    {
        1.0f, 1.0f,
        -1.0f, 1.0f, 
        -1.0f, -1.0f,
        1.0f, -1.0f
    };

    const GLfloat quad_texcoords[] =
    {
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };

    //pyramid data
    const GLfloat pyramidVertices[] = 
    {
        //near
        0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,

        //right
        0.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
    
        //far
        0.0f, 1.0f, 0.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,

        //left
        0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f
    };

    //Pyramid
    glGenVertexArrays(1, &vao_pyramid);
    glBindVertexArray(vao_pyramid);
        glGenBuffers(1, &vbo_pyramid_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_pyramid_vertices);
            glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);
            glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //setup vao and vbo for sphere
    glGenVertexArrays(1, &vao_inner_sphere);
    glBindVertexArray(vao_inner_sphere);
        glGenBuffers(1, &vbo_inner_sphere_position);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_inner_sphere_position);
            glBufferData(GL_ARRAY_BUFFER, vertices_inner_sphere.size() * sizeof(float), vertices_inner_sphere.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_inner_sphere_indices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_inner_sphere_indices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_inner_sphere.size() * sizeof(float), indices_inner_sphere.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenVertexArrays(1, &vao_outer_sphere);
    glBindVertexArray(vao_outer_sphere);
        glGenBuffers(1, &vbo_outer_sphere_position);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_outer_sphere_position);
            glBufferData(GL_ARRAY_BUFFER, vertices_outer_sphere.size() * sizeof(float), vertices_outer_sphere.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_outer_sphere_indices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_outer_sphere_indices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_outer_sphere.size() * sizeof(float), indices_outer_sphere.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //setup vao and vbo for screen Quad
    glGenVertexArrays(1, &vao_quad);
    glBindVertexArray(vao_quad);
        glGenBuffers(1, &vbo_quad_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_quad_vertices);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
            glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_quad_texcoords);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_quad_texcoords);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quad_texcoords), quad_texcoords, GL_STATIC_DRAW);
            glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //Sphere
    glGenVertexArrays(1, &vao_sphere);
    glBindVertexArray(vao_sphere);
        glGenBuffers(1, &vbo_sphere_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_vertices);
            glBufferData(GL_ARRAY_BUFFER, vertices_small_sphere.size() * sizeof(GLfloat), vertices_small_sphere.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_sphere_indices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_indices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_small_sphere.size() * sizeof(GLuint), indices_small_sphere.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //famebuffer configurations
    glGenFramebuffers(1, &fbo_occluded);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_occluded);
        //create a color attachment texture
        glGenTextures(1, &occluded_texture);
        glBindTexture(GL_TEXTURE_2D, occluded_texture);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, scr_width, scr_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, occluded_texture, 0);

        //create depth and stencil attachment
        glGenRenderbuffers(1, &rbo_occluded);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_occluded);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, scr_width, scr_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_occluded);

        //check if framebuffer is complete
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            fprintf(gpFile, "Error : framebuffer is not complete\n");
            DestroyWindow(ghwnd);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &fbo_scene);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_scene);
        //create a color attchment texture
        glGenTextures(1, &scene_texture);
        glBindTexture(GL_TEXTURE_2D, scene_texture);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, scr_width, scr_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene_texture, 0);

        //create depth and stencil attachment
        glGenRenderbuffers(1, &rbo_scene);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_scene);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, scr_width, scr_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_scene);

        //check if framebuffer is complete
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            fprintf(gpFile, "Error : framebuffer is not complete\n");
            DestroyWindow(ghwnd);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //smooth shading  
    glShadeModel(GL_SMOOTH);                  

    //depth
    glClearDepth(1.0f);                                     
    glEnable(GL_DEPTH_TEST);                                
    glDepthFunc(GL_LEQUAL);

    //quality of color and texture coordinate interpolation
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    
    //glEnable(GL_CULL_FACE);

    //set clearing color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  

    //set perspective projection matrix to identity
    perspectiveProjectionMatrix = mat4::identity();

    //initialize camera properties
    camera_pos = vec3(0.0f, innerRadius, 0.0f);
    camera_front = vec3(0.0f, 0.0f, 1.0f);
    camera_up = vec3(0.0f, 1.0f, 0.0f);

    //initialize atmospheric scattering variables
    lightPos = vec3(100.0f, 0.0f, 1000.0f);

    wavelength = vec3(0.65f, 0.57f, 0.475f);
    wavelength[0] = powf(wavelength[0], 4.0f);
    wavelength[1] = powf(wavelength[1], 4.0f);
    wavelength[2] = powf(wavelength[2], 4.0f);
    
    godRays_lightPosition = vec4(100.0f, 801.0f, 1000.0f, 1.0f);

    fscale = 1.0f / (outerRadius - innerRadius);

    //warm-up  call
    Resize(WIN_WIDTH, WIN_HEIGHT);
}

void GenerateSphere(float radius, float sectorCount, float stackCount, std::vector<float> &vertices, std::vector<int> &indices)
{
    //variable declaration
    float x, y, z, xy;
    int t1, t2;
    
    float sectorStep = 2.0f * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    //code
    for(int i = 0; i <= stackCount; i++)
    {
        stackAngle = M_PI / 2.0f - i * stackStep;
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);

        for(int j = 0; j <= sectorCount; j++)
        {
            sectorAngle = j * sectorStep;

            //vertices
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    for(int i = 0; i < stackCount; i++)
    {
        t1 = i * (sectorCount + 1);
        t2 = t1 + sectorCount + 1;

        for(int j = 0; j < sectorCount; j++, t1++, t2++)
        {
            if(i != 0)
            {
                indices.push_back(t1);
                indices.push_back(t2);
                indices.push_back(t1 + 1);
            }

            if(i != (stackCount - 1))
            {
                indices.push_back(t1 + 1);
                indices.push_back(t2);
                indices.push_back(t2 + 1);
            }
        }
    }
}

void Resize(int width, int height)
{
    //code
    if(height == 0)
        height = 1;

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    perspectiveProjectionMatrix = vmath::perspective(45.0f, (float)width / (float)height, 0.1f, 10000.0f);
}

void Display(void)
{
    //function declarations
    vec4 transform(vec4 vector, mat4 matrix);

    //variable declarations
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 mvpMatrix;
    
    vec2 lightPositionOnScreen;
    vec4 projected;

    //code
    viewMatrix = vmath::lookat(camera_pos, camera_pos + camera_front, camera_up);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_occluded);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        modelMatrix = mat4::identity();
        mvpMatrix = mat4::identity();

        //occluded shader
        glUseProgram(occluded_shaderProgramObject);
            //white sphere         
            modelMatrix = vmath::translate(godRays_lightPosition[0], godRays_lightPosition[1], godRays_lightPosition[2]);
            mvpMatrix = perspectiveProjectionMatrix * viewMatrix * modelMatrix;

            glUniformMatrix4fv(occluded_mvpMatrixUniform, 1, GL_FALSE, mvpMatrix);
            glUniform3f(occluded_colorUniform, 1.0f, 1.0f, 1.0f);

            glBindVertexArray(vao_sphere);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_indices);
            if(godRays_lightPosition[1] >= 780.0f)
                glDrawElements(GL_TRIANGLES, indices_small_sphere.size(), GL_UNSIGNED_INT, NULL);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            
            //black occluding scene
            modelMatrix = vmath::translate(0.0f, 801.0f, -5.0f);
            mvpMatrix = perspectiveProjectionMatrix * viewMatrix * modelMatrix;

            glUniformMatrix4fv(occluded_mvpMatrixUniform, 1, GL_FALSE, mvpMatrix);
            glUniform3f(occluded_colorUniform, 0.0f, 0.0f, 0.0f);

            glBindVertexArray(vao_pyramid);
            if(godRays_lightPosition[1] >= 700.0f)
                glDrawArrays(GL_TRIANGLES, 0, 12);
            glBindVertexArray(0);
        glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_scene);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
        modelMatrix = mat4::identity();
        mvpMatrix = mat4::identity();

        lightPos = lightPos / sqrtf(lightPos[0]*lightPos[0] + lightPos[1]*lightPos[1] + lightPos[2]*lightPos[2]);
        cameraHeight = sqrtf(camera_pos[0]*camera_pos[0] + camera_pos[1]*camera_pos[1] + camera_pos[2]*camera_pos[2]);

        glUseProgram(ground_shaderProgramObject);
            //transform matrics
            modelMatrix = mat4::identity();

            //pass the data to uniforms
            glUniformMatrix4fv(ground_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
            glUniformMatrix4fv(ground_viewMatrixUniform, 1, GL_FALSE, viewMatrix);
            glUniformMatrix4fv(ground_projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

            glUniform3fv(ground_v3CameraPosUniform, 1, camera_pos);
            glUniform3fv(ground_v3LightPos, 1, lightPos);
            glUniform3fv(ground_v3InvWavelength, 1, 1.0f / wavelength);
            glUniform1f(ground_fCameraHeight, cameraHeight);
            glUniform1f(ground_fCameraHeight2, cameraHeight * cameraHeight);
            glUniform1f(ground_fOuterRadius, outerRadius);
            glUniform1f(ground_fOuterRadius2, outerRadius * outerRadius);
            glUniform1f(ground_fInnerRadius, innerRadius);
            glUniform1f(ground_fInnerRadius2, innerRadius * innerRadius);
            glUniform1f(ground_fKrESun, rayleigh_constant * eSun);
            glUniform1f(ground_fKmESun, mie_constant * eSun);
            glUniform1f(ground_fKr4PI, rayleigh_constant * 4 * M_PI);
            glUniform1f(ground_fKm4PI, mie_constant * 4 * M_PI);
            glUniform1f(ground_fScale, fscale);
            glUniform1f(ground_fScaleDepth, scaleDepth);
            glUniform1f(ground_fScaleOverScaleDepth, fscale / scaleDepth);

            //draw sphere
            glBindVertexArray(vao_inner_sphere);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_inner_sphere_indices);
            glDrawElements(GL_TRIANGLES, indices_inner_sphere.size(), GL_UNSIGNED_INT, NULL);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        glUseProgram(0);

        glUseProgram(sky_shaderProgramObject);
            //pass the data to uniforms
            glUniformMatrix4fv(sky_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
            glUniformMatrix4fv(sky_viewMatrixUniform, 1, GL_FALSE, viewMatrix);
            glUniformMatrix4fv(sky_projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

            glUniform3fv(sky_v3CameraPosUniform, 1, camera_pos);
            glUniform3fv(sky_v3LightPos, 1, lightPos);
            glUniform3fv(sky_v3InvWavelength, 1, 1.0f / wavelength);
            glUniform1f(sky_fCameraHeight, cameraHeight);
            glUniform1f(sky_fCameraHeight2, cameraHeight * cameraHeight);
            glUniform1f(sky_fOuterRadius, outerRadius);
            glUniform1f(sky_fOuterRadius2, outerRadius * outerRadius);
            glUniform1f(sky_fInnerRadius, innerRadius);
            glUniform1f(sky_fInnerRadius2, innerRadius * innerRadius);
            glUniform1f(sky_fKrESun, rayleigh_constant * eSun);
            glUniform1f(sky_fKmESun, mie_constant * eSun);
            glUniform1f(sky_fKr4PI, rayleigh_constant * 4 * M_PI);
            glUniform1f(sky_fKm4PI, mie_constant * 4 * M_PI);
            glUniform1f(sky_fScale, fscale);
            glUniform1f(sky_fScaleDepth, scaleDepth);
            glUniform1f(sky_fScaleOverScaleDepth, fscale / scaleDepth);
            glUniform1f(sky_g, g);
            glUniform1f(sky_g2, g*g);

            glBindVertexArray(vao_outer_sphere);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_outer_sphere_indices);
            glDrawElements(GL_TRIANGLES, indices_outer_sphere.size(), GL_UNSIGNED_INT, NULL);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        glUseProgram(0);

        glUseProgram(occluded_shaderProgramObject);
            modelMatrix = vmath::translate(0.0f, 801.0f, -5.0f);
            mvpMatrix = perspectiveProjectionMatrix * viewMatrix * modelMatrix;

            glUniformMatrix4fv(occluded_mvpMatrixUniform, 1, GL_FALSE, mvpMatrix);
            glUniform3f(occluded_colorUniform, 0.5f, 0.5f, 0.5f);

            glBindVertexArray(vao_pyramid);
            
            glDrawArrays(GL_TRIANGLES, 0, 12);
            glBindVertexArray(0);
        glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(godRays_shaderProgramObject);
        //3D light position to 2D screen space position
        projected = transform(godRays_lightPosition, perspectiveProjectionMatrix * viewMatrix);
        lightPositionOnScreen[0] = (projected[0] + 1.0f) / 2.0f; 
        lightPositionOnScreen[1] = (projected[1] + 1.0f) / 2.0f; 

        glUniform2fv(godRays_lightPositionOnScreenUniform, 1, lightPositionOnScreen);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, scene_texture);
        glUniform1i(godRays_sceneSamplerUniform, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, occluded_texture);
        glUniform1i(godRays_occludedSamplerUniform, 1);

        glBindVertexArray(vao_quad);
        glDrawArrays(GL_QUADS, 0, 4);
        glBindVertexArray(0);
    glUseProgram(0);

    glDisable(GL_BLEND);

    SwapBuffers(ghdc);
}

vec4 transform(vec4 vector, mat4 matrix)
{
    //variable declaration
    vec4 transformed_vector;

    //code
    for(int i = 0; i < 4; i++)
    {
        transformed_vector[i] = vector[0] * matrix[0][i] + 
                                vector[1] * matrix[1][i] + 
                                vector[2] * matrix[2][i] + 
                                vector[3] * matrix[3][i];
    }

    transformed_vector[0] /= transformed_vector[3];
    transformed_vector[1] /= transformed_vector[3];
    transformed_vector[2] /= transformed_vector[3];
    transformed_vector[3] /= transformed_vector[3];

    return (transformed_vector);
}

void UnInitialize(void)
{
    //code
    //if window is in fullscreen mode toggle
    if(gbFullscreen == true)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
        SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowPos(ghwnd,
            HWND_TOP,
            0, 
            0,
            0,
            0,
            SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
    
        ShowCursor(true);
        gbFullscreen = false;
    }

    //sphere
    if(vao_sphere)
    {
        glDeleteVertexArrays(1, &vao_sphere);
        vao_sphere = 0;
    }

    if(vbo_sphere_vertices)
    {
        glDeleteBuffers(1, &vbo_sphere_vertices);
        vbo_sphere_vertices = 0;
    }

    if(vbo_sphere_indices)
    {
        glDeleteBuffers(1, &vbo_sphere_indices);
        vbo_sphere_indices = 0;
    }

    //pyramid
    if(vao_pyramid)
    {
        glDeleteVertexArrays(1, &vao_pyramid);
        vao_pyramid = 0;
    }

    if(vbo_pyramid_vertices)
    {
        glDeleteBuffers(1, &vbo_pyramid_vertices);
        vbo_pyramid_vertices = 0;
    }

    //release texture attachments
    if(occluded_texture)
    {
        glDeleteTextures(1, &occluded_texture);
        occluded_texture = 0;
    }

    if(scene_texture)
    {
        glDeleteTextures(1, &scene_texture);
        scene_texture = 0;
    }

    //release fbo and rbo
    if(rbo_occluded)
    {
        glDeleteRenderbuffers(1, &rbo_occluded);
        rbo_occluded = 0;
    }

    if(fbo_occluded)
    {
        glDeleteFramebuffers(1, &fbo_occluded);
        fbo_occluded = 0; 
    }

    if(rbo_scene)
    {
        glDeleteRenderbuffers(1, &rbo_scene);
        rbo_scene = 0;
    }

    if(fbo_scene)
    {
        glDeleteFramebuffers(1, &fbo_scene);
        fbo_scene = 0;
    }

    //quad
    if(vao_quad)
    {
        glDeleteVertexArrays(1, &vao_quad);
        vao_quad = 0;
    }

    if(vbo_quad_vertices)
    {
        glDeleteBuffers(1, &vbo_quad_vertices);
        vbo_quad_vertices = 0;
    }

    if(vbo_quad_texcoords)
    {
        glDeleteBuffers(1, &vbo_quad_texcoords);
        vbo_quad_texcoords = 0;
    }

    //release vao and vbo for sphere
    if(vao_inner_sphere)
    {
        glDeleteVertexArrays(1, &vao_inner_sphere);
        vao_inner_sphere = 0;
    }

    if(vbo_inner_sphere_position)
    {
        glDeleteBuffers(1, &vbo_inner_sphere_position);
        vbo_inner_sphere_position = 0;
    }

    if(vbo_inner_sphere_indices)
    {
        glDeleteBuffers(1, &vbo_inner_sphere_indices);
        vbo_inner_sphere_indices = 0;
    }

    if(vao_outer_sphere)
    {
        glDeleteVertexArrays(1, &vao_outer_sphere);
        vao_outer_sphere = 0;
    }

    if(vbo_outer_sphere_position)
    {
        glDeleteBuffers(1, &vbo_outer_sphere_position);
        vbo_outer_sphere_position = 0;
    }

    if(vbo_outer_sphere_indices)
    {
        glDeleteBuffers(1, &vbo_outer_sphere_indices);
        vbo_outer_sphere_indices = 0;
    }

    //safe shader cleanup
    if(ground_shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(ground_shaderProgramObject);
        glGetProgramiv(ground_shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(ground_shaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(ground_shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(ground_shaderProgramObject);
        ground_shaderProgramObject = 0;
        glUseProgram(0);
    }

    if(sky_shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(sky_shaderProgramObject);
        glGetProgramiv(sky_shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(sky_shaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(sky_shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(sky_shaderProgramObject);
        sky_shaderProgramObject = 0;
        glUseProgram(0);
    }

    if(occluded_shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(occluded_shaderProgramObject);
        glGetProgramiv(occluded_shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(occluded_shaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(occluded_shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(occluded_shaderProgramObject);
        occluded_shaderProgramObject = 0;
        glUseProgram(0);
    }

    if(godRays_shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(godRays_shaderProgramObject);
        glGetProgramiv(godRays_shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(godRays_shaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(godRays_shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(godRays_shaderProgramObject);
        godRays_shaderProgramObject = 0;
        glUseProgram(0);
    }

    //HGLRC : NULL means calling thread's current rendering context 
    //        is no longer current as well as it releases the device 
    //        context used by that rendering context
    //HDC : is ignored if HGLRC is passed as NULL
    if(wglGetCurrentContext() == ghrc)
    {
        wglMakeCurrent((HDC)NULL, (HGLRC)NULL);
    }

    //delete rendering context 
    if(ghrc)
    {
        wglDeleteContext(ghrc);
        ghrc = (HGLRC)NULL;
    }

    //release the device context
    if(ghdc)
    {
        ReleaseDC(ghwnd, ghdc);
        ghdc = (HDC)NULL;
    }

    //close the log file
    if(gpFile)
    {
        fprintf(gpFile, "\n-> Program Completed Successfully\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}
