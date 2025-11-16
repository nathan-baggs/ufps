# μfps

A small FPS game written in modern C++ and OpenGL, from scratch, live.

Feel free to join us on [Discord](https://discord.gg/9FkkMgXSUV) to discuss the project.

## "From Scratch"

Where possible we will be building everything from scratch. This includes: maths, windowing, rendering, etc

The few concessions I'm willing to make are:
- File format parsing (e.g. loading images, models)
- Physics (too complex for a small project like this)
- Lua scripting (although we will be writing our own bindings)

## Live

I'll be streaming the development live:
- [Twitch](https://www.twitch.tv/nathan_baggs)
- [YouTube](https://www.youtube.com/@nathanbaggs)

My plan is to do the bulk of the development live, and then do some polishing and tidying up off-stream.

## Building

Feel free to try and build project yourself, but I won't be distributing any assets I don't have the license for.

The build system is Dockerised and you can use the top level Makefile

```bash
# build the docker image (one off)
make docker-build

# configure the project
make configure

# build and run the game
make run

# (optional build without running)
make build

# run tests
make tests
```

## Contributing

I am not currently accepting contributions, but feel free to fork the project and make your own changes.
