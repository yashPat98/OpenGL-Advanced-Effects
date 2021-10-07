//headers
#include <windows.h>               //standard windows header
#include <windowsx.h>              //for camera
#include <stdio.h>                 //C header 
#include <gl/glew.h>               //OpenGL extension wrangler (must be included before gl.h)
#include <gl/gl.h>                 //OpenGL header
#include <vector>                  //standard vector header
#include "RESOURCES.h"             //Resources header
#include "include/Camera.h"        //Camera header
#include "include/Model.h"         //Assimp model loader

//import libraries
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "assimp-vc142-mt.lib")

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

GLuint diffuseSamplerUniform;
GLuint normalSamplerUniform;
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
GLuint diffuse_texture;
GLuint normal_texture;

GLuint ml_vertexShaderObject;         
GLuint ml_fragmentShaderObject;       
GLuint ml_shaderProgramObject;        

GLuint ml_modelMatrixUniform;
GLuint ml_viewMatrixUniform;
GLuint ml_projectionMatrixUniform;

GLuint ml_viewPosUniform;
GLuint ml_lightPositionUniform;
GLuint ml_lightAmbientUniform;
GLuint ml_lightDiffuseUniform;
GLuint ml_lightSpecularUniform;

GLuint skybox_vertexShaderObject;
GLuint skybox_fragmentShaderObject;
GLuint skybox_shaderProgramObject;

GLuint skybox_mvpMatrixUniform;
GLuint skybox_skyBoxUniform;

GLuint vao_skybox;
GLuint vbo_skybox_position;

GLuint skybox_texture;

mat4 perspectiveProjectionMatrix;  
Camera camera(vec3(43.0f, 40.0f, 780.0f));
GLfloat initial_val_Zoom = 0.0f;

std::vector<GLfloat> vertices;
GLuint mesh_width;
GLuint mesh_height;

Model Alien;

//windows entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    //function declarations
    void Initialize(void);                                 //initialize OpenGL state machine
    void Display(void);                                    //render scene
    void ToggleFullscreen(void);

    //variable declarations
    WNDCLASSEX wndclass;                                   //structure holding window class attributes
    MSG msg;                                               //structure holding message attributes
    HWND hwnd;                                             //handle to a window
    TCHAR szAppName[] = TEXT("Terrain");                   //name of window class

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

    ShowWindow(hwnd, SW_MAXIMIZE);                 //set specified window's show state
    SetForegroundWindow(hwnd);                  //brings the thread that created the specified window to foreground
    SetFocus(hwnd);                             //set the keyboard focus to specified window 

    //ToggleFullscreen();

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

                case 'v':
                case 'V':
                    fprintf(gpFile, "camera_Pos : %f %f %f\n", camera.Position[0], camera.Position[1], camera.Position[2]);
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
    bool loadGLTexture(GLuint *texture, unsigned int *width, unsigned int *height, TCHAR ResourceID[]);
    void loadGLCubeMap(GLuint *texture);

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
    
        "uniform sampler2D diffuse_sampler;"                                                                                        \
        "uniform sampler2D normal_sampler;"                                                                                         \

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
        "   float a = 0.0003f;"                                                                                                     \
        "   float b = 0.0001f;"                                                                                                     \
        "   float fogAmount = (a/b) * exp(-rayOri.y * b) * (1.0f - exp(-distance * rayDir.y * b)) / rayDir.y;"                      \
        "   vec3 fogColor = vec3(0.75f, 0.9f, 0.7f);"                                                                                \
        "   return (mix(color, fogColor, fogAmount));"                                                                              \
        "}"                                                                                                                         \

        "void main(void)"                                                                                                           \
        "{"                                                                                                                         \
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

        "   vec3 tangent_normal = texture(normal_sampler, te_texcoord * tex_tiling_factor).xyz * 2.0 - 1.0;"                        \
        "   normal = normalize(TBN * tangent_normal);"                                                                              \

        "   vec3 normalized_view_vector = normalize(-world_pos);"                                                                   \
        "   vec3 normalized_light_direction = normalize(light_position - world_pos);"                                               \
        "   vec3 reflection_vector = reflect(-normalized_light_direction, normal);"                                                 \

        "   vec3 color = texture(diffuse_sampler, te_texcoord * tex_tiling_factor).rgb;"                                            \
    
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

    diffuseSamplerUniform = glGetUniformLocation(shaderProgramObject, "diffuse_sampler");
    normalSamplerUniform = glGetUniformLocation(shaderProgramObject, "normal_sampler");

    heightMapUniform = glGetUniformLocation(shaderProgramObject, "height_map");
    heightStepUniform = glGetUniformLocation(shaderProgramObject, "height_step");
    gridSpacingUniform = glGetUniformLocation(shaderProgramObject, "grid_spacing");
    
    scaleFactorUniform = glGetUniformLocation(shaderProgramObject, "scale_factor");
    textureTilingUniform = glGetUniformLocation(shaderProgramObject, "tex_tiling_factor");
    lightPositionUniform = glGetUniformLocation(shaderProgramObject, "light_position");
    viewPosUniform = glGetUniformLocation(shaderProgramObject, "view_pos");

    //Model Loading Shader
    fprintf(gpFile, "\n-> Normal Mapped Model Loading Shader\n");

    //vertex shader
    ml_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* ml_vertexShaderSourceCode = 
        "#version 450 core"                                                                 \
        "\n"                                                                                \

        "in vec3 vPosition;"                                                                \
        "in vec3 vNormal;"                                                                  \
        "in vec2 vTexCoord;"                                                                \
        "in vec3 vTangent;"                                                                 \

        "out vec3 out_fragPos;"                                                             \
        "out vec2 out_texCoord;"                                                            \
        "out vec3 out_tangentLightPos;"                                                     \
        "out vec3 out_tangentViewPos;"                                                      \
        "out vec3 out_tangentFragPos;"                                                      \

        "uniform mat4 u_modelMatrix;"                                                       \
        "uniform mat4 u_viewMatrix;"                                                        \
        "uniform mat4 u_projectionMatrix;"                                                  \
        "uniform vec3 u_lightPos;"                                                          \
        "uniform vec3 u_viewPos;"                                                           \

        "void main(void)"                                                                   \
        "{"                                                                                 \
        "   out_fragPos = vec3(u_modelMatrix * vec4(vPosition, 1.0f));"                     \
        "   out_texCoord = vTexCoord;"                                                      \

        "   mat3 normalMatrix = transpose(inverse(mat3(u_modelMatrix)));"                   \
        "   vec3 T = normalize(normalMatrix * vTangent);"                                   \
        "   vec3 N = normalize(normalMatrix * vNormal);"                                    \
        "   T = normalize(T - dot(T, N) * N);"                                              \
        "   vec3 B = cross(N, T);"                                                          \

        "   mat3 TBN = transpose(mat3(T, B, N));"                                           \
        "   out_tangentLightPos = TBN * u_lightPos;"                                        \
        "   out_tangentViewPos = TBN * u_viewPos;"                                          \
        "   out_tangentFragPos = TBN * out_fragPos;"                                        \

        "   gl_Position = u_projectionMatrix * u_viewMatrix * vec4(out_fragPos, 1.0f);"     \
        "}";

    //provide source code to shader object
    glShaderSource(ml_vertexShaderObject, 1, (const GLchar**)&ml_vertexShaderSourceCode, NULL);

    //compile shader 
    glCompileShader(ml_vertexShaderObject);

    //shader compilation error checking
    glGetShaderiv(ml_vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(ml_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(ml_vertexShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> vertex shader compilation log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    } 

    fprintf(gpFile, "-> vertex shader compiled successfully\n");

    //fragment shader
    ml_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* ml_fragmentShaderSourceCode = 
        "#version 450 core"                                                                                         \
        "\n"                                                                                                        \
        
        "layout(binding = 0)uniform sampler2D diffuse_texture;"                                                     \
        "layout(binding = 1)uniform sampler2D specular_texture;"                                                    \
        "layout(binding = 2)uniform sampler2D normal_texture;"                                                      \

        "uniform vec3 u_lightAmbient;"                                                                              \
        "uniform vec3 u_lightDiffuse;"                                                                              \
        "uniform vec3 u_lightSpecular;"                                                                             \

        "uniform vec3 u_matAmbient;"                                                                                \
        "uniform vec3 u_matDiffuse;"                                                                                \
        "uniform vec3 u_matSpecular;"                                                                               \
        "uniform float u_matShininess;"                                                                             \

        "in vec3 out_fragPos;"                                                                                      \
        "in vec2 out_texCoord;"                                                                                     \
        "in vec3 out_tangentLightPos;"                                                                              \
        "in vec3 out_tangentViewPos;"                                                                               \
        "in vec3 out_tangentFragPos;"                                                                               \

        "out vec4 FragColor;"                                                                                       \
        
        "void main(void)"                                                                                           \
        "{"                                                                                                         \
            //obtain normal from normal map in range [0, -1]
        "   vec3 normal = texture(normal_texture, out_texCoord).rgb;"                                               \

            //transform normal vector to range [-1, 1]
        "   normal = normalize(normal * 2.0f - 1.0f);"                                                              \

            //ambient
        "   vec3 ambient = u_lightAmbient * u_matAmbient * texture(diffuse_texture, out_texCoord).rgb;"             \
            
            //diffuse
        "   vec3 light_direction = normalize(out_tangentLightPos - out_tangentFragPos);"                            \
        "   float diff = max(dot(light_direction, normal), 0.0f);"                                                  \
        "   vec3 diffuse = u_lightDiffuse * u_matDiffuse * diff * texture(diffuse_texture, out_texCoord).rgb;"      \

            //specular
        "   vec3 view_direction = normalize(out_tangentViewPos - out_tangentFragPos);"                              \
        "   vec3 reflect_direction = reflect(-light_direction, normal);"                                            \
        "   vec3 halfway_direction = normalize(light_direction + view_direction);"                                  \
        "   float spec = pow(max(dot(normal, halfway_direction), 0.0f), u_matShininess);"                           \
        "   vec3 specular = u_lightSpecular * u_matSpecular * spec * texture(specular_texture, out_texCoord).rgb;"  \

        "   float alpha = texture(diffuse_texture, out_texCoord).a;"                                                \
        "   vec3 ads_light = ambient + diffuse + specular;"                                                         \
        "   FragColor = vec4(ads_light, alpha);"                                                                    \
        "}";        

    //provide source code to shader object 
    glShaderSource(ml_fragmentShaderObject, 1, (const GLchar**)&ml_fragmentShaderSourceCode, NULL);

    //compile shader
    glCompileShader(ml_fragmentShaderObject);

    //shader compilation error checking
    glGetShaderiv(ml_fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(ml_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(ml_fragmentShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> fragment shader compilation log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "-> fragment shader compiled successfully\n");

    //shader program 
    ml_shaderProgramObject = glCreateProgram();

    //attach vertex shader to shader program
    glAttachShader(ml_shaderProgramObject, ml_vertexShaderObject);

    //attach fragment shader to shader program
    glAttachShader(ml_shaderProgramObject, ml_fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(ml_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPositon");
    glBindAttribLocation(ml_shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");
    glBindAttribLocation(ml_shaderProgramObject, AMC_ATTRIBUTE_TEXCOORD, "vTexCoord");
    glBindAttribLocation(ml_shaderProgramObject, AMC_ATTRIBUTE_TANGENT, "vTangent");

    //link shader program 
    glLinkProgram(ml_shaderProgramObject);

    //shader linking error checking
    glGetProgramiv(ml_shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(ml_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(ml_shaderProgramObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> shader program link log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "-> shader program linked successfully\n");

    //get uniform locations
    ml_modelMatrixUniform = glGetUniformLocation(ml_shaderProgramObject, "u_modelMatrix");
    ml_viewMatrixUniform = glGetUniformLocation(ml_shaderProgramObject, "u_viewMatrix");
    ml_projectionMatrixUniform = glGetUniformLocation(ml_shaderProgramObject, "u_projectionMatrix");

    ml_viewPosUniform = glGetUniformLocation(ml_shaderProgramObject, "u_viewPos");
    ml_lightPositionUniform = glGetUniformLocation(ml_shaderProgramObject, "u_lightPos");
    ml_lightAmbientUniform = glGetUniformLocation(ml_shaderProgramObject, "u_lightAmbient");
    ml_lightDiffuseUniform = glGetUniformLocation(ml_shaderProgramObject, "u_lightDiffuse");
    ml_lightSpecularUniform = glGetUniformLocation(ml_shaderProgramObject, "u_lightSpecular");

    //skybox shader
    fprintf(gpFile, "\n-> Skybox Shader\n");

    //vertex Shader 
    skybox_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    const GLchar* skybox_vertexShaderSourceCode = 
        "#version 450 core"                                         \
        "\n"                                                        \
        "in vec3 vPosition;"                                        \
        "out vec3 out_texcoord;"                                    \
        "uniform mat4 u_mvpMatrix;"                                 \
        "void main(void)"                                           \
        "{"                                                         \
        "   out_texcoord = -vPosition;"                              \
        "   vec4 pos = u_mvpMatrix * vec4(vPosition, 1.0f);"        \
        "   gl_Position = pos.xyww;"                                \
        "}";

    glShaderSource(skybox_vertexShaderObject, 1, (const GLchar**)&skybox_vertexShaderSourceCode, NULL);
    glCompileShader(skybox_vertexShaderObject);

    //shader compilation error checking
    glGetShaderiv(skybox_vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(skybox_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(skybox_vertexShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> Vertex Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    } 
    fprintf(gpFile, "-> vertex shader compiled successfully\n");

    //fragment shader
    skybox_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* skybox_fragmentShaderSourceCode = 
        "#version 450 core"                                 \
        "\n"                                                \
        "in vec3 out_texcoord;"                             \
        "out vec4 FragColor;"                               \
        "uniform samplerCube skybox;"                       \
        "void main(void)"                                   \
        "{"                                                 \
        "   FragColor = texture(skybox, out_texcoord);"     \
        "}";

    //provide source code to shader object 
    glShaderSource(skybox_fragmentShaderObject, 1, (const GLchar**)&skybox_fragmentShaderSourceCode, NULL);

    //compile shader
    glCompileShader(skybox_fragmentShaderObject);

    //shader compilation error checking
    glGetShaderiv(skybox_fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(skybox_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(skybox_fragmentShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> fragment shader compilation log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }
    fprintf(gpFile, "-> fragment shader compiled successfully\n");

    //shader program 
    skybox_shaderProgramObject = glCreateProgram();

    //attach vertex shader to shader program
    glAttachShader(skybox_shaderProgramObject, skybox_vertexShaderObject);
    glAttachShader(skybox_shaderProgramObject, skybox_fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(skybox_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPositon");

    //link shader program 
    glLinkProgram(skybox_shaderProgramObject);

    //shader linking error checking
    glGetProgramiv(skybox_shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(skybox_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(skybox_shaderProgramObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> shader program link log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }
    fprintf(gpFile, "-> shader program linked successfully\n");

    //get uniform locations
    skybox_mvpMatrixUniform = glGetUniformLocation(skybox_shaderProgramObject, "u_mvpMatrix"); 
    skybox_skyBoxUniform = glGetUniformLocation(skybox_shaderProgramObject, "skybox"); 

    //load heightmap
    loadGLTexture(&height_texture,&mesh_width, &mesh_height, MAKEINTRESOURCE(HEIGHTMAP));

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

    //skybox data
    float skyboxVertices[] = {
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

    glGenVertexArrays(1, &vao_skybox);
    glBindVertexArray(vao_skybox);
        glGenBuffers(1, &vbo_skybox_position);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_skybox_position);
            glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
            glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
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
    loadGLTexture(&diffuse_texture, NULL, NULL, MAKEINTRESOURCE(DIFFUSE_BITMAP));
    loadGLTexture(&normal_texture, NULL, NULL, MAKEINTRESOURCE(NORMAL_BITMAP));
    loadGLCubeMap(&skybox_texture);

    //initialize global variables
    perspectiveProjectionMatrix = mat4::identity();

    //load model 
    Alien.LoadModel("models/alien.obj");

    Resize(WIN_WIDTH, WIN_HEIGHT);
}

bool loadGLTexture(GLuint *texture, unsigned int *width, unsigned int *height, TCHAR ResourceID[])
{
    //variable declarations
    bool bResult = false;
    HBITMAP hBitmap = NULL;
    BITMAP bmp;

    //code
    hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), ResourceID, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    if(hBitmap)
    {
        bResult = true;
        GetObject(hBitmap, sizeof(BITMAP), &bmp);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGenTextures(1, texture);
        glBindTexture(GL_TEXTURE_2D, *texture);

        //set up texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        //push the data to texture memory
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.bmWidth, bmp.bmHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, bmp.bmBits);
        glGenerateMipmap(GL_TEXTURE_2D);
    
        if(width != NULL)
            *width = bmp.bmWidth;

        if(height != NULL)
            *height = bmp.bmHeight;

        //free resource
        DeleteObject(hBitmap);
        hBitmap = NULL;
    }

    return (bResult);
}

void loadGLCubeMap(GLuint *texture)
{
    //variable declarations
    HBITMAP hBitmap[6];
    BITMAP bmp;

    //code
    hBitmap[0] = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(RIGHT_BITMAP), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    hBitmap[1] = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(LEFT_BITMAP), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    hBitmap[2] = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(BOTTOM_BITMAP), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    hBitmap[3] = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(TOP_BITMAP), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    hBitmap[4] = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(FRONT_BITMAP), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    hBitmap[5] = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(BACK_BITMAP), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, *texture);

    //set up texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for(int i = 0; i < 6; i++)
    {
        //get data 
        ZeroMemory(&bmp, sizeof(BITMAP));
        GetObject(hBitmap[i], sizeof(BITMAP), &bmp);

        //push the data to texture memory
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, bmp.bmWidth, bmp.bmHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, bmp.bmBits);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        //free resource
        DeleteObject(hBitmap[i]);
        hBitmap[i] = NULL;
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Resize(int width, int height)
{
    //code
    if(height == 0)
        height = 1;

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    perspectiveProjectionMatrix = vmath::perspective(45.0f, (float)width / (float)height, 0.1f, 2000.0f);
}

void Display(void)
{
    //variable declarations
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 mvpMatrix;

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //set matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    viewMatrix = vmath::lookat(camera.Position, vec3(560.0f, 25.0f, 440.0f), vec3(0.0f, 1.0f, 0.0f));

    glDepthMask(GL_FALSE);
    glUseProgram(skybox_shaderProgramObject);
        modelMatrix = vmath::translate(camera.Position);
        mvpMatrix = perspectiveProjectionMatrix * viewMatrix * modelMatrix;
        glUniformMatrix4fv(skybox_mvpMatrixUniform, 1, GL_FALSE, mvpMatrix);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
        glUniform1i(skybox_skyBoxUniform, 0);

        glBindVertexArray(vao_skybox);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    glUseProgram(0);
    glDepthMask(GL_TRUE);

    glUseProgram(shaderProgramObject);
        modelMatrix = vmath::translate(0.0f, 0.0f, 0.0f);

        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

        glUniform1f(heightStepUniform, 70.0f);
        glUniform1f(gridSpacingUniform, 0.5f);
        glUniform1f(textureTilingUniform, 10.0f);
        glUniform1i(scaleFactorUniform, 1); 

        glUniform3f(lightPositionUniform, 100.0f, 100.0f, 100.0f);
        glUniform3fv(viewPosUniform, 1, camera.Position);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, height_texture);
        glUniform1i(heightMapUniform, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, diffuse_texture);
        glUniform1i(diffuseSamplerUniform, 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, normal_texture);
        glUniform1i(normalSamplerUniform, 2);

        glBindVertexArray(vao_terrain);
        glPatchParameteri(GL_PATCH_VERTICES, 1);
        glDrawArrays(GL_PATCHES, 0, vertices.size() / 2);
        glBindVertexArray(0);
    glUseProgram(0);

    glUseProgram(ml_shaderProgramObject);
        //set matrices to identity
        modelMatrix = mat4::identity();

        //transform model matrix
        modelMatrix = vmath::translate(588.0f, 9.35f, 415.0f);
        modelMatrix = modelMatrix * vmath::scale(10.0f, 10.0f, 10.0f);

        //pass data to shader uniform variables
        glUniformMatrix4fv(ml_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        glUniformMatrix4fv(ml_viewMatrixUniform, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(ml_projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

        glUniform3fv(ml_viewPosUniform, 1, camera.Position);

        glUniform3f(ml_lightPositionUniform, 100.0f, 100.0f, 100.0f);
        glUniform3f(ml_lightAmbientUniform, 0.2f, 0.2f, 0.2f);
        glUniform3f(ml_lightDiffuseUniform, 1.0f, 1.0f, 1.0f);
        glUniform3f(ml_lightSpecularUniform, 1.0f, 1.0f, 1.0f);

        //draw model
        Alien.Draw(ml_shaderProgramObject);

        modelMatrix = vmath::translate(500.0f, 9.35f, 400.0f);
        modelMatrix = modelMatrix * vmath::scale(10.0f, 10.0f, 10.0f);
        modelMatrix = modelMatrix * vmath::rotate(90.0f, 0.0f, 1.0f, 0.0f);
        glUniformMatrix4fv(ml_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        Alien.Draw(ml_shaderProgramObject);

        modelMatrix = vmath::translate(550.0f, 9.35f, 380.0f);
        modelMatrix = modelMatrix * vmath::scale(10.0f, 10.0f, 10.0f);
        modelMatrix = modelMatrix * vmath::rotate(20.0f, 0.0f, 1.0f, 0.0f);
        glUniformMatrix4fv(ml_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        Alien.Draw(ml_shaderProgramObject);

        modelMatrix = vmath::translate(580.0f, 9.35f, 450.0f);
        modelMatrix = modelMatrix * vmath::scale(10.0f, 10.0f, 10.0f);
        modelMatrix = modelMatrix * vmath::rotate(60.0f, 0.0f, 1.0f, 0.0f);
        glUniformMatrix4fv(ml_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        Alien.Draw(ml_shaderProgramObject);

        modelMatrix = vmath::translate(650.0f, 9.35f, 340.0f);
        modelMatrix = modelMatrix * vmath::scale(10.0f, 10.0f, 10.0f);
        modelMatrix = modelMatrix * vmath::rotate(70.0f, 0.0f, 1.0f, 0.0f);
        glUniformMatrix4fv(ml_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        Alien.Draw(ml_shaderProgramObject);

        modelMatrix = vmath::translate(472.1f, 10.7f, 459.8f);
        modelMatrix = modelMatrix * vmath::scale(10.0f, 10.0f, 10.0f);
        modelMatrix = modelMatrix * vmath::rotate(90.0f, 0.0f, 1.0f, 0.0f);
        glUniformMatrix4fv(ml_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        Alien.Draw(ml_shaderProgramObject);

        modelMatrix = vmath::translate(511.5f, 8.3f, 495.8f);
        modelMatrix = modelMatrix * vmath::scale(10.0f, 10.0f, 10.0f);
        modelMatrix = modelMatrix * vmath::rotate(120.0f, 0.0f, 1.0f, 0.0f);
        glUniformMatrix4fv(ml_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        Alien.Draw(ml_shaderProgramObject);

        modelMatrix = vmath::translate(597.7f, 7.8f, 355.0f);
        modelMatrix = modelMatrix * vmath::scale(10.0f, 10.0f, 10.0f);
        modelMatrix = modelMatrix * vmath::rotate(150.0f, 0.0f, 1.0f, 0.0f);
        glUniformMatrix4fv(ml_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        Alien.Draw(ml_shaderProgramObject);
    glUseProgram(0);

    //update
    if(camera.Position[0] < 600.0f)
        camera.Position[0] += 0.3f;

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

    if(diffuse_texture)
    {
        glDeleteTextures(1, &diffuse_texture);
        diffuse_texture = 0;
    }

    if(normal_texture)
    {
        glDeleteTextures(1, &normal_texture);
        normal_texture = 0;
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

    if(vao_skybox)
    {
        glDeleteVertexArrays(1, &vao_skybox);
        vao_skybox = 0;
    }

    if(vbo_skybox_position)
    {
        glDeleteBuffers(1, &vbo_skybox_position);
        vbo_skybox_position = 0;
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

    if(ml_shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(ml_shaderProgramObject);
        glGetProgramiv(ml_shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(ml_shaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(ml_shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(ml_shaderProgramObject);
        ml_shaderProgramObject = 0;
        glUseProgram(0);
    }

    if(skybox_shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(skybox_shaderProgramObject);
        glGetProgramiv(skybox_shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(skybox_shaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(skybox_shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(skybox_shaderProgramObject);
        skybox_shaderProgramObject = 0;
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
