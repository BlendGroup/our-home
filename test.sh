rm -f main.run
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib
if [[ $1 == "DEBUG" ]]; then
	make test DEBUG=true
	printf "Run using:\n1. qrenderdoc renderdocsettings.cap\n2. gdb main.run\n3. ./main.run\n"
	if [[ $2 == 1 ]]; then
		qrenderdoc renderdocsettings.cap
	elif [[ $2 == 2 ]]; then
		gdb main.test
	else
		./main.test
	fi
else
	make test
	./main.test
fi
