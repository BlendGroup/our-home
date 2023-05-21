rm -f main.run
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib
if [[ $1 == "DEBUG" ]]; then
	g++ lib/*.cpp src/*.cpp -I ./include -L ./lib -lX11 -lGL -lGLEW -lassimp -lOpenCL -lopenal -lalut -o main.run -g -D DEBUG
	printf "Run using:\n1. qrenderdoc renderdocsettings.cap\n2. gdb main.run\n3. ./main.run\n"
	if [[ $2 == 1 ]]; then
		qrenderdoc renderdocsettings.cap
	elif [[ $2 == 2 ]]; then
		gdb main.run
	else
		./main.run
	fi
else
	make run
	./main.run
fi
