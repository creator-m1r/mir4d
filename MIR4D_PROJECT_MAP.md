# МИР 4D — каноническая карта проекта

Версия: 1.9
Дата: 2026-08-14

Этот файл является единой картой архитектуры репозитория. Перед структурными изменениями сначала читается эта карта; новые файлы создаются только в соответствующем слое, а старые имена сохраняются лишь как совместимые фасады, если это необходимо для переходного периода.

## 1. Архитектурный принцип

```text
MirEngine/Math
    ↓
MirEngine/Geometry
    ↓
MirEngine/Sketch
    ↓
MirEngine/BRep/Core
    ↓
MirEngine/BRep/Topology + Geometry
    ↓
MirEngine/BRep/Builders / Boolean / Converters / Validator
    ↓
MirEngine/Document / Scene / Commands
    ↓
MirEngine/Features
    ↓
MirEngine/Rendering + Viewport
    ↓
MirEngine/IO
    ↓
MirUI C++ interop
    ↓
SwiftUI
```

Зависимость разрешена только сверху вниз по этой схеме. Core не знает о UI, GPU, SwiftUI, OpenGL, CMake и STEP/OCCT.

## 2. Канонические области MirEngine

- `Math/` — Vector/Matrix/Quaternion, скаляры, численная база. Вторые реализации математических типов запрещены.
- `Geometry/` — независимая математическая геометрия и базовые геометрические сущности; legacy-топология (Vertex/Edge/Face/Shell/Loop/Body/Assembly/Solid) и примитивы (Line/Ray/Segment/Direction) удалены; остались Curve (Curve/CurveLoop/Circle/Arc), Model (Profile/FacetedSolid/TriangleMesh3), Query (Line3/Ray3/Segment3 + GeometryQuery: пересечения, расстояния, проекции — P3, тест MIR4D_GeometryQuery), Scene, Tessellation.
- `Sketch/` — эскизы и профили (namespace mir, header-only, ~46 файлов).
- `BRep/Core/` — владельцы модели, транзакционные checkpoints и стабильные handle-типы.
- `BRep/Topology/` — топологические записи, их связи и редактор.
- `BRep/Geometry/` — геометрические носители и связи topology → geometry.
- `BRep/Builders/` — построители параметрической геометрии и топологии; составные операции обязаны быть транзакционными. MakeBox: build (оси X/Y/Z) и buildOriented (произвольный ортонормированный базис — повёрнутый box, P2.4a).
- `BRep/Boolean/` — операции над B-Rep. Fuse (непересекающиеся тела; пересекающиеся axis-aligned box'ы — точное объединение замощением граней по общей сетке), Cut (копия / замкнутая полость / сквозной проход-туннель с отверстиями на торцах / частичное перекрытие box'ов — вырез со стенками единой оболочкой), Common (пересечение axis-aligned box'ов) реализованы (BRepBooleanAPI, тест MIR4D_BRepBoolean, 15 сценариев); обобщение на произвольные грани — следующий проход.
- `BRep/Converters/` — преобразования между Sketch/Model/B-Rep.
- `BRep/Tessellator/` — дискретизация B-Rep в render mesh.
- `BRep/Validator/` — проверки геометрии и топологии.
- `BRep/Commands/` — адаптеры командного слоя; здесь не размещается UI.
- `Document/` — документ, история, объекты и жизненный цикл проектных данных.
- `Features/` — параметрические операции моделирования (эскиз → операция → B-Rep).
- `Scene/` — представление объектов для пространственной сцены.
- `Rendering/` — CPU/GPU render bridge и render cache.
- `Viewport/` — viewport-слой и камера.
- `IO/` — STL/STEP и прочие внешние форматы. OCCT остаётся только здесь.
- `Time/` — модель времени и 4D-данные.
- Научные модули: `Simulation/`, `Physics/`, `Mechanics/`, `Materials/`, `Chemistry/`, `Acoustics/`, `World/`, `Interaction/`, `Platform/`, `Config/` — развиваются отдельно от геометрического ядра.
- `MirUI/` — presentation/interoperability layer: `Foundation/` (без зависимостей), `Core/` (виджеты, состояние, темы, команды), `Schema/`, `Widgets/`, `Workspace/`, `Designer/` (визуальный редактор), `Interop/` (CBridge), `Renderers/`, `Swift/`, `App/`, `Viewport/`.
- `MirServer/` — подсистема совместной работы и обмена с сервером MIR 4D (Swift, независимый library target):
  - `Models/` — `MirServerModels.swift` (MirTeamMember, MirTeamMessage, MirProjectExportRequest/Result, MirServerConnectionStatus) — все `Sendable`/`Codable`.
  - `Core/` — `MirServerConfiguration.swift` (конфигурация подключения к сайту/API) и `MirServerTransport.swift` (сетевой `actor`: REST-экспорт проекта, WebSocket-поток сообщений команды).
  - `Events/` — `MirServerEvents.swift` (Event Bus: `.mir4DServerStatusChanged`, `.mir4DTeamMessageReceived`, `.mir4DTeamUpdated`, `.mir4DProjectExported`, `.mir4DServerError`).
  - `Export/` — `MirProjectExporter.swift` (упаковка каталога проекта в переносимый `Data`-архив без зависимости от UI/CAD-ядра).
  - `Chat/` — `MirTeamChat.swift` (`@MainActor` надстройка: лента сообщений, подписка на Event Bus).
  - `Collaboration/` — подсистема совместной работы инженеров над одним проектом:
    - `Models/MirCollaborationModels.swift` — `MirOperationClock` (Lamport-часы), `MirCollaborator` (presence), `MirCollaborationOperation`, `MirProjectSnapshot`, `MirConflict`, `MirCollaborationEnvelope`/`MirCollaborationWireMessage` (Wire-формат WebSocket).
    - `MirCollaborationController.swift` — `@MainActor ObservableObject` координатор: присоединение к общему проекту, локальные/входящие операции, Lamport-синхронизация, presence, разрешение конфликтов (last-writer-wins по часам), протокол `MirCollaborativeDocument` для применения операций к геометрии (адаптер предоставляет уровень приложения).
  - `MirServer.swift` — `MirServerManager` (`@MainActor` координатор: connect/disconnect, отправка сообщений, экспорт, broadcastCollaboration, открытие проекта в браузере).
  - Интеграция UI (`MirUI/App`): `MIR4DTeamServerView.swift` (чат/экспорт), `MIR4DCollaborationView.swift` (совместная работа), команды меню и окна `mir4d-server` / `mir4d-collab`. Создание примитива в `MIR4DModelCommands` транслируется в операцию совместной работы.
  - Зависимости: Foundation, AppKit (только `NSWorkspace` для открытия сайта). MirEngine и SwiftUI не требуются; модуль изолирован от CAD-ядра.

## 3. Канон B-Rep

Публичная точка входа: `MirEngine/BRep/BRep.hpp`.

### Core
- `Core/BRepHandles.hpp`
- `Core/BRepModel.hpp` — единая точка checkpoint/rollback для многошаговых построителей.

### Topology
- `Topology/BRepTypes.hpp` — типы, индексы, ориентации и tolerance.
- `Topology/BRepTopology.hpp` — Vertex/Edge/Wire/Face/Shell/Solid записи.
- `Topology/BRepTopologyStore.hpp` — владение и доступ к topology + rollback checkpoint.
- `Topology/BRepTopologyEditor.hpp` — контролируемая модификация topology.

### Geometry
- `Geometry/BRepGeometry.hpp` — point/curve/surface носители.
- `Geometry/BRepGeometryLinks.hpp` — связи topology ↔ geometry.
- `Geometry/BRepGeometryStore.hpp` — владение geometry + rollback checkpoint.
- `Geometry/BRepAdaptor.hpp` — адаптация геометрии для операций.

### Builders
Все построители находятся только в `BRep/Builders/`. Составная операция сначала проверяет входные связи, затем создаёт записи под checkpoint; при любой ошибке модель возвращается к исходному состоянию.

### Validation
Канонический путь для `BRepGeometryValidator` — `BRep/Validator/BRepGeometryValidator.hpp`. Старый путь в `Geometry/` допускается только как compatibility facade.

### Tessellation
Канонический B-Rep tessellator — `BRep/Tessellator/BRepTessellator.hpp`. Дублирующая реализация в `MirEngine/Tessellation/` не должна развиваться параллельно; при миграции используется один adapter/facade.

## 4. Правила include

Использовать абсолютные project-relative includes:

```cpp
#include "MirEngine/BRep/Core/BRepModel.hpp"
```

Запрещать новые хрупкие цепочки вида `../Core/...` между архитектурными слоями.

Header должен включать только необходимые зависимости. Forward declaration предпочтительнее тяжёлого include там, где тип используется только через указатель/ссылку.

## 5. Правила владения и транзакций

- `BRepModel` владеет stores и корневыми solids.
- Topology хранит ссылки через handle/index, а не через случайные raw owning pointers.
- Geometry хранится отдельно от topology.
- Scene/Document владеют объектами уровня приложения, но не подменяют BRepModel.
- GPU cache никогда не является источником истины для CAD-данных.
- Checkpoint относится к текущему синхронному изменению модели и откатывает только добавленные после него записи.
- Публичные builder-операции не должны оставлять частично созданные сущности при неуспешном результате.
- После успешного построения существующие handle'ы не меняются.

## 6. Правила изменений

1. Сначала изменить канонический файл.
2. Затем обновить все includes и точки сборки.
3. Compatibility header создавать только при реальной необходимости.
4. Не дублировать реализацию ради старого имени.
5. Не добавлять UI-зависимости в MirEngine.
6. Не добавлять OCCT в Core/BRep.
7. Не создавать вторые Vector3/Matrix/Transform.
8. Любая новая операция должна иметь место в архитектурной карте.
9. Любая многошаговая мутация должна быть либо предварительно валидирована целиком, либо защищена checkpoint/rollback.
10. Тесты должны проверять не только успешный результат, но и отсутствие побочных изменений после отказа операции.

## 7. Целевой полный цикл МИР 4D

```text
Идея
 → параметрическая модель
 → B-Rep
 → tessellation
 → игровая цифровая сцена
 → физические/технологические свойства
 → симуляция и испытания
 → чертежи и спецификации
 → технологическая подготовка
 → экспорт в производство
 → обратная связь производства
 → обновление цифрового двойника
```

B-Rep является точной геометрической основой этого цикла, но не должен поглощать Document, Simulation, Manufacturing или UI.
