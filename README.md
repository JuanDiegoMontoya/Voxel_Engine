# 3D_Voxel_Engine
Yet another voxel engine.

This is a tech demo designed to demonstrate the implementation of various rendering techniques in a large and dynamic environment.
(*WIP*) indicates that a feature is mostly complete but has a few bugs that need to be smoothed out. 

Performance was tested on this system for reference:
- AMD Ryzen 5 2600X Processor
- 16 GB RAM
- NVIDIA GeForce GTX 1060 6GB

## Features
### Voxels
- Capable of rendering 10+ million blocks with 144+ FPS
- Infinite size world
- Interesting terrain including hills, mountains, plains, rivers, etc.
- Block picking (destroying and placing)
- (*WIP*) Marching cubes implementation for smooth voxels (in worlds defined by a density field)

### Graphics
- Chunk frustum culling
- Baked ambient occlusion on blocks
- Realistic water effects
- Direction environment lighting
- (*WIP*) Deferred rendering pipeline
- (*WIP*) Ray traced screen space water reflections
- (*WIP*) Cascaded shadow maps
