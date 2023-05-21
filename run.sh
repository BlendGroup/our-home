rm -f main.run
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib
if [[ $1 == "DEBUG" ]]; then
	make run DEBUG=true
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
