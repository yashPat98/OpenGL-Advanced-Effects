# OpenGL-Advanced-Effects
This Repository contains advanced OpenGL effects on windows platform

I dont rely on IDEs like Visual Studios or QT 
An experienced developer can figure out how to build the projects inside this repo

Additional dependencies
1 -> ASSIMP library for model loading (https://github.com/assimp/assimp)
2 -> GLEW library for OpenGL core profile context (http://glew.sourceforge.net/)

These projects are build using command line 
following are the commands for visual studios developer command prompt 

//compilation 
cl.exe /c /EHsc OGLPP.cpp + (include path for assimp headers) + (include path for glew headers)

//resource compilation
rc.exe RESOURCES.rc

//linking (dependent libraries are linked using compilar directive)
link.exe OGLPP.obj RESOURCES.res

//launch exe 
OGLPP.exe

if you still cant figure out how to build above projects 
drop a mail at prime.ash75@gmail.com
