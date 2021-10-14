//headers
#include <windows.h>               //standard windows header
#include <windowsx.h>              //for camera
#include <stdio.h>                 //C header 
#include <gl/glew.h>               //OpenGL extension wrangler (must be included before gl.h)
#include <gl/gl.h>                 //OpenGL header
#include <vector>                  //standard vector header      
#include <cmath>                   //standard math header                      
#include "RESOURCES.h"             //Resources header
#include "include/vmath.h"         //Maths header
#include "include/Camera.h"        //Camera header

#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

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

#define DEPTH_SIZE 512

//namespaces
using namespace std;
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

GLuint vertexShaderObject;          
GLuint fragmentShaderObject;        
GLuint shaderProgramObject;         

GLuint modelMatrixUniform; 
GLuint viewMatrixUniform; 
GLuint projectionMatrixUniform; 

GLuint lightPositionUniform; 
GLuint lightColorUniform;
GLuint cameraPositionUniform;  

GLuint albedoMapUniform; 
GLuint normalMapUniform; 
GLuint metallicMapUniform; 
GLuint roughnessMapUniform; 
GLuint aoMapUniform;        

GLuint irradianceMapUniform;
GLuint prefilterMapUniform;
GLuint brdfMapUniform;

GLuint vao_sphere;
GLuint vbo_sphere_position;
GLuint vbo_sphere_normal;
GLuint vbo_sphere_texcoord;
GLuint vbo_sphere_indices;

GLuint cubemap_vertexShaderObject;
GLuint cubemap_fragmentShaderObject;
GLuint cubemap_shaderProgramObject;

GLuint cubemap_projectionMatrixUniform;
GLuint cubemap_viewMatrixUniform;
GLuint cubemap_cubemapSamplerUniform;

GLuint background_vertexShaderObject;
GLuint background_fragmentShaderObject;
GLuint background_shaderProgramObject;

GLuint background_projectionMatrixUniform;
GLuint background_viewMatrixUniform;
GLuint background_cubemapSamplerUniform;

GLuint irradiance_fragmentShaderObject;
GLuint irradiance_shaderProgramObject;

GLuint irradiance_projectionMatrixUniform;
GLuint irradiance_viewMatrixUniform;
GLuint irradiance_cubemapSamplerUniform;

GLuint prefilter_fragmentShaderObject;
GLuint prefilter_shaderProgramObject;

GLuint prefilter_projectionMatrixUniform;
GLuint prefilter_viewMatrixUniform;
GLuint prefilter_cubemapSamplerUniform;
GLuint prefilter_roughnessUniform;

GLuint brdf_vertexShaderObject;
GLuint brdf_fragmentShaderObject;
GLuint brdf_shaderProgramObject;

GLuint fbo_capture;
GLuint rbo_capture;

GLuint hdr_texture;
GLuint cubemap_texture;
GLuint irradiance_texture;
GLuint prefilter_texture;
GLuint brdf_texture;

GLuint vao_cube;
GLuint vbo_cube;

GLuint vao_quad;
GLuint vbo_quad;

GLuint silver_albedo_texture;
GLuint silver_normal_texture;
GLuint silver_metallic_texture;
GLuint silver_roughness_texture;
GLuint silver_ao_texture;

GLuint gold_albedo_texture;
GLuint gold_normal_texture;
GLuint gold_metallic_texture;
GLuint gold_roughness_texture;
GLuint gold_ao_texture;

GLuint copper_albedo_texture;
GLuint copper_normal_texture;
GLuint copper_metallic_texture;
GLuint copper_roughness_texture;
GLuint copper_ao_texture;

GLuint black_albedo_texture;
GLuint black_normal_texture;
GLuint black_metallic_texture;
GLuint black_roughness_texture;
GLuint black_ao_texture;

GLuint painted_albedo_texture;
GLuint painted_normal_texture;
GLuint painted_metallic_texture;
GLuint painted_roughness_texture;
GLuint painted_ao_texture;

GLuint blue_albedo_texture;
GLuint blue_normal_texture;
GLuint blue_metallic_texture;
GLuint blue_roughness_texture;
GLuint blue_ao_texture;

GLuint rusted_albedo_texture;
GLuint rusted_normal_texture;
GLuint rusted_metallic_texture;
GLuint rusted_roughness_texture;
GLuint rusted_ao_texture;

mat4 perspectiveProjectionMatrix;  

Camera camera(vec3(0.0f, 0.0f, 5.0f));
GLfloat initial_val_Zoom = 0.0f;

vector<float> vertices;
vector<float> normals;
vector<float> texcoords;
vector<int> indices;

//windows entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    //function declarations
    void Initialize(void);                                      //initialize OpenGL state machine
    void Display(void);                                         //render scene

    //variable declarations
    WNDCLASSEX wndclass;                                        //structure holding window class attributes
    MSG msg;                                                    //structure holding message attributes
    HWND hwnd;                                                  //handle to a window
    TCHAR szAppName[] = TEXT("Physically Based Rendering");     //name of window class

    int cxScreen, cyScreen;                                     //screen width and height for centering window
    int init_x, init_y;                                         //top-left coordinates of centered window
    bool bDone = false;                                         //flag indicating whether or not to exit from game loop

    //code
    //create/open  'log.txt' file
    if(fopen_s(&gpFile, "log.txt", "w") != 0)
    {
        MessageBox(NULL, TEXT("Failed to open log.txt file"), TEXT("Error"), MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }
    else
    {
        fprintf(gpFile, "----- Program Started Successfully -----\n\n");
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

    //variable declarations
    POINT pt;
	static int old_x_pos;
	static int old_y_pos;
	static int new_x_pos;
	static int new_y_pos;
	static int x_offset;
	static int y_offset;

    //code
    switch(iMsg)
    {
        case WM_CREATE:
            GetCursorPos(&pt);
            old_x_pos = pt.x;
            old_y_pos = pt.y;
            break;

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
                
                default:
                    break;
            }
            break;

        case WM_CHAR:
            switch(wParam)
            {
                case 'A':
                case 'a':
                    camera.ProcessKeyboard(LEFT);
                    break;

                case 'W':
                case 'w':
                    camera.ProcessKeyboard(FORWARD);
                    break;

                case 'D':
                case 'd':
                    camera.ProcessKeyboard(RIGHT);
                    break;

                case 'S':
                case 's':
                    camera.ProcessKeyboard(BACKWARD);
                    break;

                default:
                    break;
            }
            break;

        case WM_MOUSEMOVE:
            new_x_pos	= GET_X_LPARAM(lParam);
            new_y_pos	= GET_Y_LPARAM(lParam);

            x_offset	= new_x_pos - old_x_pos;
            y_offset	= new_y_pos - old_y_pos;
            
            old_x_pos = new_x_pos;
            old_y_pos = new_y_pos;
    
            camera.ProcessMouseMovement(x_offset, y_offset);
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

void Initialize(void)
{
    //function declarations
    void Resize(int, int);          //warm-up call
    void UnInitialize(void);        //release resources
    void load_texture(const char *, GLuint *);
    void GenerateSphere(float, float, float);

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

    //--- Vertex Shader ---

    //create shader
    vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* vertexShaderSourceCode = 
        "#version 450 core"                                                             \
        "\n"                                                                            \
        "in vec3 a_position;"                                                           \
        "in vec3 a_normal;"                                                             \
        "in vec2 a_texcoord;"                                                           \

        "uniform mat4 modelMatrix;"                                                     \
        "uniform mat4 viewMatrix;"                                                      \
        "uniform mat4 projectionMatrix;"                                                \

        "out vec2 out_texcoord;"                                                        \
        "out vec3 out_worldPos;"                                                        \
        "out vec3 out_normal;"                                                          \
        
        "void main(void)"                                                               \
        "{"                                                                             \
        "   out_texcoord = a_texcoord;"                                                 \
        "   out_worldPos = vec3(modelMatrix * vec4(a_position, 1.0f));"                 \
        "   out_normal = mat3(modelMatrix) * a_normal;"                                 \
        
        "   gl_Position = projectionMatrix * viewMatrix * vec4(out_worldPos, 1.0f);"    \
        "}";

    //provide source code to shader object
    glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);

    //compile shader 
    glCompileShader(vertexShaderObject);

    //shader compilation error checking
    GLint infoLogLength = 0;
    GLint shaderCompiledStatus = 0;
    GLchar* szInfoLog = NULL;

    glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(vertexShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    } 

    fprintf(gpFile, "\n----- Vertex Shader Compiled Successfully -----\n");

    //--- Fragment Shader ---

    //create shader
    fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* fragmentShaderSourceCode = 
        "#version 450 core"                                                                     \
        "\n"                                                                                    \
        "in vec2 out_texcoord;"                                                                 \
        "in vec3 out_worldPos;"                                                                 \
        "in vec3 out_normal;"                                                                   \
        
        //material properties
        "uniform sampler2D albedoMap;"                                                          \
        "uniform sampler2D normalMap;"                                                          \
        "uniform sampler2D metallicMap;"                                                        \
        "uniform sampler2D roughnessMap;"                                                       \
        "uniform sampler2D aoMap;"                                                              \

        //IBL
        "uniform samplerCube irradianceMap;"                                                    \
        "uniform samplerCube prefilterMap;"                                                     \
        "uniform sampler2D brdfLUT;"                                                            \

        "uniform vec3 lightPosition;"                                                           \
        "uniform vec3 lightColor;"                                                              \
        "uniform vec3 cameraPosition;"                                                          \

        "out vec4 FragColor;"                                                                   \

        "const float PI = 3.14159265359f;"                                                      \

        "vec3 getNormalFromMap()"                                                               \
        "{"                                                                                     \
        "   vec3 tangentNormal = texture(normalMap, out_texcoord).xyz * 2.0f - 1.0f;"           \
        
        "   vec3 Q1 = dFdx(out_worldPos);"                                                      \
        "   vec3 Q2 = dFdy(out_worldPos);"                                                      \
        "   vec2 st1 = dFdx(out_texcoord);"                                                     \
        "   vec2 st2 = dFdy(out_texcoord);"                                                     \

        "   vec3 N = normalize(out_normal);"                                                    \
        "   vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);"                                       \
        "   vec3 B = -normalize(cross(N, T));"                                                  \
        "   mat3 TBN = mat3(T, B, N);"                                                          \

        "   return (normalize(TBN * tangentNormal));"                                           \
        "}"                                                                                     \

        "float DistributionGGX(vec3 N, vec3 H, float roughness)"                                \
        "{"                                                                                     \
        "   float a = roughness * roughness;"                                                   \
        "   float a2 = a * a;"                                                                  \
        "   float NdotH = max(dot(N, H), 0.0f);"                                                \
        "   float NdotH2 = NdotH * NdotH;"                                                      \

        "   float nom = a2;"                                                                    \
        "   float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);"                                       \
        "   denom = PI * denom * denom;"                                                        \

        "   return (nom / denom);"                                                              \
        "}"                                                                                     \

        "float GeometrySchlickGGX(float NdotV, float roughness)"                                \
        "{"                                                                                     \
        "   float r = (roughness + 1.0f);"                                                      \
        "   float k = (r * r) / 8.0f;"                                                          \

        "   float nom = NdotV;"                                                                 \
        "   float denom = NdotV * (1.0f - k) + k;"                                              \

        "   return (nom / denom);"                                                              \
        "}"                                                                                     \

        "float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)"                          \
        "{"                                                                                     \
        "   float NdotV = max(dot(N, V), 0.0f);"                                                \
        "   float NdotL = max(dot(N, L), 0.0f);"                                                \
        "   float ggx2 = GeometrySchlickGGX(NdotV, roughness);"                                 \
        "   float ggx1 = GeometrySchlickGGX(NdotL, roughness);"                                 \

        "   return (ggx1 * ggx2);"                                                              \
        "}"                                                                                     \

        "vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)"                \
        "{"                                                                                     \
        "   return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);"   \
        "}"                                                                                     \

        "vec3 fresnelSchlick(float cosTheta, vec3 F0)"                                          \
        "{"                                                                                     \
        "   return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);"           \
        "}"                                                                                     \

        "void main(void)"                                                                       \
        "{"                                                                                     \
        "   vec3 albedo = pow(texture(albedoMap, out_texcoord).rgb, vec3(2.2f));"               \
        "   float metallic = texture(metallicMap, out_texcoord).r;"                             \
        "   float roughness = texture(roughnessMap, out_texcoord).r;"                           \
        "   float ao = texture(aoMap, out_texcoord).r;"                                         \
        
        "   vec3 N = getNormalFromMap();"                                                       \
        "   vec3 V = normalize(cameraPosition - out_worldPos);"                                 \
        "   vec3 R = reflect(-V, N);"                                                           \

            //0.04 = dia-electric , albedo = metal
        "   vec3 F0 = vec3(0.04f);"                                                             \
        "   F0 = mix(F0, albedo, metallic);"                                                    \

            //reflectance equation
        "   vec3 Lo = vec3(0.0f);"                                                              \
        "   vec3 L = normalize(lightPosition - out_worldPos);"                                  \
        "   vec3 H = normalize(V + L);"                                                         \

        "   float distance = length(lightPosition - out_worldPos);"                             \
        "   float attenuation = 1.0f / (distance * distance);"                                  \
        "   vec3 radiance = lightColor * attenuation;"                                          \

            //cook-torrance BRDF
        "   float NDF = DistributionGGX(N, H, roughness);"                                      \
        "   float G = GeometrySmith(N, V, L, roughness);"                                       \
        "   vec3 F = fresnelSchlick(max(dot(H, L), 0.0f), F0);"                                 \

        "   vec3 numerator = NDF * G * F;"                                                      \
        "   float denominator = 4 * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f;"     \
        "   vec3 specular = numerator / denominator;"                                           \

        "   vec3 kS = F;"                                                                       \
        "   vec3 kD = vec3(1.0f) - kS;"                                                         \
        "   kD *= 1.0f - metallic;"                                                             \

        "   float NdotL = max(dot(N, L), 0.0f);"                                                \
        "   Lo += (kD * albedo / PI + specular) * radiance * NdotL;"                            \

            //ambient lighting (IBL)
        "   kS = fresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, roughness);"                 \
        "   kD = 1.0f - kS;"                                                                    \
        "   kD *= 1.0f - metallic;"                                                             \
        
        "   vec3 irradiance = texture(irradianceMap, N).rgb;"                                   \
        "   vec3 diffuse = irradiance * albedo;"                                                \
        
        "   const float MAX_REFLECTION_LOD = 4.0f;"                                             \
        "   vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;"    \
        "   vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0f), roughness)).rg;"            \
        "   specular = prefilteredColor * (F * brdf.x + brdf.y);"                               \

        "   vec3 ambient = (kD * diffuse + specular) * ao * attenuation;"                       \
        "   vec3 color = ambient + Lo;"                                                         \

            //HDR and gamma correction
        "   color = color / (color + vec3(1.0f));"                                              \
        "   color = pow(color, vec3(1.0f / 2.2f));"                                             \

        "   FragColor = vec4(color, 1.0f);"                                                     \
        "}";

    //provide source code to shader object 
    glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);

    //compile shader
    glCompileShader(fragmentShaderObject);

    //shader compilation error checking
    glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(fragmentShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "----- Fragment Shader Compiled Successfully -----\n");

    //--- Shader Program ---

    //create shader program
    shaderProgramObject = glCreateProgram();

    //attach vertex shader to shader program
    glAttachShader(shaderProgramObject, vertexShaderObject);

    //attach fragment shader to shader program
    glAttachShader(shaderProgramObject, fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "a_position");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "a_normal");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_TEXCOORD, "a_texcoord");

    //link shader program 
    glLinkProgram(shaderProgramObject);

    //shader linking error checking
    GLint shaderProgramLinkStatus = 0;
    glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(shaderProgramObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Shader Program Link Log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "----- Shader Program Linked Successfully -----\n");

    //get MVP uniform location
    modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "modelMatrix"); 
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "viewMatrix"); 
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "projectionMatrix"); 

    lightPositionUniform = glGetUniformLocation(shaderProgramObject, "lightPosition"); 
    lightColorUniform = glGetUniformLocation(shaderProgramObject, "lightColor");
    cameraPositionUniform = glGetUniformLocation(shaderProgramObject, "cameraPosition");  

    albedoMapUniform = glGetUniformLocation(shaderProgramObject, "albedoMap"); 
    normalMapUniform = glGetUniformLocation(shaderProgramObject, "normalMap"); 
    metallicMapUniform = glGetUniformLocation(shaderProgramObject, "metallicMap"); 
    roughnessMapUniform = glGetUniformLocation(shaderProgramObject, "roughnessMap"); 
    aoMapUniform = glGetUniformLocation(shaderProgramObject, "aoMap"); 
    
    irradianceMapUniform = glGetUniformLocation(shaderProgramObject, "irradianceMap"); 
    prefilterMapUniform = glGetUniformLocation(shaderProgramObject, "prefilterMap"); 
    brdfMapUniform = glGetUniformLocation(shaderProgramObject, "brdfLUT"); 

    //cubemap shader

    //create shader
    cubemap_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* cubemap_vertexShaderSourceCode = 
        "#version 450 core"                                                             \
        "\n"                                                                            \
        "in vec3 a_position;"                                                           \
        "out vec3 out_worldPos;"                                                        \

        "uniform mat4 projectionMatrix;"                                                \
        "uniform mat4 viewMatrix;"                                                      \

        "void main(void)"                                                               \
        "{"                                                                             \
        "   out_worldPos = a_position;"                                                 \
        "   gl_Position = projectionMatrix * viewMatrix * vec4(a_position, 1.0f);"      \
        "}";

    //provide source code to shader object
    glShaderSource(cubemap_vertexShaderObject, 1, (const GLchar**)&cubemap_vertexShaderSourceCode, NULL);

    //compile shader 
    glCompileShader(cubemap_vertexShaderObject);

    //shader compilation error checking
    glGetShaderiv(cubemap_vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(cubemap_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(cubemap_vertexShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    } 
    fprintf(gpFile, "\n----- Vertex Shader Compiled Successfully -----\n");

    //fragment shader
    cubemap_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* cubemap_fragmentShaderSourceCode = 
        "#version 450 core"                                                             \
        "\n"                                                                            \
        "in vec3 out_worldPos;"                                                         \
        "out vec4 fragColor;"                                                           \

        "uniform sampler2D cubemap;"                                                    \

        "const vec2 invAtan = vec2(0.1591f, 0.3183f);"                                  \
        "vec2 SampleSphericalMap(vec3 v)"                                               \
        "{"                                                                             \
        "   vec2 uv = vec2(atan(v.z, v.x), asin(v.y));"                                 \
        "   uv *= invAtan;"                                                             \
        "   uv += 0.5f;"                                                                \
        "   return (uv);"                                                               \
        "}"                                                                             \

        "void main(void)"                                                               \
        "{"                                                                             \
        "   vec2 uv = SampleSphericalMap(normalize(out_worldPos));"                     \
        "   vec3 color = texture(cubemap, uv).rgb;"                                     \

        "   fragColor = vec4(color, 1.0f);"                                             \
        "}";

    //provide source code to shader object 
    glShaderSource(cubemap_fragmentShaderObject, 1, (const GLchar**)&cubemap_fragmentShaderSourceCode, NULL);

    //compile shader
    glCompileShader(cubemap_fragmentShaderObject);

    //shader compilation error checking
    glGetShaderiv(cubemap_fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(cubemap_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(cubemap_fragmentShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "----- Fragment Shader Compiled Successfully -----\n");

    //shader program
    cubemap_shaderProgramObject = glCreateProgram();

    //attach vertex shader to shader program
    glAttachShader(cubemap_shaderProgramObject, cubemap_vertexShaderObject);

    //attach fragment shader to shader program
    glAttachShader(cubemap_shaderProgramObject, cubemap_fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(cubemap_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "a_position");

    //link shader program 
    glLinkProgram(cubemap_shaderProgramObject);

    //shader linking error checking
    glGetProgramiv(cubemap_shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(cubemap_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(cubemap_shaderProgramObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Shader Program Link Log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }
    fprintf(gpFile, "----- Shader Program Linked Successfully -----\n");

    //get MVP uniform location
    cubemap_projectionMatrixUniform = glGetUniformLocation(cubemap_shaderProgramObject, "projectionMatrix");
    cubemap_viewMatrixUniform = glGetUniformLocation(cubemap_shaderProgramObject, "viewMatrix");
    cubemap_cubemapSamplerUniform = glGetUniformLocation(cubemap_shaderProgramObject, "cubemap");

    //cubemap shader

    //create shader
    background_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* background_vertexShaderSourceCode = 
        "#version 450 core"                                                             \
        "\n"                                                                            \
        "in vec3 a_position;"                                                           \
        "out vec3 out_worldPos;"                                                        \

        "uniform mat4 projectionMatrix;"                                                \
        "uniform mat4 viewMatrix;"                                                      \

        "void main(void)"                                                               \
        "{"                                                                             \
        "   out_worldPos = a_position;"                                                 \
        
        "   mat4 rotView = mat4(mat3(viewMatrix));"                                     \
        "   vec4 clipPos = projectionMatrix * rotView * vec4(out_worldPos, 1.0f);"      \
        "   gl_Position = clipPos.xyww;"                                                \
        "}";

    //provide source code to shader object
    glShaderSource(background_vertexShaderObject, 1, (const GLchar**)&background_vertexShaderSourceCode, NULL);

    //compile shader 
    glCompileShader(background_vertexShaderObject);

    //shader compilation error checking
    glGetShaderiv(background_vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(background_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(background_vertexShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    } 
    fprintf(gpFile, "\n----- Vertex Shader Compiled Successfully -----\n");

    //fragment shader
    background_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* background_fragmentShaderSourceCode = 
        "#version 450 core"                                                             \
        "\n"                                                                            \
        "in vec3 out_worldPos;"                                                         \
        "out vec4 fragColor;"                                                           \

        "uniform samplerCube cubemap;"                                                  \

        "void main(void)"                                                               \
        "{"                                                                             \
        "   vec3 envColor = texture(cubemap, out_worldPos).rgb;"                        \

            //HDR and gamma correction
        "   envColor = envColor / (envColor + vec3(1.0f));"                             \
        "   envColor = pow(envColor, vec3(1.0f / 2.2f));"                               \

        "   fragColor = vec4(envColor, 1.0f);"                                          \
        "}";

    //provide source code to shader object 
    glShaderSource(background_fragmentShaderObject, 1, (const GLchar**)&background_fragmentShaderSourceCode, NULL);

    //compile shader
    glCompileShader(background_fragmentShaderObject);

    //shader compilation error checking
    glGetShaderiv(background_fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(background_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(background_fragmentShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "----- Fragment Shader Compiled Successfully -----\n");

    //shader program
    background_shaderProgramObject = glCreateProgram();

    //attach vertex shader to shader program
    glAttachShader(background_shaderProgramObject, background_vertexShaderObject);

    //attach fragment shader to shader program
    glAttachShader(background_shaderProgramObject, background_fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(background_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "a_position");

    //link shader program 
    glLinkProgram(background_shaderProgramObject);

    //shader linking error checking
    glGetProgramiv(background_shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(background_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(background_shaderProgramObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Shader Program Link Log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }
    fprintf(gpFile, "----- Shader Program Linked Successfully -----\n");

    //get MVP uniform location
    background_projectionMatrixUniform = glGetUniformLocation(background_shaderProgramObject, "projectionMatrix");
    background_viewMatrixUniform = glGetUniformLocation(background_shaderProgramObject, "viewMatrix");
    background_cubemapSamplerUniform = glGetUniformLocation(background_shaderProgramObject, "cubemap");
    
    //irradiance shader 

    //fragment shader
    irradiance_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* irradiance_fragmentShaderSourceCode = 
        "#version 450 core"                                                                                 \
        "\n"                                                                                                \
        "in vec3 out_worldPos;"                                                                             \
        "out vec4 fragColor;"                                                                               \

        "uniform samplerCube cubemap;"                                                                      \

        "const float PI = 3.14159265359f;"                                                                  \

        "void main(void)"                                                                                   \
        "{"                                                                                                 \
        "   vec3 N = normalize(out_worldPos);"                                                              \
        "   vec3 irradiance = vec3(0.0f);"                                                                  \

        "   vec3 up = vec3(0.0f, 1.0f, 0.0f);"                                                              \
        "   vec3 right = normalize(cross(up, N));"                                                          \
        "   up = normalize(cross(N, right));"                                                               \

        "   float sampleDelta = 0.025f;"                                                                    \
        "   float nrSamples = 0.0f;"                                                                        \
        "   for(float phi = 0.0f; phi < 2.0f * PI; phi += sampleDelta)"                                     \
        "   {"                                                                                              \
        "       for(float theta = 0.0f; theta < 0.5f * PI; theta += sampleDelta)"                           \
        "       {"                                                                                          \
        "           vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));"   \
        "           vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;" \
        "           irradiance += texture(cubemap, sampleVec).rgb * cos(theta) * sin(theta);"               \
        "           nrSamples++;"                                                                           \
        "       }"                                                                                          \
        "   }"                                                                                              \

        "   irradiance = PI * irradiance * (1.0f / float(nrSamples));"                                      \
        "   fragColor = vec4(irradiance, 1.0f);"                                                            \
        "}";

    //provide source code to shader object 
    glShaderSource(irradiance_fragmentShaderObject, 1, (const GLchar**)&irradiance_fragmentShaderSourceCode, NULL);

    //compile shader
    glCompileShader(irradiance_fragmentShaderObject);

    //shader compilation error checking
    glGetShaderiv(irradiance_fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(irradiance_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(irradiance_fragmentShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "\n----- Fragment Shader Compiled Successfully -----\n");

    //shader program
    irradiance_shaderProgramObject = glCreateProgram();

    //attach vertex shader to shader program
    glAttachShader(irradiance_shaderProgramObject, cubemap_vertexShaderObject);

    //attach fragment shader to shader program
    glAttachShader(irradiance_shaderProgramObject, irradiance_fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(irradiance_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "a_position");

    //link shader program 
    glLinkProgram(irradiance_shaderProgramObject);

    //shader linking error checking
    glGetProgramiv(irradiance_shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(irradiance_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(irradiance_shaderProgramObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Shader Program Link Log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }
    fprintf(gpFile, "----- Shader Program Linked Successfully -----\n");

    //get MVP uniform location
    irradiance_projectionMatrixUniform = glGetUniformLocation(irradiance_shaderProgramObject, "projectionMatrix");
    irradiance_viewMatrixUniform = glGetUniformLocation(irradiance_shaderProgramObject, "viewMatrix");
    irradiance_cubemapSamplerUniform = glGetUniformLocation(irradiance_shaderProgramObject, "cubemap");

    //prefilter shader 

    //fragment shader
    prefilter_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* prefilter_fragmentShaderSourceCode = 
        "#version 450 core"                                                                                 \
        "\n"                                                                                                \
        "in vec3 out_worldPos;"                                                                             \
        "out vec4 fragColor;"                                                                               \

        "uniform samplerCube cubemap;"                                                                      \
        "uniform float roughness;"                                                                          \

        "const float PI = 3.14159265359f;"                                                                  \

        "float DistributionGGX(vec3 N, vec3 H, float roughness)"                                            \
        "{"                                                                                                 \
        "   float a = roughness * roughness;"                                                               \
        "   float a2 = a * a;"                                                                              \
        "   float NdotH = max(dot(N, H), 0.0f);"                                                            \
        "   float NdotH2 = NdotH * NdotH;"                                                                  \

        "   float nom = a2;"                                                                                \
        "   float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);"                                                   \
        "   denom = PI * denom * denom;"                                                                    \

        "   return (nom/denom);"                                                                            \
        "}"                                                                                                 \

        "float RadicalInverse_VdC(uint bits)"                                                               \
        "{"                                                                                                 \
        "   bits = (bits << 16u) | (bits >> 16u);"                                                          \
        "   bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);"                            \
        "   bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);"                            \
        "   bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);"                            \
        "   bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);"                            \
        "   return float(bits) * 2.3283064365386963e-10;"                                                   \
        "}"                                                                                                 \

        "vec2 Hammersley(uint i, uint N)"                                                                   \
        "{"                                                                                                 \
        "   return vec2(float(i)/float(N), RadicalInverse_VdC(i));"                                         \
        "}"                                                                                                 \

        "vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)"                                        \
        "{"                                                                                                 \
        "   float a = roughness * roughness;"                                                               \
        "   float phi = 2.0f * PI * Xi.x;"                                                                  \
        "   float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));"                         \
        "   float sinTheta = sqrt(1.0f - cosTheta * cosTheta);"                                             \

        "   vec3 H;"                                                                                        \
        "   H.x = cos(phi) * sinTheta;"                                                                     \
        "   H.y = sin(phi) * sinTheta;"                                                                     \
        "   H.z = cosTheta;"                                                                                \

        "   vec3 up = abs(N.z) < 0.999f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);"                 \
        "   vec3 tangent = normalize(cross(up, N));"                                                        \
        "   vec3 bitangent = cross(N, tangent);"                                                            \
        
        "   vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;"                                    \
        "   return (normalize(sampleVec));"                                                                 \
        "}"                                                                                                 \

        "void main(void)"                                                                                   \
        "{"                                                                                                 \
        "   vec3 N = normalize(out_worldPos);"                                                              \
        "   vec3 R = N;"                                                                                    \
        "   vec3 V = R;"                                                                                    \

        "   const uint SAMPLE_COUNT = 1024u;"                                                               \
        "   vec3 prefilteredColor = vec3(0.0f);"                                                            \
        "   float totalWeight = 0.0f;"                                                                      \

        "   for(uint i = 0u; i < SAMPLE_COUNT; i++)"                                                        \
        "   {"                                                                                              \
        "       vec2 Xi = Hammersley(i, SAMPLE_COUNT);"                                                     \
        "       vec3 H = ImportanceSampleGGX(Xi, N, roughness);"                                            \
        "       vec3 L = normalize(2.0f * dot(V, H) * H - V);"                                              \

        "       float NdotL = max(dot(N, L), 0.0f);"                                                        \
        "       if(NdotL > 0.0f)"                                                                           \
        "       {"                                                                                          \
        "           float D = DistributionGGX(N, H, roughness);"                                            \
        "           float NdotH = max(dot(N, H), 0.0f);"                                                    \
        "           float HdotV = max(dot(H, V), 0.0f);"                                                    \
        "           float pdf = D * NdotH / (4.0f * HdotV) + 0.0001f;"                                      \

        "           float resolution = 512.0f;"                                                             \
        "           float saTexel = 4.0f * PI / (6.0f * resolution * resolution);"                          \
        "           float saSample = 1.0f / (float(SAMPLE_COUNT) * pdf + 0.0001f);"                         \
        "           float mipLevel = roughness == 0.0f ? 0.0f : 0.5f * log2(saSample / saTexel);"           \

        "           prefilteredColor += textureLod(cubemap, L, mipLevel).rgb * NdotL;"                      \
        "           totalWeight += NdotL;"                                                                  \
        "       }"                                                                                          \
        "   }"                                                                                              \

        "   prefilteredColor = prefilteredColor / totalWeight;"                                             \
        "   fragColor = vec4(prefilteredColor, 1.0f);"                                                      \
        "}";

    //provide source code to shader object 
    glShaderSource(prefilter_fragmentShaderObject, 1, (const GLchar**)&prefilter_fragmentShaderSourceCode, NULL);

    //compile shader
    glCompileShader(prefilter_fragmentShaderObject);

    //shader compilation error checking
    glGetShaderiv(prefilter_fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(prefilter_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(prefilter_fragmentShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "\n----- Fragment Shader Compiled Successfully -----\n");

    //shader program
    prefilter_shaderProgramObject = glCreateProgram();

    //attach vertex shader to shader program
    glAttachShader(prefilter_shaderProgramObject, cubemap_vertexShaderObject);

    //attach fragment shader to shader program
    glAttachShader(prefilter_shaderProgramObject, prefilter_fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(prefilter_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "a_position");

    //link shader program 
    glLinkProgram(prefilter_shaderProgramObject);

    //shader linking error checking
    glGetProgramiv(prefilter_shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(prefilter_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(prefilter_shaderProgramObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Shader Program Link Log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }
    fprintf(gpFile, "----- Shader Program Linked Successfully -----\n");

    //get MVP uniform location
    prefilter_projectionMatrixUniform = glGetUniformLocation(prefilter_shaderProgramObject, "projectionMatrix");
    prefilter_viewMatrixUniform = glGetUniformLocation(prefilter_shaderProgramObject, "viewMatrix");
    prefilter_cubemapSamplerUniform = glGetUniformLocation(prefilter_shaderProgramObject, "cubemap");
    prefilter_roughnessUniform = glGetUniformLocation(prefilter_shaderProgramObject, "roughness");

    //brdf shader

    //create shader
    brdf_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* brdf_vertexShaderSourceCode = 
        "#version 450 core"                                                             \
        "\n"                                                                            \
        "in vec3 a_position;"                                                           \
        "in vec2 a_texcoord;"                                                           \
        "out vec2 out_texcoord;"                                                        \

        "void main(void)"                                                               \
        "{"                                                                             \
        "   out_texcoord = a_texcoord;"                                                 \
        "   gl_Position = vec4(a_position, 1.0f);"                                      \
        "}";

    //provide source code to shader object
    glShaderSource(brdf_vertexShaderObject, 1, (const GLchar**)&brdf_vertexShaderSourceCode, NULL);

    //compile shader 
    glCompileShader(brdf_vertexShaderObject);

    //shader compilation error checking
    glGetShaderiv(brdf_vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(brdf_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(brdf_vertexShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    } 
    fprintf(gpFile, "\n----- Vertex Shader Compiled Successfully -----\n");

    //fragment shader
    brdf_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* brdf_fragmentShaderSourceCode = 
        "#version 450 core"                                                             \
        "\n"                                                                            \
        "in vec2 out_texcoord;"                                                         \
        "out vec2 fragColor;"                                                           \

        "const float PI = 3.14159265359f;"                                              \

        "float RadicalInverse_VdC(uint bits)"                                           \
        "{"                                                                             \
        "   bits = (bits << 16u) | (bits >> 16u);"                                      \
        "   bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);"        \
        "   bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);"        \
        "   bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);"        \
        "   bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);"        \
        "   return float(bits) * 2.3283064365386963e-10;"                               \
        "}"                                                                             \

        "vec2 Hammersley(uint i, uint N)"                                               \
        "{"                                                                             \
        "   return vec2(float(i)/float(N), RadicalInverse_VdC(i));"                     \
        "}"                                                                             \

        "vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)"                    \
        "{"                                                                             \
        "   float a = roughness * roughness;"                                           \

        "   float phi = 2.0f * PI * Xi.x;"                                              \
        "   float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));"     \
        "   float sinTheta = sqrt(1.0f - cosTheta * cosTheta);"                         \

        "   vec3 H;"                                                                    \
        "   H.x = cos(phi) * sinTheta;"                                                 \
        "   H.y = sin(phi) * sinTheta;"                                                 \
        "   H.z = cosTheta;"                                                            \

        "   vec3 up = abs(N.z) < 0.999f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);" \
        "   vec3 tangent = normalize(cross(up, N));"                                    \
        "   vec3 bitangent = cross(N, tangent);"                                        \

        "   vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;"                \
        "   return (normalize(sampleVec));"                                             \
        "}"                                                                             \

        "float GeometrySchlickGGX(float NdotV, float roughness)"                        \
        "{"                                                                             \
        "   float r = roughness;"                                                       \
        "   float k = (r * r) / 2.0f;"                                                  \

        "   float nom = NdotV;"                                                         \
        "   float denom = NdotV * (1.0f - k) + k;"                                      \

        "   return (nom / denom);"                                                      \
        "}"                                                                             \

        "float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)"                  \
        "{"                                                                             \
        "   float NdotV = max(dot(N, V), 0.0f);"                                        \
        "   float NdotL = max(dot(N, L), 0.0f);"                                        \
        "   float ggx2 = GeometrySchlickGGX(NdotV, roughness);"                         \
        "   float ggx1 = GeometrySchlickGGX(NdotL, roughness);"                         \

        "   return (ggx1 * ggx2);"                                                      \
        "}"                                                                             \

        "vec2 IntegrateBRDF(float NdotV, float roughness)"                              \
        "{"                                                                             \
        "   vec3 V;"                                                                    \
        "   V.x = sqrt(1.0f - NdotV * NdotV);"                                          \
        "   V.y = 0.0f;"                                                                \
        "   V.z = NdotV;"                                                               \

        "   float A = 0.0f;"                                                            \
        "   float B = 0.0f;"                                                            \

        "   vec3 N = vec3(0.0f, 0.0f, 1.0f);"                                           \
        "   const uint SAMPLE_COUNT = 1024u;"                                           \
        "   for(uint i = 0u; i < SAMPLE_COUNT; i++)"                                    \
        "   {"                                                                          \
        "       vec2 Xi = Hammersley(i, SAMPLE_COUNT);"                                 \
        "       vec3 H = ImportanceSampleGGX(Xi, N, roughness);"                        \
        "       vec3 L = normalize(2.0f * dot(V, H) * H - V);"                          \
        
        "       float NdotL = max(L.z, 0.0f);"                                          \
        "       float NdotH = max(H.z, 0.0f);"                                          \
        "       float VdotH = max(dot(V, H), 0.0f);"                                    \

        "       if(NdotL > 0.0f)"                                                       \
        "       {"                                                                      \
        "           float G = GeometrySmith(N, V, L, roughness);"                       \
        "           float G_Vis = (G * VdotH) / (NdotH * NdotV);"                       \
        "           float Fc = pow(1.0f - VdotH, 5.0f);"                                \

        "           A += (1.0f - Fc) * G_Vis;"                                          \
        "           B += Fc * G_Vis;"                                                   \
        "       }"                                                                      \
        "   }"                                                                          \

        "   A /= float(SAMPLE_COUNT);"                                                  \
        "   B /= float(SAMPLE_COUNT);"                                                  \
        "   return (vec2(A, B));"                                                       \
        "}"                                                                             \

        "void main(void)"                                                               \
        "{"                                                                             \
        "   vec2 integratedBRDF = IntegrateBRDF(out_texcoord.x, out_texcoord.y);"       \
        "   fragColor = integratedBRDF;"                                                \
        "}";

    //provide source code to shader object 
    glShaderSource(brdf_fragmentShaderObject, 1, (const GLchar**)&brdf_fragmentShaderSourceCode, NULL);

    //compile shader
    glCompileShader(brdf_fragmentShaderObject);

    //shader compilation error checking
    glGetShaderiv(brdf_fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(brdf_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(brdf_fragmentShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "----- Fragment Shader Compiled Successfully -----\n");

    //shader program
    brdf_shaderProgramObject = glCreateProgram();

    //attach vertex shader to shader program
    glAttachShader(brdf_shaderProgramObject, brdf_vertexShaderObject);

    //attach fragment shader to shader program
    glAttachShader(brdf_shaderProgramObject, brdf_fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(brdf_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "a_position");
    glBindAttribLocation(brdf_shaderProgramObject, AMC_ATTRIBUTE_TEXCOORD, "a_texcoord");


    //link shader program 
    glLinkProgram(brdf_shaderProgramObject);

    //shader linking error checking
    glGetProgramiv(brdf_shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(brdf_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(brdf_shaderProgramObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Shader Program Link Log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }
    fprintf(gpFile, "----- Shader Program Linked Successfully -----\n");

    //construct sphere data
    GenerateSphere(1.0f, 50, 50);

    //setup vao and vbo
    glGenVertexArrays(1, &vao_sphere);
    glBindVertexArray(vao_sphere);
        glGenBuffers(1, &vbo_sphere_position);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_position);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_sphere_normal);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_normal);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_sphere_texcoord);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_texcoord);
        glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(float), texcoords.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_sphere_indices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_indices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(float), indices.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);

    //setup cube data
    float cube_vertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &vao_cube);
    glBindVertexArray(vao_cube);
        glGenBuffers(1, &vbo_cube);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_cube);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //setup quad data
    float quad_vertices[] = 
    {
        // positions            // texture Coords
        -1.0f,  1.0f, 0.0f,     0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,     0.0f, 0.0f,
         1.0f,  1.0f, 0.0f,     1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,     1.0f, 0.0f,
    };

    glGenVertexArrays(1, &vao_quad);
    glBindVertexArray(vao_quad);
        glGenBuffers(1, &vbo_quad);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_quad);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
        
        glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //framebuffer configurations
    glGenFramebuffers(1, &fbo_capture);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_capture);
        glGenRenderbuffers(1, &rbo_capture);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_capture);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, DEPTH_SIZE, DEPTH_SIZE);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_capture);
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
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    //set clearing color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  

    //set perspective projection matrix to identity
    perspectiveProjectionMatrix = mat4::identity();

    //load textures

    //material 1
    load_texture("textures/Silver/albedo.png", &silver_albedo_texture);
    load_texture("textures/Silver/normal.png", &silver_normal_texture);
    load_texture("textures/Silver/metallic.png", &silver_metallic_texture);
    load_texture("textures/Silver/roughness.png", &silver_roughness_texture);
    load_texture("textures/Silver/ao.png", &silver_ao_texture);

    //material 2
    load_texture("textures/Copper/albedo.png", &copper_albedo_texture);
    load_texture("textures/Copper/normal.png", &copper_normal_texture);
    load_texture("textures/Copper/metallic.png", &copper_metallic_texture);
    load_texture("textures/Copper/roughness.png", &copper_roughness_texture);
    load_texture("textures/Copper/ao.png", &copper_ao_texture);

    //material 3
    load_texture("textures/Gold/albedo.png", &gold_albedo_texture);
    load_texture("textures/Gold/normal.png", &gold_normal_texture);
    load_texture("textures/Gold/metallic.png", &gold_metallic_texture);
    load_texture("textures/Gold/roughness.png", &gold_roughness_texture);
    load_texture("textures/Gold/ao.png", &gold_ao_texture);

    //material 4
    load_texture("textures/Black/albedo.png", &black_albedo_texture);
    load_texture("textures/Black/normal.png", &black_normal_texture);
    load_texture("textures/Black/metallic.png", &black_metallic_texture);
    load_texture("textures/Black/roughness.png", &black_roughness_texture);
    load_texture("textures/Black/ao.png", &black_ao_texture);

    //material 5
    load_texture("textures/Painted/albedo.png", &painted_albedo_texture);
    load_texture("textures/Painted/normal.png", &painted_normal_texture);
    load_texture("textures/Painted/metallic.png", &painted_metallic_texture);
    load_texture("textures/Painted/roughness.png", &painted_roughness_texture);
    load_texture("textures/Painted/ao.png", &painted_ao_texture);

    //material 6
    load_texture("textures/Blue/albedo.png", &blue_albedo_texture);
    load_texture("textures/Blue/normal.png", &blue_normal_texture);
    load_texture("textures/Blue/metallic.png", &blue_metallic_texture);
    load_texture("textures/Blue/roughness.png", &blue_roughness_texture);
    load_texture("textures/Blue/ao.png", &blue_ao_texture);

    //material 7
    load_texture("textures/Rusted/albedo.png", &rusted_albedo_texture);
    load_texture("textures/Rusted/normal.png", &rusted_normal_texture);
    load_texture("textures/Rusted/metallic.png", &rusted_metallic_texture);
    load_texture("textures/Rusted/roughness.png", &rusted_roughness_texture);
    load_texture("textures/Rusted/ao.png", &rusted_ao_texture);

    //load HDR environment map
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    float *pixel_data = stbi_loadf("textures/Factory.hdr", &width, &height, &nrComponents, 0);
    if(pixel_data == NULL)
    {
        fprintf(gpFile, "Error : failed to load environment textures.\n");
        DestroyWindow(ghwnd);
    }

    glGenTextures(1, &hdr_texture);
    glBindTexture(GL_TEXTURE_2D, hdr_texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, pixel_data);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(pixel_data);
    pixel_data = NULL;

    //setup cubemap
    glGenTextures(1, &cubemap_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        for(int i = 0; i < 6; i++)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, DEPTH_SIZE, DEPTH_SIZE, 0, GL_RGB, GL_FLOAT, NULL);
        }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    //convert HDR equirectangular environment map to cubemap equivalent 
    mat4 capture_projectionMatrix = vmath::perspective(90.0f, 1.0f, 0.1f, 10.0f);
    mat4 capture_viewMatrix[] = 
    {
        lookat(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)),
        lookat(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)),
        lookat(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)),
        lookat(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f)),
        lookat(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f)),
        lookat(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f)),
    };
    
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_capture);
        glViewport(0, 0, DEPTH_SIZE, DEPTH_SIZE);

        glUseProgram(cubemap_shaderProgramObject);
            glUniformMatrix4fv(cubemap_projectionMatrixUniform, 1, GL_FALSE, capture_projectionMatrix);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, hdr_texture);
            glUniform1i(cubemap_cubemapSamplerUniform, 0);

            for(int i = 0; i < 6; i++)
            {
                glUniformMatrix4fv(cubemap_viewMatrixUniform, 1, GL_FALSE, capture_viewMatrix[i]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap_texture, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
                glBindVertexArray(vao_cube);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
            }
        glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    //create irradiance cubemap
    glGenTextures(1, &irradiance_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_texture);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        for(int i = 0; i < 6; i++)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, NULL);
        }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    //rescale capture fbo
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_capture);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_capture);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_capture);
        glViewport(0, 0, 32, 32);

        glUseProgram(irradiance_shaderProgramObject);
            glUniformMatrix4fv(irradiance_projectionMatrixUniform, 1, GL_FALSE, capture_projectionMatrix);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
            glUniform1i(irradiance_cubemapSamplerUniform, 0);

            for(int i = 0; i < 6; i++)
            {
                glUniformMatrix4fv(irradiance_viewMatrixUniform, 1, GL_FALSE, capture_viewMatrix[i]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_texture, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
                glBindVertexArray(vao_cube);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
            }
        glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //create pre-filter cubemap
    glGenTextures(1, &prefilter_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_texture);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        for(int i = 0; i < 6; i++)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, NULL);
        }
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    //create prefilter cubemap from environmental lighting
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_capture);
        glUseProgram(prefilter_shaderProgramObject);
            glUniformMatrix4fv(prefilter_projectionMatrixUniform, 1, GL_FALSE, capture_projectionMatrix);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
            glUniform1i(prefilter_cubemapSamplerUniform, 0);

            unsigned int maxMipLevels = 5;
            for(int miplevel = 0; miplevel < maxMipLevels; miplevel++)
            {
                unsigned int mipWidth = 128 * std::pow(0.5f, miplevel);
                unsigned int mipHeight = 128 * std::pow(0.5f, miplevel);

                glBindRenderbuffer(GL_RENDERBUFFER, rbo_capture);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
                glViewport(0, 0, mipWidth, mipHeight);

                float roughness = (float)miplevel / (float)(maxMipLevels - 1);
                glUniform1f(prefilter_roughnessUniform, roughness);

                for(int i = 0; i < 6; i++)
                {
                    glUniformMatrix4fv(prefilter_viewMatrixUniform, 1, GL_FALSE, capture_viewMatrix[i]);
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilter_texture, miplevel);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
                    glBindVertexArray(vao_cube);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                    glBindVertexArray(0);
                }
            }
        glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //generate a 2D LUT from the BRDF equations used
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_capture);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_capture);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);                        
    
    glGenTextures(1, &brdf_texture);
    glBindTexture(GL_TEXTURE_2D, brdf_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdf_texture, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //render to brdf texture
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_capture);
        glViewport(0, 0, 512, 512);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(brdf_shaderProgramObject);
            glBindVertexArray(vao_quad);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);
        glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //warm-up  call
    Resize(WIN_WIDTH, WIN_HEIGHT);
}

void load_texture(const char *filename, GLuint *texture)
{
    //variable declarations
    int width, height;
    unsigned char *pixel_data = NULL;

    //code
    pixel_data = stbi_load(filename, &width, &height, NULL, 0);
    if(pixel_data == NULL)
    {
        fprintf(gpFile, "failed to load texture.\n");
        DestroyWindow(ghwnd);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    //set up texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    //push the data to texture memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLint)width, (GLint)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)pixel_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(pixel_data);
    pixel_data = NULL;
}

void GenerateSphere(float radius, float sectorCount, float stackCount)
{
    //variable declaration
    float x, y, z, xy;
    float nx, ny, nz, lengthInv = 1.0f / radius;
    float s, t;
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

            //normals
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;

            normals.push_back(nx);
            normals.push_back(ny);
            normals.push_back(nz);

            //texcoords
            s = (float)j / sectorCount;
            t = (float)i / stackCount;

            texcoords.push_back(s);
            texcoords.push_back(t);
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
    //if current height is 0 set 1 to avoid 
    //divide by 0 error 
    if(height == 0)
        height = 1;

    //set viewport transformation
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    perspectiveProjectionMatrix = vmath::perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
}

void Display(void)
{
    //variable declarations
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 rotationMatrix;

    static float light_translation = -10.0f;
    static bool bRot = false;

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    rotationMatrix = mat4::identity();
    
    viewMatrix = camera.GetViewMatrix();
    rotationMatrix = vmath::rotate(90.0f, 1.0f, 0.0f, 0.0f);

    glUseProgram(shaderProgramObject);
        glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

        glUniform3f(lightPositionUniform, light_translation, 5.0f, 0.0f);
        glUniform3f(lightColorUniform, 350.0f, 350.0f, 350.0f);
        glUniform3fv(cameraPositionUniform, 1, camera.Position);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_texture);
        glUniform1i(irradianceMapUniform, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_texture);
        glUniform1i(prefilterMapUniform, 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brdf_texture);
        glUniform1i(brdfMapUniform, 2);

        //sphere 1
        modelMatrix = vmath::translate(9.0f, 0.0f, -3.0f);
        modelMatrix = modelMatrix * rotationMatrix;
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, rusted_albedo_texture);
        glUniform1i(albedoMapUniform, 3);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, rusted_normal_texture);
        glUniform1i(normalMapUniform, 4);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, rusted_metallic_texture);
        glUniform1i(metallicMapUniform, 5);

        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, rusted_roughness_texture);
        glUniform1i(roughnessMapUniform, 6);

        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, rusted_ao_texture);
        glUniform1i(aoMapUniform, 7);

        glBindVertexArray(vao_sphere);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, NULL);
        glBindVertexArray(0);

        //sphere 2
        modelMatrix = vmath::translate(6.0f, 0.0f, -3.0f);
        modelMatrix = modelMatrix * rotationMatrix;
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, copper_albedo_texture);
        glUniform1i(albedoMapUniform, 3);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, copper_normal_texture);
        glUniform1i(normalMapUniform, 4);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, copper_metallic_texture);
        glUniform1i(metallicMapUniform, 5);

        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, copper_roughness_texture);
        glUniform1i(roughnessMapUniform, 6);

        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, copper_ao_texture);
        glUniform1i(aoMapUniform, 7);

        glBindVertexArray(vao_sphere);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, NULL);
        glBindVertexArray(0);

        //sphere 3
        modelMatrix = vmath::translate(3.0f, 0.0f, -3.0f);
        modelMatrix = modelMatrix * rotationMatrix;
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, silver_albedo_texture);
        glUniform1i(albedoMapUniform, 3);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, silver_normal_texture);
        glUniform1i(normalMapUniform, 4);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, silver_metallic_texture);
        glUniform1i(metallicMapUniform, 5);

        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, silver_roughness_texture);
        glUniform1i(roughnessMapUniform, 6);

        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, silver_ao_texture);
        glUniform1i(aoMapUniform, 7);

        glBindVertexArray(vao_sphere);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, NULL);
        glBindVertexArray(0);        

        //sphere 4
        modelMatrix = vmath::translate(0.0f, 0.0f, -3.0f);
        modelMatrix = modelMatrix * rotationMatrix;
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, gold_albedo_texture);
        glUniform1i(albedoMapUniform, 3);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, gold_normal_texture);
        glUniform1i(normalMapUniform, 4);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, gold_metallic_texture);
        glUniform1i(metallicMapUniform, 5);

        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, gold_roughness_texture);
        glUniform1i(roughnessMapUniform, 6);

        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, gold_ao_texture);
        glUniform1i(aoMapUniform, 7);

        glBindVertexArray(vao_sphere);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, NULL);
        glBindVertexArray(0);        

        //sphere 5
        modelMatrix = vmath::translate(-3.0f, 0.0f, -3.0f);
        modelMatrix = modelMatrix * rotationMatrix;
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, black_albedo_texture);
        glUniform1i(albedoMapUniform, 3);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, black_normal_texture);
        glUniform1i(normalMapUniform, 4);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, black_metallic_texture);
        glUniform1i(metallicMapUniform, 5);

        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, black_roughness_texture);
        glUniform1i(roughnessMapUniform, 6);

        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, black_ao_texture);
        glUniform1i(aoMapUniform, 7);

        glBindVertexArray(vao_sphere);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, NULL);
        glBindVertexArray(0);  

        //sphere 6
        modelMatrix = vmath::translate(-6.0f, 0.0f, -3.0f);
        modelMatrix = modelMatrix * rotationMatrix;
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, painted_albedo_texture);
        glUniform1i(albedoMapUniform, 3);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, painted_normal_texture);
        glUniform1i(normalMapUniform, 4);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, painted_metallic_texture);
        glUniform1i(metallicMapUniform, 5);

        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, painted_roughness_texture);
        glUniform1i(roughnessMapUniform, 6);

        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, painted_ao_texture);
        glUniform1i(aoMapUniform, 7);

        glBindVertexArray(vao_sphere);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, NULL);
        glBindVertexArray(0);  

        //sphere 7
        modelMatrix = vmath::translate(-9.0f, 0.0f, -3.0f);
        modelMatrix = modelMatrix * rotationMatrix;
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, blue_albedo_texture);
        glUniform1i(albedoMapUniform, 3);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, blue_normal_texture);
        glUniform1i(normalMapUniform, 4);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, blue_metallic_texture);
        glUniform1i(metallicMapUniform, 5);

        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, blue_roughness_texture);
        glUniform1i(roughnessMapUniform, 6);

        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, blue_ao_texture);
        glUniform1i(aoMapUniform, 7);

        glBindVertexArray(vao_sphere);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, NULL);
        glBindVertexArray(0);  
    glUseProgram(0);

    glUseProgram(background_shaderProgramObject);
        glUniformMatrix4fv(background_projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
        glUniformMatrix4fv(background_viewMatrixUniform, 1, GL_FALSE, viewMatrix);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
        glUniform1i(background_cubemapSamplerUniform, 0);
    
        glBindVertexArray(vao_cube);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    glUseProgram(0);

    //update 
    if(bRot)
    {
        light_translation -= 0.01f;
        if(light_translation <= -10.0f)
            bRot = false;
    }
    else 
    {
        light_translation += 0.01f;
        if(light_translation >= 10.0f)
            bRot = true;
    }

    SwapBuffers(ghdc);
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

    //release fbo
    if(fbo_capture)
    {
        glDeleteFramebuffers(1, &fbo_capture);
        fbo_capture = 0;
    }

    if(rbo_capture)
    {
        glDeleteRenderbuffers(1, &rbo_capture);
        rbo_capture = 0;
    }

    //release cube vao and vbo
    if(vao_cube)
    {
        glDeleteVertexArrays(1, &vao_cube);
        vao_cube = 0;
    }

    if(vbo_cube)
    {
        glDeleteBuffers(1, &vbo_cube);
        vbo_cube = 0;
    }

    //release quad vao and vbo
    if(vao_quad)
    {
        glDeleteVertexArrays(1, &vao_quad);
        vao_quad = 0;
    }

    if(vbo_quad)
    {
        glDeleteBuffers(1, &vbo_quad);
        vbo_quad = 0;
    }

    //release textures
    if(brdf_texture)
    {
        glDeleteTextures(1, &brdf_texture);
        brdf_texture = 0;      
    }

    if(prefilter_texture)
    {
        glDeleteTextures(1, &prefilter_texture);
        prefilter_texture = 0;      
    }

    if(irradiance_texture)
    {
        glDeleteTextures(1, &irradiance_texture);
        irradiance_texture = 0;
    }

    if(cubemap_texture)
    {
        glDeleteTextures(1, &cubemap_texture);
        cubemap_texture = 0;
    }

    if(hdr_texture)
    {
        glDeleteTextures(1, &hdr_texture);
        hdr_texture = 0;
    }

    if(copper_albedo_texture)
    {
        glDeleteTextures(1, &copper_albedo_texture);
        copper_albedo_texture = 0;
    }

    if(copper_normal_texture)
    {
        glDeleteTextures(1, &copper_normal_texture);
        copper_normal_texture = 0;
    }

    if(copper_metallic_texture)
    {
        glDeleteTextures(1, &copper_metallic_texture);
        copper_metallic_texture = 0;
    }

    if(copper_roughness_texture)
    {
        glDeleteTextures(1, &copper_roughness_texture);
        copper_roughness_texture = 0;
    }

    if(copper_ao_texture)
    {
        glDeleteTextures(1, &copper_ao_texture);
        copper_ao_texture = 0;
    }

    //release vao and vbo 
    if(vao_sphere)
    {
        glDeleteVertexArrays(1, &vao_sphere);
        vao_sphere = 0;
    }

    if(vbo_sphere_position)
    {
        glDeleteBuffers(1, &vbo_sphere_position);
        vbo_sphere_position = 0;
    }

    if(vbo_sphere_normal)
    {
        glDeleteBuffers(1, &vbo_sphere_normal);
        vbo_sphere_normal = 0;
    }

    if(vbo_sphere_texcoord)
    {
        glDeleteBuffers(1, &vbo_sphere_texcoord);
        vbo_sphere_texcoord = 0;
    }

    if(vbo_sphere_indices)
    {
        glDeleteBuffers(1, &vbo_sphere_indices);
        vbo_sphere_indices = 0;
    }


    //safe shader cleanup
    if(shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(shaderProgramObject);
        glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(shaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(shaderProgramObject);
        shaderProgramObject = 0;
        glUseProgram(0);
    }

    if(cubemap_shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(cubemap_shaderProgramObject);
        glGetProgramiv(cubemap_shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(cubemap_shaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(cubemap_shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(cubemap_shaderProgramObject);
        cubemap_shaderProgramObject = 0;
        glUseProgram(0);
    }

    if(background_shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(background_shaderProgramObject);
        glGetProgramiv(background_shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(background_shaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(background_shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(background_shaderProgramObject);
        background_shaderProgramObject = 0;
        glUseProgram(0);
    }

    if(irradiance_shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(irradiance_shaderProgramObject);
        glGetProgramiv(irradiance_shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(irradiance_shaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(irradiance_shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(irradiance_shaderProgramObject);
        irradiance_shaderProgramObject = 0;
        glUseProgram(0);
    }

    if(prefilter_shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(prefilter_shaderProgramObject);
        glGetProgramiv(prefilter_shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(prefilter_shaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(prefilter_shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(prefilter_shaderProgramObject);
        prefilter_shaderProgramObject = 0;
        glUseProgram(0);
    }

    if(brdf_shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(brdf_shaderProgramObject);
        glGetProgramiv(brdf_shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(brdf_shaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(brdf_shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(brdf_shaderProgramObject);
        brdf_shaderProgramObject = 0;
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
        fprintf(gpFile, "\n----- Program Completed Successfully -----\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}
