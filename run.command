mkdir -p main.app/Contents/MacOS
clang++ -o main.app/Contents/MacOS/main src/*.cpp lib/mac/*.m -framework cocoa -framework QuartzCore -framework OpenGL
open main.app