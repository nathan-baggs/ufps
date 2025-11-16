.PHONY: docker-image config build run tests clean

docker-image:
	docker build -t custom-mingw64 .

config:
	docker run --rm -u $(shell id -u):$(shell id -g) -v "$(PWD)":"$(PWD)" -w "$(PWD)" custom-mingw64 cmake -B build -DCMAKE_TOOLCHAIN_FILE=mingw_toolchain.cmake -G "Ninja Multi-Config"

build:
	docker run --rm -u $(shell id -u):$(shell id -g) -v "$(PWD)":"$(PWD)" -w "$(PWD)" custom-mingw64 cmake --build build --config Debug

run: build
	./build/src/Debug/ufps.exe

tests: build
	./build/tests/Debug/unit_tests.exe --gtest_color=yes

pack: build
	docker run --rm -u $(shell id -u):$(shell id -g) -v "$(PWD)":"$(PWD)" -w "$(PWD)/build" custom-mingw64 cpack . -C Debug

clean:
	rm -rf build
