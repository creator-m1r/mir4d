# МИР 4D — Hand Modeling Vertical Slice v0.1

**Статус:** Architecture / Implementation Slice  
**Цель:** первый полностью замкнутый путь от жеста руки перед камерой MacBook до реального изменения инженерной модели в MirEngine с поддержкой Preview, Command, History, Undo/Redo и возвратом результата во viewport.

## 1. Главная цель

Реализовать первый настоящий сценарий пространственного моделирования руками:

> **Pinch → указать на тело → захватить → переместить руку → отпустить → объект реально перемещён в MirEngine.**

Жесты не должны оставаться UI-анимацией. Любое подтверждённое изменение модели должно проходить через Command и History.

## 2. Вертикальный поток

```text
MacBook Camera
      ↓
Vision / VNHumanHandPoseObservation
      ↓
MIRHandTrackingSource
      ↓
Temporal Filter
      ↓
MIRHandIntent
      ↓
MIRIntentRouter
      ↓
MIR4DIntentBridge
      ↓
C/C++ Interop
      ↓
MirEngine Command
      ↓
Document / Model
      ↓
History
      ↓
Renderer / Viewport
```

## 3. Слои

### 3.1 Hand Tracking

Источник должен выдавать:

- UUID руки;
- handedness;
- 21 landmark;
- palmPosition;
- palmNormal;
- confidence;
- timestamp.

Поддержать одновременно две руки.

### 3.2 Temporal Filter

Между Vision и Intent добавить фильтрацию:

- confidence threshold;
- dead zone;
- temporal smoothing;
- hysteresis;
- velocity estimation;
- optional prediction.

Для pinch использовать разные пороги включения и выключения, например:

```text
PINCH_BEGIN > 0.65
PINCH_END   < 0.40
```

чтобы исключить дрожание состояния.

## 4. MIRHandIntent

Ввести единый транспортный intent:

```swift
struct MIRHandIntent: Sendable {
    enum Action: Sendable {
        case hover
        case select
        case grabBegin
        case grabMove
        case grabEnd
        case sculptBegin
        case sculptMove
        case sculptEnd
    }

    let action: Action
    let handID: UUID
    let position: SIMD3<Double>
    let direction: SIMD3<Double>
    let confidence: Double
    let timestamp: Date
}
```

UI не должен напрямую изменять геометрию.

## 5. Intent Router

Создать маршрутизатор:

```text
MIRHandIntent
      ↓
MIRIntentRouter
      ├── Selection
      ├── Grab / Transform
      ├── Sculpt
      ├── Sketch
      └── Future CAD intents
```

Первым реализуется только Selection + Grab/Transform.

## 6. 3D Picking

Рука должна порождать физический 3D ray.

```cpp
struct HandRay {
    Vec3 origin;
    Vec3 direction;
};
```

Результат picking:

```cpp
struct PickResult {
    bool hit = false;
    ObjectId objectId = InvalidObjectId;
    FaceId faceId = InvalidFaceId;
    EdgeId edgeId = InvalidEdgeId;
    VertexId vertexId = InvalidVertexId;
    Vec3 worldPosition{};
    Vec3 normal{};
    double distance = 0.0;
};
```

API:

```cpp
PickResult RayPicker::pick(
    const Document& document,
    const HandRay& ray
);
```

Приоритет определения геометрии:

```text
Vertex → Edge → Face → Body → Object
```

Viewport должен визуально показывать элемент под рукой.

## 7. Калибровка

Ввести состояние калибровки:

```swift
struct MIRHandCalibration: Codable, Sendable {
    var scale: Double
    var offset: SIMD3<Double>
    var rotation: simd_quatd
    var viewportWidth: Double
    var viewportHeight: Double
}
```

Поток координат:

```text
Camera
 ↓
Hand landmarks
 ↓
Filter
 ↓
Calibration
 ↓
Viewport coordinates
 ↓
Ray
 ↓
World coordinates
```

Калибровка должна быть отдельным сервисом и не смешиваться с Vision.

## 8. GrabTransformCommand

Первый реальный Command ядра:

```cpp
class GrabTransformCommand final : public Command {
public:
    GrabTransformCommand(
        ObjectId objectId,
        const Transform& initial,
        const Transform& final
    );

    bool execute(Document& document) override;
    bool undo(Document& document) override;

private:
    ObjectId m_objectId;
    Transform m_initial;
    Transform m_final;
};
```

Правило: **не создавать Command на каждый camera frame**.

Один жест захвата = одна транзакционная команда.

## 9. Preview vs Commit

Во время движения:

```text
grabBegin
   ↓
Capture initial transform
   ↓
grabMove × N
   ↓
Preview Transform
   ↓
Viewport
```

После отпускания:

```text
grabEnd
   ↓
final transform
   ↓
GrabTransformCommand
   ↓
History.push()
```

Preview не должен засорять Undo stack.

## 10. History

Каждое подтверждённое изменение модели обязано проходить через History.

Минимальный цикл:

```text
Gesture
 ↓
Preview
 ↓
Commit
 ↓
Command
 ↓
History
 ↓
Document
```

Должны работать:

- Undo;
- Redo;
- повторяемость команды;
- отмена незавершённого Preview без изменения History.

## 11. C / C++ Interop

Создать стабильный C ABI между Swift и MirEngine.

Пример направления:

```c
bool mir4d_pick_hand_ray(
    MirHandleRay ray,
    MirPickResult* result
);

bool mir4d_begin_grab(
    MirObjectId objectId,
    MirTransform* initialTransform
);

bool mir4d_preview_grab(
    MirObjectId objectId,
    MirTransform transform
);

bool mir4d_commit_grab(
    MirObjectId objectId,
    MirTransform finalTransform
);

bool mir4d_undo(void);
bool mir4d_redo(void);
```

Имена должны быть адаптированы к существующему CBridge/Interop API проекта, а новые параллельные ABI без необходимости не создавать.

## 12. Состояния Grab

```text
Idle
 ↓ pinch
Candidate
 ↓ stable pick
Grabbed
 ↓ hand movement
Dragging
 ↓ release
Committed
```

При потере confidence:

```text
Dragging
   ↓ confidence low
Grace Period
   ├── tracking recovered → Dragging
   └── timeout → Cancel Preview
```

Это предотвращает случайное бросание модели.

## 13. Визуальная обратная связь

До захвата:

```text
hover → highlight
```

При захвате:

```text
grab → selected highlight
```

При движении:

```text
selected model → preview transform
```

После commit:

```text
normal rendering
```

Никаких постоянных debug-индикаторов поверх viewport.

## 14. Sculpt — следующий этап

После завершения Grab Vertical Slice подготовить интерфейс для Sculpt.

```cpp
struct SculptSample {
    Vec3 position;
    Vec3 normal;
    double radius;
    double strength;
    double timestamp;
};
```

```cpp
class SurfaceSculptCommand : public Command {
public:
    bool begin(...);
    bool addSample(const SculptSample& sample);
    bool commit(Document& document);
    bool undo(Document& document);
};
```

Поток:

```text
Hand gesture
 ↓
SculptStroke
 ↓
Raycast
 ↓
Surface samples
 ↓
Brush influence
 ↓
Deformation operator
 ↓
Mesh / BRep
 ↓
Command
 ↓
History
```

Sculpt не является частью обязательного v0.1 Grab milestone, но ABI и Command architecture должны быть спроектированы с его учётом.

## 15. Ограничения v0.1

Не делать пока:

- eye tracking;
- gaze selection;
- полноценный BRep sculpt;
- extrude;
- fillet;
- assembly constraints;
- sketch-in-air;
- сложные двухручные CAD operations.

Приоритет — стабильный один жестовой вертикальный срез.

## 16. Acceptance Criteria

Vertical Slice считается готовым только если одновременно выполнены все пункты:

- MacBook camera распознаёт руку;
- pinch стабильно определяется;
- рука создаёт ray;
- ray пересекает реальную модель в MirEngine;
- объект подсвечивается при наведении;
- pinch захватывает объект;
- перемещение руки меняет Preview Transform;
- отпускание создаёт один Command;
- Command изменяет модель в Document/MirEngine;
- History получает одну операцию;
- Undo возвращает исходное положение;
- Redo возвращает новое положение;
- потеря руки не оставляет модель в повреждённом промежуточном состоянии;
- Preview не попадает в History;
- обычное мышиное управление продолжает работать.

## 17. Definition of Done

```text
CAMERA
  ✓
HAND TRACKING
  ✓
FILTER
  ✓
INTENT
  ✓
ROUTER
  ✓
3D RAY
  ✓
PICK
  ✓
HIGHLIGHT
  ✓
GRAB
  ✓
PREVIEW
  ✓
COMMAND
  ✓
HISTORY
  ✓
UNDO / REDO
  ✓
MIRENGINE MODEL UPDATE
  ✓
VIEWPORT UPDATE
  ✓
```

## 18. Следующий milestone

После закрытия v0.1:

**MIR 4D Hand Modeling v0.2 — Surface Sculpt**

Сценарий:

```text
🤏 touch surface
       ↓
press
       ↓
move hand
       ↓
local deformation
       ↓
release
       ↓
SurfaceSculptCommand
       ↓
History
```

Это будет первая версия, в которой пользователь сможет буквально **«прикоснуться» к модели и начать формировать её руками**.
