.PHONY: docker-image config build run tests sysroot clean

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

sysroot:
	docker run --rm -v "$(PWD)/sysroot":/out custom-mingw64 bash -c "cp -r /usr/local/x86_64-w64-mingw32/include /out/ && cp -r /usr/local/x86_64-w64-mingw32/include/c++ /out/ || true"

clean:
	rm -rf build
