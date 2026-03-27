# GL-Engine

## Build and Run

### Build

1. Make sure [CMake](https://cmake.org/) is installed on your local machine
2. Clone the repository to your local machine and initialize submodules

```shell
# Clone the project
git clone https://github.com/BrennanAndruss/GL-Engine.git

# Initialize and update all submodules
git submodule update --init --recursive
```

3. Create a new directory and build the project

```shell
mkdir build
cd build
cmake ..
```

### Run

```shell
./game
```