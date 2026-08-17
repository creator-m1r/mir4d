# MIR 4D — MirUI CAD Workspace Blueprint

## Purpose

This document is the implementation blueprint for the next MirUI UX stage after Project Hub. It translates the MirUI UX specification into a concrete CAD workspace shell while preserving the existing runtime and renderer boundaries.

## Core layout

```text
┌──────────────────────────────────────────────────────────────────┐
│ ◇ МИР 4D   Проект   Моделирование   Анализ   4D       ● Сохранено│
├───────────────┬───────────────────────────────────────┬──────────┤
│ СТРУКТУРА     │                                       │ СВОЙСТВА │
│               │                                       │          │
│ Проект        │                                       │ Выбрано  │
│ └─ Тело 1     │             VIEWPORT                  │          │
│   └─ Эскиз    │                                       │ Размеры  │
│   └─ Операция │                                       │ Материал │
│               │                                       │          │
├───────────────┴───────────────────────────────────────┴──────────┤
│ Контекст: Выбрано тело 1     [Изменить] [Удалить] [Свойства]     │
├──────────────────────────────────────────────────────────────────┤
│ Timeline / история / сообщения                                  │
└──────────────────────────────────────────────────────────────────┘
```

## Navigation

Top-level navigation has only four user-facing domains:

- **Проект** — save, open, import/export, project information.
- **Моделирование** — sketch, feature, transform, geometry operations.
- **Анализ** — materials, loads, CAE, diagnostics and results.
- **4D** — timeline, states, playback and time-dependent changes.

The navigation must not enumerate internal subsystems such as B-Rep, Runtime, Event Bus or kernel services.

## Context model

The workspace has one active context:

```text
No selection
    → project-level actions
Body selected
    → body actions
Sketch selected
    → sketch actions
Feature selected
    → feature/history actions
Analysis result selected
    → result actions
```

Only the current context may promote actions to the primary toolbar.

## Left panel — Structure

The structure panel represents the user's model, not the engine internals.

Recommended hierarchy:

```text
Проект
├── Сборка
├── Тело
│   ├── Эскиз
│   ├── Операция
│   └── Операция
├── Конструктивные элементы
└── Анализы
```

The tree must remain synchronized with the persisted model document. Runtime engine IDs are implementation details.

## Center — Viewport

The viewport is the dominant area. It must receive the majority of available space.

Required interactions:

- hover highlight;
- selection;
- orbit;
- pan;
- zoom;
- fit view;
- object context menu;
- sketch interaction;
- work-plane interaction;
- 4D playback overlay when time mode is active.

No permanent tool palette should obscure the viewport.

## Right panel — Properties

When nothing is selected, show a concise project summary.

When an object is selected, show:

1. identity;
2. primary parameters;
3. engineering properties;
4. dependencies;
5. advanced information.

Advanced information is collapsed by default.

## Context bar

The context bar is the primary replacement for a large permanent toolbar.

Examples:

```text
Тело выбрано
[Изменить] [Переместить] [Удалить] [Свойства]
```

```text
Эскиз выбран
[Открыть] [Изменить] [Операция] [Удалить]
```

```text
Ничего не выбрано
[Создать эскиз] [Создать тело]
```

## Bottom area

The bottom area is reserved for one of three contextual instruments:

- **История** — model feature history;
- **Timeline** — 4D time control;
- **Messages** — diagnostics and operation feedback.

Only the relevant instrument is expanded.

## State model

Every workspace view must explicitly support:

```text
idle
loading
ready
selection
editing
processing
saved
modified
warning
error
```

The user must never be left without a visible explanation during a long operation.

## Save state

The top-right state indicator should be concise:

- `Сохранено`
- `Изменено`
- `Сохранение…`
- `Ошибка сохранения`

It should not become a permanent large notification.

## Commands

The UX shell must consume commands rather than mutate the model directly.

Initial command taxonomy:

```text
Project
  new
  open
  save
  close

Selection
  select
  clear
  delete

Sketch
  create
  edit
  finish

Model
  createBody
  createFeature
  transform
  suppress

Analysis
  inspect
  configure
  run
  showResult

4D
  setTime
  play
  pause
  step
```

The existing `MIR4DModelCommands` remains the integration point for model operations. The UX shell must not bypass it. The current command layer already routes creation into the runtime, marks the project dirty, schedules autosave and emits model notifications. fileciteturn18file0

## Runtime boundary

The existing `MIR4DModelRuntime` remains authoritative for the evaluated engine document and synchronizes persisted model state with the viewport. The new UX shell must consume published state and notifications rather than recreate a second model. fileciteturn19file0

## Project lifecycle

```text
Project Hub
   ↓ open/create
CAD Workspace
   ↓ edit
Modified
   ↓ save
Saved
   ↓ close
Project Hub
```

The workspace must preserve the existing project session and `.mir4d` lifecycle.

## Responsive behavior

### Wide window

Show:

- structure;
- viewport;
- properties;
- context bar;
- bottom instrument.

### Medium window

Collapse Properties into a tab or inspector button.

### Narrow window

Collapse Structure and Properties into toolbar-accessible sheets while preserving the viewport.

The viewport is always the last area to sacrifice.

## UX rules for implementation

1. Do not add a permanent button for every engine capability.
2. Do not expose internal engine terminology in primary navigation.
3. Do not duplicate model state in View structs.
4. Do not mutate `MIR4DModelDocument` directly from a view.
5. Do not put long-running engine work on the main thread.
6. Do not allow secondary panels to visually overpower the viewport.
7. Do not change `MirEngine` merely to solve a presentation problem.
8. Every new feature must define its empty, ready, processing and error states.
9. Every selected object must have a predictable context.
10. Every destructive action must have clear confirmation or undo semantics.

## Implementation sequence

### Phase A — Shell

- establish top navigation;
- establish three-panel workspace;
- establish context bar;
- establish save state;
- establish bottom instrument container.

### Phase B — Selection

- connect viewport selection to AppState;
- connect tree selection to viewport;
- connect properties to selection;
- connect context actions.

### Phase C — Sketch

- plane selection;
- sketch mode;
- snap;
- constraints;
- finish sketch.

### Phase D — Feature history

- feature tree;
- operation editing;
- rebuild state;
- dependency display.

### Phase E — 4D

- timeline;
- playback;
- event markers;
- state inspection.

### Phase F — AI

- Ask MIR;
- contextual inspector;
- error explanation.

## Definition of done

The CAD shell is ready when a user can:

1. open/create a `.mir4d` project;
2. see the model tree;
3. create a primitive through the command layer;
4. select it in the viewport;
5. see its properties;
6. execute a context action;
7. save the project;
8. understand whether the project is saved or modified;
9. return to Project Hub;
10. perform the above without exposing or understanding MirEngine internals.
