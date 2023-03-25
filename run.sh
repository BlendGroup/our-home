rm main.run
g++ lib/*.cpp src/*.cpp -lX11 -lGL -lGLEW -lassimp -o main.run
./main.run
