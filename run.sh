rm main.run
g++ lib/*.cpp lib/linux/*.cpp src/*.cpp -lX11 -lGL -lGLEW -o main.run
./main.run