//headers
#include <windows.h>               //standard windows header
#include <stdio.h>                 //C header 
#include <vector>                  //standard vector library
#include <gl/glew.h>               //OpenGL extension wrangler (must be included before gl.h)
#include <gl/gl.h>                 //OpenGL header
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

GLuint occluded_vertexShaderObject;
GLuint occluded_fragmentShaderObject;
GLuint occluded_shaderProgramObject;

GLuint mvpMatrixUniform;
GLuint colorUniform;                  

GLuint godRays_vertexShaderObject;
GLuint godRays_fragmentShaderObject;
GLuint godRays_shaderProgramObject;

GLuint sceneSamplerUniform;
GLuint occludedSamplerUniform;
GLuint lightPositionOnScreenUniform;
GLuint godRays_mvpMatrixUniform;

GLuint vertexShaderObject;         
GLuint fragmentShaderObject;       
GLuint shaderProgramObject; 

GLuint modelMatrixUniform;
GLuint viewMatrixUniform;
GLuint perspectiveProjectionMatrixUniform;
GLuint LaUniform;
GLuint LdUniform;
GLuint LsUniform;
GLuint lightPositionUniform;
GLuint KaUniform;
GLuint KdUniform;
GLuint KsUniform;
GLuint materialShininessUniform;

GLuint fbo_occluded;
GLuint rbo_occluded;
GLuint occluded_texture;

GLuint fbo_scene;
GLuint rbo_scene;
GLuint scene_texture;

GLuint vao_quad;
GLuint vbo_quad_vertices;
GLuint vbo_quad_texcoords;

GLuint vao_sphere;
GLuint vbo_sphere_vertices;
GLuint vbo_sphere_indices;   

GLuint vao_cube;
GLuint vbo_cube_vertices;
GLuint vbo_cube_normals;

GLuint vao_pyramid;
GLuint vbo_pyramid_vertices;
GLuint vbo_pyramid_normals;

mat4 perspectiveProjectionMatrix;  
mat4 orthographicProjectionMatrix;

vec4 La;
vec4 Ld;
vec4 Ls;
vec4 lightPosition;

vec4 Ka;
vec4 Kd;
vec4 Ks;
float materialShininess;

vector<GLfloat> vertices;
vector<GLuint> indices;

int scr_width;
int scr_height;

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
    TCHAR szAppName[] = TEXT("OpenGL : God Rays");         //name of window class

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
                    lightPosition[1] += 0.1f;
                    break;
                
                case VK_DOWN:
                    lightPosition[1] -= 0.1f;
                    break;

                case VK_LEFT:
                    lightPosition[0] -= 0.1f;
                    break;

                case VK_RIGHT:
                    lightPosition[0] += 0.1f;
                    break; 

                case VK_ADD:
                    lightPosition[2] += 0.1f;
                    break;

                case VK_SUBTRACT:
                    lightPosition[2] -= 0.1f;
                    break;

                default:
                    break;
            }
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
    void Resize(int, int);                                                 //warm-up call
    void UnInitialize(void);                                               //release resources
    void GenerateSphere(float radius, int sectorCount, int stackCount);

    //variable declarations
    PIXELFORMATDESCRIPTOR pfd;                                             //structure describing the pixel format
    int iPixelFormatIndex;                                                 //index of the pixel format structure in HDC

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

    //Occluded Shader
    //--- Vertex Shader ---
    fprintf(gpFile, "\n***** Pass Through Shader *****\n");

    //create shader
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
    GLint infoLogLength = 0;
    GLint shaderCompiledStatus = 0;
    GLchar* szInfoLog = NULL;

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
                fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    } 

    fprintf(gpFile, "----- Vertex Shader Compiled Successfully -----\n");

    //--- Fragment Shader ---

    //create shader
    occluded_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* occluded_fragmentShaderSource = 
        "#version 450 core"                             \
        "\n"                                            \
        "out vec4 FragColor;"                           \
        "uniform vec3 u_color;"
        "void main(void)"                               \
        "{"                                             \
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
                fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "----- Fragment Shader Compiled Successfully -----\n");

    //--- Shader Program ---

    //create shader program
    occluded_shaderProgramObject = glCreateProgram();

    //attach vertex shader to shader program
    glAttachShader(occluded_shaderProgramObject, occluded_vertexShaderObject);

    //attach fragment shader to shader program
    glAttachShader(occluded_shaderProgramObject, occluded_fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(occluded_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPositon");

    //link shader program 
    glLinkProgram(occluded_shaderProgramObject);

    //shader linking error checking
    GLint shaderProgramLinkStatus = 0;
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
                fprintf(gpFile, "Shader Program Link Log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "----- Shader Program Linked Successfully -----\n");

    //get uniform locations
    mvpMatrixUniform = glGetUniformLocation(occluded_shaderProgramObject, "u_mvpMatrix"); 
    colorUniform = glGetUniformLocation(occluded_shaderProgramObject, "u_color"); 

    //Per-Fragment Lighting
    //--- Vertex Shader ---
    fprintf(gpFile, "\n***** Per-Fragment Lighting *****\n");

    //create shader
    vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* vertexShaderSourceCode = 
        "#version 450 core"                                                                                     \
        "\n"                                                                                                    \
        
        "in vec4 vPosition;"                                                                                    \
        "in vec3 vNormal;"                                                                                      \
        
        "uniform mat4 u_modelMatrix;"                                                                           \
        "uniform mat4 u_viewMatrix;"                                                                            \
        "uniform mat4 u_perspectiveProjectionMatrix;"                                                           \
        "uniform vec4 u_lightPosition;"
        
        "out vec3 transformed_normal;"                                                                          \
        "out vec3 light_direction;"                                                                             \
        "out vec3 view_vector;"                                                                                 \
        
        "void main(void)"                                                                                       \
        "{"                                                                                                     \
        "   vec4 eye_coords = u_viewMatrix * u_modelMatrix * vPosition;"                                        \
        "   mat3 normal_matrix = mat3(transpose(inverse(u_viewMatrix * u_modelMatrix))); "                      \
        "   transformed_normal = normal_matrix * vNormal;"                                                      \
        "   light_direction = vec3(u_lightPosition - eye_coords);"                                              \
        "   view_vector = -eye_coords.xyz;"                                                                     \
        "   gl_Position = u_perspectiveProjectionMatrix * u_viewMatrix * u_modelMatrix * vPosition;"            \
        "}"; 

    //provide source code to shader object
    glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);

    //compile shader 
    glCompileShader(vertexShaderObject);

    //shader compilation error checking
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

    fprintf(gpFile, "----- Vertex Shader Compiled Successfully -----\n");

    //--- Fragment Shader ---

    //create shader
    fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* fragmentShaderSourceCode = 
        "#version 450 core"                                                                                                             \
        "\n"                                                                                                                            \
        
        "in vec3 transformed_normal;"                                                                                                   \
        "in vec3 light_direction;"                                                                                                      \
        "in vec3 view_vector;"                                                                                                          \
        
        "uniform vec3 u_La;"                                                                                                            \
        "uniform vec3 u_Ld;"                                                                                                            \
        "uniform vec3 u_Ls;"                                                                                                            \
        "uniform vec3 u_Ka;"                                                                                                            \
        "uniform vec3 u_Kd;"                                                                                                            \
        "uniform vec3 u_Ks;"                                                                                                            \
        "uniform float u_materialShininess;"                                                                                            \
        
        "out vec4 fragColor;"                                                                                                           \
        
        "void main(void)"                                                                                                               \
        "{"                                                                                                                             \
        "   vec3 phong_ads_light;"                                                                                                      \

        "   vec3 normalized_transformed_normal = normalize(transformed_normal);"                                                        \
        "   vec3 normalized_view_vector = normalize(view_vector);"                                                                      \
        "   vec3 normalized_light_direction = normalize(light_direction);"                                                              \
        "   vec3 reflection_vector = reflect(-normalized_light_direction, normalized_transformed_normal);"                              \
        "   vec3 ambient = u_La * u_Ka;"                                                                                                \
        "   vec3 diffuse = u_Ld * u_Kd  * max(dot(normalized_light_direction, normalized_transformed_normal), 0.0f);"                   \
        "   vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, normalized_view_vector), 0.0f), u_materialShininess);"         \
        "   phong_ads_light = ambient + diffuse + specular;"                                                                            \
        
        "   fragColor = vec4(phong_ads_light, 1.0f);"                                                                                   \
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
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPositon");

    //binding of shader program object with vertex shader normal attribute
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");

    //link shader program 
    glLinkProgram(shaderProgramObject);

    //shader linking error checking
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
    modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_modelMatrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_viewMatrix");
    perspectiveProjectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_perspectiveProjectionMatrix");

    LaUniform = glGetUniformLocation(shaderProgramObject, "u_La");
    LdUniform = glGetUniformLocation(shaderProgramObject, "u_Ld");
    LsUniform = glGetUniformLocation(shaderProgramObject, "u_Ls");
    lightPositionUniform = glGetUniformLocation(shaderProgramObject, "u_lightPosition");

    KaUniform = glGetUniformLocation(shaderProgramObject, "u_Ka");
    KdUniform = glGetUniformLocation(shaderProgramObject, "u_Kd");
    KsUniform = glGetUniformLocation(shaderProgramObject, "u_Ks");
    materialShininessUniform = glGetUniformLocation(shaderProgramObject, "u_materialShininess");

    //God Rays 
    fprintf(gpFile, "\n***** God Rays Shader *****");
    
    //create shader
    godRays_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* godRays_vertexShaderSource = 
        "#version 450 core"                                                             \
        "\n"                                                                            \

        "in vec2 vPosition;"                                                            \
        "in vec2 vTexCoord;"                                                            \

        "uniform mat4 mvpMatrix;"                                                       \

        "out vec2 out_texcoord;"                                                        \

        "void main(void)"                                                               \
        "{"                                                                             \
        "   out_texcoord = vTexCoord;"                                                  \
        "   gl_Position = mvpMatrix * vec4(vPosition.x, vPosition.y, 0.0f, 1.0f);"      \
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
                fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    } 

    fprintf(gpFile, "\n----- Vertex Shader Compiled Successfully -----\n");

    //--- Fragment Shader ---

    //create shader
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
        "const float density = 0.926f;"                                                         \
        "const float weight = 0.58767f;"                                                        \

        "void main(void)"                                                                       \
        "{"                                                                                     \
        "   int NUM_SAMPLES = 150;"                                                             \
        
        "   vec2 tc = out_texcoord.xy;"                                                         \
        "   vec2 deltaTexCoord = (tc - lightPositionOnScreen);"                                 \
        "   deltaTexCoord *= 1.0f / float(NUM_SAMPLES) * density;"                              \
        "   float illuminationDecay = 1.0f;"                                                    \

        "   vec4 base_color = texture2D(sceneSampler, tc);"                                     \
        "   vec4 occluded_color = texture2D(occludedSampler, tc) * 0.4f;"                       \

        "   for(int i = 0; i < NUM_SAMPLES; i++)"                                               \
        "   {"                                                                                  \
        "       tc -= deltaTexCoord;"                                                           \
        "       vec4 tex_sample = texture2D(occludedSampler, tc) * 0.4f;"                       \
        "       tex_sample *= illuminationDecay * weight;"                                      \
        "       occluded_color += tex_sample;"                                                  \
        "       illuminationDecay *= decay;"                                                    \
        "   }"                                                                                  \

        "   FragColor = vec4(vec3(occluded_color) * exposure, 1.0f) + (base_color * 1.1f);"     \
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
                fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "----- Fragment Shader Compiled Successfully -----\n");

    //--- Shader Program ---

    //create shader program
    godRays_shaderProgramObject = glCreateProgram();

    //attach vertex shader to shader program
    glAttachShader(godRays_shaderProgramObject, godRays_vertexShaderObject);

    //attach fragment shader to shader program
    glAttachShader(godRays_shaderProgramObject, godRays_fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(godRays_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPositon");

    //binding of shader program object with vertex shader texcoord attribute
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
                fprintf(gpFile, "Shader Program Link Log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "----- Shader Program Linked Successfully -----\n");

    //get uniform locations
    sceneSamplerUniform = glGetUniformLocation(godRays_shaderProgramObject, "sceneSampler");
    occludedSamplerUniform = glGetUniformLocation(godRays_shaderProgramObject, "occludedSampler");
    lightPositionOnScreenUniform = glGetUniformLocation(godRays_shaderProgramObject, "lightPositionOnScreen");
    godRays_mvpMatrixUniform = glGetUniformLocation(godRays_shaderProgramObject, "mvpMatrix");

    //sphere data
    GenerateSphere(0.5f, 30, 30);

    //cube data
    const GLfloat cubeVertices[] = 
    {
        //near 
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,

        //right
        1.0f, 1.0f, -1.0f, 
        1.0f, 1.0f, 1.0f, 
        1.0f, -1.0f, 1.0f, 
        1.0f, -1.0f, -1.0f, 

        //far
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, 
        -1.0f, -1.0f, -1.0f, 

        //left
        -1.0f, 1.0f, 1.0f, 
        -1.0f, 1.0f, -1.0f, 
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f, 

        //top
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 

        //bottom
        -1.0f, -1.0f, -1.0f, 
        1.0f, -1.0f, -1.0f, 
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f
    };

    static const GLfloat cubeNormals[] = 
    {
        //near 
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,

        //right
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        //far
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,

        //left
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,

        //top
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        //bottom
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
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

    const GLfloat pyramidNormals[] = 
    {
        //near 
        0.0f, 0.447214f, 0.894427f,
        0.0f, 0.447214f, 0.894427f,
        0.0f, 0.447214f, 0.894427f,
        
        //right
        0.894427f, 0.447214f, 0.0f,
        0.894427f, 0.447214f, 0.0f,
        0.894427f, 0.447214f, 0.0f,

        //far
        0.0f, 0.447214f, -0.894427f,
        0.0f, 0.447214f, -0.894427f,
        0.0f, 0.447214f, -0.894427f,

        //left
        -0.894427f, 0.447214f, 0.0f,
        -0.894427f, 0.447214f, 0.0f,
        -0.894427f, 0.447214f, 0.0f
    };

    //screen quad
    const GLfloat quadVertices[] = 
    {
        (float)scr_width, (float)scr_height,
        0.0f, (float)scr_height, 
        0.0f, 0.0f,
        (float)scr_width, 0.0f
    };

    /*
    const GLfloat quadVertices[] = 
    {
        1.0f, 1.0f,
        -1.0f, 1.0f, 
        -1.0f, -1.0f,
        1.0f, -1.0f
    };
    */

    const GLfloat quadTexCoords[] =
    {
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };
    

    //setup vao and vbo
    //Screen Quad
    glGenVertexArrays(1, &vao_quad);
    glBindVertexArray(vao_quad);
        glGenBuffers(1, &vbo_quad_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_quad_vertices);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_quad_texcoords);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_quad_texcoords);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadTexCoords), quadTexCoords, GL_STATIC_DRAW);

        glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //Sphere
    glGenVertexArrays(1, &vao_sphere);
    glBindVertexArray(vao_sphere);
        glGenBuffers(1, &vbo_sphere_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_vertices);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_sphere_indices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_indices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);

    //Cube
    glGenVertexArrays(1, &vao_cube);
    glBindVertexArray(vao_cube);
        glGenBuffers(1, &vbo_cube_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_cube_normals);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_normals);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNormals), cubeNormals, GL_STATIC_DRAW);

        glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //Pyramid
    glGenVertexArrays(1, &vao_pyramid);
    glBindVertexArray(vao_pyramid);
        glGenBuffers(1, &vbo_pyramid_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_pyramid_vertices);
        glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_pyramid_normals);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_pyramid_normals);
        glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidNormals), pyramidNormals, GL_STATIC_DRAW);

        glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
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
    glEnable(GL_CULL_FACE);

    //set clearing color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  

    //enable texture memory
    glEnable(GL_TEXTURE_2D);

    //set projection matrices to identity
    perspectiveProjectionMatrix = mat4::identity();
    orthographicProjectionMatrix = mat4::identity();

    //initialize light properties
    La = vec4(0.4f, 0.4f, 0.4f, 1.0f);
    Ld = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    Ls = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    lightPosition = vec4(3.0f, 2.0f, -8.0f, 1.0f);

    //initialize material properties
    Ka = vec4(0.4f, 0.4f, 0.4f, 1.0f);
    Kd = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    Ks = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    materialShininess = 128.0f;

    //warm-up  call
    Resize(WIN_WIDTH, WIN_HEIGHT);
}

void GenerateSphere(float radius, int sectorCount, int stackCount)
{
    //variable declaration
    float x, y, z, xy;
    int t1, t2;
    
    float sectorStep = 2.0f * M_PI / (float)sectorCount;
    float stackStep = M_PI / (float)stackCount;
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

            vertices.push_back((GLfloat)x);
            vertices.push_back((GLfloat)y);
            vertices.push_back((GLfloat)z);
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
                indices.push_back((GLuint)t1);
                indices.push_back((GLuint)t2);
                indices.push_back((GLuint)(t1 + 1));
            }

            if(i != (stackCount - 1))
            {
                indices.push_back((GLuint)(t1 + 1));
                indices.push_back((GLuint)t2);
                indices.push_back((GLuint)(t2 + 1));
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
    orthographicProjectionMatrix = vmath::ortho(0.0f, (float)width, 0.0f, (float)height, 1.0f, -1.0f);
}

void Display(void)
{
    //function declarations
    vec4 transform(vec4 vector, mat4 matrix);

    //variable declarations
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 modelViewProjectionMatrix;
    vec2 lightPositionOnScreen;
    vec4 projected;

    //code
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_occluded);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        modelMatrix = mat4::identity();
        viewMatrix = mat4::identity();
        modelViewProjectionMatrix = mat4::identity();

        //occluded shader
        glUseProgram(occluded_shaderProgramObject);

        //white sphere
        glUniform3f(colorUniform, 1.0f, 1.0f, 1.0f);

        modelMatrix = vmath::translate(lightPosition[0], lightPosition[1], lightPosition[2]);
        modelViewProjectionMatrix = perspectiveProjectionMatrix * viewMatrix * modelMatrix;
        glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);

        glBindVertexArray(vao_sphere);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, NULL);
        
        //black occluding scene
        glUniform3f(colorUniform, 0.1f, 0.1f, 0.1f);

        //cube
        modelMatrix = vmath::translate(0.0f, 0.0f, -10.0f);
        modelViewProjectionMatrix = perspectiveProjectionMatrix * viewMatrix * modelMatrix;
        glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);

        glBindVertexArray(vao_cube);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

        //pyramid
        modelMatrix = vmath::translate(3.0f, 0.0f, -10.0f);
        modelViewProjectionMatrix = perspectiveProjectionMatrix * viewMatrix * modelMatrix;
        glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);

        glBindVertexArray(vao_pyramid);
        glDrawArrays(GL_TRIANGLES, 0, 12);
    
        glBindVertexArray(0);
        glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_scene);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //per-fragment lighting shader
        glUseProgram(shaderProgramObject);

        glUniform3fv(LaUniform, 1, La);
        glUniform3fv(LdUniform, 1, Ld);
        glUniform3fv(LsUniform, 1, Ls);
        glUniform4fv(lightPositionUniform, 1, lightPosition);

        glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(perspectiveProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

        //cube
        modelMatrix = vmath::translate(0.0f, 0.0f, -10.0f);
    
        Ka = vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
        Kd = vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
        Ks = vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
        materialShininess = 0.4f * 128.0f;

        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);

        glUniform3fv(KaUniform, 1, Ka);
        glUniform3fv(KdUniform, 1, Kd);
        glUniform3fv(KsUniform, 1, Ks);
        glUniform1f(materialShininessUniform, materialShininess);

        glBindVertexArray(vao_cube);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

        //pyramid
        modelMatrix = vmath::translate(3.0f, 0.0f, -10.0f);
    
        Ka = vec4(0.4f, 0.4f, 0.4f, 1.0f);
        Kd = vec4(0.5f, 0.0f, 0.0f, 1.0f);
        Ks = vec4(0.7f, 0.6f, 0.6f, 1.0f);
        materialShininess = 0.25f * 128.0f;
    
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);

        glUniform3fv(KaUniform, 1, Ka);
        glUniform3fv(KdUniform, 1, Kd);
        glUniform3fv(KsUniform, 1, Ks);
        glUniform1f(materialShininessUniform, materialShininess);

        glBindVertexArray(vao_pyramid);
        glDrawArrays(GL_TRIANGLES, 0, 12);

        glBindVertexArray(0);
        glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(godRays_shaderProgramObject);

    //3D light position to 2D screen space position
    projected = transform(lightPosition, perspectiveProjectionMatrix * viewMatrix);
    lightPositionOnScreen[0] = (projected[0] + 1.0f) / 2.0f; 
    lightPositionOnScreen[1] = (projected[1] + 1.0f) / 2.0f; 

    glUniformMatrix4fv(godRays_mvpMatrixUniform, 1, GL_FALSE, orthographicProjectionMatrix);
    glUniform2fv(lightPositionOnScreenUniform, 1, lightPositionOnScreen);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene_texture);
    glUniform1i(sceneSamplerUniform, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, occluded_texture);
    glUniform1i(occludedSamplerUniform, 1);

    glBindVertexArray(vao_quad);
    glDrawArrays(GL_QUADS, 0, 4);

    glBindVertexArray(0);
    glUseProgram(0);

    //swap the buffers
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

    //release textures
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

    //cube
    if(vao_cube)
    {
        glDeleteVertexArrays(1, &vao_cube);
        vao_cube = 0;
    }

    if(vbo_cube_vertices)
    {
        glDeleteBuffers(1, &vbo_cube_vertices);
        vbo_cube_vertices = 0;
    }

    if(vbo_cube_normals)
    {
        glDeleteBuffers(1, &vbo_cube_normals);
        vbo_cube_vertices = 0;
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

    if(vbo_pyramid_normals)
    {
        glDeleteBuffers(1, &vbo_pyramid_normals);
        vbo_pyramid_normals = 0;
    }

    //safe shader cleanup
    //occluded shader
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

    //per-fragment lighting
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

    //god rays shader
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
        fprintf(gpFile, "\n----- Program Completed Successfully -----\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}
