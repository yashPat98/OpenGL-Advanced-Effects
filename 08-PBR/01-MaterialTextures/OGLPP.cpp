//headers
#include <windows.h>               //standard windows header
#include <stdio.h>                 //C header 
#include <gl/glew.h>               //OpenGL extension wrangler (must be included before gl.h)
#include <gl/gl.h>                 //OpenGL header
#include <vector>                  //standard vector header
#include "RESOURCES.h"             //Resources header
#include "include/vmath.h"         //Maths header

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

GLuint vao_sphere;
GLuint vbo_sphere_position;
GLuint vbo_sphere_normal;
GLuint vbo_sphere_texcoord;
GLuint vbo_sphere_indices;

GLuint albedo_texture;
GLuint normal_texture;
GLuint metallic_texture;
GLuint roughness_texture;
GLuint ao_texture;

mat4 perspectiveProjectionMatrix;  

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
        "#version 450 core"                                                             \
        "\n"                                                                            \
        "in vec2 out_texcoord;"                                                         \
        "in vec3 out_worldPos;"                                                         \
        "in vec3 out_normal;"                                                           \
        
        "uniform sampler2D albedoMap;"                                                  \
        "uniform sampler2D normalMap;"                                                  \
        "uniform sampler2D metallicMap;"                                                \
        "uniform sampler2D roughnessMap;"                                               \
        "uniform sampler2D aoMap;"                                                      \

        "uniform vec3 lightPosition;"                                                   \
        "uniform vec3 lightColor;"                                                      \
        "uniform vec3 cameraPosition;"                                                  \

        "out vec4 FragColor;"                                                           \

        "const float PI = 3.14159265359f;"                                              \

        "vec3 getNormalFromMap()"                                                       \
        "{"                                                                             \
        "   vec3 tangentNormal = texture(normalMap, out_texcoord).xyz * 2.0f - 1.0f;"   \
        
        "   vec3 Q1 = dFdx(out_worldPos);"                                              \
        "   vec3 Q2 = dFdy(out_worldPos);"                                              \
        "   vec2 st1 = dFdx(out_texcoord);"                                             \
        "   vec2 st2 = dFdy(out_texcoord);"                                             \

        "   vec3 N = normalize(out_normal);"                                            \
        "   vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);"                               \
        "   vec3 B = -normalize(cross(N, T));"                                          \
        "   mat3 TBN = mat3(T, B, N);"                                                  \

        "   return (normalize(TBN * tangentNormal));"                                   \
        "}"                                                                             \

        "float DistributionGGX(vec3 N, vec3 H, float roughness)"                        \
        "{"                                                                             \
        "   float a = roughness * roughness;"                                           \
        "   float a2 = a * a;"                                                          \
        "   float NdotH = max(dot(N, H), 0.0f);"                                        \
        "   float NdotH2 = NdotH * NdotH;"                                              \

        "   float nom = a2;"                                                            \
        "   float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);"                               \
        "   denom = PI * denom * denom;"                                                \

        "   return (nom / denom);"                                                      \
        "}"                                                                             \

        "float GeometrySchlickGGX(float NdotV, float roughness)"                        \
        "{"                                                                             \
        "   float r = (roughness + 1.0f);"                                              \
        "   float k = (r * r) / 8.0f;"                                                  \

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

        "vec3 fresnelSchlick(float cosTheta, vec3 F0)"                                  \
        "{"                                                                             \
        "   return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);"   \
        "}"                                                                             \

        "void main(void)"                                                               \
        "{"                                                                             \
        "   vec3 albedo = pow(texture(albedoMap, out_texcoord).rgb, vec3(2.2f));"       \
        "   float metallic = texture(metallicMap, out_texcoord).r;"                     \
        "   float roughness = texture(roughnessMap, out_texcoord).r;"                   \
        "   float ao = texture(aoMap, out_texcoord).r;"                                 \
        
        "   vec3 N = getNormalFromMap();"                                               \
        "   vec3 V = normalize(cameraPosition - out_worldPos);"                         \

            //0.04 = dia-electric , albedo = metal
        "   vec3 F0 = vec3(0.04f);"                                                     \
        "   F0 = mix(F0, albedo, metallic);"                                            \

            //reflectance equation
        "   vec3 Lo = vec3(0.0f);"                                                      \
        "   vec3 L = normalize(lightPosition - out_worldPos);"                          \
        "   vec3 H = normalize(V + L);"                                                 \

        "   float distance = length(lightPosition - out_worldPos);"                     \
        "   float attenuation = 1.0f / (distance * distance);"                          \
        "   vec3 radiance = lightColor * attenuation;"                                  \

            //cook-torrance BRDF
        "   float NDF = DistributionGGX(N, H, roughness);"                              \
        "   float G = GeometrySmith(N, V, L, roughness);"                               \
        "   vec3 F = fresnelSchlick(max(dot(H, L), 0.0f), F0);"                         \

        "   vec3 numerator = NDF * G * F;"                                              \
        "   float denominator = 4 * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f;"     \
        "   vec3 specular = numerator / denominator;"                                           \

        "   vec3 kS = F;"                                                                       \
        "   vec3 kD = vec3(1.0f) - kS;"                                                         \
        "   kD *= 1.0f - metallic;"                                                             \

        "   float NdotL = max(dot(N, L), 0.0f);"                                                \
        "   Lo += (kD * albedo / PI + specular) * radiance * NdotL;"                            \

            //ambient lighting
        "   vec3 ambient = vec3(0.03f) * albedo * ao;"                                          \
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

    //set perspective projection matrix to identity
    perspectiveProjectionMatrix = mat4::identity();

    //load textures
    load_texture("textures/albedo.png", &albedo_texture);
    load_texture("textures/normal.png", &normal_texture);
    load_texture("textures/metallic.png", &metallic_texture);
    load_texture("textures/roughness.png", &roughness_texture);
    load_texture("textures/ao.png", &ao_texture);

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

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();

    glUseProgram(shaderProgramObject);
        modelMatrix = vmath::translate(0.0f, 0.0f, -5.0f);
        modelMatrix = modelMatrix * vmath::rotate(90.0f, 1.0f, 0.0f, 0.0f);
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

        glUniform3f(lightPositionUniform, 5.0f, 5.0f, 5.0f);
        glUniform3f(lightColorUniform, 350.0f, 350.0f, 350.0f);
        glUniform3f(cameraPositionUniform, 0.0f, 0.0f, 0.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedo_texture);
        glUniform1i(albedoMapUniform, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normal_texture);
        glUniform1i(normalMapUniform, 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, metallic_texture);
        glUniform1i(metallicMapUniform, 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, roughness_texture);
        glUniform1i(roughnessMapUniform, 3);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, ao_texture);
        glUniform1i(aoMapUniform, 4);

        glBindVertexArray(vao_sphere);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, NULL);
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

    //release textures
    if(albedo_texture)
    {
        glDeleteTextures(1, &albedo_texture);
        albedo_texture = 0;
    }

    if(normal_texture)
    {
        glDeleteTextures(1, &normal_texture);
        normal_texture = 0;
    }

    if(metallic_texture)
    {
        glDeleteTextures(1, &metallic_texture);
        metallic_texture = 0;
    }

    if(roughness_texture)
    {
        glDeleteTextures(1, &roughness_texture);
        roughness_texture = 0;
    }

    if(ao_texture)
    {
        glDeleteTextures(1, &ao_texture);
        ao_texture = 0;
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
