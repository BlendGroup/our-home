mkdir tempobj
cl /EHsc src/*.cpp lib/windows/*.cpp /Fo"tempobj/" /Fe"main.exe" user32.lib gdi32.lib OpenGL32.lib glew32.lib FreeImage.lib
main.exe
rmdir /S /Q tempobj