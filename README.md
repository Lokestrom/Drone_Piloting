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
Control using W, A, S, D, left shift, and space, and ESC for menu, TAB for overlay, F for freecam, T for orbit cam. <br>
Drones and maps stored in assets, there is 2 drones and one map there.
The map and the test drone is loaded on startup but you can change the drone or map 
by clicking on view in the overlay or menu and opening the drone or map selection windows.

Further documentation is in the `documentation` folder

## Plans:
Current plans for what i want to implement
### Core:
* Custom forces
* Collisions
* Fixed time step simulation
* Saving and loading

### Rendering:
* Dynamic lighting
* Seperate it out into a rendering engine

### Other:
* FreeCAD live link and editor


## License
This project is licensed under the GNU General Public License v3.0 (GPLv3), 
except for third-party components and the API.

The API is licensed under the MIT License, See `src/API/DroneAPI.h` header 
for details. Note that this is only the `DroneAPI.h` file, the helpers in
the directory are still under GPLv3.

Third-party components are located in the `external` directory and are 
distributed under their respective licenses. See the license files in 
the directories or the headers of individual files for details.