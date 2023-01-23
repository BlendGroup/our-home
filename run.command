mkdir -p main.app/Contents/MacOS
mkdir tempdir
cd tempdir
clang++ -std=c++11 ../src/*.cpp ../lib/*.cpp -c
cd ..
clang++ -o main.app/Contents/MacOS/main tempdir/*.o lib/mac/*.m -framework cocoa -framework QuartzCore -framework OpenGL
./main.app/Contents/MacOS/main
rm -r tempdir