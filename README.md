# Net Ghost Engine (Linux)

## Python Build System [WIP]

```bash
python Tools/build.py
```

## bpy Blender Tooling [WIP]

```bash
python Tools/blender.py
```

## Features Implemented

#### Animated 3D models, Animation Blending, Particle System, Rigid Body Physics, Advanced Lighting, Directional/Point Shadows, Object Highlighting (Stencil Testing), Entity Component System, GUI, MultiThreaded Audio (OpenAL)

## Features To-Do

#### WASM build, Networking, Server build(s), TensorFlow in Blender w/ bpy

## Libraries Included

- GLEW & GLFW (OpenGL Software Implementation)
- OpenAL-soft (OpenAL Software Implementation)
- Bullet Physics
- FreeType
- GLM
- Assimp (Open Asset Import Library)
- Other Header Only C Libraries

## Controls

WASD: Camera movement / walk,
Space: Jump (when controlling rigid body),
Z: Lock camera to rigid body,
X: Lock/Unlock cursor for mouse panning,
O: toggle draw wires,
P: Emit smoke particles,
E: Select Physics Object,
LEFT/RIGHT ARROW KEYS: Decrease/Increase interpolation between animations,
F9: windowed,
F10: fullscreen
