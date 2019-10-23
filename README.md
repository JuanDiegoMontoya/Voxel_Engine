# 3D_Voxel_Engine
Yet another voxel engine.

This is a tech demo designed to demonstrate the implementation of various rendering and procedural techniques in a large and dynamic environment.
(*WIP*) indicates that a feature is basically complete but has a few bugs that need to be smoothed out. 

Performance was tested on this system for reference:
- AMD Ryzen 5 2600X CPU
- 16 GB RAM
- NVIDIA GeForce GTX 1060 6GB

## Features
### Voxels
- Capable of rendering 10+ million blocks at 500+ FPS (when not loading chunks)
- Infinite size world
- Procedurally generated terrain including hills, mountains, plains, rivers, etc.
- A variety of biomes, each with their own unique properties and features
- Meandering natural tunnels and caves
- Block picking (destroying and placing)
- A prefab editor
- Marching cubes implementation for smooth voxels (in worlds defined by a density field):
- Basic movement with terrain collision

### Graphics
- Phong illumination model
- Frustum culling of chunks
- Baked ambient occlusion on blocks
- Realistic water effects
- Directional environment lighting
- Deferred rendering pipeline
- Post processing effects
- (*WIP*) Ray traced screen space water reflections
- (*WIP*) Cascaded shadow maps

### Other
- Portability. Uses (to my knowledge) no platform dependent libraries or headers. (Certain libraries would have to be rebuilt for platforms other than x64 Windows)
- Multithreaded mesh building and (*WIP*) terrain generation (which can be enabled via a preprocessor flag in chunk.h).
- Graphics effects can be toggled dynamically by the user.


## In-Engine
### Controls
- WASD for camera movement, mouse for looking.
- \` (Grave accent) will toggle the mouse cursor so the user can interact with screen elements.
- Mouse 1 (LMB) will remove the currently highlighted block on the screen.
- Mouse 2 (RMB) will place a block of the type currently shown rotating on the bottom half of the screen.
- Scrolling up or down will change the active block to place.
- Left Shift will increase camera speed by 10 times.
- Left Control will slow camera speed to 1/10th.

#### Prefab editing
- Tab will toggle the prefab editor menu.
  - The most recently highlighted block will be highlighted in purple instead of white.
  - Pressing 'F' will select the block and begin a region.
    - Once three blocks have been selected, the region will be completed and will be encompassed by purple wireframe.
  - Pressing the "save" button in the prefab menu will save the current region under the name written in the adjacent text box.
  - Pressing the "load" button will generate a prefab of the given name in the adjacent box at the most recent highlighted purple position.
  - Toggling the prefab editor will reset the current region if a mistake has been made.

## Gallery
Hover to see detail:

![Image of distant terrain.](https://github.com/JuanDiegoMontoya/3D_Voxel_Engine/blob/master/Images/distance03.png "Distant terrain showcasing fog, reflections, and biomes.")
![Image of distant and near terrain.](https://github.com/JuanDiegoMontoya/3D_Voxel_Engine/blob/master/Images/distance02.png "Distant and near terrain showcasing shading and shadows.")
![Image of snowy cave.](https://github.com/JuanDiegoMontoya/3D_Voxel_Engine/blob/master/Images/snow_cave.png "Snow cave.")
![Image showing marched cubes example.](https://github.com/JuanDiegoMontoya/3D_Voxel_Engine/blob/master/Images/marched01.png "Marching cubes implementation with scalar field.")
