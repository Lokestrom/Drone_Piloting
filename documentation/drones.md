# Drones

## Using
**When loading a drone it can run arbetrary code at the same privilege as the app, only load drones with scripts that you trust.**

Pilot the drone using W, A, S, D, left shift, and space. <br>
These can be exchanged for other buttons in the info window, joysticks are supported.
But it is currently under development therefore it will be inconsistent 
on what can be joystick input do to testing.

Open the info window to see more info like position, orientation and mode. 
Here they can also be edited, and setting can also be changed.
If you edit rotation be aware that it has to be a unit quaternion also, 
it makes life a lot easier if you do it in the menu.

To change drone open the drone select window.
From here all the drones in the `asset/Drones` folder that have a parseable `config.json`, 
with a valid `name` field will show up. 
Or click the find drone button(only available on windows) to open the 
file explorer and find any folder on disk.

To create multiple drones press the player menu and create a new player then load a drone,
here you can also just not load a drone an use it as a camera. When swiping players the 
normal behavior is that only the selected players inputs are used in drones. But this behavior
is up to the drone implementation and when loading 2 of the same drone they will share 
the same static data, this will later be fixed.

## Custom drones
This is where the real fun starts, you can create your own drones by following these instructions. <br>
Note that the code uses SI units, so make sure to scale your models accordingly. <br>

Create a folder that must contain:
* `config.json`: The properties of the drone, such as mass, max thrust, etc.
* `model.obj`: The 3D model of the drone, must be a .obj file.
* `Debug/control.dll` and `Release/control.dll`: The control code for the drone, this is a dynamic library that contains the logic for controlling the drone.

Optionally you can add a README.md file or any other useful files to the folder. <br>
To make it show automatically in the selection window put it in `assets/Drones`.

### Config.json
The `config.json` file contains the properties of the drone, these include:
* `name`: The name of the drone, this is used for display purposes and to identify the drone.
* *`description`: A short description of the drone, this is used for display purposes.
* *`model`: A custom path to the model, if not specified it will default to `model.obj` in the same folder.
* `mass`: The mass of the drone, this is used for physics calculations.
* `inertiaTensor`: The inertia tensor of the drone, this is used for physics calculations.
* *`modelScale`: The scale of the model, this is used for rendering.
* *`modelRotation`: The rotation of the model, this is used for rendering.
* `engines`: An array of engines, each engine has the following properties:
  * `id`: The id of the engine, this is used to identify the engine in the control code.
  * `position`: The position of the engine relative to the center of mass, this is used for physics calculations.
  * `maxThrust`: The maximum thrust produced by the engine, this is used for error checking user code.
  * `direction`: The direction of the force the engine produses.
* *`inputs`: An array of the inputs that can be used, each input has the following properties:
  * `name`: This name is used to identify it in the code.
  * `type`: The type of the input, se [DroneAPI.h](..\src\API\DroneAPI.h).
  * `default key` This is the default key for the input, for `axis2` it must be an array of 2 keys.

__*__ is optional


### Control code
The control code for the drone is a dynamic library (`.dll` on Windows, `.so` on Linux) that contains the logic for controlling the drone. 
All definitions and structures needed for the control code are defined in [DroneAPI.h](..\src\API\DroneAPI.h), looking at the definitions in here is probably the best way to understand how the data is structured. <br>

This library must implement a function with the following signature:
```cpp
DRONE_API void update(
	const DroneState* state,
	const float dt,
	const float active,
	CommandBuffer* outCommands)
```
This function is called every frame and is responsible for updating the drone's state based on 
the user input and the current state of the drone.
#### Parameters:
* `state`: A pointer to a `DroneState` structure that contains the current state of the drone.
* `dt`: The delta time.
* `active`: This is set if the drone belongs to the currently selected player, this has no effect on behavior and the drone still receives input. The recommended use case is to ignore user input but this is totally optional. 
* `outCommands`: A pointer to a `CommandBuffer` structure where the function should write the commands for the drone.

#### Output:
The function should write the commands for the drone to the `outCommands` buffer.

The best way of understating how to write the control code is to look at the example drones in `assets/drones/*/src`,
here is some example code and a `CMkeLists.txt` that structures everything for you, Debug and Release folders with the dynamic library.
When running in the respective configurations the software uses the correct library, there is no error checking so the program will crash
silently and this can be a reason why.

### Optional's
The code also supports:
```cpp
DRONE_API void setup(const char* dronePath, const UserInput* inputs);
DRONE_API void getTargetPosition(float* outPosition);
DRONE_API SettingsBuffer* getSettings();
```

The `setup` function runs in the drone constructor, for now it only resives the path to the drone 
folder and the user input struct. May extend to resiveing a start state. <br>
The `getTargetPosition` function must output a 3d position that can be totally dictated 
by the user but it is displayed as a point in the renderer. <br>
The `getSettings` function outputs the settings of the drone that can be tweaked by the user.

### Errors
When loading a drone there is many checks that will happen and each fail will be in the console.
The script might fail and that crach the app.
### Backward compatibility
I don't plan on keeping any backward compatibility. 
At some point i may begin adding a change log and deprecation warnings.

## License
When creating a drone it is your unique creation I have no rights over it,
except if you use any code that is not form the [DroneAPI.h](..\src\API\DroneAPI.h) file  
in the creation of your code as this makes your code subject to the 
license in the `LICENSE` file.