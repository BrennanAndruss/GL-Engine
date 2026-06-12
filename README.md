# 🎨 CMYKingdom

You find yourself in a magical kingdom void of color. Your quest is to explore the world to collect cyan, magenta, and yellow crystals scattered throughout the land to restore color to the kingdom!

Made over 10 weeks with C++ and OpenGL for CSC 476: Real-time 3D Computer Graphics Software.

![Forest Scene](./docs/images/forest.png)

For project details, visit the [project website](https://brennanandruss.github.io/CMYKingdom/)

## Build and Run

### Build

1. Make sure [CMake](https://cmake.org/) is installed on your local machine
2. Clone the repository to your local machine

```shell
git clone https://github.com/BrennanAndruss/GL-Engine.git
```

3. Create a new directory and build the project

```shell
mkdir build
cd build
cmake ..
```

4. Build with a native build tool (e.g., Make, Visual Studio)

```shell
# For Linux and Mac
make
```

### Run

Building the program compiles the engine code into a static library in `build/engine/` and creates the game executable in `build/game`.

```shell
cd game
./game
```
