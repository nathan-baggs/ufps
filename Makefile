.PHONY: docker-image config config-ci build run resources tests sysroot sysroot-linux clean clean-cache

DOCKER_CCACHE := -e CCACHE_DIR=/ccache -e CCACHE_MAXSIZE=50G -v ufps-ccache:/ccache

docker-image:
	docker build -t custom-mingw64 .

config:
	docker run --rm -u $(shell id -u):$(shell id -g) -v "$(PWD)":"$(PWD)" -w "$(PWD)" $(DOCKER_CCACHE) custom-mingw64 cmake -B build -DCMAKE_TOOLCHAIN_FILE=mingw_toolchain.cmake -G "Ninja Multi-Config"

config-ci:
	docker run --rm -u $(shell id -u):$(shell id -g) -v "$(PWD)":"$(PWD)" -w "$(PWD)" $(DOCKER_CCACHE) custom-mingw64 cmake -B build -DUFPS_USE_EMBEDDED_RESOURCE_LOADER=ON -DCMAKE_TOOLCHAIN_FILE=mingw_toolchain.cmake -G "Ninja Multi-Config"

build:
	docker run --rm -u $(shell id -u):$(shell id -g) -v "$(PWD)":"$(PWD)" -w "$(PWD)" $(DOCKER_CCACHE) custom-mingw64 cmake --build build --config Debug

run: build
	./build/src/Debug/ufps.exe

run-stream: build
	~/.local/bin/run_game.sh

resources:
	docker run --rm -u $(shell id -u):$(shell id -g) -v "$(PWD)":"$(PWD)" -w "$(PWD)" $(DOCKER_CCACHE) custom-mingw64 cmake -S . -B build-host -DUFPS_RESOURCE_PACKER_ONLY=ON -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -G "Ninja Multi-Config"
	docker run --rm -u $(shell id -u):$(shell id -g) -v "$(PWD)":"$(PWD)" -w "$(PWD)" $(DOCKER_CCACHE) custom-mingw64 cmake --build build-host --config Debug --target resource_packer
	docker run --rm -u $(shell id -u):$(shell id -g) -v "$(PWD)":"$(PWD)" -w "$(PWD)" $(DOCKER_CCACHE) custom-mingw64 ./build-host/tools/Debug/resource_packer ./build/ ./assets ./secret-assets

tests: build
	./build/tests/Debug/unit_tests.exe --gtest_color=yes
pack: build
	docker run --rm -u $(shell id -u):$(shell id -g) -v "$(PWD)":"$(PWD)" -w "$(PWD)/build" $(DOCKER_CCACHE) custom-mingw64 cpack . -C Debug

ci: 
	$(MAKE) config
	$(MAKE) resources
	rm -rf build/CMake*
	$(MAKE) config-ci
	$(MAKE) build
	$(MAKE) tests
	$(MAKE) pack

sysroot: sysroot-linux
	docker run --rm -v "$(PWD)/sysroot":/out custom-mingw64 bash -c "cp -r /usr/local/x86_64-w64-mingw32/include /out/ && cp -r /usr/local/x86_64-w64-mingw32/include/c++ /out/ || true"

sysroot-linux:
	docker run --rm -v "$(PWD)/sysroot-linux":/out custom-mingw64 bash -eu -c '\
		version="$$(g++ -dumpfullversion)"; \
		target="$$(g++ -dumpmachine)"; \
		mkdir -p /out/c++ /out/c++-target /out/include; \
		cp -a "/usr/local/include/c++/$$version/." /out/c++/; \
		cp -a "/usr/local/include/c++/$$version/$$target/." /out/c++-target/; \
		cp -a /usr/include/. /out/include/'

clean-cache:
	docker run --rm -v ufps-ccache:/ccache custom-mingw64 ccache --clear

clean:
	rm -rf build build-host
