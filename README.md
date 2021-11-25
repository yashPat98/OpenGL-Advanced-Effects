# OpenGL-Advanced-Effects <br />
This Repository contains advanced OpenGL effects on windows platform <br />

Additional dependencies <br />
1 -> ASSIMP library for model loading (https://github.com/assimp/assimp) <br />
2 -> GLEW library for OpenGL core profile context (http://glew.sourceforge.net/) <br />

These projects are build using command line <br />
following are the commands for visual studios developer command prompt <br />

//compilation <br />
cl.exe /c /EHsc OGLPP.cpp + (include path for assimp headers) + (include path for glew headers) <br />

//resource compilation <br />
rc.exe RESOURCES.rc <br />

//linking (dependent libraries are linked using compilar directive) <br />
link.exe OGLPP.obj RESOURCES.res <br />

//launch exe  <br />
OGLPP.exe <br />

if you still cant figure out how to build above projects <br />
drop a mail at prime.ash75@gmail.com <br />
