if [ ! -d "build" ]; then
	mkdir build
	echo "Created build directory"
fi

cmake -S . -B build

cmake --build build

