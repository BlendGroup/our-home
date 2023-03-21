rm main.run
g++ lib/*.cpp src/*.cpp -lX11 -lGL -lGLEW -o main.run
./main.run