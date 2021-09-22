//headers
#include <windows.h>               //standard windows header
#include <stdio.h>                 //C header 
#include <gl/glew.h>               //OpenGL extension wrangler (must be included before gl.h)
#include <gl/gl.h>                 //OpenGL header
#include "RESOURCES.h"             //Resources header
#include "include/vmath.h"         //Maths header
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

#define DEPTH_TEXTURE_SIZE  2048
#define FRUSTUM_DEPTH       800.0f

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
GLuint shaderProgramObject;        

GLuint modelMatrixUniform;
GLuint viewMatrixUniform;
GLuint projectionMatrixUniform;
GLuint shadowMatrixUniform;

GLuint viewPosUniform;
GLuint lightPositionUniform;
GLuint lightAmbientUniform;
GLuint lightDiffuseUniform;
GLuint lightSpecularUniform;
GLuint diffuseTextureUniform;
GLuint specularTextureUniform;
GLuint normalTextureUniform;
GLuint depthTextureUniform;

GLuint materialAmbientUniform;
GLuint materialDiffuseUniform;
GLuint materialSpecularUniform;
GLuint materialShininessUniform;

GLuint pass_vertexShaderObject;         
GLuint pass_fragmentShaderObject;       
GLuint pass_shaderProgramObject;        

GLuint pass_mvpMatrixUniform;

GLuint fbo_depth;
GLuint depth_texture;

GLuint vao_square;
GLuint vbo_square_position;
GLuint vbo_square_normal;
GLuint vbo_square_texcoord;
GLuint vbo_square_tangent;

GLuint floor_diffuse_texture;
GLuint floor_specular_texture;
GLuint floor_normal_texture;

mat4 perspectiveProjectionMatrix;  
vec3 lightPosition;
GLsizei win_width;
GLsizei win_height;

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

Model Ganesha;

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
    TCHAR szAppName[] = TEXT("OpenGL : Model Loading");         //name of window class

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
    void mouse_move(double xPos, double yPos);   //camera move

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
            win_width = LOWORD(lParam);
            win_height = HIWORD(lParam);
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
    void loadGLTexture(GLuint *texture, const char *filename);

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

    //Normal Mapped Model Loading 
    fprintf(gpFile, "\n-> Normal Mapped Model Loading\n");

    //vertex shader
    vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* vertexShaderSourceCode = 
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
        "out vec4 out_shadowCoord;"                                                         \

        "uniform mat4 u_modelMatrix;"                                                       \
        "uniform mat4 u_viewMatrix;"                                                        \
        "uniform mat4 u_projectionMatrix;"                                                  \
        "uniform mat4 u_shadowMatrix;"                                                      \
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
        "   out_shadowCoord = u_shadowMatrix * vec4(out_fragPos, 1.0f);"                    \

        "   gl_Position = u_projectionMatrix * u_viewMatrix * vec4(out_fragPos, 1.0f);"     \
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
                fprintf(gpFile, "-> vertex shader compilation log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    } 

    fprintf(gpFile, "-> vertex shader compiled successfully\n");

    //fragment shader
    fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* fragmentShaderSourceCode = 
        "#version 450 core"                                                                                         \
        "\n"                                                                                                        \
        
        "layout(binding = 0)uniform sampler2D diffuse_texture;"                                                     \
        "layout(binding = 1)uniform sampler2D specular_texture;"                                                    \
        "layout(binding = 2)uniform sampler2D normal_texture;"                                                      \
        "layout(binding = 3)uniform sampler2DShadow depth_texture;"                                                 \

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
        "in vec4 out_shadowCoord;"                                                                                  \

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

        "   float f = textureProj(depth_texture, out_shadowCoord);"                                                 \

        "   vec3 ads_light = ambient + f * (diffuse + specular);"                                                   \
        "   FragColor = vec4(ads_light, 1.0f);"                                                                     \
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
                fprintf(gpFile, "-> fragment shader compilation log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "-> fragment shader compiled successfully\n");

    //shader program
    shaderProgramObject = glCreateProgram();

    //attach shaders to shader program
    glAttachShader(shaderProgramObject, vertexShaderObject);
    glAttachShader(shaderProgramObject, fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPosition");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_TEXCOORD, "vTexCoord");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_TANGENT, "vTangent");

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
                fprintf(gpFile, "-> shader program link log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "-> shader program linked successfully\n");

    //get uniform locations
    modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_modelMatrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_viewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_projectionMatrix");
    shadowMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_shadowMatrix");

    viewPosUniform = glGetUniformLocation(shaderProgramObject, "u_viewPos");
    lightPositionUniform = glGetUniformLocation(shaderProgramObject, "u_lightPos");
    lightAmbientUniform = glGetUniformLocation(shaderProgramObject, "u_lightAmbient");
    lightDiffuseUniform = glGetUniformLocation(shaderProgramObject, "u_lightDiffuse");
    lightSpecularUniform = glGetUniformLocation(shaderProgramObject, "u_lightSpecular");
    diffuseTextureUniform = glGetUniformLocation(shaderProgramObject, "diffuse_texture");
    specularTextureUniform = glGetUniformLocation(shaderProgramObject, "specular_texture");
    normalTextureUniform = glGetUniformLocation(shaderProgramObject, "normal_texture");
    depthTextureUniform = glGetUniformLocation(shaderProgramObject, "depth_texture");

    materialAmbientUniform = glGetUniformLocation(shaderProgramObject, "u_matAmbient");
    materialDiffuseUniform = glGetUniformLocation(shaderProgramObject, "u_matDiffuse");
    materialSpecularUniform = glGetUniformLocation(shaderProgramObject, "u_matSpecular");
    materialShininessUniform = glGetUniformLocation(shaderProgramObject, "u_matShininess");

    //Pass-Through Shader
    fprintf(gpFile, "\n-> Pass-Through Shader\n");

    //vertex shader
    pass_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* pass_vertexShaderSourceCode = 
        "#version 450 core"                             \
        "\n"                                            \

        "in vec4 vPosition;"                            \
        "uniform mat4 u_mvpMatrix;"                     \

        "void main(void)"                               \
        "{"                                             \
        "   gl_Position = u_mvpMatrix * vPosition;"     \
        "}";

    //provide source code to shader object
    glShaderSource(pass_vertexShaderObject, 1, (const GLchar**)&pass_vertexShaderSourceCode, NULL);

    //compile shader 
    glCompileShader(pass_vertexShaderObject);

    //shader compilation error checking
    glGetShaderiv(pass_vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(pass_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(pass_vertexShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> vertex shader compilation log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    } 

    fprintf(gpFile, "-> vertex shader compiled successfully\n");

    //fragment shader
    pass_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* pass_fragmentShaderSourceCode = 
        "#version 450 core"             \
        "\n"                            \
        
        "out vec4 fragColor;"           \

        "void main(void)"               \
        "{"                             \
        "   fragColor = vec4(1.0f);"    \
        "}";                 

    //provide source code to shader object 
    glShaderSource(pass_fragmentShaderObject, 1, (const GLchar**)&pass_fragmentShaderSourceCode, NULL);

    //compile shader
    glCompileShader(pass_fragmentShaderObject);

    //shader compilation error checking
    glGetShaderiv(pass_fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(pass_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(pass_fragmentShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> fragment shader compilation log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "-> fragment shader compiled successfully\n");

    //shader program
    pass_shaderProgramObject = glCreateProgram();

    //attach shaders to shader program
    glAttachShader(pass_shaderProgramObject, pass_vertexShaderObject);
    glAttachShader(pass_shaderProgramObject, pass_fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(pass_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPosition");

    //link shader program 
    glLinkProgram(pass_shaderProgramObject);

    //shader linking error checking
    glGetProgramiv(pass_shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(pass_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(pass_shaderProgramObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> shader program link log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "-> shader program linked successfully\n");

    //get uniform locations
    pass_mvpMatrixUniform = glGetUniformLocation(pass_shaderProgramObject, "u_mvpMatrix");

    //setup square data
    vec3 pos1(1.0f, 0.0f, -1.0f);
    vec3 pos2(-1.0f, 0.0f, -1.0f);
    vec3 pos3(-1.0f, 0.0f, 1.0f);
    vec3 pos4(1.0f, 0.0f, 1.0f);

    vec2 uv1(1.0f, 1.0f);
    vec2 uv2(0.0f, 1.0f);
    vec2 uv3(0.0f, 0.0f);
    vec2 uv4(1.0f, 0.0f);

    vec3 nm1(0.0f, 1.0f, 0.0f);

    vec3 tangent1;
    vec3 tangent2;

    //triangle 1 (1, 2, 3)
    vec3 edge1 = pos2 - pos1;
    vec3 edge2 = pos3 - pos1;
    vec2 deltaUV1 = uv2 - uv1;
    vec2 deltaUV2 = uv3 - uv1;

    float f = 1.0f / (deltaUV1[0] * deltaUV2[1] - deltaUV2[0] * deltaUV1[1]);

    tangent1[0] = f * (deltaUV2[1] * edge1[0] - deltaUV1[1] * edge2[0]);
    tangent1[1] = f * (deltaUV2[1] * edge1[1] - deltaUV1[1] * edge2[1]);
    tangent1[2] = f * (deltaUV2[1] * edge1[2] - deltaUV1[1] * edge2[2]);

    //triangle 2(1, 3, 4)
    edge1 = pos3 - pos1;
    edge2 = pos4 - pos1;
    deltaUV1 = uv3 - uv1;
    deltaUV2 = uv4 - uv1;

    f = 1.0f / (deltaUV1[0] * deltaUV2[1] - deltaUV2[0] * deltaUV1[1]);

    tangent2[0] = f * (deltaUV2[1] * edge1[0] - deltaUV1[1] * edge2[0]);
    tangent2[1] = f * (deltaUV2[1] * edge1[1] - deltaUV1[1] * edge2[1]);
    tangent2[2] = f * (deltaUV2[1] * edge1[2] - deltaUV1[1] * edge2[2]);

    //vertex data
    const GLfloat squareVertices[] = 
    {
        pos1[0], pos1[1], pos1[2],
        pos2[0], pos2[1], pos2[2],
        pos3[0], pos3[1], pos3[2],

        pos1[0], pos1[1], pos1[2],
        pos3[0], pos3[1], pos3[2],
        pos4[0], pos4[1], pos4[2]
    };

    //normal data
    static const GLfloat squareNormals[] = 
    {
        nm1[0], nm1[1], nm1[2],
        nm1[0], nm1[1], nm1[2],
        nm1[0], nm1[1], nm1[2],

        nm1[0], nm1[1], nm1[2],
        nm1[0], nm1[1], nm1[2],
        nm1[0], nm1[1], nm1[2],
    };

    //texcoord data
    const GLfloat squareTexCoords[] = 
    {
        uv1[0], uv1[1],
        uv2[0], uv2[1],
        uv3[0], uv3[1],

        uv1[0], uv1[1],
        uv3[0], uv3[1],
        uv4[0], uv4[1]
    };

    const GLfloat squareTangents[] =
    {
        tangent1[0], tangent1[1], tangent1[2],
        tangent1[0], tangent1[1], tangent1[2],
        tangent1[0], tangent1[1], tangent1[2],

        tangent2[0], tangent2[1], tangent2[2],
        tangent2[0], tangent2[1], tangent2[2],
        tangent2[0], tangent2[1], tangent2[2],
    };

    //setup vao and vbo 
    glGenVertexArrays(1, &vao_square);
    glBindVertexArray(vao_square);
        glGenBuffers(1, &vbo_square_position);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_square_position);
        glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_square_normal);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_square_normal);
        glBufferData(GL_ARRAY_BUFFER, sizeof(squareNormals), squareNormals, GL_STATIC_DRAW);

        glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_square_texcoord);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_square_texcoord);
        glBufferData(GL_ARRAY_BUFFER, sizeof(squareTexCoords), squareTexCoords, GL_STATIC_DRAW);

        glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_square_tangent);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_square_tangent);
        glBufferData(GL_ARRAY_BUFFER, sizeof(squareTangents), squareTangents, GL_STATIC_DRAW);

        glVertexAttribPointer(AMC_ATTRIBUTE_TANGENT, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_TANGENT);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //framebuffer configurations
    glGenFramebuffers(1, &fbo_depth);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_depth);
        //depth attchment texture
        glGenTextures(1, &depth_texture);
        glBindTexture(GL_TEXTURE_2D, depth_texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, DEPTH_TEXTURE_SIZE, DEPTH_TEXTURE_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            //incoming depth values are compared to texture
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glBindTexture(GL_TEXTURE_2D, 0);

        //attach depth texture to framebuffer
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);

        //diable color rendering as there are no color buffers 
        glDrawBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //smooth shading  
    glShadeModel(GL_SMOOTH);                  

    //depth
    glClearDepth(1.0f);                                     
    glEnable(GL_DEPTH_TEST);                                
    glDepthFunc(GL_LEQUAL);

    //backface culling 
    glEnable(GL_CULL_FACE);

    //quality of color and texture coordinate interpolation
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    

    //set clearing color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  

    //set up global variables
    perspectiveProjectionMatrix = mat4::identity();
    lightPosition = vmath::vec3(50.0f, 20.0f, 50.0f);
    Ganesha.LoadModel("models/Ganesha.obj");

    //load textures
    loadGLTexture(&floor_diffuse_texture, "textures/marble_albedo.png");
    loadGLTexture(&floor_specular_texture, "textures/marble_roughness.png");
    loadGLTexture(&floor_normal_texture, "textures/marble_normal.png");

    camera_pos = vec3(0.0f, 10.0f, 50.0f);
    camera_front = vec3(0.0f, 0.0f, 0.0f);
    camera_up = vec3(0.0f, 1.0f, 0.0f);

    //warm-up  call
    Resize(WIN_WIDTH, WIN_HEIGHT);
}

void loadGLTexture(GLuint *texture, const char *filename)
{
    //variable declarations
    unsigned char *pixel_data = NULL;
    int width, height, nrComponents;
    GLenum format;

    //code
    pixel_data = stbi_load(filename, &width, &height, &nrComponents, 0);
    if(pixel_data == NULL)
    {
        fprintf(gpFile, "Error : failed to load texture %s.\n", filename);
        DestroyWindow(ghwnd);
    }

    if (nrComponents == 1)
        format = GL_RED;
    else if (nrComponents == 3)
        format = GL_RGB;
    else if (nrComponents == 4)
        format = GL_RGBA;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    //set up texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    //push the data to texture memory
    glTexImage2D(GL_TEXTURE_2D, 0, format, (GLint)width, (GLint)height, 0, format, GL_UNSIGNED_BYTE, (const void*)pixel_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(pixel_data);
    pixel_data = NULL;
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

    perspectiveProjectionMatrix = vmath::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
}

void Display(void)
{
    //variable declarations
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 shadowMatrix;
    mat4 mvpMatrix;

    mat4 light_viewMatrix;
    mat4 light_projectionMatrix;

    mat4 scale_bias_matrix = mat4(vec4(0.5f, 0.0f, 0.0f, 0.0f),
                                  vec4(0.0f, 0.5f, 0.0f, 0.0f),
                                  vec4(0.0f, 0.0f, 0.5f, 0.0f),
                                  vec4(0.5f, 0.5f, 0.5f, 1.0f));

    static GLfloat light_angle = 0.0f;

    //code
    //set matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    mvpMatrix = mat4::identity();
    light_viewMatrix = mat4::identity();
    light_projectionMatrix = mat4::identity();

    //rotate light around y axis
    lightPosition[0] = 50.0f * sinf(light_angle);
    lightPosition[2] = 50.0f * cosf(light_angle);

    //shadow pass 1
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_depth);
        //set up viewport for depth texture
        glViewport(0, 0, DEPTH_TEXTURE_SIZE, DEPTH_TEXTURE_SIZE);

        //clear the depth buffer
        glClearDepth(1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);

        //to resolve depth-fighting issues
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(2.0f, 4.0f);

        glUseProgram(pass_shaderProgramObject);
            //render the scene from light's perspective
            light_viewMatrix = vmath::lookat(lightPosition, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
            
            //set up projection matrix for depth texture 
            light_projectionMatrix = vmath::frustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, FRUSTUM_DEPTH);

            //Draw Model
            modelMatrix = mat4::identity();
            modelMatrix = vmath::translate(0.0f, -1.9f, 3.0f);
            modelMatrix = modelMatrix * vmath::scale(0.2f, 0.2f, 0.2f);

            mvpMatrix = light_projectionMatrix * light_viewMatrix * modelMatrix;
            glUniformMatrix4fv(pass_mvpMatrixUniform, 1, GL_FALSE, mvpMatrix);

            Ganesha.Draw(pass_shaderProgramObject);
        
            //Draw Floor
            modelMatrix = mat4::identity();
            modelMatrix = vmath::translate(0.0f, -1.8f, 0.0f);
            modelMatrix = modelMatrix * vmath::scale(50.0f, 50.0f, 50.0f);

            mvpMatrix = light_projectionMatrix * light_viewMatrix * modelMatrix;
            glUniformMatrix4fv(pass_mvpMatrixUniform, 1, GL_FALSE, mvpMatrix);

            glBindVertexArray(vao_square);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        glUseProgram(0);

        //reset states
        glDisable(GL_POLYGON_OFFSET_FILL);
        glViewport(0, 0, win_width, win_height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);    

    //shadow pass 2
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramObject);
        //set up scene view matrix 
        viewMatrix = vmath::lookat(camera_pos, camera_pos + camera_front, camera_up);

        glUniform3fv(viewPosUniform, 1, camera_pos);     

        glUniform3fv(lightPositionUniform, 1, lightPosition);
        glUniform3f(lightAmbientUniform, 0.2f, 0.2f, 0.2f);
        glUniform3f(lightDiffuseUniform, 1.0f, 1.0f, 1.0f);
        glUniform3f(lightSpecularUniform, 1.0f, 1.0f, 1.0f);

        //Draw Model 
        modelMatrix = mat4::identity();
        modelMatrix = vmath::translate(0.0f, -1.9f, 3.0f);
        modelMatrix = modelMatrix * vmath::scale(0.2f, 0.2f, 0.2f);

        shadowMatrix = scale_bias_matrix * light_projectionMatrix * light_viewMatrix;

        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
        glUniformMatrix4fv(shadowMatrixUniform, 1, GL_FALSE, shadowMatrix);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depth_texture);
        glUniform1i(depthTextureUniform, 3);

        Ganesha.Draw(shaderProgramObject);

        //draw Floor
        modelMatrix = mat4::identity();
        modelMatrix = vmath::translate(0.0f, -1.8f, 0.0f);
        modelMatrix = modelMatrix * vmath::scale(50.0f, 50.0f, 50.0f);

        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floor_diffuse_texture);
        glUniform1i(diffuseTextureUniform, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floor_specular_texture);
        glUniform1i(specularTextureUniform, 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, floor_normal_texture);
        glUniform1i(normalTextureUniform, 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depth_texture);
        glUniform1i(depthTextureUniform, 3);

        glUniform3f(materialAmbientUniform, 0.2f, 0.2f, 0.2f);
        glUniform3f(materialDiffuseUniform, 1.0f, 1.0f, 1.0f);
        glUniform3f(materialSpecularUniform, 1.0f, 1.0f, 1.0f);
        glUniform1f(materialShininessUniform, 50.0f);

        glBindVertexArray(vao_square);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    glUseProgram(0);

    //update 
    light_angle += 0.005f;
    if(light_angle >= 360.0f)
        light_angle = 0.0f;

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

    //release vao and vbo
    if(vao_square)
    {
        glDeleteVertexArrays(1, &vao_square);
        vao_square = 0;
    }

    if(vbo_square_position)
    {
        glDeleteBuffers(1, &vbo_square_position);
        vbo_square_position = 0;
    }

    if(vbo_square_normal)
    {
        glDeleteBuffers(1, &vbo_square_normal);
        vbo_square_normal = 0;
    }

    if(vbo_square_texcoord)
    {
        glDeleteBuffers(1, &vbo_square_texcoord);
        vbo_square_texcoord = 0;
    }

    if(vbo_square_tangent)
    {
        glDeleteBuffers(1, &vbo_square_tangent);
        vbo_square_tangent = 0;
    }

    //release textures
    if(floor_diffuse_texture)
    {
        glDeleteTextures(1, &floor_diffuse_texture);
        floor_diffuse_texture = 0;
    }

    if(floor_specular_texture)
    {
        glDeleteTextures(1, &floor_specular_texture);
        floor_specular_texture = 0;
    }

    if(floor_normal_texture)
    {
        glDeleteTextures(1, &floor_normal_texture);
        floor_normal_texture = 0;
    }

    if(depth_texture)
    {
        glDeleteTextures(1, &depth_texture);
        depth_texture = 0;
    }

    //release fbo
    if(fbo_depth)
    {
        glDeleteFramebuffers(1, &fbo_depth);
        fbo_depth = 0;
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

    if(pass_shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(pass_shaderProgramObject);
        glGetProgramiv(pass_shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(pass_shaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(pass_shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(pass_shaderProgramObject);
        pass_shaderProgramObject = 0;
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
