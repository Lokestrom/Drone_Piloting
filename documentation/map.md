# Maps

## Using
To select a different map open the map select window.
From here all the drones in the `asset/Maps` folder that have a parseable `config.json`, 
with a valid `name` field will show up. 
Or click the find map button(only available on windows) to open the 
file explorer and find any folder on disk.

Also note that a map is just visual and has no effect on the physics, 
there is no collision detection implemented yet, so you can fly through the map.

## Custom maps
You can create your own custom maps for the sim by following this.

Create a folder that must contain:
* `config.json`: This is the core and decides everything about the map.

To make it show automatically in the selection window put it in `assets/Maps`.

### Config.json
The `config.json` file contains the properties of the map, these include:
* `name`: The name of the map, this is used for display purposes and to identify the map.
* `lightSource`: This is the direction that the light will shine. Fex [0,-1,0] is strait down and the program normalizes it so no control over brightness, just direction.
* `objects`: An array of objects that must have the following:
	* `model`: A 3d model, must be a .obj file.
	* `position`: The position of the object.<br>
	*Can also contain:*
	* `modelScale`: A 3d vector for scale in all directions.
	* `modelRotation`: A Euler rotation in degrees for rotating the object.

### Rendering
The renderer chunks the map in to 1000 x 1000 chunks and renders a 3000 x 3000 area.
And it does not respect models that go past the chunks it only cares about its `position`.
Also textures are limited to 1024 x 1024, as there is no texture streaming and 
larger texture fill VRAM fast. 

These are not limitation of the software I just have a crap graphics card.
So feel free to change the chunk size and the texture size limit. 

### Capabilities
I have my self loaded a city map of Trondheim with 150k objects and it runs at ca 50 fps on my GeForce GTX 950m.
With 142 different materials and 173 different geometries.

### Errors
When loading the map there is many checks that will happen and each fail will be in the console.
A bad map wil not crash the application as opposed to a drone that can.

### Backward compatibility
I don't plan on keeping any backward compatibility. 
At some point i may begin adding a change log and deprecation warnings.

## License
When creating a map it is your unique creation I have no rights over it.