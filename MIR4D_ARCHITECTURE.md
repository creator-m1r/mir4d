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
└── Tolerance.hpp
```

Существующую хорошую математическую реализацию переносим в canonical-файл, а не дублируем.

## 11. Geometry

Фактически существующие направления, которые переписываются:

```text
Curve
Direction
Line
Model
Operations
Plane
Point
Primitives
Profile
Ray
Scene
Segment
Solid
Tessellation
Vector
```

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
├── Resources/       Shader, ShaderLibrary, Texture, Framebuffer, Vertex, Buffer
├── Passes/          GridPass, GeometryPass, GizmoPass, OverlayPass, SelectionPass
├── OpenGL/          OpenGLRenderer, OpenGLFrameRenderer, OpenGLShader, OpenGLTexture,
│                    OpenGLFrameBuffer, OpenGLMeshRenderer, OpenGLShaderProgram
├── Camera/          RenderCamera, RenderTypes
├── Selection/       выделение объектов (RenderSelection, picking, hit-testing)
├── Viewport/        RenderViewport, ViewportController
├── Geometry/        контуры и преобразования
└── Material/        материалы
```

Legacy `MirEngine/Render/` перенесён в `MirEngine/Rendering/` (namespace `MirEngine::Rendering`).

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
