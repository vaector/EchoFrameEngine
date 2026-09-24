# Echo Frame Engine

> A custom **C++ 2D/3D game engine and editor** built with **Raylib 5.5**, designed around sector-based world building, real-time editing, and experimental game development.

Echo Frame Engine is a custom-built game engine focused on creating retro-inspired **Doom/Quake-style 3D environments** while providing the flexibility of a modern editor workflow.

The engine combines a sector-based level system, 3D rendering, custom tools, model and animation support, lighting, audio, scripting-oriented systems, and an in-engine editor into a single project.

---

## ✨ Features

### 🗺️ Sector-Based World Editor

Build 3D environments by drawing and editing 2D sector geometry directly inside the editor.

* 2D polygon-based level creation
* Automatic 2D → 3D sector conversion
* Floor and ceiling generation
* Arbitrary sector shapes
* Polygon triangulation using **Mapbox Earcut**
* Adjustable floor and ceiling heights
* Sector-based architecture inspired by classic FPS engines
* Real-time editing and preview

This approach makes it possible to rapidly prototype environments without manually creating every wall and surface in a traditional 3D modelling package.

---

### 🧱 3D Environment System

The engine converts sector geometry into explorable 3D environments.

* 3D walls
* Floors and ceilings
* Variable sector heights
* Ramps
* Doors
* Environmental geometry
* Collision geometry
* Texture mapping
* Runtime level rendering

The goal is to combine the simplicity of classic sector-based engines with a more flexible modern rendering pipeline.

---

### 🎨 Rendering

Built on top of **Raylib 5.5**, with additional custom rendering systems.

* Hardware-accelerated 3D rendering
* Textured geometry
* Custom GLSL shaders
* Dynamic lighting
* Point lights
* Directional lights
* Spotlights
* Material-based rendering experiments
* Runtime shader support

---

### 💡 Dynamic Lighting

The engine includes a custom lighting system designed for atmospheric environments.

Supported lighting concepts include:

* Point lights
* Directional lights
* Spotlights
* Light range
* Light intensity
* Light color
* Shader-based lighting
* Real-time editor manipulation

The lighting system is particularly useful for horror and atmospheric environments where lighting is an important gameplay and visual element.

---

### 🚪 Interactive World Objects

The engine is being designed around interactive environments rather than static geometry.

Planned and implemented systems include:

* Doors
* Ramps
* NPCs
* Interactive objects
* Collision
* Trigger-based interactions
* Animated objects

---

### 🧍 3D Models & Animation

The engine supports importing and working with external 3D assets.

* GLB/GLTF model support
* Animated 3D characters
* Skeletal animation
* Mixamo-compatible character workflows
* Animation playback
* Multiple animation clips
* Runtime model rendering

This allows characters and other animated assets to coexist with the sector-based environment system.

---

### 🎬 Cutscenes

A custom cutscene system is being developed for scripted sequences.

Potential uses include:

* Camera movement
* Character animation
* Dialogue
* Story sequences
* Environmental events
* Horror sequences
* Timeline-based events

---

### 💬 Dialogue System

The engine is designed to support an **Undertale-inspired dialogue system**.

Features being developed include:

* Dialogue boxes
* Character names
* Sequential dialogue
* Branching conversations
* Events triggered by dialogue
* Character interaction
* Story-driven sequences

---

### 🔊 Audio

The engine includes an audio system for both gameplay and environmental sound.

Planned/implemented functionality includes:

* Sound effects
* Background music
* Environmental audio
* Positional audio
* Event-triggered sounds
* Audio playback through gameplay systems

---

### 💾 JSON-Based Data

Game and engine data can be stored using JSON.

This allows systems such as:

* Level data
* Object properties
* Game configuration
* Dialogue
* Entity information
* Editor settings

to be saved and loaded without hard-coding everything directly into C++.

---

### 🖥️ Custom Editor

The engine includes an integrated editor built using **Dear ImGui** and **rlImGui**.

The editor is intended to provide direct control over the game world without constantly switching between external tools.

Editor functionality includes:

* Scene editing
* Sector creation
* Geometry manipulation
* Object placement
* Lighting controls
* Model placement
* Transform editing
* Real-time previews
* Property editing
* Level management

---

## 🛠️ Technology Stack

| Technology        | Purpose                                                        |
| ----------------- | -------------------------------------------------------------- |
| **C++**           | Core engine language                                           |
| **Raylib 5.5**    | Rendering, windowing, input, audio and core game functionality |
| **Dear ImGui**    | Editor UI                                                      |
| **rlImGui**       | Raylib ↔ ImGui integration                                     |
| **GLSL**          | Custom shaders and rendering                                   |
| **Mapbox Earcut** | Polygon triangulation                                          |
| **JSON**          | Game and editor data                                           |
| **GLB / GLTF**    | 3D model assets                                                |
| **Mixamo**        | Character and animation assets                                 |

---

## 🏗️ Engine Architecture

The engine is being developed as a combination of several interconnected systems:

```text
                    ┌─────────────────────┐
                    │    Echo Frame       │
                    │       Engine        │
                    └──────────┬──────────┘
                               │
          ┌────────────────────┼────────────────────┐
          │                    │                    │
          ▼                    ▼                    ▼
   ┌─────────────┐      ┌─────────────┐      ┌─────────────┐
   │    Editor   │      │   Renderer  │      │ Game Systems│
   └──────┬──────┘      └──────┬──────┘      └──────┬──────┘
          │                    │                    │
          ▼                    ▼                    ▼
      Sectors               GLSL                  Audio
      Objects               Lighting              Dialogue
      Entities              Materials             Cutscenes
      Properties            Models                NPCs
          │                    │                    │
          └────────────────────┼────────────────────┘
                               │
                               ▼
                        ┌─────────────┐
                        │   Runtime   │
                        └─────────────┘
```

---

## 🎮 Design Philosophy

Echo Frame Engine is built around a simple idea:

> **Make it possible to go from an idea to a playable 3D environment without leaving the engine.**

Instead of relying entirely on external level editors and modelling software, the engine aims to provide a workflow where environments, objects, lighting, dialogue, characters, and gameplay systems can be assembled directly inside the editor.

The sector-based approach also makes rapid prototyping possible while maintaining the visual language of classic FPS engines.

---

## 🧪 Current Experimental Project

The engine is currently being tested through a psychological-horror prototype.

### Ethan Cole

The prototype follows **Ethan Cole**, a young man returning to a distorted version of a childhood town.

The environment gradually shifts from a familiar abandoned town into something increasingly surreal, with the player's perception of the world changing as the story progresses.

The prototype is intended to test several engine systems simultaneously:

* Sector-based environments
* Dynamic lighting
* Atmospheric rendering
* NPCs
* Dialogue
* Cutscenes
* Character animation
* Audio
* Environmental events
* Time-of-day transitions
* Horror-focused gameplay

The project serves as a practical stress test for the engine rather than simply being a standalone game.

---

## 📸 Screenshots/Videos

<img width="1666" height="998" alt="image2" src="https://github.com/user-attachments/assets/76b9a639-3f72-4ae9-89f5-97f61829338c" />
<img width="1603" height="942" alt="image1" src="https://github.com/user-attachments/assets/c8d2e79d-4263-44e4-8650-2d35d7ef0582" />


https://github.com/user-attachments/assets/92791907-45de-49be-b06a-7e5c5de51635



https://github.com/user-attachments/assets/a4a64d3c-77a2-4ede-956e-295777f18ef8



https://github.com/user-attachments/assets/3084f3e3-5887-4ee3-9db9-666e2671de07

<img width="1674" height="1016" alt="Lights" src="https://github.com/user-attachments/assets/bfd68a9f-3d78-4944-8ab8-2bd58b65ade2" />
<img width="1668" height="1016" alt="TexturesAndLights" src="https://github.com/user-attachments/assets/28b8b566-8837-4e63-b979-02f908113b71" />


---

## 🚧 Roadmap

### Core

* [x] Raylib rendering foundation
* [x] C++ engine architecture
* [x] ImGui editor
* [x] Sector-based geometry
* [x] Polygon triangulation
* [x] 2D → 3D environment generation
* [x] Basic collision
* [x] JSON data handling

### Rendering

* [x] Textured geometry
* [x] GLSL shaders
* [x] Basic dynamic lighting
* [ ] Advanced lighting
* [ ] Improved shadow system
* [ ] Material system
* [ ] Post-processing

### World

* [x] Sectors
* [x] Floors
* [x] Ceilings
* [x] Walls
* [x] Basic doors
* [x] Ramps
* [ ] More interactive objects
* [ ] Improved entity system

### Characters

* [x] 3D model loading
* [x] Skeletal animation support
* [x] Mixamo workflow
* [ ] Improved animation state system
* [ ] Character controller
* [ ] NPC behavior system

### Gameplay

* [ ] Dialogue system
* [ ] Event system
* [ ] Trigger system
* [ ] Inventory
* [ ] Save/load system
* [ ] More advanced scripting

### Cinematics

* [ ] Cutscene timeline
* [ ] Camera tracks
* [ ] Animation sequencing
* [ ] Dialogue integration
* [ ] Cinematic events

---

## 📁 Project Structure

The exact structure is still evolving, but the engine is generally organized around systems such as:

```text
EchoFrame/
├── Engine/
│   ├── Core/
│   ├── Renderer/
│   ├── World/
│   ├── Entities/
│   ├── Audio/
│   ├── Animation/
│   └── Gameplay/
│
├── Editor/
│   ├── UI/
│   ├── Scene/
│   ├── Sector/
│   └── Properties/
│
├── Assets/
│   ├── Models/
│   ├── Textures/
│   ├── Audio/
│   └── Shaders/
│
├── Games/
│   └── Prototype/
│
└── README.md
```

---

## 🚀 Goals

The long-term goal of Echo Frame Engine is to become a lightweight, flexible engine capable of handling complete small-to-medium-scale games while retaining the simplicity and immediacy of classic game development tools.

The engine is particularly focused on:

* Retro FPS-style environments
* Horror games
* Atmospheric exploration
* Story-driven games
* Rapid prototyping
* Custom gameplay systems
* Experimental rendering
* Learning engine architecture

---

## 📜 Status

**Early Development / Experimental**

Echo Frame Engine is a personal engine project and is under active development. APIs, systems, file formats, and editor workflows may change significantly as development continues.

---

## 👤 Author

**Echo Frame**

Built from scratch in C++ with Raylib.

---

## 📄 License

License information will be added as the project develops.
