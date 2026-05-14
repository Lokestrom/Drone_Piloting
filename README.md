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
Only the testDrone has a proper PID controller.<br>
Drones and maps stored in assets. Currently only drones can be loaded dynamically but map has to be 
changed in code or in the original folder.

Further documentation is in the `documentation` folder

## Plans:
Current plans for what i want to implement
### Core:
* Dynamic loading of maps
* Custom forces
* Collisions
* Multi drone support
* Fixed time step simulation
* Saving and loading

### Rendering:
* Shadows
* Dynamic lighting

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