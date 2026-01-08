# badCAD

A lean, fast-starting 3D parametric CAD application built with C++ and OpenGL.

## Current Status

**v0.1.0-alpha** - Initial UI implementation
- ✅ Self-contained binary with static linking (no system dependencies)
- ✅ ImGui-based UI (lightweight, cross-platform)
- ✅ Splash screen (1-second display)
- ✅ Home screen with navigation buttons
- ✅ Binary size: ~4.3 MB (target: <60 MB for MVP)

## Build Environment

### Installed Tools (Open Source)
- **MinGW-w64 GCC 15.2.0** - C++ compiler (via MSYS2)
- **CMake 4.2.1** - Build system generator
- **Ninja 1.13.2** - Fast build tool
- **Git 2.52.0** - Version control

### Libraries
- **GLFW 3.4** - Window and input handling (static)
- **ImGui 1.91.7** - Immediate-mode UI (vendored in third_party/)
- **OpenGL 3.3+** - Graphics rendering

### Future Dependencies (not yet integrated)
- **OpenCASCADE 7.8+** - Geometry kernel (~15MB)
- **Eigen3** - Linear algebra for constraint solver
- **STB Image** - Image loading for icons/splash

## Building

```bash
# Configure the project
cmake -B build -G Ninja

# Build
cmake --build build

# Run
./build/src/badCAD.exe
```

## Development

All tools are installed via MSYS2/MinGW64. The PATH is configured in `.vscode/settings.json` to use:
`C:\msys64\mingw64\bin`

### Static Linking
The application uses static linking for all dependencies to ensure portability:
- No runtime dependencies on system libraries (GTK, Qt, etc.)
- Fonts and UI resources will be embedded/bundled
- OpenGL is the only system dependency (universally available)

## Project Structure
```
badCAD/
├── src/                   # Source code modules
│   ├── core/             # App startup, crash recovery
│   ├── geometry/         # OpenCASCADE wrappers (TODO)
│   ├── model/            # Part/Assembly data structures (TODO)
│   ├── ui/               # ImGui UI implementation
│   │   ├── application.cpp   # Main app loop
│   │   ├── splash_screen.cpp # Splash screen
│   │   └── home_screen.cpp   # Home screen
│   ├── render/           # OpenGL rendering (TODO)
│   └── main.cpp          # Entry point
├── resources/            # Application assets
│   ├── fonts/           # TrueType fonts (to be added)
│   ├── icons/           # SVG/PNG UI icons (to be added)
│   └── ui/              # UI definition JSONs
├── third_party/         # Vendored dependencies
│   └── imgui/           # Dear ImGui library
├── build/               # Build output (gitignored)
├── CMakeLists.txt       # Root build configuration
├── SPECIFICATION.md     # Complete product specification
└── README.md            # This file
```

## Next Steps (MVP Roadmap)

### Immediate (Next Session)
- [ ] Integrate OpenCASCADE geometry kernel
- [ ] Implement basic Part Editor viewport
- [ ] Add simple sketch creation (line, circle)
- [ ] Implement extrude operation

### 3-Month MVP
- [ ] Full sketch constraints (coincident, dimension)
- [ ] Boolean operations (union, cut)
- [ ] Parametric feature tree
- [ ] STEP import/export
- [ ] Undo/redo (20 levels)
- [ ] File save/load (.bCAD format)

See [SPECIFICATION.md](SPECIFICATION.md) for complete roadmap and acceptance criteria.
