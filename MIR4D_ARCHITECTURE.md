# МИР 4D — ЕДИНЫЙ АРХИТЕКТУРНЫЙ ЧЕРТЁЖ

Дата обновления: 2026-08-13
Статус: ЕДИНСТВЕННЫЙ ИСТОЧНИК АРХИТЕКТУРНОЙ ИСТИНЫ.

## 0. Обязательное правило работы

Перед ЛЮБЫМ изменением репозитория сначала читать этот файл, затем проверять реальные файлы GitHub.

Порядок каждого архитектурного прохода:

```text
MIR4D_ARCHITECTURE.md
        ↓
реальное дерево GitHub
        ↓
определение canonical-владельца
        ↓
переписывание существующего кода / создание TARGET
        ↓
тесты + CMake/CI
        ↓
немедленное обновление этого файла
```

Нельзя создавать дополнительные архитектурные карты, аудиты, списки удалений, roadmap-файлы, project-map-файлы или architecture guard/sync scripts.

Этот файл не является отчётом о прошлом. Это текущий инженерный чертёж проекта.

## 1. Именование

Продукт: **МИР 4D**.

Исходный проект: **`mir4d`**.

Новый C++ API: **`namespace mir4d`**.

Новые идентификаторы и файлы не используют `M1R`, `M1R_` или новый `mir::`. Существующий `mir::` является временным legacy-слоем и постепенно переписывается.

GitHub-аккаунт `creator-m1r` не переименовывается.

Формат проекта: `.mir4d`.

## 2. Главная цель

Промышленная параметрическая САПР МИР 4D:

```text
SwiftUI
   ↓
MirUI
   ↓
Interop
   ↓
MirEngine Runtime
   ↓
Document / Commands / History / Time
   ↓
Features / Sketch / BRep
   ↓
Geometry / Math / Core
```

`MirEngine` — единственный владелец инженерной модели.

`MirUI` — presentation, UI state, input routing, viewport и interop.

Swift/SwiftUI не владеет CAD-моделью, BRep, историей или инженерной математикой.

## 3. Архитектурные законы

1. Один инженерный концепт = одна canonical-реализация.
2. `mir4d::ObjectId` — единый идентификатор инженерных объектов.
3. `mir4d::ObjectRegistry` — единый владелец allocation/reservation/release ID.
4. `Document` — корень инженерного проекта и владелец project-local `ObjectRegistry`.
5. `ObjectStore` — canonical ownership boundary объектов документа.
6. `Math` владеет Vector/Point/Matrix/Transform.
7. `Geometry` владеет математической геометрией.
8. `BRep` владеет topology и solid geometry.
9. `Sketch` владеет эскизом и ограничениями.
10. `Features` владеет параметрическими операциями.
11. `History` владеет undo/redo, transaction и replay.
12. `Time/FourD` используют единую History-модель.
13. `Render` читает модель и не изменяет Document.
14. UI не изменяет BRep напрямую.
15. AI изменяет модель только через Diagnostic → Proposal → Command.
16. Ошибки: `mir4d::Error` / `mir4d::Result`.
17. Скрытые глобальные владельцы состояния запрещены.
18. `Config` и `Tests` находятся внутри `MirEngine`.
19. Корневые `Config/` и `Tests/` запрещены.
20. При архитектурном изменении этот файл обновляется в том же проходе.

## 4. Корневая структура

```text
mir4d/
├── CMakeLists.txt
├── Package.swift
├── MIR4D_ARCHITECTURE.md
├── .github/workflows/
├── MirEngine/
│   ├── Config/
│   ├── Core/
│   ├── Math/
│   ├── Geometry/
│   ├── BRep/
│   ├── Sketch/
│   ├── Features/
│   ├── Document/
│   ├── History/
│   ├── Time/
│   ├── Interaction/
│   ├── Render/
│   ├── IO/
│   ├── Materials/
│   ├── Assembly/
│   ├── Drawing/
│   ├── FourD/
│   ├── Simulation/
│   ├── Physics/
│   ├── Mechanics/
│   ├── Acoustics/
│   ├── Chemistry/
│   ├── AI/
│   ├── Platform/
│   └── Tests/
└── MirUI/
    ├── App/
    ├── Core/
    ├── Interop/
    ├── Viewport/
    ├── Widgets/
    ├── Renderers/
    └── Swift/
```

## 5. Направление зависимостей

```text
Core
 ↓
Math
 ↓
Geometry
 ↓
BRep
 ↓
Sketch / Features
 ↓
Document / ObjectStore
 ↓
History / Time / FourD / Assembly / Drawing
 ↓
Interaction / Render / IO / Simulation / AI
 ↓
MirUI Interop
 ↓
SwiftUI
```

Запрещено:

```text
Core → UI
Math → Document
Geometry → UI
BRep → SwiftUI
Document → SwiftUI
Render → Document mutation
AI → direct model mutation
Production → Tests
```

## 6. Canonical ownership

| Ответственность | Единственный владелец |
|---|---|
| ObjectId | `MirEngine/Core/Identity/ObjectId.hpp` |
| Object identity registry | `MirEngine/Core/Identity/ObjectRegistry.hpp` |
| Errors | `MirEngine/Core/Error.hpp` |
| Results | `MirEngine/Core/Result.hpp` |
| Engine events | `MirEngine/Core/Events/EventBus.hpp` |
| Dependency Injection | `MirEngine/Core/Services/ServiceContainer.hpp` |
| Math | `MirEngine/Math` |
| Geometry | `MirEngine/Geometry` |
| BRep | `MirEngine/BRep` |
| Sketch | `MirEngine/Sketch` |
| Features | `MirEngine/Features` |
| Document | `MirEngine/Document/Document.hpp` |
| Object storage | `MirEngine/Document/ObjectStore.hpp` |
| History | `MirEngine/History` + transition from DocumentHistory |
| Time | `MirEngine/Time` |
| FourD | `MirEngine/FourD` |
| Selection/Picking | `MirEngine/Interaction` |
| Render | `MirEngine/Render` |
| File IO | `MirEngine/IO` |
| Config | `MirEngine/Config` |
| Assembly | `MirEngine/Assembly` |
| Drawing | `MirEngine/Drawing` |
| Simulation | `MirEngine/Simulation` |
| AI | `MirEngine/AI` |
| UI | `MirUI` |
| Tests | `MirEngine/Tests` |

## 7. Core — canonical

```text
MirEngine/Core/
├── Identity/
│   ├── ObjectId.hpp
│   └── ObjectRegistry.hpp
├── Events/
│   ├── Event.hpp
│   ├── EventBus.hpp
│   └── Subscription.hpp
├── Services/
│   └── ServiceContainer.hpp
├── Error.hpp
└── Result.hpp
```

TARGET:

```text
MirEngine/Core/
├── EventType.hpp
├── Handles.hpp
├── Types.hpp
├── UUID.hpp
├── Logger.hpp
└── Runtime.hpp
```

### Identity

```cpp
namespace mir4d
{
using ObjectId = std::uint64_t;
inline constexpr ObjectId InvalidObjectId = 0;
}
```

`ObjectRegistry` предоставляет:

```text
allocate()
reserve()
release()
contains()
clear()
```

Других инженерных ID-систем создавать нельзя.

Удалённые legacy-системы:

```text
MirEngine/Core/IDs/ID.hpp
MirEngine/Core/IDs/ObjectID.hpp
MirEngine/Core/IDs/EntityID.hpp
MirEngine/Core/IDs/ComponentID.hpp
MirEngine/Core/IDs/FeatureID.hpp
MirEngine/Core/IDs/DocumentID.hpp
MirEngine/Core/IDs/ProjectID.hpp
MirEngine/Core/IDs/CommandID.hpp
MirEngine/Core/IDs/IDGenerator.hpp
MirEngine/Core/IDs/IDGenerator.cpp
```

`MirUI/Core/Object/ObjectID.hpp` не создаёт ID. Это только alias на `mir4d::ObjectId`.

## 8. Events

Единая engine-шина:

```text
mir4d::Event
    ↓
mir4d::EventBus
    ↓
subscriptions
```

`MirEngine/Core/Events/EventDispatcher.hpp` удалён.

`MirUI/Core/Events/EventDispatcher.hpp` разрешён только как UI capture/target/bubble dispatcher. Это не вторая engine-шина.

## 9. Services / DI

```text
MirEngine/Core/Services/ServiceContainer.hpp
```

Сервисы передаются явно. Скрытые singleton-владельцы запрещены.

## 10. Math

Фактический текущий canonical-носитель 3D-вектора:

```text
MirEngine/Math/Vector/Vector.hpp
```

Он пока содержит legacy `mir::Vector3`. Новый API должен переноситься в `mir4d::` без создания второго Vector3.

TARGET:

```text
MirEngine/Math/
├── Scalar.hpp
├── Constants.hpp
├── Vector2.hpp
├── Vector3.hpp
├── Vector4.hpp
├── Point2.hpp
├── Point3.hpp
├── Matrix3.hpp
├── Matrix4.hpp
├── Quaternion.hpp
├── Transform.hpp
├── Ray.hpp
├── Plane.hpp
├── Bounds.hpp
└── Precision.hpp
```

Существующую хорошую математическую реализацию переносим в canonical-файл, а не дублируем.

## 11. Geometry

Фактическое состояние после чистки legacy-среза (проход «битые include»):

```text
MirEngine/Geometry/
├── Curve/           Curve.hpp, CurveLoop.hpp, Circle.hpp, Arc.hpp (живые: Profile3)
├── Model/           Model.hpp (Profile + FacetedSolid + TriangleMesh3)
├── Profile/         Profile.hpp (Profile3 — конструкционный источник Model3)
├── Scene/           Scene.hpp (render-oriented scene nodes)
├── Solid/           FacetedSolid.hpp
├── Tessellation/    Tessellator.hpp, TriangleMesh.hpp
├── Geometry.hpp
```

Удалено в этом проходе (некомпилируемый / мёртвый legacy-код):

```text
Solid/    Solid, Box, Cone, Cylinder, Pyramid, Sphere, Torus, Wedge
Surface/  Surface, PlaneSurface, CylindricalSurface, NurbsSurface
Plane/    Plane
Curve/    ParametricCurve, LineCurve, BezierCurve, NurbsCurve, ArcCurve
Operations/  Boolean, Chamfer, Draft, Extrude, Fillet, Loft, Mirror,
             Offset, Pattern, Revolve, Shell, Split, Sweep
Primitives/  Box
Bounding/    BoundingBox, BoundingSphere
Coordinate/  CoordinateSystem
Profile/     RectangleProfileBuilder
```

Позднее (P0 — чистка мёртвых дублей) удалено дополнительно:

```text
Topology/    Vertex, Edge, Face, Shell, Loop, Body, Assembly, Solid,
             TopologyStore, TopologyFwd (legacy; держались только тестами
             EdgeTest/ExtendedDomains, переведены на BRep-покрытие)
Line/        Line
Ray/         Ray
Segment/     Segment
Direction/   Direction
GeometryFwd.hpp   (forward-объявления удалённых типов)
Math/Tolerance.hpp       (дубль Precision)
Math/Transform/Transform.hpp  (compat-заглушка #include ../Transform.hpp)
```

Геометрические операции живут в `MirEngine/BRep` (Builders, Boolean, Converters)
и `MirEngine/Geometry/Model`. Дублирующие Solid/Surface/Operations-системы
создавать нельзя.

TARGET:

```text
MirEngine/Geometry/
├── Curves/
├── Surfaces/
├── Primitives/
├── Intersections/
├── Distance/
├── Projection/
└── Algorithms/
```

`mir::` здесь временный legacy. Новый код — `mir4d::`.

## 12. BRep

TARGET:

```text
MirEngine/BRep/
├── Core/
├── Geometry/
├── Topology/
├── Builders/
├── Boolean/
├── Tessellator/
├── Converters/
├── Commands/
└── Validator/
```

BRep — единственный владелец topology. Features не создают вторую topology-систему.

`BRep/Geometry/BRepGeometry.hpp` использует фактический canonical-файл `MirEngine/Math/Vector/Vector.hpp`; отсутствующий `Vector3.hpp` больше не должен использоваться как include-цель.

## 13. Sketch

```text
MirEngine/Sketch/
├── Sketch.hpp
├── Entities/
├── Constraints/
├── Solver/
├── Inference/
├── Profiles/
└── Commands/
```

## 14. Features

```text
MirEngine/Features/
├── Feature.hpp
├── FeatureGraph.hpp
├── FeatureContext.hpp
├── FeatureResult.hpp
├── SketchBased/
├── Solid/
├── Pattern/
├── Transform/
└── Commands/
```

## 15. Document — корень инженерной модели

Фактически существующие canonical-файлы:

```text
MirEngine/Document/
├── Document.hpp
├── Command.hpp
├── CommandHandler.hpp
├── CreateBoxCommandHandler.hpp
├── DocumentHistory.hpp
└── DocumentSnapshot.hpp
```

### Canonical ownership

```text
mir4d::Document
├── name
├── ObjectRegistry
├── ObjectStore
├── DocumentHistory        ← transition
├── Time
├── revision
└── modified
```

`Document` API Identity:

```text
objectRegistry()
allocateObjectId()
reserveObjectId()
releaseObjectId()
```

`CreateBoxCommandHandler` является canonical command implementation в `mir4d`, а геометрические типы, которые ещё находятся в legacy `mir::`, используются только через явную границу.

## 16. ObjectStore — новый canonical boundary

```text
MirEngine/Document/ObjectStore.hpp
```

`ObjectStore` является ownership boundary между `Document` и текущей legacy spatial Scene.

Сейчас:

```text
Document
   ↓
ObjectRegistry
   ↓
ObjectStore
   ↓
mir::Scene       ← временная реализация storage
   ↓
ModelNode
```

Критически важно: `ObjectStore` сначала сохраняет ссылку на `Document`-owned `ObjectRegistry`, затем конструирует `Scene` с той же ссылкой. Второго document-level allocator нет.

`ObjectStore` предоставляет:

```text
scene()
registry()
size()
empty()
find(ObjectId)
contains(ObjectId)
isValid()
clear()
```

### Следующая миграция

```text
ObjectStore
   ↓
ModelNode → MirEngine/Document/ModelNode.hpp
   ↓
ObjectStore native storage
   ↓
удаление зависимости Document → mir::Scene
```

## 17. ModelNode / Scene migration

Текущее состояние:

```text
MirEngine/Geometry/Model/ModelNode.hpp
MirEngine/Geometry/Scene/Scene.hpp
```

Они являются transitional legacy boundary.

`ModelNode` уже использует canonical `mir4d::ObjectId`.

`Scene` уже использует canonical `mir4d::ObjectId` и `mir4d::ObjectRegistry`, включая корректное освобождение ID при удалении/очистке.

Целевое состояние:

```text
MirEngine/Document/ModelNode.hpp
MirEngine/Document/ObjectStore.hpp
```

`ModelNode` должен в итоге использовать `mir4d::ObjectId` и `mir4d::Transform` и не владеть собственной системой ID.

`Scene` после переноса ответственности удаляется как document ownership concept. Render может иметь собственные snapshots, но не Scene как источник истины.

## 18. History / Time / FourD

TARGET:

```text
MirEngine/History/
├── Command.hpp
├── Transaction.hpp
├── CommandHistory.hpp
├── UndoRedo.hpp
└── Replay.hpp

MirEngine/Time/
├── Time.hpp
├── TimePoint.hpp
├── TimeRange.hpp
└── Timeline.hpp

MirEngine/FourD/
├── FourDDocument.hpp
├── State.hpp
├── Timeline.hpp
├── Playback.hpp
└── ChangeSet.hpp
```

`DocumentHistory` — переходный существующий API. Вторую параллельную History-систему создавать нельзя.

`MirEngine/Time/TimeMachine.hpp` теперь находится в `namespace mir4d` и работает через canonical `Document`, `CommandHandler`, `Command` и `Time`.

## 19. Interaction

```text
MirEngine/Interaction/
├── SelectionState.hpp
├── RayPicker.hpp
├── SelectionQuery.hpp          TARGET
├── InputState.hpp              TARGET
└── Commands/                   TARGET
```

`SelectionState` пока остаётся transitional-классом в `mir`, но все его идентификаторы используют только `mir4d::ObjectId`. Новый Interaction API создаётся в `mir4d`.

## 20. Render

Единый контур рендеринга:

```text
MirEngine/Rendering/
├── Core/            RenderCommand, RenderContext, RenderDevice, RenderPass
├── Resources/       Shader, ShaderLibrary, Vertex, VertexBuffer, IndexBuffer, VertexArray
├── Passes/          GridPass, GeometryPass
├── OpenGL/          OpenGLRenderer, OpenGLShader, OpenGLDevice, OpenGLState,
│                    OpenGLVertexArray, OpenGLVertexBuffer, OpenGLIndexBuffer,
│                    OpenGLContext, OpenGLDebug, OpenGLCapabilities
├── Selection/       RenderSelection, RenderSelectionProperties, форматирование свойств
├── Geometry/        контуры и преобразования
└── Material/        материалы
```

Проходы рендеринга: фон (студийный градиент), процедурная бесконечная сетка с осями (GridPass) и геометрия сцены (GeometryPass, camera-relative, с подсветкой выделения). Выделение объектов передаётся в контекст кадра через `RenderContext::selectionIds` (указатель на `SelectionState::ids()`).

Legacy `MirEngine/Render/` перенесён в `MirEngine/Rendering/` (namespace `MirEngine::Rendering`). Каноническая камера и viewport runtime живут в `MirEngine/Viewport/` (`Camera.hpp`, `ViewportRuntime.hpp`).

Render никогда не изменяет Document.

## 21. IO

```text
MirEngine/IO/
├── ImportService.hpp/.cpp
├── ExportService.hpp/.cpp
├── Mesh/
└── Step/
```

IO command handlers используют canonical `mir4d::Command`, `mir4d::CommandHandler` и `mir4d::CommandResult`.

`Exporter` и `StlExporter` используют `mir4d::Document`; selection IDs используют `mir4d::ObjectId`.

## 22. Config

Только:

```text
MirEngine/Config/
└── Materials/
    └── water.mir.material
```

Корневой `Config/` запрещён.

`Config` хранит конфигурацию и данные, но не является владельцем Document или runtime state.

## 23. Tests

Все тесты находятся внутри Engine:

```text
MirEngine/Tests/
├── Core/
├── Math/
├── Geometry/
├── BRep/
├── Sketch/
├── Features/
├── Document/
├── History/
├── Interaction/
├── Render/
├── IO/
└── Integration/
```

Текущие архитектурно значимые тесты:

```text
MirEngine/Tests/Document/ObjectStoreTests.cpp
MirEngine/Tests/Interaction/SelectionStateTests.cpp
MirEngine/Tests/Integration/Integration4D.cpp
```

Тесты `BuildCheck`, `CreateBox`, `SelectionState` и `Integration4D` используют canonical `mir4d` API там, где соответствующий слой уже мигрирован.

Корневой `Tests/` запрещён.

## 24. MirUI

```text
MirUI/
├── App/
├── Core/
│   ├── Events/
│   ├── Object/
│   └── Bridge/
├── Interop/
├── Viewport/
├── Widgets/
├── Renderers/
└── Swift/
```

`MirUI/Core/Object/ObjectID.hpp` — alias на `mir4d::ObjectId`.

`MirUI/Core/Transform/TransformProperties.hpp` использует `mir4d::ObjectId`; сам legacy `mir::Transform` остаётся временной геометрической границей.

SwiftUI не владеет C++ Document.

## 25. Остальные engine-подсистемы

```text
MirEngine/
├── Materials/
├── Assembly/
├── Drawing/
├── FourD/
├── Simulation/
├── Physics/
├── Mechanics/
├── Acoustics/
├── Chemistry/
├── AI/
└── Platform/
```

Если существующий файл уже выполняет ответственность — он переписывается под canonical owner. Новый дубликат не создаётся.

## 26. Тестовая сборка

`CMakeLists.txt` обязан регистрировать тесты только из `MirEngine/Tests`.

Текущие цели:

```text
MIR4D_BuildCheck
MIR4D_Integration4D
MIR4D_StlRoundTrip
MIR4D_DocumentRenderBridge
MIR4D_CreateBox
MIR4D_ObjectStore
MIR4D_BRepBox
MIR4D_BRepTopology
MIR4D_BRepProfileExtrude
MIR4D_SketchRectangleProfile
MIR4D_SketchRectangleSolver
MIR4D_ViewportFoundation
MIR4D_ViewportRuntime
MIR4D_SelectionState
```

## 27. Текущий завершённый этап

Выполнено:

```text
legacy IDs
    ↓
mir4d::ObjectId
    ↓
mir4d::ObjectRegistry
    ↓
mir4d::Document
    ↓
mir4d::ObjectStore
    ↓
legacy Scene boundary
```

Также выполнено:

- `mir4d::EventBus` — единая engine EventBus;
- engine `EventDispatcher` удалён;
- `Config` перенесён внутрь `MirEngine`;
- `Tests` перенесены внутрь `MirEngine`;
- `Time`, `Command`, `CommandHandler`, `DocumentHistory`, `DocumentSnapshot`, `Document` используют `mir4d::`;
- `Error` и `Result` используют canonical `mir4d::`;
- дублирующая `MirEngine/Core/Result/Result.hpp` удалена;
- legacy ID-системы удалены;
- `MirUI` использует canonical `mir4d::ObjectId`;
- `ObjectStore` введён как ownership boundary и корректно инициализирует registry до Scene;
- `Scene` при работе через Document использует Document-owned `ObjectRegistry`;
- `ModelNode` использует canonical `mir4d::ObjectId`;
- `SelectionState` использует canonical `mir4d::ObjectId`;
- BRep geometry использует фактический `Math/Vector/Vector.hpp`;
- IO export boundary использует canonical `mir4d::Document` и `mir4d::ObjectId`;
- `CreateBoxCommandHandler` переведён в `mir4d`;
- `TimeMachine` переведён в `mir4d`;
- тесты `BuildCheck`, `CreateBox`, `SelectionState`, `Integration4D` синхронизированы с canonical API.

Проход «битые include + чистка Geometry legacy» (следующий за миграцией Identity):

- починены include: `Math/Bounds/AABB.hpp` → `TransformMatrix.hpp`; `Math/PlanarMatrix.hpp` → `Vector/PlanarVector.hpp` и `Core/Types/Scalar.hpp`; `Geometry/Topology/Vertex.hpp` → `Math/Point.hpp`; `MirUI/Designer/Core/DesignerCore.hpp` → `Foundation/Animation/AnimationManager.hpp`; `Core/Services/IService.hpp` и `ServiceRegistry.hpp` → canonical `Core/Result.hpp` + `using mir4d::Result/ErrorCode`;
- `ServiceRegistry::initializeAll` возвращает `std::unexpected(Error{...})` через canonical `mir4d::Error`;
- удалены некомпилируемые/мёртвые файлы: `Core/Engine/Engine.{hpp,cpp}` (битый include, `IDGenerator` не существует), `Document/BasicCommandHandler.hpp`, `MirUI/Exports/MirUIExports.{cpp,h}` (включал несуществующий `Application/CADApplication.hpp`);
- удалён legacy-срез Geometry (см. §11): Solid (кроме FacetedSolid), Surface, Plane, сломанные Curve, Operations, Primitives, Bounding, Coordinate, RectangleProfileBuilder;
- `Geometry.hpp` больше не включает `Bounding/BoundingBox.hpp`;
- `SketchGeometryStore` получил `add(SketchGeometry)`, `find(id)`, `findMutable(id)` — Sketch-команды (Create/Parameter/Drag/Resolver) снова компилируются.

## 28. Проверка CI

Последний известный старый CI-запуск до этого исправляющего прохода завершился ошибкой из-за массового несоответствия namespace после миграции Identity и нескольких отсутствующих include/constructor initialization.

Исправлены причины, подтверждённые этим запуском:

```text
Math/Vector/Vector3.hpp not found
ObjectId / InvalidObjectId / isValidObjectId namespace mismatch
ObjectStore reference member not initialized
CommandHandler / Command / CommandResult namespace mismatch
Document namespace mismatch
TimeMachine namespace mismatch
MirUI TransformProperties namespace mismatch
Exporter / StlExporter Document boundary mismatch
```

После последнего исправляющего коммита GitHub Actions запущен заново. Его результат является внешней проверкой и не считается PASS до фактического завершения workflow.

## 29. Следующий архитектурный проход

```text
ObjectStore
    ↓
ModelNode
    ↓
native ObjectStore storage
    ↓
Document FeatureGraph
    ↓
Parameters
    ↓
Dependencies
    ↓
History consolidation
    ↓
Geometry
    ↓
BRep
```

Цель следующего прохода — убрать `Document → mir::Scene` и сделать `ObjectStore` настоящим canonical storage, не создавая ещё одну параллельную модель.

Перед этим закрыть остатки предыдущего прохода:

```text
[v] MirUI Designer: WidgetDescriptor.hpp (битый include ../Inspector/PropertyDescriptor.hpp),
                   Container.hpp (LayoutDirection/Insets не включены), UIProject.hpp (WidgetFactory сигнатура)
[v] BRep: BRepExtrudeBuilder.hpp (include "BRepValidator.hpp" без пути → Validator/BRepValidator.hpp)
[x] Geometry Topology legacy: Vertex/Edge/Face/Shell/Loop/Body/Assembly/Solid/
                   TopologyStore удалены (P0); тесты переведены: EdgeTest удалён
                   (покрыт MIR4D_BRepTopology), ExtendedDomains избавлен от Assembly
[x] Geometry Query: Intersections/Distance/Projection (P3, см. §29.4)
[ ] Geometry TARGET: Curves/Surfaces/Primitives/Algorithms — дальнейшее развитие
```

### 29.1. MirUI Designer — доведение до компиляции

Ядро Designer (DesignerCore, DesignerApplication, InspectorModel, PreviewManager,
UIProjectSerializer, WidgetLibrary, WidgetFactory) переведено в компилируемое состояние:

```text
[v] WidgetFactory::create(WidgetType, name) → std::unique_ptr<Widget> (убраны m_owned/clearOwned)
[v] WidgetDescriptor (Schema) и WidgetDescriptor (Designer) — конфликт имён: Designer → WidgetCatalogDescriptor
[v] PropertyDescriptor: StateValue → PropertyValue; PropertyType::Float → PropertyType::Number
[v] ThemeManager: поле theme → current(); UIProjectSerializer сохраняет тему через registerTheme/setTheme
[v] AnimationManager: пустой Foundation/Animation/AnimationManager.hpp дополнен контрактом
    (setWidgetTree/update/animate/stopWidget); анимационный движок — отдельная веха
[v] Отсутствующие include: WidgetClipboard, ChangePropertyCommand, UIReader/UIWriter,
    GridManager/GuideManager (включены над namespace — иначе классы попадали в вложенный namespace)
[v] GuideManager: setVisible/isVisible (синоним setEnabled/isEnabled)
```

### 29.2. Зонтик MirUI.hpp — закрыт

Обзорная TU (все includes MirUI.hpp) вскрыла никогда не компилировавшийся пласт
(Themes/Schema/Workspace/Inspector editors). Все пункты исправлены:

```text
[v] Core/Layout: Unit.hpp и LayoutData.hpp объявляли enum Unit дважды —
    LayoutData.hpp теперь включает Unit.hpp (канонический Unit остаётся один)
[v] Schema/PropertySchema.hpp: свой struct PropertyDescriptor — конфликт
    с Core/Widget/PropertyDescriptor.hpp; переименован в SchemaPropertyDescriptor
[v] Designer/Inspector: ColorEditor/PropertyEditor — Color::fromHex/toHex добавлены
    в Foundation/Color/Color.hpp (форматы "#RRGGBB" и "#RRGGBBAA")
[v] Designer/Canvas: DragController — DesignerCanvas::HitZone перенесён в public
[v] Designer/Themes: ThemeEditor/ColorTokenEditor/MetricsEditor/TypographyEditor —
    ThemeManager::theme → current() + сохранение изменённой темы
    через registerTheme/setTheme (7 мест)
[v] Workspace/LayoutManager: добавлено недостающее определение DockPosition
[v] PropertyEditor: break вне switch (удалён)
```

Остаются только предупреждения C4100/C4101/C4189 в pre-existing коде
(LayoutManager, RenderCommandBuffer, UIFormatVersion, UIWriter) — не ошибки.

### 29.3. BRep Boolean API — P1/P2/P2.1/P2.2/P2.3

```text
[v] BRepBooleanAPI::fuse — объединение:
    • непересекающиеся тела        → копия обоих (makeSolid из оболочек);
    • пересекающиеся box'ы         → точное объединение: общая сетка
      координат (границы обоих тел), замощение 12 граней клетками,
      клетки, полностью поглощённые другим телом, не строятся;
      вершины и рёбра дедуплицируются по сетке (gridVertex/gridEdge),
      контуры клеток CCW снаружи (по cross(axisA, axisB)), общая
      топология соседних клеток и граней;
    • прочие тела                  → NotImplemented (kernel пересечений)
[v] BRepBooleanAPI::cut — разность:
    • tool не пересекает A            → копия A (математически A \ B = A);
    • tool строго внутри A            → полость: внешняя оболочка A +
      внутренняя оболочка B с обратной ориентацией (замкнутая полость);
    • сквозной проход (tool протыкает → туннель: отверстия (inner wires)
      на торцах A + внутренняя оболочка «короб» из 4 граней вдоль оси
      прохода; требует axis-aligned box'ы, tool без inner wires;
      passThroughAxis: протыкание по одной оси и строгое попадание
      по двум другим);
    • частичное перекрытие box'ов    → вырез W = A ∩ B: клетки граней A
      вне W (нестрогое поглощение — дыры открываются и на гранях,
      совпадающих с W) + стенки сторон W, не совпадающих с границами A;
      кольца клеток сшиваются со стенками по рёбрам общей сетки —
      единая замкнутая оболочка без inner wires (колодец / угловой
      вырез / вырез с касанием грани);
    • прочие тела                     → NotImplemented (обобщённый kernel)
[v] BRepBooleanAPI::common — пересечение:
    • не пересекаются                 → EmptyResult;
    • оба — axis-aligned box'ы        → новый box по пересечению AABB;
    • прочие тела                     → NotImplemented
[v] copyModel: опция копирования без оболочек (copyShells=false) для
    пересборки оболочек; remap пустой карты → InvalidBRepIndex
[v] Отверстия: BRepTopologyEditor::addInnerWire — inner wires на гранях
    торцов (cut-through)
[v] Тест MIR4D_BRepBoolean (таргет + add_test): 15 сценариев (fuse ×6,
    cut ×9, common ×2, execute ×1)
[v] P2.4a: BRepPrimAPI_MakeBox::buildOriented — box в произвольном
    ортонормированном базисе (повёрнутый box): 8 вершин в базисных
    координатах, грани с базисными нормалями; build() — обёртка над
    buildOriented с единичным базисом; невалидный базис отклоняется;
    булевы операции для повёрнутых box'ов честно возвращают
    NotImplemented; тест MIR4D_BRepBox расширен (проверка вершин
    повёрнутого box'а + честные отказы)
[ ] Обобщение замощения на произвольные (не плоскостные, не
    параллельные осям) грани — следующий проход (P2.4b: разбиение
    рёбер через GeometryQuery + сшивка граней для повёрнутых box'ов)
```

Ограничения честно задокументированы в коде: каждый нереализованный
случай возвращает NotImplemented (или EmptyResult), а не невалидный результат.

### 29.4. Geometry Query — P3 (Intersections / Distance / Projection)

Канонический аналитический слой CAD-запросов — фундамент для булева ядра
(разбиение рёбер/граней), измерений, привязок и предварительных выборов.
Не зависит от рендера, B-Rep и UI.

```text
[v] Примитивы: Line3 (P(t) = origin + direction·t), Ray3 (t ≥ 0),
    Segment3 (t ∈ [0,1]); направления не обязаны быть единичными
    (MirEngine/Geometry/Query/Query.hpp)
[v] Projection: точка → прямая / луч / отрезок (projectPointOnLine/Ray/Segment);
    точка → плоскость — MathPlane::project (не дублируется)
[v] Distance: точка → прямая/луч/отрезок; прямая → прямая
    (скрещивающиеся и параллельные); отрезок → отрезок
    (Ericson, Real-Time Collision Detection, 5.1.9 — кратчайший отрезок
    с ограничением параметров)
[v] Intersections: прямая/луч/отрезок → плоскость (параметр за пределами
    → nullopt); прямая → прямая (параллельные/скрещивающиеся → nullopt);
    отрезок → отрезок (общий случай, коллинеарное перекрытие — точка
    начала общей части); луч → треугольник (Мёллер–Трумбор)
[v] MathPlane: добавлены перегрузки signedDistance/distance/project/
    isOnPositiveSide для Point3 (аддитивно, Vector3-версии сохранены)
[v] Зонтик Geometry.hpp включает Query/Query.hpp; все функции noexcept,
    дегенеративные входы дают nullopt
[v] Тест MIR4D_GeometryQuery (таргет + add_test): 48 проверок —
    примитивы, проекции, расстояния, пересечения, треугольник
[ ] Примитивы → CAD: использование Line3/Segment3 в булевом kernel
    (разбиение рёбер) и в скетч-привязках — следующий проход
```

## 30. Правило обновления этого файла

При любом изменении:

- папки;
- файла;
- namespace;
- canonical owner;
- зависимости;
- API;
- TARGET-структуры;
- порядка миграции;

`MIR4D_ARCHITECTURE.md` обновляется **в том же ответе и том же архитектурном проходе**.

Других архитектурных файлов не создаём.
