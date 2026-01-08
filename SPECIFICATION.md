# badCAD Product Specification

## 1. Product Pitch

badCAD is a lean, fast-starting 3D parametric CAD application for mechanical part design and assembly, built on OpenCASCADE with ruthless attention to binary size (<50MB), memory efficiency (<200MB idle), and sub-2-second cold starts. It targets solo engineers and small teams who need STEP/IGES interop, basic parametric sketching, part/assembly workflows, and fast iteration without the bloat of commercial CAD suites. Trade-off: we sacrifice advanced surfacing, sheet metal, and simulation to stay compact and predictable.

## 2. High-Level Feature List

- **Parametric sketching**: 2D constraint solver, basic shapes (line, arc, circle, spline), dimensions, coincident/tangent/perpendicular/parallel constraints
- **Solid modeling**: Extrude, revolve, sweep, loft; boolean ops (union, cut, intersection); fillet, chamfer
- **Assemblies**: Multi-part documents, mate constraints (coincident, concentric, distance, angle), bottom-up assembly
- **Import/Export**: STEP AP203/214, IGES 5.3, STL, OBJ (mesh export only)
- **Parametric feature tree**: Ordered history with editable parameters; drag-to-reorder; rollback
- **Lightweight rendering**: OpenGL 3.3+ with adaptive tessellation; selection highlighting; wireframe/shaded/hidden-line modes
- **Undo/redo**: 50-level history; incremental feature recompute
- **Crash recovery**: Autosave every 5min; recover last session on startup if dirty
- **Plugin API**: Minimal C ABI for custom importers/exporters; no scripting in MVP
- **Cross-platform**: Windows, Linux, macOS (same binary size budget)

**Deliberately omitted (to keep small):**
- Sheet metal, surfacing, FEA, CAM toolpaths, advanced curvature analysis, rendering/animation, Python scripting, cloud sync

## 3. Detailed UI/UX Specification

### 3.1 Home Screen

**Layout:**
- Top toolbar: `New Part`, `New Assembly`, `Open`, recent files dropdown (max 10)
- Center: large welcome pane with recent files grid (thumbnails if cached, else file icons)
- Bottom status bar: version, active OpenCASCADE version

**Visible Controls:**
- `New Part` → opens blank part in Part Editor
- `New Assembly` → opens blank assembly in Assembly Editor
- `Open` → file picker (`.bCAD`, `.step`, `.iges`, `.stp`, `.igs`)
- Recent files: click to open, right-click for "Remove from list"

**Default Action:** If no file open, show home screen. If last session dirty, show recovery dialog first.

### 3.2 Part Editor

**Modes:**
- **Sketch Mode**: 2D constraint sketching on a selected plane/face
- **Model Mode**: 3D feature ops (extrude, revolve, booleans)
- **Inspect Mode**: read-only measure/analyze

**Toolbars:**
- **Sketch Toolbar** (visible only in Sketch Mode): `Line`, `Arc`, `Circle`, `Rectangle`, `Spline`, `Coincident`, `Tangent`, `Perpendicular`, `Parallel`, `Dimension`, `Exit Sketch`
- **Model Toolbar** (visible in Model Mode): `New Sketch`, `Extrude`, `Revolve`, `Boolean Union`, `Boolean Cut`, `Boolean Intersect`, `Fillet`, `Chamfer`, `Shell`
- **View Toolbar** (always visible): `Fit All`, `Front/Top/Right/Iso views`, `Wireframe`, `Shaded`, `Hidden Line`, `View Cube toggle`

**Context Menus:**
- Right-click on face: `New Sketch on Face`, `Delete Face` (if safe), `Measure`
- Right-click on edge: `Fillet`, `Chamfer`, `Measure`
- Right-click on feature in tree: `Edit Parameters`, `Suppress`, `Delete`, `Reorder`

**Inspector Panels** (dockable right sidebar):
- **Properties**: selected entity type, dimensions, parent feature
- **Parameters**: live editable values for active feature (e.g., extrude depth, fillet radius)
- **Constraints** (Sketch Mode only): list of active constraints with status (satisfied/over-constrained)

### 3.3 Assembly Editor

**Modes:**
- **Assembly Mode**: place/mate components
- **Part Edit Mode**: edit a part in-context (opens Part Editor for selected component, assembly visible as ghost)

**Toolbars:**
- **Assembly Toolbar**: `Insert Part`, `Mate: Coincident`, `Mate: Concentric`, `Mate: Distance`, `Mate: Angle`, `Fix Component`, `Pattern: Linear`, `Pattern: Circular`, `Exploded View`
- **View Toolbar**: same as Part Editor

**Context Menus:**
- Right-click on component: `Edit In-Context`, `Replace Component`, `Make Flexible` (allow motion), `Suppress`, `Delete`
- Right-click on mate: `Edit`, `Suppress`, `Delete`

**BOM Panel** (dockable bottom):
- Table: Part Name | Qty | File Path | Mass (if known)
- Export BOM as CSV

**Exploded View:**
- Slider: 0% (assembled) → 100% (max explode along mate normals)
- Auto-generate arrows showing explode direction (optional, toggleable)

### 3.4 Sketcher

**Constraints:**
- Geometric: Coincident, Tangent, Perpendicular, Parallel, Horizontal, Vertical, Equal, Symmetric, Midpoint
- Dimensional: Distance, Angle, Radius, Diameter

**Hints:**
- Live inference while drawing (cyan ghost lines for horizontal/vertical/tangent)
- Highlight over-constrained entities in red; under-constrained in orange; fully constrained in black
- Small constraint icons appear near entities (e.g., padlock for fixed, `⊥` for perpendicular)

**Visibility Rules:**
- Construction geometry (dashed blue lines) never exported to 3D, used only for constraints
- Constraints auto-hide when zoomed out beyond readable scale
- Dimension text scales with zoom but clamps to min/max size

### 3.5 Parametric Feature Tree

**Appearance:**
- Tree widget (left sidebar): hierarchical
  - `Sketches/` folder → individual sketches (e.g., `Sketch001`, `Sketch002`)
  - `Features/` folder → ordered list (e.g., `Extrude001`, `Fillet002`)
  - Each feature shows icon + name + key param (e.g., `Extrude001 (50mm)`)
- Active feature highlighted in blue; suppressed features grayed with strikethrough

**Reorder Rules:**
- Drag feature up/down in tree; blocked if move would break parent-child dependency (show error tooltip)
- On successful reorder, recompute all downstream features

**Param Edit:**
- Double-click feature → opens inline edit for primary param, or modal dialog for complex features
- Changes trigger incremental recompute (only affected features + children)

### 3.6 Viewport and Navigation

**Controls:**
- **Orbit**: Middle mouse drag (or Alt+LMB drag)
- **Pan**: Shift+MMB drag (or Shift+RMB drag)
- **Zoom**: Mouse wheel (or Ctrl+RMB drag)
- **Selection**: LMB click; Ctrl+LMB for multi-select; box-select with LMB drag (no modifier)

**View Cube:**
- Fixed top-right corner, 3D cube showing orientation; click face/edge/corner to snap camera
- Toggleable via View menu

**Selection Highlighting:**
- Hover: entity glows yellow
- Selected: entity outlined in orange (thick wireframe overlay)
- Multi-select: all selected entities orange, last-selected brighter

**Home/Fit:**
- `F` key or toolbar button fits all visible geometry to viewport

### 3.7 Render vs. Fast Preview Modes

**Modes:**
- **Fast Preview** (default on drag/orbit): lower tessellation, skip edge highlighting, 30 FPS cap
- **High Quality** (on release/idle >200ms): high tessellation, anti-aliased edges, shadows optional

**Tessellation Strategy:**
- Compute tessellation on first display; cache in feature metadata
- Adaptive: curved surfaces get higher density than planar faces
- LOD: if viewport shows part <10% screen, drop to low-poly cached mesh
- On param edit: only re-tessellate changed features + dependents; reuse cache for unchanged

**Warning:** Large assemblies (>100 parts) will degrade; consider assembly-level LOD or culling in stretch goals.

### 3.8 UI Visibility Matrix

| **Mode**       | **State**               | **Visible Toolbars**                          | **Visible Panels**         | **Special Buttons**                |
|----------------|-------------------------|-----------------------------------------------|----------------------------|------------------------------------|
| Home           | No file open            | Top bar (New/Open/Recent)                     | None                       | Recent files grid                  |
| Part Editor    | Model Mode, no selection| Model Toolbar, View Toolbar                   | Feature Tree               | -                                  |
| Part Editor    | Model Mode, face sel    | Model Toolbar, View Toolbar                   | Feature Tree, Properties   | Context menu on right-click        |
| Part Editor    | Sketch Mode             | Sketch Toolbar, View Toolbar                  | Feature Tree, Constraints  | Exit Sketch (prominent)            |
| Assembly Editor| Assembly Mode           | Assembly Toolbar, View Toolbar                | Feature Tree, BOM          | Exploded View slider               |
| Assembly Editor| Part Edit Mode          | Model Toolbar (subset), View Toolbar          | Feature Tree, Properties   | Exit Part Edit                     |
| Inspect Mode   | Any                     | View Toolbar only                             | Properties, Measure        | Measure tools, no edit actions     |

### 3.9 Keyboard Shortcuts

| **Action**           | **Shortcut**     | **Context**           |
|----------------------|------------------|-----------------------|
| New Part             | Ctrl+N           | Global                |
| Open                 | Ctrl+O           | Global                |
| Save                 | Ctrl+S           | Global                |
| Undo                 | Ctrl+Z           | Global                |
| Redo                 | Ctrl+Y / Ctrl+Shift+Z | Global           |
| Delete               | Del              | Selection active      |
| Fit All              | F                | Viewport active       |
| Wireframe mode       | W                | Viewport active       |
| Shaded mode          | S                | Viewport active       |
| Hidden line mode     | H                | Viewport active       |
| New Sketch           | K                | Part/Assembly Editor  |
| Extrude              | E                | Model Mode            |
| Exit Sketch          | Esc              | Sketch Mode           |
| Orbit                | MMB drag         | Viewport              |
| Pan                  | Shift+MMB        | Viewport              |
| Zoom                 | Wheel            | Viewport              |

## 4. Interaction Flows (6 Core Tasks)

### Flow 1: Create Sketch → Extrude → Boolean

1. User opens Part Editor (new part)
2. Click `New Sketch` → prompted to select plane or face (if none, default to XY)
3. Sketch Mode activates; user draws rectangle, dimensions it (e.g., 50×30mm)
4. Click `Exit Sketch`
5. With sketch selected in tree, click `Extrude` → modal dialog for depth (e.g., 20mm, "New Body")
6. System creates `Extrude001`, adds to tree, viewport shows 3D solid
7. Repeat steps 2-6 on a different face to create second solid
8. Select first body, click `Boolean Union` → click second body → bodies merge, tree shows `BooleanUnion001`

### Flow 2: Create Part from Primitives

1. User opens Part Editor
2. Click `New Sketch` on default plane, draw circle (radius 25mm), exit sketch
3. Click `Revolve` with sketch selected → specify axis (e.g., sketch X-axis), angle 360° → creates cylinder
4. Click `Fillet` → select top circular edge, enter radius 5mm → edge rounds
5. Result: rounded cylinder; feature tree shows `Sketch001`, `Revolve001`, `Fillet001`

### Flow 3: Constrain Two Parts in Assembly

1. User opens Assembly Editor (new assembly)
2. Click `Insert Part` → file picker for first part → part appears at origin, tree shows `Part1:1`
3. Click `Insert Part` again for second part → appears offset, `Part2:1` in tree
4. Click `Mate: Coincident` → click planar face on Part1 → click planar face on Part2 → faces align
5. Click `Mate: Concentric` → click cylindrical face on Part1 → click hole on Part2 → axes align
6. Assembly updates in viewport; parts snap together; mates listed under `Mates/` folder in tree

### Flow 4: Edit Dimension Param and Update Assembly

1. Assembly open with two parts constrained
2. In tree, expand `Part1:1` → expand `Features/` → double-click `Extrude001`
3. Inline edit changes depth from 20mm → 30mm, hit Enter
4. System recomputes Part1 geometry, propagates to assembly, re-solves mates
5. Viewport updates to show longer part; assembly mates remain satisfied (or show conflict icon if broken)

### Flow 5: Export STEP

1. File → `Export As...` → choose STEP format
2. Modal: "STEP AP214" or "AP203" radio buttons, "Include colors" checkbox
3. Click `Export` → background task writes file (progress bar if >1000 faces)
4. On completion, status bar shows "Exported to X.step"

### Flow 6: Undo/Redo and Crash Recovery

1. User performs several operations (sketch, extrude, fillet)
2. Mistake made → Ctrl+Z repeatedly to undo back to desired state
3. Redo with Ctrl+Y to reapply if needed
4. **Crash scenario:** App crashes mid-edit (simulated)
5. On restart, recovery dialog appears: "Recover unsaved work from [timestamp]?" with preview thumbnail
6. Click `Recover` → opens last autosaved state; status bar notes "(Recovered)"

## 5. Data Model & File Format Plan

### Internal Object Model

**Entities:**
- **Part**: container for Features, Sketches, Parameters, BRep (TopoDS_Shape)
  - Features: ordered list (Sketch, Extrude, Revolve, Boolean, Fillet, etc.)
  - Each Feature has: type, params (JSON blob), parent refs, resultShape (TopoDS_Shape or null if suppressed)
- **Assembly**: container for ComponentInstances, Mates
  - ComponentInstance: partRef (file path or embedded Part), transform (gp_Trsf), suppressed flag
  - Mate: type (coincident/concentric/distance/angle), entity refs (faces/edges), params
- **Sketch**: 2D entities (lines, arcs, circles, splines), constraints (geometric/dimensional), plane (gp_Pln)
- **Parameters**: name-value pairs (doubles, strings); support expressions (e.g., `width = 2 * height`)

### On-Disk Serialization

**Format:** Custom binary (`.bCAD`) with JSON manifest + binary BRep streams

**Structure:**
```
.bCAD file (zip archive):
  manifest.json          // part/assembly metadata, feature tree, params, UI state
  breps/
    feature_001.brep     // OpenCASCADE BRep binary (BRepTools::Write)
    feature_002.brep
  sketches/
    sketch_001.json      // 2D geometry + constraints as JSON
  thumbnails/
    preview.png          // 256x256 viewport capture for file browser
```

**Manifest JSON Schema (example):**
```json
{
  "version": "0.1",
  "type": "part",
  "parameters": {"height": 50.0, "width": 100.0},
  "features": [
    {"id": "f001", "type": "sketch", "name": "Sketch001", "plane": [0,0,0,0,0,1], "file": "sketches/sketch_001.json"},
    {"id": "f002", "type": "extrude", "name": "Extrude001", "parent": "f001", "depth": 20.0, "brep": "breps/feature_002.brep"}
  ]
}
```

**What to Persist vs. Reconstruct:**
- **Persist:** BReps for each feature (rebuild is slow, cache them)
- **Reconstruct:** Tessellations (geometry-dependent, not portable), display meshes, bounding boxes
- **Persist Sketches as JSON:** easier to diff, edit externally if needed; reconstruct TopoDS_Wire on load

**Versioning:** Major.minor in manifest; backward compat for minor bumps; reject future major versions

### Import/Export Strategy

**STEP (AP203/214):**
- Import: STEPControl_Reader → single TopoDS_Compound → optionally split into parts by labels
- Export: Iterate BReps, STEPControl_Writer, preserve colors via StepData metadata
- **Caveat:** Complex assemblies may lose mate info (STEP AP214 has limited constraint semantics); import as individual parts + transform only

**IGES 5.3:**
- Import/export via IGESControl classes
- **Caveat:** IGES less reliable than STEP for solids; prefer STEP for roundtrip

**STL:**
- Export only: tessellate BRep → write binary STL
- **Caveat:** No re-import to solid (mesh-only); warn user

**OBJ:**
- Export only: tessellate → write OBJ with normals
- **Caveat:** No material/texture support in MVP

**Warning:** STEP import of poorly-formed files (CAD vendor quirks) often fails silently or produces garbage; add validation pass to detect open shells, invalid topology.

## 6. Architecture & Modules (C++)

### Module Breakdown

| **Module**       | **Responsibility**                                      | **Dependencies**                  | **Size Impact** |
|------------------|---------------------------------------------------------|-----------------------------------|-----------------|
| `core`           | App startup, event loop, crash recovery                 | OS APIs, `utils`                  | ~5KB            |
| `geometry`       | Wrap OCC: BRep ops, sketching, booleans, constraints    | OpenCASCADE (selective)           | ~15MB (OCC)     |
| `model`          | Part/Assembly/Feature data structures, serialization    | `geometry`, `utils`, JSON lib     | ~50KB           |
| `ui`             | Viewport, toolbars, panels, input handling              | OpenGL, UI framework (Qt/custom)  | ~10-30MB (Qt)   |
| `importexport`   | STEP/IGES/STL/OBJ readers/writers                       | `geometry`, OCC exchange modules  | ~3MB            |
| `solver`         | 2D constraint solver (sketch), assembly mate solver     | Eigen (minimal)                   | ~200KB          |
| `render`         | Tessellation, OpenGL rendering, LOD, selection          | OpenGL, `geometry`                | ~100KB          |
| `history`        | Undo/redo command stack, delta serialization            | `model`                           | ~20KB           |
| `plugins`        | C ABI plugin loader (optional importer/exporter DLLs)   | OS DLL APIs                       | ~10KB           |
| `utils`          | Logging, file I/O, JSON parse, math helpers             | std, zlib (for .bCAD zip)         | ~50KB           |

**Total Estimated Binary (static link, stripped):** ~35-55MB depending on Qt vs. lightweight UI

### Minimal External Dependencies

- **OpenCASCADE:** 7.8+ (mandatory; selective module linking, see §9)
- **UI Framework:** Options:
  - **Qt6 (minimal modules):** QtCore, QtGui, QtWidgets, QtOpenGL (~30MB static; familiar, well-documented)
  - **Dear ImGui + GLFW + glad:** (~500KB; immediate-mode, less conventional for CAD but tiny; requires custom docking/layout)
  - **Recommendation:** Qt6 minimal for MVP (trade binary size for dev speed), ImGui for "lite" variant if size critical
- **JSON:** nlohmann/json (header-only, ~100KB compiled)
- **Eigen3:** Minimal sparse/dense matrix for constraint solver (~200KB)
- **zlib:** For .bCAD archive compression (~50KB)

**Build System:** CMake 3.20+; Ninja for fast incremental builds

**Plugin API Surface:**
```c
// C ABI (stable across C++ ABI changes)
typedef struct ImporterContext ImporterContext;
typedef int (*ImportFunc)(ImporterContext* ctx, const char* filepath, TopoDS_Shape* outShape);
typedef int (*ExportFunc)(const TopoDS_Shape* shape, const char* filepath);

// Plugin DLL exports: badcad_plugin_get_importer(), badcad_plugin_get_exporter()
```

### Keeping Binary Size & Memory Down

**Selective Linking:**
- Only link OCC modules actually used (avoid ModelingAlgorithms surfacing, avoid DataExchange VRML/glTF unless needed)
- Strip debug symbols in release builds (`-s` linker flag)
- Use `-ffunction-sections -fdata-sections` + `-Wl,--gc-sections` (GCC/Clang) to dead-code eliminate unused OCC classes

**Optional Modules:**
- Ship plugins as separate DLLs (e.g., IGES importer optional); lazy-load only if user opens .iges file
- Advanced surfacing (loft, sweep) as optional plugin if not in MVP

**Lazy Init:**
- Don't instantiate OCC sessions until first geometry operation
- Load tessellation caches only when viewport opens
- Defer BOM/mass calculations until panel accessed

**Memory:**
- Release BRep data for suppressed features; keep only JSON params
- Limit undo stack to 50 commands; prune oldest
- Use OCC Handle<> smart pointers; avoid memory leaks in OCC (common pitfall: dangling TopoDS references)

## 7. OpenCASCADE Integration Specifics

### OCC Modules to Use (Minimal Footprint)

**Include:**
- **TKernel, TKMath:** Core foundation (mandatory)
- **TKG2d, TKG3d, TKGeomBase:** 2D/3D geometry primitives
- **TKBRep:** Boundary representation (mandatory)
- **TKTopAlgo, TKPrim:** Boolean ops, primitives (box, cylinder, etc.)
- **TKFillet:** Fillet/chamfer
- **TKBO:** Boolean operations
- **TKMesh:** Tessellation (BRepMesh_IncrementalMesh)
- **TKSTEP, TKIGES:** Import/export (optional as plugins)
- **TKOffset:** Shelling (if needed)

**Avoid (to save ~10-20MB):**
- **TKSTEPBase, TKSTL, TKVRML, TKRWMesh:** unless explicitly needed (load as plugins)
- **TKDraw, TKDCAF:** OCC's GUI/application framework (we roll our own)
- **Surfacing modules (TKGeomAlgo advanced):** unless stretch goals include complex surfaces
- **TKHLR, TKOpenGl (OCC's built-in 3D viewer):** we use custom OpenGL renderer

**Why:** OCC is modular but defaults to linking everything; explicit module selection in CMakeLists.txt cuts binary by 30-40%.

### Tessellation, BRep Management, Topology Caching

**Tessellation:**
- Use `BRepMesh_IncrementalMesh` with adaptive deflection (e.g., 0.1mm for high-quality, 1mm for preview)
- Cache triangulation in `TopoDS_Face` user data; check `BRep_Tool::Triangulation()` before re-meshing
- On param change: only re-mesh affected faces (walk topology graph from changed feature)

**BRep Management:**
- Each feature stores its `TopoDS_Shape`; avoid rebuilding entire part on every edit
- Use `BRepBuilderAPI_Copy` to clone shapes when needed (avoids shared topology bugs)
- Explicitly call `BRepTools::Clean()` on shapes before deletion to free memory

**Topology Caching:**
- Cache TopExp_Explorer results (e.g., list of faces/edges) in feature metadata; invalidate on geometry change
- **Pitfall:** OCC shares topology (edges/vertices) between faces; modifying one face can affect another; always operate on copies or use `BRepBuilderAPI_MakeShape::Modified()` to track changes

**Common Pitfalls:**
- **Memory leaks:** Forgetting to release `Handle<Standard_Transient>` refs; use `Handle<>::Nullify()` explicitly
- **Invalid topology after boolean:** Boolean ops can produce degenerate edges/faces; validate with `BRepCheck_Analyzer`; discard bad results, revert to pre-op state
- **Thread safety:** OCC is NOT thread-safe; all OCC calls must be on main thread or explicitly locked; use mutex around any BRepBuilderAPI calls

### Threading Model

**Recommended:**
- **Single-threaded for OCC ops:** All geometry modeling on main UI thread
- **Background threads for:** File I/O, tessellation (if isolated), thumbnail generation
- **Warning:** Tessellation (`BRepMesh`) is barely thread-safe; wrap in mutex or defer to main thread

**Safe Use:**
```cpp
// Good: wrap OCC op in mutex
std::lock_guard<std::mutex> lock(g_occMutex);
BRepAlgoAPI_Fuse boolOp(shape1, shape2);
TopoDS_Shape result = boolOp.Shape();

// Bad: accessing same TopoDS_Shape from multiple threads without lock → corruption
```

## 8. Rendering & Performance

### Small GPU/CPU Rendering Pipeline

**Choice: OpenGL 3.3+** (widely supported, smaller driver footprint than Vulkan; defer Vulkan to stretch goals)

**Pipeline:**
1. **Geometry Upload:**
   - On first display: tessellate BRep → extract vertex/normal/index buffers → upload to VBOs
   - Cache VBOs per feature; tag with feature version hash to detect stale data
2. **Rendering:**
   - **Shaded Mode:** Draw triangles with simple Phong shading (1 directional light + ambient)
   - **Wireframe:** Draw edges (use `BRepTools::UVBounds` to extract edge polylines)
   - **Hidden Line:** Render solid to depth buffer (no color), then overlay wireframe
3. **Selection:**
   - Render scene to offscreen FBO with unique color per entity (color-picking)
   - On mouse click, read pixel color → map to entity ID
   - Highlight: render selected entity with thick wireframe overlay in orange

**LOD:**
- Compute 3 tessellation levels per part: low (fast preview), medium (normal), high (export)
- Switch based on viewport coverage: <5% screen → low, 5-50% → medium, >50% → high

**Progressive Tessellation:**
- On file open: tessellate in background thread (non-OCC method: pre-compute on save)
- Display bounding box wireframe until tessellation ready
- **Warning:** Tessellating 1000+ faces can take 5-10s; show progress bar, don't block UI

**Viewport Caching:**
- Cache rendered framebuffer on idle; reuse if camera/scene unchanged
- Invalidate on: geometry change, selection change, camera move

**Memory Budget:**
- Target: <200MB idle (empty part), <500MB with medium assembly (50 parts, 10K faces total)
- Monitor with `glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE)` to track VRAM usage

**Startup Budget:**
- **Target: <1.5s cold start** (SSD, modern CPU)
  - 200ms: process launch, DLL load
  - 300ms: Qt/UI init
  - 500ms: OCC session init
  - 300ms: home screen render, recent files scan
  - 200ms: buffer
- **Tactics:** Delay-load OCC DLLs until first geometry op; lazy-init UI panels (don't create hidden panels)

## 9. Undo/Redo, History, and Parametric Update Semantics

### Granularity

- **Command Pattern:** Each user action = one Command object (e.g., `ExtrudeCommand`, `SetParameterCommand`)
- **Atomic:** Command encapsulates all changes (geometry + metadata); undo must fully revert

### Command Pattern Implementation

```cpp
class Command {
public:
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string describe() = 0; // for UI display
};

class ExtrudeCommand : public Command {
    Sketch* sketch;
    double depth;
    TopoDS_Shape oldShape, newShape;
public:
    void execute() override {
        oldShape = part->getShape();
        newShape = performExtrude(sketch, depth);
        part->setShape(newShape);
    }
    void undo() override {
        part->setShape(oldShape);
    }
};
```

### Feature Recompute Strategies

**Incremental Recompute:**
- On param edit: mark feature + all downstream features dirty
- Traverse tree in order; rebuild only dirty features
- **Optimistic:** Assume topology unchanged (e.g., changing extrude depth 20→25mm); reuse cached references
- **Transactional:** If rebuild fails (e.g., boolean op fails), revert feature to old state, show error, mark feature as "failed" in tree

**Dependency Tracking:**
- Each feature stores parent refs (e.g., Extrude depends on Sketch001)
- Build directed acyclic graph (DAG); topological sort for rebuild order
- **Circular dependency:** Detect during param edit, block with error dialog

**Redo Semantics:**
- Redo stack cleared on any new command (standard behavior)
- Undo/redo does NOT recompute; replays cached geometry changes (faster)

**History Depth:**
- Max 50 commands (configurable); prune oldest on overflow
- Each command stores delta (old/new shape) → memory proportional to model size; estimate ~5-10MB for typical 50-command stack

## 10. Testing, CI, and QA

### Unit Tests

**Geometry Correctness:**
- Test each feature type (extrude, revolve, boolean) with known inputs → validate output BRep properties (volume, face count, bounding box)
- Example: `TEST(Extrude, SimpleRectangle)` → extrude 10x10mm square 5mm → expect volume = 500mm³
- Use `BRepGProp_Face` to compute volumes, `GProp_GProps` for mass properties

**File Roundtrip:**
- Save `.bCAD` → load → compare feature tree, params, BRep hash (use `BRepTools::Write` to canonical format, compute MD5)
- STEP export → import → validate topology equivalence (face count, edge count; geometry may differ slightly due to tolerance)

**Constraint Solver:**
- Unit test sketch constraints: add coincident + distance → solve → verify entity positions within tolerance (1e-6)

### Integration Tests

**UI Flows:**
- Automate 6 core tasks (§4) with scripted UI events (Qt Test framework or platform-specific UI automation)
- Validate: feature tree state, viewport rendering (compare screenshots with reference images, allow pixel tolerance)

**Crash Recovery:**
- Simulate crash (kill process mid-edit) → restart → verify recovery dialog appears, recovered file matches autosave

### Fuzzing

**Input Fuzzing:**
- Fuzz STEP/IGES importers with malformed files (use AFL or libFuzzer)
- Expected: graceful error, no crash, no infinite loop
- Budget: 1hr fuzz session per importer per release

**Param Fuzzing:**
- Random param values (e.g., extrude depth = -1e10, NaN, zero) → expect validation error or safe fallback

### End-to-End Tests

- **Create part → save → close → reopen → edit → export STEP:** validate STEP file with external validator (e.g., FreeCAD import test)
- **Assembly with 10 parts → suppress half → export → validate BOM matches unsuppressed parts**

### CI Pipeline

- **On commit:** Build on Windows/Linux/macOS, run unit tests (~5min total)
- **Nightly:** Integration tests, fuzzing, memory leak check (Valgrind on Linux)
- **Release:** Full end-to-end suite, STEP roundtrip with commercial CAD (manual QA)

## 11. Security, Stability, and Error Handling

### Invalid STEP Files

**Handling:**
- Wrap `STEPControl_Reader::ReadFile()` in try-catch (OCC throws `Standard_Failure`)
- On exception: log error, show user-friendly dialog ("Import failed: invalid STEP file"), continue (don't crash)
- Validate imported shape with `BRepCheck_Analyzer`; reject if not solid/closed (warn user, offer to import as shell)

**Sanitization:**
- Limit imported geometry complexity: max 10K faces per part (reject larger with warning)
- Timeout: if import takes >30s, cancel with error (detect malicious/huge files)

### Corrupt Caches

**Tessellation Cache:**
- On load: verify cache version matches feature hash; discard if mismatch, re-tessellate
- If tessellation fails: fall back to bounding box display, mark feature as "render failed"

**Autosave/Recovery:**
- Validate autosave file CRC on load; discard if corrupt, notify user
- Keep last 3 autosaves (rotating); if latest corrupt, try older

### Safe Recovery

**Crash Handling:**
- Install signal handlers (SIGSEGV, SIGABRT) to flush autosave before exit (best-effort; OS may kill hard)
- On next startup: check for `.bCAD.autosave` with timestamp newer than last clean exit → prompt recovery

**Validation:**
- Before saving: run `BRepCheck_Analyzer` on all features; if invalid, warn user, allow save anyway (mark file as "potentially corrupt")

**Fail-Safe Defaults:**
- If param parse fails (e.g., JSON corrupt), fall back to hardcoded defaults (e.g., extrude depth = 10mm)
- If entire file unreadable: offer to open blank part + error log

## 12. MVP Roadmap & Priorities

### 3-Month MVP (Must-Have)

**Deliverables:**
- **Part Editor:** Sketch (line, arc, circle, constraints: coincident, dimension), Extrude, Revolve, Boolean (union/cut), Fillet
- **Parametric Tree:** Ordered features, edit params, suppress/delete
- **Viewport:** Orbit/pan/zoom, shaded/wireframe, selection, fit-all
- **File I/O:** Save/load `.bCAD`, import/export STEP AP214
- **Undo/Redo:** 20-level stack (reduced from 50 for MVP)
- **Crash Recovery:** Autosave every 5min, recovery dialog on restart
- **Platforms:** Windows only (defer macOS/Linux to 6-month)

**Cut from MVP:**
- Assembly editor (defer to 6-month)
- Advanced sketch entities (spline, ellipse)
- IGES, STL, OBJ export
- Chamfer, shell, patterns
- Plugin API

**Acceptance Criteria (3-month):**
- Import STEP file (≤500 faces) in <3s
- Extrude/boolean operations complete in <500ms (simple geometry)
- Save/load `.bCAD` in <1s (typical part with 10 features)
- Cold start <2s (Windows, SSD)
- Binary size <60MB (Windows .exe + bundled OCC DLLs)
- No crashes in 1hr exploratory testing

### 6-Month (Near Term)

**Add:**
- **Assembly Editor:** Insert parts, coincident/concentric mates, BOM, exploded view
- **Advanced Sketching:** Spline, rectangle, tangent/perpendicular constraints
- **Additional Ops:** Chamfer, shell, linear/circular patterns
- **Export:** IGES, STL (binary), OBJ
- **Platforms:** macOS, Linux builds
- **Plugin API:** C ABI for custom importers
- **Undo/Redo:** 50-level stack

**Acceptance Criteria (6-month):**
- Assembly with 20 parts, 10 mates: open in <5s, navigate at 30 FPS
- Export STL (1000 faces) in <2s
- All 6 core flows (§4) automatable in CI
- <10 critical bugs in issue tracker

### Stretch Goals (Beyond 6-Month)

- **Surfacing:** Loft, sweep, boundary surfaces
- **Sheet Metal:** Flange, bend, unfold
- **Drawings:** 2D orthographic views, dimensions, annotations
- **Scripting:** Python API for automation
- **Cloud Sync:** Optional cloud save (requires backend infra)
- **Vulkan Renderer:** For better multi-GPU support
- **Mobile Viewer:** Read-only iOS/Android app

## 13. Realistic Risks & Scope Cuts

**Risks:**
1. **OCC Learning Curve:** Team unfamiliar with OCC APIs → budget 1 month ramp-up; expect initial bugs in boolean ops
2. **Tessellation Performance:** Large models (>5K faces) may render slowly → mitigation: aggressive LOD, warn users
3. **STEP Interop:** Commercial CAD exports quirky STEP → expect 10-20% import failure rate; no fix without deep OCC expertise
4. **Qt Binary Size:** Qt adds 30MB even minimal → if unacceptable, pivot to ImGui (requires custom layout code, 2-week detour)
5. **Cross-Platform Bugs:** macOS/Linux OpenGL drivers differ → budget 1 week per platform for rendering fixes

**Where to Cut Scope (to stay compact):**
- **No Scripting in MVP:** Python adds 20MB; defer to plugins
- **No Advanced Surfacing:** Loft/sweep require TKGeomAlgo → +5MB; postpone
- **No FEA/CAM:** Out of scope; recommend separate tools
- **No Texture/Material Editor:** Simple colors only; advanced rendering = stretch goal
- **No Realtime Collaboration:** Local-first; cloud sync optional, far future

**Pragmatic Note:** A "small CAD app" is an oxymoron. OpenCASCADE alone is 15-20MB. With Qt, you're at 50MB minimum. If 50MB is too large, reconsider Qt → ImGui, but accept 2-3x UI dev time. If STEP import must be 100% reliable, budget 6 months just for testing/workarounds.

## 14. Deliverables for Implementer

### Per-Module Artifacts

**`core` Module:**
- API: `Application::init()`, `Application::run()`, `Application::shutdown()`
- JSON Schema: `config.json` (window size, recent files, autosave interval)
- Unit Tests: `test_crash_recovery.cpp` (simulate crash, verify autosave flush)
- CLI Build Target: `badcad` (main executable)

**`geometry` Module:**
- API: `Geometry::extrude(Sketch, depth)`, `Geometry::boolean(op, shape1, shape2)`
- Unit Tests: `test_extrude.cpp`, `test_boolean.cpp` (validate volumes, face counts)
- CLI Build Target: `libgeometry.a` (static lib)

**`model` Module:**
- API: `Part::addFeature(feature)`, `Part::save(path)`, `Part::load(path)`
- JSON Schema: `manifest.json` (see §5)
- Unit Tests: `test_serialization.cpp` (roundtrip save/load, compare hashes)
- CLI Build Target: `libmodel.a`

**`ui` Module (Part Editor screen):**
- API: `PartEditorWidget::setMode(mode)`, `PartEditorWidget::updateViewport()`
- JSON Schema (UI definition):
```json
{
  "screen": "PartEditor",
  "modes": ["Sketch", "Model", "Inspect"],
  "toolbars": [
    {
      "id": "model_toolbar",
      "visible_in_modes": ["Model"],
      "buttons": [
        {"id": "new_sketch", "label": "New Sketch", "icon": "sketch.png", "shortcut": "K"},
        {"id": "extrude", "label": "Extrude", "icon": "extrude.png", "shortcut": "E", "enabled_if": "selection.type == 'sketch'"}
      ]
    },
    {
      "id": "sketch_toolbar",
      "visible_in_modes": ["Sketch"],
      "buttons": [
        {"id": "line", "label": "Line", "icon": "line.png"},
        {"id": "arc", "label": "Arc", "icon": "arc.png"},
        {"id": "exit_sketch", "label": "Exit Sketch", "icon": "exit.png", "shortcut": "Esc"}
      ]
    }
  ],
  "panels": [
    {"id": "feature_tree", "visible_in_modes": ["Sketch", "Model", "Inspect"], "dock": "left"},
    {"id": "properties", "visible_in_modes": ["Model", "Inspect"], "visible_if": "selection != null", "dock": "right"}
  ]
}
```
- Unit Tests: `test_ui_visibility.cpp` (verify toolbar shown/hidden per mode)
- CLI Build Target: `libbadcad_ui.a` + `badcad_ui_tests`

**`importexport` Module:**
- API: `STEP::import(path)`, `STEP::export(shape, path, options)`
- Unit Tests: `test_step_roundtrip.cpp` (export → import → compare topology)
- CLI Build Target: `libimportexport.a` (or plugin DLLs: `step_plugin.dll`, `iges_plugin.dll`)

**`solver` Module:**
- API: `SketchSolver::solve(constraints)`, `AssemblySolver::solveMates(mates)`
- Unit Tests: `test_sketch_solver.cpp` (add constraints, verify solution)
- CLI Build Target: `libsolver.a`

**`render` Module:**
- API: `Renderer::tessellate(shape, quality)`, `Renderer::drawShape(shape, mode)`
- Unit Tests: `test_tessellation.cpp` (verify triangle count, normals)
- CLI Build Target: `librender.a`

**`history` Module:**
- API: `History::execute(command)`, `History::undo()`, `History::redo()`
- Unit Tests: `test_undo_redo.cpp` (execute commands, undo, verify state)
- CLI Build Target: `libhistory.a`

**`plugins` Module:**
- API: C header `badcad_plugin.h` (import/export function typedefs)
- Sample Plugin: `sample_importer.c` (no-op importer, demonstrates API)
- CLI Build Target: `sample_importer.dll`

**`utils` Module:**
- API: `Logger::log(level, msg)`, `JSON::parse(file)`, `FileIO::readZip(path)`
- Unit Tests: `test_json.cpp`, `test_zip.cpp`
- CLI Build Target: `libutils.a`

### Global Deliverables

- **CMakeLists.txt:** Top-level build script; targets for all modules, tests, executable
- **README.md:** Build instructions (see §2 in this spec)
- **ARCHITECTURE.md:** Module diagram, data flow (auto-generate from Doxygen)
- **CI Config:** `.github/workflows/build.yml` (build + test on push)
- **Install Package:** MSI (Windows), AppImage (Linux), DMG (macOS) with bundled dependencies

## 15. Acceptance Criteria (Detailed)

### 3-Month MVP Criteria

| **Feature**            | **Criterion**                                                                 | **Target**       | **Notes**                          |
|------------------------|-------------------------------------------------------------------------------|------------------|------------------------------------|
| STEP Import            | Import file with ≤500 faces                                                   | <3s              | Conservative; OCC can be slow      |
| STEP Import            | Import file with ≤5000 faces                                                  | <15s             | Stress test; show progress bar     |
| Extrude Operation      | Simple sketch (4 edges) → extrude                                             | <200ms           | Typical case                       |
| Boolean Operation      | Union of two 100-face solids                                                  | <500ms           | OCC boolean is expensive           |
| File Save              | Save part with 10 features                                                    | <1s              | Including manifest + BReps         |
| File Load              | Load part with 10 features                                                    | <1s              | Including tessellation cache       |
| Cold Startup           | Launch app to home screen (no file)                                           | <2s              | Windows, SSD, modern CPU           |
| Undo/Redo              | 20-level stack; undo/redo action                                              | <50ms            | Instant feel                       |
| Viewport FPS           | Navigate (orbit/pan) with 500-face part                                       | ≥30 FPS          | Shaded mode                        |
| Binary Size            | Windows .exe + OCC DLLs (release build)                                       | <60MB            | Stripped, no debug symbols         |
| Memory (Idle)          | App open, no file loaded                                                      | <200MB           | Measured via Task Manager          |
| Memory (Loaded)        | Part with 10 features, 500 faces total                                        | <350MB           | Including tessellation cache       |
| Crash Rate             | No crashes in 1hr exploratory testing                                         | 0 crashes        | Manual QA session                  |
| STEP Export            | Export part to STEP, re-import in FreeCAD                                     | No errors        | Visual inspection (topology match) |

### 6-Month Criteria

| **Feature**            | **Criterion**                                                                 | **Target**       | **Notes**                          |
|------------------------|-------------------------------------------------------------------------------|------------------|------------------------------------|
| Assembly Load          | Open assembly with 20 parts (500 faces each)                                  | <5s              | Including mate solve               |
| Assembly FPS           | Navigate assembly (20 parts)                                                  | ≥30 FPS          | May drop to 20 FPS on low-end GPU  |
| STL Export             | Export 1000-face part to STL                                                  | <2s              | Binary STL                         |
| Undo Stack             | 50-level stack                                                                | <100MB total     | Memory for command history         |
| Cross-Platform         | Build + pass tests on macOS, Linux                                            | 100% pass rate   | CI enforced                        |
| Plugin API             | Load custom importer DLL, import file                                         | <500ms overhead  | Compared to built-in               |
| Autosave Reliability   | Autosave every 5min; recover on crash                                         | 95% success rate | Manual crash test (10 trials)      |

### Continuous (Quality Gates)

- **No critical bugs:** P0 (crash, data loss) count = 0 before release
- **Test Coverage:** ≥70% line coverage for `geometry`, `model`, `solver` modules
- **Fuzz Stability:** 1hr fuzz session on STEP importer with 0 crashes
- **Documentation:** All public APIs documented in Doxygen; README build instructions validated

**Warning on Targets:** These are conservative estimates; actual performance depends heavily on OCC version, compiler optimizations, and input geometry complexity. Budget 20% margin for variance.

---

## Appendix: UI Visibility Matrix (Full Table)

| **Mode**         | **Submode/State**          | **Model Toolbar** | **Sketch Toolbar** | **Assembly Toolbar** | **View Toolbar** | **Feature Tree** | **Properties Panel** | **Constraints Panel** | **BOM Panel** |
|------------------|----------------------------|-------------------|---------------------|----------------------|------------------|------------------|----------------------|------------------------|---------------|
| Home             | No file open               | ✗                 | ✗                   | ✗                    | ✗                | ✗                | ✗                    | ✗                      | ✗             |
| Part Editor      | Model Mode, no selection   | ✓                 | ✗                   | ✗                    | ✓                | ✓                | ✗                    | ✗                      | ✗             |
| Part Editor      | Model Mode, face selected  | ✓                 | ✗                   | ✗                    | ✓                | ✓                | ✓ (read-only)        | ✗                      | ✗             |
| Part Editor      | Sketch Mode                | ✗                 | ✓                   | ✗                    | ✓ (subset)       | ✓                | ✗                    | ✓                      | ✗             |
| Assembly Editor  | Assembly Mode, no sel      | ✗                 | ✗                   | ✓                    | ✓                | ✓                | ✗                    | ✗                      | ✓             |
| Assembly Editor  | Assembly Mode, part sel    | ✗                 | ✗                   | ✓                    | ✓                | ✓                | ✓ (read-only)        | ✗                      | ✓             |
| Assembly Editor  | Part Edit Mode             | ✓ (subset)        | ✗                   | ✗                    | ✓                | ✓                | ✓ (editable)         | ✗                      | ✗             |
| Inspect Mode     | Any                        | ✗                 | ✗                   | ✗                    | ✓                | ✓                | ✓ (read-only)        | ✗                      | ✗             |

**Notes:**
- "Subset" = reduced button set (e.g., Part Edit Mode in assembly hides boolean ops to avoid confusing assembly/part context)
- View Toolbar always visible except Home screen
- Feature Tree visible in all editor modes (primary navigation)

---

## Appendix: JSON UI Definition Example (Part Editor Screen)

```json
{
  "screen": "PartEditor",
  "modes": [
    {"id": "model", "label": "Model", "default": true},
    {"id": "sketch", "label": "Sketch"},
    {"id": "inspect", "label": "Inspect"}
  ],
  "toolbars": [
    {
      "id": "model_toolbar",
      "label": "Model Tools",
      "visible_in_modes": ["model"],
      "visible_if": "!advanced_mode || user_level >= 2",
      "buttons": [
        {
          "id": "new_sketch",
          "label": "New Sketch",
          "icon": "icons/sketch.svg",
          "tooltip": "Create a new 2D sketch on a plane or face",
          "shortcut": "K",
          "enabled_if": "true"
        },
        {
          "id": "extrude",
          "label": "Extrude",
          "icon": "icons/extrude.svg",
          "tooltip": "Extrude a sketch into a 3D solid",
          "shortcut": "E",
          "enabled_if": "selection.type == 'sketch' || selection.type == 'face'"
        },
        {
          "id": "revolve",
          "label": "Revolve",
          "icon": "icons/revolve.svg",
          "tooltip": "Revolve a sketch around an axis",
          "enabled_if": "selection.type == 'sketch'"
        },
        {
          "id": "bool_union",
          "label": "Union",
          "icon": "icons/union.svg",
          "tooltip": "Combine two solids",
          "enabled_if": "selection.count == 2 && selection.all_type == 'solid'"
        },
        {
          "id": "bool_cut",
          "label": "Cut",
          "icon": "icons/cut.svg",
          "tooltip": "Subtract one solid from another",
          "enabled_if": "selection.count == 2 && selection.all_type == 'solid'"
        },
        {
          "id": "fillet",
          "label": "Fillet",
          "icon": "icons/fillet.svg",
          "tooltip": "Round selected edges",
          "enabled_if": "selection.type == 'edge' || selection.type == 'edge_list'"
        }
      ]
    },
    {
      "id": "sketch_toolbar",
      "label": "Sketch Tools",
      "visible_in_modes": ["sketch"],
      "buttons": [
        {"id": "line", "label": "Line", "icon": "icons/line.svg", "shortcut": "L"},
        {"id": "arc", "label": "Arc", "icon": "icons/arc.svg", "shortcut": "A"},
        {"id": "circle", "label": "Circle", "icon": "icons/circle.svg", "shortcut": "C"},
        {"id": "constraint_coincident", "label": "Coincident", "icon": "icons/coincident.svg"},
        {"id": "constraint_dimension", "label": "Dimension", "icon": "icons/dimension.svg", "shortcut": "D"},
        {"id": "exit_sketch", "label": "Exit Sketch", "icon": "icons/exit.svg", "shortcut": "Esc", "style": "prominent"}
      ]
    },
    {
      "id": "view_toolbar",
      "label": "View",
      "visible_in_modes": ["model", "sketch", "inspect"],
      "buttons": [
        {"id": "fit_all", "label": "Fit", "icon": "icons/fit.svg", "shortcut": "F"},
        {"id": "view_front", "label": "Front", "icon": "icons/front.svg"},
        {"id": "view_top", "label": "Top", "icon": "icons/top.svg"},
        {"id": "view_iso", "label": "Isometric", "icon": "icons/iso.svg"},
        {"id": "mode_wireframe", "label": "Wireframe", "icon": "icons/wireframe.svg", "shortcut": "W", "toggle_group": "render_mode"},
        {"id": "mode_shaded", "label": "Shaded", "icon": "icons/shaded.svg", "shortcut": "S", "toggle_group": "render_mode"},
        {"id": "mode_hiddenline", "label": "Hidden Line", "icon": "icons/hiddenline.svg", "shortcut": "H", "toggle_group": "render_mode"}
      ]
    }
  ],
  "panels": [
    {
      "id": "feature_tree",
      "label": "Features",
      "dock": "left",
      "visible_in_modes": ["model", "sketch", "inspect"],
      "width": 250,
      "resizable": true
    },
    {
      "id": "properties",
      "label": "Properties",
      "dock": "right",
      "visible_in_modes": ["model", "inspect"],
      "visible_if": "selection != null",
      "width": 300,
      "resizable": true
    },
    {
      "id": "constraints",
      "label": "Constraints",
      "dock": "right",
      "visible_in_modes": ["sketch"],
      "width": 300,
      "resizable": true
    }
  ],
  "context_menus": [
    {
      "id": "face_menu",
      "trigger": "right_click && selection.type == 'face'",
      "items": [
        {"id": "sketch_on_face", "label": "New Sketch on This Face"},
        {"id": "measure_face", "label": "Measure Area"},
        {"separator": true},
        {"id": "delete_face", "label": "Delete Face", "enabled_if": "face.is_removable"}
      ]
    },
    {
      "id": "edge_menu",
      "trigger": "right_click && selection.type == 'edge'",
      "items": [
        {"id": "fillet_edge", "label": "Fillet Edge"},
        {"id": "chamfer_edge", "label": "Chamfer Edge"},
        {"id": "measure_edge", "label": "Measure Length"}
      ]
    }
  ]
}
```

**Visibility Rule Semantics:**
- `visible_in_modes`: array of mode IDs where widget shown
- `visible_if`: JavaScript-like expression evaluated at runtime (variables: `selection`, `advanced_mode`, `user_level`)
- `enabled_if`: button enabled only if expression true
- `toggle_group`: mutually exclusive buttons (radio behavior)
- `style: "prominent"`: highlighted (e.g., Exit Sketch button larger/colored)

---

**End of Specification**

*Warning: This spec is ambitious for a 3-6 month timeline with a small team. Prioritize ruthlessly. If slipping, cut Assembly Editor from MVP, ship Part-only; assemblies are complex and can wait. Keep binary size honest by measuring weekly; feature creep kills compactness.*
