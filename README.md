# ToyPixelSim

A little toy about pixel simulation written in C++ & OpenGL. Currently, it can simulate sand, water and static wood. This is a very immature project, but sufficient for my coursework.

[Demo](https://www.youtube.com/watch?v=tzqdVu-Q3sk) on YouTube.

Inspired by [SandSim](https://github.com/GameEngineering/EP01_SandSim).

## Usage

Only tested and run on Linux, haven't adapted it for Windows / MacOS.

- `F1` Water
- `F2` Sand
- `F3` Wood
- `F4` Eraser
- `MouseButton` Draw / Erase

## Dependencies

- gcc (C++20 required)
- antient OpenGL (2.0)
- glfw
- glm
- CMake

## Todo

- Tidy the messy code, make it OOP. 
- Unify the behavioral motivations of all particles, reduce ad-hocs.
- Simulate based on frame rate, not a fixed number.
- Add more types of particles.
- Heading towards modern OpenGL.
- Make it parallel.