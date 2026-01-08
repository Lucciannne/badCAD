# badCAD Directory Structure

```
badCAD/
├── src/                    # Source code modules
│   ├── core/              # App startup, event loop, crash recovery (~5KB)
│   ├── geometry/          # OpenCASCADE wrappers, BRep ops, constraints (~15MB OCC)
│   ├── model/             # Part/Assembly/Feature data structures, serialization (~50KB)
│   ├── ui/                # Viewport, toolbars, panels, input handling (~10-30MB Qt)
│   ├── importexport/      # STEP/IGES/STL/OBJ readers/writers (~3MB)
│   ├── solver/            # 2D constraint solver, assembly mate solver (~200KB)
│   ├── render/            # Tessellation, OpenGL rendering, LOD, selection (~100KB)
│   ├── history/           # Undo/redo command stack, delta serialization (~20KB)
│   ├── plugins/           # Plugin loader, C ABI definitions (~10KB)
│   ├── utils/             # Logging, file I/O, JSON, math helpers (~50KB)
│   └── main.cpp           # Application entry point
│
├── include/               # Public API headers
│   └── badcad/           # Public interface for badCAD library
│
├── tests/                 # Unit and integration tests
│   ├── core/             # test_crash_recovery.cpp
│   ├── geometry/         # test_extrude.cpp, test_boolean.cpp
│   ├── model/            # test_serialization.cpp
│   ├── ui/               # test_ui_visibility.cpp
│   ├── importexport/     # test_step_roundtrip.cpp
│   ├── solver/           # test_sketch_solver.cpp
│   ├── render/           # test_tessellation.cpp
│   ├── history/          # test_undo_redo.cpp
│   └── utils/            # test_json.cpp, test_zip.cpp
│
├── resources/             # Application assets
│   ├── fonts/            # TrueType/OpenType fonts for UI and dimension text
│   ├── icons/            # SVG/PNG icons for toolbars, buttons (sketch.svg, extrude.svg, etc.)
│   ├── shaders/          # GLSL shaders for OpenGL rendering (phong.vert, phong.frag, etc.)
│   └── ui/               # UI definition JSONs (part_editor.json, assembly_editor.json)
│
├── docs/                  # Documentation
│   ├── ARCHITECTURE.md   # Module diagram, data flow (to be generated)
│   └── API/              # Doxygen-generated API docs (output directory)
│
├── examples/              # Sample .bCAD files and usage examples
│   ├── simple_box.bCAD   # Basic extrude example
│   └── bracket.bCAD      # Fillet and boolean example
│
├── plugins/               # Plugin development
│   └── samples/          # sample_importer.c, badcad_plugin.h
│
├── third_party/           # External dependencies (if vendored)
│   └── (eigen, nlohmann_json, etc. - optional if using system packages)
│
├── build/                 # Build output (gitignored)
├── .vscode/              # VS Code configuration
│   └── settings.json     # CMake, compiler paths
├── .github/              # CI/CD workflows (to be created)
│   └── workflows/
│       └── build.yml
├── CMakeLists.txt        # Root build configuration
├── README.md             # Project overview, build instructions
├── SPECIFICATION.md      # Complete product specification
├── DIRECTORY_STRUCTURE.md # This file
├── .gitignore            # Git ignore rules
└── setup-env.ps1         # Environment setup script for Windows
```

## Directory Purposes

### Source Modules (`src/`)
Each subdirectory represents a module from the specification:
- **core**: Application lifecycle, main event loop
- **geometry**: All OpenCASCADE operations (extrude, revolve, booleans, fillets)
- **model**: Data structures for Parts, Assemblies, Features, Sketches
- **ui**: Qt/ImGui widgets, viewport, toolbars, panels
- **importexport**: File format handlers (STEP, IGES, STL, OBJ)
- **solver**: 2D sketch constraint solver, assembly mate solver
- **render**: OpenGL rendering pipeline, tessellation, LOD
- **history**: Command pattern implementation for undo/redo
- **plugins**: Plugin system, loader, C ABI definitions
- **utils**: Cross-cutting utilities (logging, JSON, file I/O)

### Resources (`resources/`)
- **fonts/**: Place UI fonts (e.g., Roboto, Inter) and monospace fonts for dimension text
  - Recommended: Include open-source fonts (Liberation Sans, Noto Sans) to avoid licensing issues
- **icons/**: All toolbar/button icons referenced in UI JSON definitions
  - Format: SVG preferred (scalable), PNG fallback (16x16, 24x24, 32x32)
  - Examples: sketch.svg, extrude.svg, revolve.svg, union.svg, cut.svg, fillet.svg, etc.
- **shaders/**: GLSL shaders for custom rendering
  - phong.vert/frag: Shaded mode
  - wireframe.vert/frag: Wireframe overlay
  - selection.frag: Selection highlighting
- **ui/**: JSON definitions for screen layouts (per spec §14)
  - part_editor.json, assembly_editor.json, home_screen.json

### Tests (`tests/`)
Each module has corresponding tests:
- Unit tests for geometry operations (volume, face count validation)
- File roundtrip tests (save → load → compare)
- UI visibility tests (toolbar shown/hidden per mode)
- Constraint solver correctness tests

### Documentation (`docs/`)
- Generated API documentation (Doxygen)
- Architecture diagrams
- User manual (future)

### Plugins (`plugins/`)
- Sample plugin implementations demonstrating the C ABI
- Plugin header files for third-party developers

### Build Artifacts (`build/`)
- Generated by CMake (gitignored)
- Contains compiled binaries, object files, etc.

## Adding New Content

**To add a font:**
1. Place .ttf or .otf file in `resources/fonts/`
2. Update font loading code in `src/ui/` to reference it
3. Ensure license allows redistribution (use OFL or Apache 2.0 fonts)

**To add an icon:**
1. Export SVG (24x24 base size, clean paths, no embedded raster)
2. Place in `resources/icons/`
3. Reference in UI JSON: `"icon": "icons/your_icon.svg"`
4. For PNG: provide @1x (24x24) and @2x (48x48) for high-DPI

**To add a shader:**
1. Create .vert and .frag pair in `resources/shaders/`
2. Load in `src/render/` using shader compilation pipeline
3. Use GLSL 330 core (compatible with OpenGL 3.3+)

**To add a module:**
1. Create `src/new_module/` and `tests/new_module/`
2. Add `add_subdirectory(src/new_module)` to root CMakeLists.txt
3. Update this document and ARCHITECTURE.md
