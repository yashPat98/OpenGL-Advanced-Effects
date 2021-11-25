//headers
#include <windows.h>               //standard windows header
#include <stdio.h>                 //C header 
#include <gl/glew.h>               //OpenGL extension wrangler (must be included before gl.h)
#include <gl/gl.h>                 //OpenGL header
#include <vector>                  //standard vector header
#include "RESOURCES.h"             //Resources header
#include "vmath.h"                 //Maths header

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"             //Texture loader header

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

#define PATCH_SIZE  64

//namespaces
using namespace vmath;

//type declarations
enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOORD,
    AMC_ATTRIBUTE_TANGENT
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
GLuint tessellationControlShaderObject;
GLuint tessellationEvaluationShaderObject;     
GLuint shaderProgramObject;                 

GLuint modelMatrixUniform;
GLuint viewMatrixUniform;
GLuint projectionMatrixUniform;
GLuint splatMapUniform;
GLuint grassSamplerUniform;
GLuint grassNormalUniform;
GLuint rockSamplerUniform;
GLuint rockNormalUniform;
GLuint heightMapUniform;
GLuint heightStepUniform;
GLuint gridSpacingUniform;
GLuint scaleFactorUniform;
GLuint textureTilingUniform;
GLuint lightPositionUniform;
GLuint viewPosUniform;

GLuint vao_terrain;
GLuint vbo_terrain_position;

GLuint height_texture;
GLuint splat_texture;
GLuint grass_texture;
GLuint grass_normal_texture;
GLuint rock_texture;
GLuint rock_normal_texture;
GLuint displacement_texture;

mat4 perspectiveProjectionMatrix;  

std::vector<GLfloat> vertices;
GLuint mesh_width;
GLuint mesh_height;

vec3 camera_pos;
vec3 camera_front;
vec3 camera_up;

float camera_speed = 1.0f;
float sensitivity = 0.1f;
bool first_mouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float last_x = 800.0f / 2.0f;
float last_y = 600.0f / 2.0f;
float fov = 45.0f;

//windows entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    //function declarations
    void Initialize(void);                                 //initialize OpenGL state machine
    void Display(void);                                    //render scene

    //variable declarations
    WNDCLASSEX wndclass;                                   //structure holding window class attributes
    MSG msg;                                               //structure holding message attributes
    HWND hwnd;                                             //handle to a window
    TCHAR szAppName[] = TEXT("Tessellated Terrain");       //name of window class

    int cxScreen, cyScreen;                                //screen width and height for centering window
    int init_x, init_y;                                    //top-left coordinates of centered window
    bool bDone = false;                                    //flag indicating whether or not to exit from game loop

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
    void load_texture(const char *filename, GLuint *tex_width, GLuint *tex_height, GLuint *texture);

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

    //tessellated terrain shader
    fprintf(gpFile, "\n-> Tessellated Terrain Shader\n");

    //vertex Shader 
    vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    const GLchar* vertexShaderSourceCode = 
        "#version 450 core"                                         \
        "\n"                                                        \

        "in vec2 a_position;"                                       \
        "out vec2 v_position;"                                      \

        "void main(void)"                                           \
        "{"                                                         \
        "   v_position = a_position;"                               \
        "}";

    glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);
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
                fprintf(gpFile, "-> Vertex Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    } 
    fprintf(gpFile, "-> vertex shader compiled successfully\n");

    //tessellation control shader
    tessellationControlShaderObject = glCreateShader(GL_TESS_CONTROL_SHADER);

    const GLchar* tessellationControlShaderSourceCode = 
        "#version 450 core"                                                 \
        "\n"                                                                \

        "layout (vertices = 1) out;"                                        \

        "in vec2 v_position[];"                                             \
        "out vec2 tc_position[];"                                           \

        "void main(void)"                                                   \
        "{"                                                                 \
        "   tc_position[gl_InvocationID] = v_position[gl_InvocationID];"    \

        "   gl_TessLevelOuter[0] = 64.0f;"                                  \
        "   gl_TessLevelOuter[1] = 64.0f;"                                  \
        "   gl_TessLevelOuter[2] = 64.0f;"                                  \
        "   gl_TessLevelOuter[3] = 64.0f;"                                  \
        
        "   gl_TessLevelInner[0] = 64.0f;"                                  \
        "   gl_TessLevelInner[1] = 64.0f;"                                  \
        "}";

    glShaderSource(tessellationControlShaderObject, 1, (const GLchar**)&tessellationControlShaderSourceCode, NULL);
    glCompileShader(tessellationControlShaderObject);

    //shader compilation error checking
    glGetShaderiv(tessellationControlShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(tessellationControlShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(tessellationControlShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> Tessellation Control Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }
    fprintf(gpFile, "-> tessellation control shader compiled successfully\n");

    //tessellation evaluation shader
    tessellationEvaluationShaderObject = glCreateShader(GL_TESS_EVALUATION_SHADER);

    const GLchar* tessellationEvaluationShaderSourceCode = 
        "#version 450 core"                                                 \
        "\n"                                                                \
        
        "layout (quads, fractional_even_spacing, cw) in;"                   \

        "in vec2 tc_position[];"                                            \
        "out vec2 te_texcoord;"                                             \
        "out vec3 world_pos;"                                               \
        "out vec3 view_direction;"                                          \

        "uniform mat4 model_matrix;"                                        \
        "uniform mat4 view_matrix;"                                         \
        "uniform mat4 projection_matrix;"                                   \

        "uniform sampler2D height_map;"                                     \
        "uniform float height_step;"                                        \
        "uniform float grid_spacing;"                                       \
        "uniform int scale_factor;"                                         \
        "uniform vec3 view_pos;"                                            \

        "void main(void)"                                                   \
        "{"                                                                 \
        "   ivec2 t_size = textureSize(height_map, 0) * scale_factor;"      \
        "   vec2 div = t_size / 64.0;"                                      \

        "   te_texcoord = tc_position[0].xy + gl_TessCoord.st / div;"       \

        "   vec4 position;"                                                 \
        "   position.xz = te_texcoord.st * t_size * grid_spacing;"          \
        "   position.y = texture(height_map, te_texcoord).r * height_step;" \
        "   position.w = 1.0;"                                              \

        "   if(te_texcoord.s == 0.0 || te_texcoord.t == 0.0 ||"             \
        "      te_texcoord.s == 1.0 || te_texcoord.t == 1.0)"               \
        "       position.y = -1.0f;"                                        \

        "   view_direction = position.xyz - view_pos;"                      \

        "   world_pos = vec3(view_matrix * model_matrix * position);"       \
        "   mat4 mvp = projection_matrix * view_matrix * model_matrix;"     \
        "   gl_Position = mvp * position;"                                  \
        "}";

    glShaderSource(tessellationEvaluationShaderObject, 1, (const GLchar**)&tessellationEvaluationShaderSourceCode, NULL);
    glCompileShader(tessellationEvaluationShaderObject);

    //shader compilation error checking
    glGetShaderiv(tessellationEvaluationShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(tessellationEvaluationShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(tessellationEvaluationShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> Tessellation Evaluation Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }
    fprintf(gpFile, "-> tessellation evaluation shader compiled successfully\n");

    //fragment shader
    fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    const GLchar* fragmentShaderSourceCode = 
        "#version 450 core"                                                                                                         \
        "\n"                                                                                                                        \
        
        "in vec2 te_texcoord;"                                                                                                      \
        "in vec3 world_pos;"                                                                                                        \
        "in vec3 light_direction;"                                                                                                  \
        "in vec3 view_direction;"                                                                                                   \

        "out vec4 frag_color;"                                                                                                      \

        "uniform mat4 model_matrix;"                                                                                                \
        "uniform sampler2D splat_map;"                                                                                              \
        "uniform sampler2D grass_sampler;"                                                                                          \
        "uniform sampler2D grass_normal;"                                                                                           \
        "uniform sampler2D rock_sampler;"                                                                                           \
        "uniform sampler2D rock_normal;"                                                                                            \

        "uniform sampler2D height_map;"                                                                                             \
        "uniform float height_step;"                                                                                                \
        "uniform float grid_spacing;"                                                                                               \
        "uniform int scale_factor;"                                                                                                 \
        "uniform float tex_tiling_factor;"                                                                                          \
        
        "uniform vec3 light_position;"                                                                                              \
        "uniform vec3 view_pos;"                                                                                                    \

        "mat3 getTBN(vec3 N)"                                                                                                       \
        "{"                                                                                                                         \
        "   vec3 Q1 = dFdx(world_pos);"                                                                                             \
        "   vec3 Q2 = dFdy(world_pos);"                                                                                             \
        "   vec2 st1 = dFdx(te_texcoord);"                                                                                          \
        "   vec2 st2 = dFdy(te_texcoord);"                                                                                          \

        "   vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);"                                                                           \
        "   vec3 B = -normalize(cross(N, T));"                                                                                      \
        
        "   return (mat3(T, B, N));"                                                                                                \
        "}"                                                                                                                         \

        "float get_height(float s, float t)"                                                                                        \
        "{"                                                                                                                         \
        "   return (texture(height_map, vec2(s, t)).r * height_step);"                                                              \
        "}"                                                                                                                         \

        "vec3 apply_fog(vec3 color, float distance, vec3 rayOri, vec3 rayDir)"                                                      \
        "{"                                                                                                                         \
        "   float a = 0.0007f;"                                                                                                     \
        "   float b = 0.0002f;"                                                                                                     \
        "   float fogAmount = (a/b) * exp(-rayOri.y * b) * (1.0f - exp(-distance * rayDir.y * b)) / rayDir.y;"                      \
        "   vec3 fogColor = vec3(0.5f, 0.6f, 0.7f);"                                                                                \
        "   return (mix(color, fogColor, fogAmount));"                                                                              \
        "}"                                                                                                                         \

        "void main(void)"                                                                                                           \
        "{"                                                                                                                         \
        "   vec4 splat_color = texture(splat_map, te_texcoord);"                                                                    \
        "   float delta = 1.0 / (textureSize(height_map, 0).x * scale_factor);"                                                     \

        "   vec3 x_delta;"                                                                                                          \
        "   x_delta[0] = tex_tiling_factor * grid_spacing;"                                                                         \
        "   x_delta[1] = get_height(te_texcoord.s + delta, te_texcoord.t) - get_height(te_texcoord.s - delta, te_texcoord.t);"      \
        "   x_delta[2] = 0.0;"                                                                                                      \
        
        "   vec3 z_delta;"                                                                                                          \
        "   z_delta[0] = 0.0;"                                                                                                      \
        "   z_delta[1] = get_height(te_texcoord.s, te_texcoord.t + delta) - get_height(te_texcoord.s, te_texcoord.t - delta);"      \
        "   z_delta[2] = tex_tiling_factor * grid_spacing;"                                                                         \
        
        "   mat3 normal_matrix = mat3(transpose(inverse(model_matrix)));"                                                           \
        "   vec3 normal = normalize(normal_matrix * cross(z_delta, x_delta));"                                                      \
        "   mat3 TBN = getTBN(normal);"                                                                                             \

        "   vec3 grass_normal = texture(grass_normal, te_texcoord * tex_tiling_factor).xyz * 2.0 - 1.0;"                            \
        "   vec3 rock_normal = texture(rock_normal, te_texcoord * tex_tiling_factor).xyz * 2.0 - 1.0;"                              \
        "   vec3 tangent_normal = grass_normal * splat_color.g + rock_normal * splat_color.r;"                                      \
        "   normal = normalize(TBN * tangent_normal);"                                                                              \

        "   vec3 normalized_view_vector = normalize(-world_pos);"                                                                   \
        "   vec3 normalized_light_direction = normalize(light_position - world_pos);"                                               \
        "   vec3 reflection_vector = reflect(-normalized_light_direction, normal);"                                                 \

        "   vec3 grass_color = texture(grass_sampler, te_texcoord * tex_tiling_factor).rgb * splat_color.g;"                        \
        "   vec3 rock_color = texture(rock_sampler, te_texcoord * tex_tiling_factor).rgb * splat_color.r;"                          \
        "   vec3 color = grass_color + rock_color;"                                                                                 \

        "   vec3 light_ambient = vec3(0.1f, 0.1f, 0.1f);"                                                                           \
        "   vec3 light_diffuse = vec3(1.0f, 1.0f, 1.0f);"                                                                           \

        "   vec3 ambient = light_ambient * color;"                                                                                  \
        "   vec3 diffuse = light_diffuse * color * max(dot(normalized_light_direction, normal), 0.0f);"                             \

        "   vec3 fog_color = apply_fog(ambient + diffuse, length(view_direction), view_pos, view_direction);"                       \

            //gamma correction
        "   float gamma = 2.0f;"                                                                                                    \
        "   frag_color = vec4(pow(fog_color, vec3(1/gamma)), 1.0f);"                                                                \
        "}";

    glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);
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
                fprintf(gpFile, "-> Fragment Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }
    fprintf(gpFile, "-> fragment shader compiled successfully\n");

    //shader program
    shaderProgramObject = glCreateProgram();

    //attach shaders to program object
    glAttachShader(shaderProgramObject, vertexShaderObject);
    glAttachShader(shaderProgramObject, tessellationControlShaderObject);
    glAttachShader(shaderProgramObject, tessellationEvaluationShaderObject);
    glAttachShader(shaderProgramObject, fragmentShaderObject);

    //bind attributes to vertex shader
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "a_position");

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
    fprintf(gpFile, "-> shader program linked successfully\n");

    //get uniform locations
    modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "model_matrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "view_matrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "projection_matrix");

    splatMapUniform = glGetUniformLocation(shaderProgramObject, "splat_map");
    grassSamplerUniform = glGetUniformLocation(shaderProgramObject, "grass_sampler");
    grassNormalUniform = glGetUniformLocation(shaderProgramObject, "grass_normal");
    rockSamplerUniform = glGetUniformLocation(shaderProgramObject, "rock_sampler");
    rockNormalUniform = glGetUniformLocation(shaderProgramObject, "rock_normal");

    heightMapUniform = glGetUniformLocation(shaderProgramObject, "height_map");
    heightStepUniform = glGetUniformLocation(shaderProgramObject, "height_step");
    gridSpacingUniform = glGetUniformLocation(shaderProgramObject, "grid_spacing");
    
    scaleFactorUniform = glGetUniformLocation(shaderProgramObject, "scale_factor");
    textureTilingUniform = glGetUniformLocation(shaderProgramObject, "tex_tiling_factor");
    lightPositionUniform = glGetUniformLocation(shaderProgramObject, "light_position");
    viewPosUniform = glGetUniformLocation(shaderProgramObject, "view_pos");

    //load heightmap
    load_texture("textures/height.png", &mesh_width, &mesh_height, &height_texture);

    //setup vertex data 
    for(int i = 0; i < (mesh_width / PATCH_SIZE); i++)
    {
        for(int j = 0; j < (mesh_height / PATCH_SIZE); j++)
        {
            GLfloat x = j * PATCH_SIZE / (GLfloat)mesh_height;
            GLfloat z = i * PATCH_SIZE / (GLfloat)mesh_width;
            vertices.push_back(x);
            vertices.push_back(z);
        }
    }

    //setup vao and vbo
    glGenVertexArrays(1, &vao_terrain);
    glBindVertexArray(vao_terrain);
        glGenBuffers(1, &vbo_terrain_position);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_terrain_position);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //set OpenGL states
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  
    glClearDepth(1.0f);                                     
    glEnable(GL_DEPTH_TEST);                                
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);

    //load textures 
    load_texture("textures/splat.png", NULL, NULL, &splat_texture);
    load_texture("textures/grass.png", NULL, NULL, &grass_texture);
    load_texture("textures/grass_normal.png", NULL, NULL, &grass_normal_texture);
    load_texture("textures/rock.png", NULL, NULL, &rock_texture);
    load_texture("textures/rock_normal.png", NULL, NULL, &rock_normal_texture);

    //initialize global variables
    perspectiveProjectionMatrix = mat4::identity();

    //initialize camera properties
    camera_pos = vec3(13.0f, 3.0f, 30.0f);
    camera_front = vec3(0.0f, 0.0f, 0.0f);
    camera_up = vec3(0.0f, 1.0f, 0.0f);

    Resize(WIN_WIDTH, WIN_HEIGHT);
}

void load_texture(const char *filename, GLuint *tex_width, GLuint *tex_height, GLuint *texture)
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

    if(tex_width)
    {
        *tex_width = width;
    }

    if(tex_height)
    {
        *tex_height = height;
    }

    stbi_image_free(pixel_data);
    pixel_data = NULL;
}

void Resize(int width, int height)
{
    //code
    if(height == 0)
        height = 1;

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    perspectiveProjectionMatrix = vmath::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
}

void Display(void)
{
    //variable declarations
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 scaleMatrix;

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //set matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();

    glUseProgram(shaderProgramObject);
        modelMatrix = vmath::translate(0.0f, 0.0f, 0.0f);
        viewMatrix = vmath::lookat(camera_pos, camera_pos + camera_front, camera_up);

        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

        glUniform1f(heightStepUniform, 40.0f);
        glUniform1f(gridSpacingUniform, 0.4f);
        glUniform1f(textureTilingUniform, 6.0f);
        glUniform1i(scaleFactorUniform, 1); 

        glUniform3f(lightPositionUniform, 100.0f, 100.0f, 100.0f);
        glUniform3fv(viewPosUniform, 1, camera_pos);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, height_texture);
        glUniform1i(heightMapUniform, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, splat_texture);
        glUniform1i(splatMapUniform, 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, grass_texture);
        glUniform1i(grassSamplerUniform, 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, grass_normal_texture);
        glUniform1i(grassNormalUniform, 3);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, rock_texture);
        glUniform1i(rockSamplerUniform, 4);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, rock_normal_texture);
        glUniform1i(rockNormalUniform, 5);

        glBindVertexArray(vao_terrain);
        glPatchParameteri(GL_PATCH_VERTICES, 1);
        glDrawArrays(GL_PATCHES, 0, vertices.size() / 2);
        glBindVertexArray(0);
    glUseProgram(0);

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

    //delete vectors
    vertices.clear();

    //delete textures
    if(height_texture)
    {
        glDeleteTextures(1, &height_texture);
        height_texture = 0;
    }

    if(splat_texture)
    {
        glDeleteTextures(1, &splat_texture);
        splat_texture = 0;
    }

    if(grass_texture)
    {
        glDeleteTextures(1, &grass_texture);
        grass_texture = 0;
    }
    
    if(grass_normal_texture)
    {
        glDeleteTextures(1, &grass_normal_texture);
        grass_normal_texture = 0;
    }

    if(rock_texture)
    {
        glDeleteTextures(1, &rock_texture);
        rock_texture = 0;
    }

    if(rock_normal_texture)
    {
        glDeleteTextures(1, &rock_normal_texture);
        rock_normal_texture = 0;
    }

    if(displacement_texture)
    {
        glDeleteTextures(1, &displacement_texture);
        displacement_texture = 0;
    }

    //release vao and vbo
    if(vao_terrain)
    {
        glDeleteVertexArrays(1, &vao_terrain);
        vao_terrain = 0;
    }

    if(vbo_terrain_position)
    {
        glDeleteBuffers(1, &vbo_terrain_position);
        vbo_terrain_position = 0;
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
