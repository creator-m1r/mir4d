# MirEngine B-Rep

Канонический модуль точной инженерной B-Rep-геометрии MIR 4D.

```text
MirEngine/BRep/
├── BRep.hpp                         # единая публичная точка входа
├── Core/                             # BRepModel и typed handles
├── Topology/                         # Vertex, Edge, Wire, Face, Shell, Solid
├── Geometry/                         # точные носители, stores, adaptors, validation
├── Builders/                         # создание топологии и примитивов
├── Tessellator/                      # BRep → TriangleMesh3
├── Boolean/                          # контракт булевых операций
├── Converters/                       # Sketch/BRep/Model преобразования
├── Commands/                         # граница Document/Scene
└── Validator/                        # целостность BRep-модели
```

## Канонические правила

1. OpenGL, SwiftUI и GPU handles не входят в B-Rep.
2. B-Rep использует только общие `MirEngine/Math` типы (`Vector3`, `Scalar` и т.д.).
3. Топология хранит typed handles (`BRepVertexHandle`, `BRepEdgeHandle`, `BRepFaceHandle`, ...).
4. Параллельные `BRep*ID` типы не создаются.
5. Параллельный `BRepVector3D` не создаётся.
6. `BRepGeometryStore` и `BRepTopologyStore` являются canonical owners данных.
7. `BRep.hpp` — единственный umbrella header модуля.
8. Root-level `BRep*.hpp` дубликаты запрещены.
9. Tessellation создаёт render mesh и не изменяет точную B-Rep модель.
10. STEP/OCCT подключается только через `MirEngine/IO/Step`.
11. Sketch/Feature pipeline обязан использовать реально существующий B-Rep API; исторические несовместимые контракты не сохраняются.
12. Boolean API может быть контрактом до появления полноценного kernel implementation, но не должен выдавать ложный `Success`.

## Текущий рабочий вертикальный срез

```text
BRepBuilderAPI
      ↓
Topology + Geometry Stores
      ↓
BRepModel
      ↓
BRepValidator
      ↓
BRepExtrudeBuilder / primitives
      ↓
BRepTessellator
      ↓
TriangleMesh3
```

Это базовый CAD kernel slice, на который дальше строятся Model, Document, Feature History и Boolean operations.

## Внешние границы

```text
Sketch ──→ BRep Converters/Builders
BRep  ──→ Model
BRep  ──→ Tessellator
BRep  ←── IO/Step adapters
BRep  ←── Document commands
```

Core B-Rep не знает о UI, render backend или файловом формате.
