# badCAD Document Format (.bCAD)

## Overview
The `.bCAD` file format is a simple text-based format that stores the complete state of a part or assembly. This format is designed to be human-readable and easy to parse.

## Current Implementation

### Version 0.1 - Basic Planes

When you create a new part, the document is initialized with the following in-memory representation:

```
planexy 1
planexz 1
planeyz 1
```

### Format Rules

- Each line represents a distinct element or feature
- Elements follow the pattern: `<type> <parameters>`
- Boolean values are represented as `1` (visible/enabled) or `0` (hidden/disabled)

### Construction Planes

The three default construction planes are:
- **planexy**: XY plane (Z=0), displayed in blue
- **planexz**: XZ plane (Y=0), displayed in green  
- **planeyz**: YZ plane (X=0), displayed in red

Syntax: `<planename> <visibility>`

Example:
```
planexy 1     # XY plane visible
planexz 0     # XZ plane hidden
planeyz 1     # YZ plane visible
```

## Future Extensions

### Sketches
```
sketch <id> <plane> <visible>
  line <x1> <y1> <x2> <y2>
  arc <cx> <cy> <r> <start_angle> <end_angle>
  circle <cx> <cy> <r>
  constraint horizontal <line_id>
  constraint vertical <line_id>
  constraint coincident <point1> <point2>
end_sketch
```

### Features
```
feature extrude <sketch_id> <distance> <direction>
feature revolve <sketch_id> <axis> <angle>
feature union <feature1_id> <feature2_id>
feature cut <target_id> <tool_id>
feature fillet <edge_id> <radius>
```

### Parameters
```
param <name> <value> <type>
```

## Document Lifecycle

1. **New Document**: Initialized with 3 default construction planes (all visible)
2. **Serialization**: `Document::serialize()` converts in-memory state to text format
3. **File Save**: Text format written to `.bCAD` file
4. **File Load**: `.bCAD` file read and parsed via `Document::deserialize()`
5. **Viewer Update**: `OccViewer::updateFromDocument()` refreshes 3D visualization

## Example Complete Document (Future)

```
planexy 1
planexz 1
planeyz 1

sketch 1 planexy 1
  line 0 0 100 0
  line 100 0 100 50
  line 100 50 0 50
  line 0 50 0 0
  constraint horizontal 1
  constraint vertical 2
  constraint horizontal 3
  constraint vertical 4
  constraint length 1 100
  constraint length 2 50
end_sketch

feature extrude 1 10 0 0 1

param base_width 100 double
param base_height 50 double
param extrude_depth 10 double
```

## Current Status

✅ **Implemented**:
- Document class with serialize/deserialize
- Default construction planes (XY, XZ, YZ)
- Plane visibility toggling
- OpenCASCADE 3D visualization of planes

🔨 **In Progress**:
- Sketch creation and storage
- Feature operations (extrude, revolve, etc.)
- Parametric constraints

📋 **Planned**:
- File I/O (.bCAD save/load)
- Undo/redo system
- Feature history editing
- Assembly support
