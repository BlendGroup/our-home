rm *.txt
rm main.run
if [[ $1 == "DEBUG" ]]; then
	g++ lib/*.cpp src/*.cpp -lX11 -lGL -lGLEW -lassimp -o main.run -g -D DEBUG
	printf "Run using:\n1. qrenderdoc renderdocsettings.cap\n2. gdb main.run\n3. ./main.run\n"
	if [[ $2 == 1 ]]; then
		qrenderdoc renderdocsettings.cap
	elif [[ $2 == 2 ]]; then
		gdb main.run
	else
		./main.run $1
	fi
else
	g++ lib/*.cpp src/*.cpp -lX11 -lGL -lGLEW  -lassimp -o main.run
	./main.run $1
fi
