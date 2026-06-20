make:
	cmake --build build --parallel

configure:
	cmake -S . -B build -G Ninja 
run:
	./build/Debug/rp.exe
