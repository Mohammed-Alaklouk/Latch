# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Latch is a desktop logic-gate sandbox: place gates on an infinite grid, wire them
with orthogonal routing, and simulate signal propagation. C++20, raylib for
rendering, Dear ImGui (via rlImGui) for panels, Catch2 for tests. All
dependencies are pulled by CMake `FetchContent` at configure time, so a clean
configure needs Git and network access.

`README.md` documents the user-facing controls and panels. `ROADMAP.md` (local
only — it is gitignored) holds the phased plan toward v1; Phase 2 replaces the
current single-pass evaluator with a BFS engine, and Phase 3 generalizes the
single-bit signal to N-bit buses. Keep those two rewrites in mind before adding
anything that hard-codes one-pass evaluation or a scalar `LogicLevel`.

## Commands

```bash
cmake --preset vs              # configure (VS 2022 x64 -> build/vs)
cmake --build --preset vs      # build
ctest --preset vs --output-on-failure
```

Use the `ninja` preset instead for a single-config Debug build with GCC/Clang/MSVC
(`cmake --preset ninja`, `ctest --preset ninja --output-on-failure`).

Run the app: `./build/vs/src/Debug/Latch.exe`.

Run a subset of tests via Catch2 tags/filters against the binary directly — each
`TEST_CASE` is also registered individually with CTest:

```bash
./build/vs/tests/Debug/latch_tests.exe "[gate]"
```

`-DLATCH_BUILD_TESTS=OFF` skips the suite and the Catch2 fetch.

## Architecture

### Target split — what is testable

`src/CMakeLists.txt` builds two targets:

- **`latch_core`** (static lib): `Circuit`, `Gate`, `Wire`, `ComponentInfo.h`,
  `IdManager.h`, `Action.h`, `Pin.h`. Linked by both the app and the tests.
- **`Latch`** (executable): `main.cpp`, `App`, `Sheet` — *not* in the library, so
  nothing in `App`/`Sheet` can be unit-tested.

Put logic that deserves tests in `latch_core`. `latch_core` links raylib
`PUBLIC` because `Circuit`/`Gate`/`Wire` still draw themselves; tests stay
headless by never calling the `draw()` paths.

Sources are listed explicitly (no globbing) — adding a file means editing
`src/CMakeLists.txt` or `tests/CMakeLists.txt`.

### App → Sheet → Circuit

`App` owns the raylib window loop, the vector of `Sheet`s, the shared clipboard,
and the globally selected component type. Each frame it reads mouse/keyboard
state and forwards it to the **focused sheet only** — keyboard shortcuts are
gated on `ImGui::GetIO().WantCaptureKeyboard` and on the cursor being inside the
sheet's viewport.

`Sheet` = one `Circuit` plus its editor state (camera, viewport, selection,
undo history, sim run/tick state). Its mouse handling is a state machine over
`MouseState { Idle, Panning, Dragging, Connecting, Selecting }`, dispatched
through `mouse_state_update_functions[]` — **that array is index-parallel with
the enum**; adding a state means adding to both, plus `mouse_state_names[]`.
Discrete editor commands (`DeleteSelected`, `CopySelected`, `Paste`, `Undo`,
`Redo`, `StepSimulation`) are public `Sheet` methods the `App` dispatches.

### Evaluation model

`Circuit::evaluate()` is a **single pass**: copy each wire's value from its
source pin, then evaluate every component once. Signals therefore advance one
gate level per tick, and a network settles over several ticks.

Tests must assert **steady state**, not per-tick timing — `tests/TestHelpers.h`
provides `settle(circuit)`, which ticks until outputs stop changing (and returns
false for oscillators). This is deliberate so the Phase 2 engine rewrite does not
invalidate the suite.

Three-valued logic (`LOW`, `HIGH`, `UNDEFINED`) comes from `constexpr
LookupTable[9]` arrays on each gate, indexed `inputs[0] * 3 + inputs[1]`
(`NotGate` uses a `[3]` table).

### Adding a component type

Three index-parallel places must stay in the same order:

1. `componentInfo::Type` enum in `src/ComponentInfo.h` — insert before
   `COMPONENT_COUNT`.
2. `GateFactories[]` at the bottom of `src/Gate.h`.
3. The button list in `App::UI()`'s "Components" panel.

The subclass provides `evaluate()`, `getLabel()`, `getNodeInfoType()`, and
usually `drawBody()`; the base `Gate` draws leads and pins. Widths and heights are
multiples of `CELL_SIZE` (a `#define` in `Gate.h`, 10.0f) so pins land on grid
intersections.

### Ids and storage

`Circuit` keeps components and wires in dense vectors, with `IdManager` mapping a
stable id to the current index and recycling freed ids. Ids survive removals and
snapshot restores; indices do not. Always check `componentExists(id)` /
`wireExists(id)` before `getComponent()` / `getWire()` — those assert in Debug
and read out of bounds in Release.

### Wire as a graph

A `Wire` is not a polyline. It is a graph of `WireNode`s (each either a `PIN`
bound to a `PinRef` or a bare `JUNCTION`), `Segments` (pairs of node ids), and an
`Adjacency` map, with one `SourceNodeID` and many `DestinationNodeIDs` — that is
how one output branches to several inputs.

Only four private primitives mutate the graph: `addNode`, `removeNode`,
`addSegment`, `removeSegment`. **Every structural edit must end with
`normalize()`**, which restores the invariants in order: `splitSegmentsAtNodes()`
→ `removeRedundantNodes()` → `pruneOrphans()`. `normalize()` returns the `PIN`
nodes it dropped; `Circuit::disconnectRemovedPins()` uses that to clear the
corresponding `m_inputWireIds` entries, which is what keeps gate inputs and wire
topology consistent.

Routing is L-shaped, with the bend direction carried as `Wire::Elbow`. Move
operations come in preview/commit pairs (`previewMoveLegs` / `moveNodeWithElbow`,
`previewMoveGroupLegs` / `moveNodesWithElbow`) that intentionally mirror each
other — change one and change its twin, or the drag preview diverges from the
committed reroute.

### Undo/redo

`ActionManager` (`src/Action.h`) stores whole-circuit value snapshots
(`Circuit::CircuitSnapshot`: serializable `componentInfo`s, wires, and both
`IdManager`s). Restoring rebuilds `Gate` objects via `restoreComponent()`.

Any editor action that mutates the circuit must record
`action_manager.AddSnapshot(circuit.GetSnapshot("note"))` *after* the change —
the note string is what the Snapshots panel displays.

## Gotchas

- `src/CMakeLists.txt` lists `sheet.cpp sheet.h` in lowercase while the files are
  `Sheet.cpp` / `Sheet.h`. That builds on Windows but breaks on case-sensitive
  filesystems; fix it if you touch that line.
- `.gitignore` excludes `build/`, `bin/`, and `ROADMAP.md`; the repo tracks only
  `src/`, `tests/`, and the top-level CMake/doc files.
