.PHONY: all clean
all:
	cmake -S . -B build
	cmake --build build --config Release

clean:
	cmake --build build --target clean
