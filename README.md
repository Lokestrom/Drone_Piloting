# Drone piloting

A program for simulating the flight of drones. 
Currently in development and only a simple MVP for now.
This is a more of a test of concept for the bigger project [Rocket sim](https://github.com/Lokestrom/rocketSim-code), with a focus on drones.

## Getting Started
Clone using `git clone --recursive https://github.com/Lokestrom/Drone_Piloting.git`
> Forgot to use --recursive just run: git submodule update --init --recursive

The project uses CMake and requires C++23 or later.

>Only tested building on Windows 10 with MSVC, results on other platforms may vary.

**Build:**
``` bash
cd Drone_Piloting
cmake -S . -B build
cmake --build build --config <config> # Debug or Release
```

**Run:**
``` bash
build/src/<config>/app # Debug or Release
```
> If running this does not work then go to the folder and run the app.exe

## Using
Control using W, A, S, D, left shift, and space. <br>
Drones and maps stored in assets.
Currently loaded using the startup method in App, change this or change the contents of the current folders to change the drone or map properties.

## Plans:
Current plans for what i want to implement
### Core:
* Rotation
* Dynamic loading of drones
* Dynamic loading of maps
* Custom control scripts
* Custom forces
* Collisions
* Multi drone support
* Saving and loading

### Rendering:
* Shadows
* Dynamic lighting