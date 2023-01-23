mkdir tempobj
del main.exe
del log.log
cl /EHsc src/*.cpp lib/*.cpp lib/windows/*.cpp /Fo"tempobj/" /Fe"main.exe" user32.lib gdi32.lib OpenGL32.lib glew32.lib FreeImage.lib
main.exe > log.log
rmdir /S /Q tempobj