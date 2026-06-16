.PHONY: docker-image config config-ci build run tests sysroot clean clean-cache

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

resources: build
	./build/tools/Debug/resource_packer.exe ./build/ ./assets ./secret-assets

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

sysroot:
	docker run --rm -v "$(PWD)/sysroot":/out custom-mingw64 bash -c "cp -r /usr/local/x86_64-w64-mingw32/include /out/ && cp -r /usr/local/x86_64-w64-mingw32/include/c++ /out/ || true"

clean-cache:
	docker run --rm -v ufps-ccache:/ccache custom-mingw64 ccache --clear

clean:
	rm -rf build
