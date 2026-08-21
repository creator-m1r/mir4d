import Foundation

/// Типобезопасная обёртка над собственной VFX-подсистемой MIR4D (MirEngine::VFX).
///
/// Подсистема полностью принадлежит MIR4D: симуляция частиц идёт на CPU,
/// отрисовка делегируется зарегистрированному sink'у. Это заменяет любые
/// внешние/сторонние движки эффектов собственным, контролируемым модулем.
public enum MIRVFXKind: Int32 {
    case confetti = 0
    case balloons = 1
    case fireworks = 2
    case rain = 3
    case hearts = 4
    case lasers = 5
    case sparks = 6
    case snow = 7
    case bubbles = 8
    case sparkles = 9
    case petals = 10
    case streamers = 11
    case scanGrid = 12
    case assemble = 13
    case smoke = 14
    case fire = 15
    case warnBurst = 16
    case selectPulse = 17
    case success = 18
}

public struct MIRVFX {
    /// Запускает эффект в собственной подсистеме MIR4D.
    public static func trigger(_ kind: MIRVFXKind) {
        MirEngineVFXTrigger(Int32(kind.rawValue))
    }

    /// Продвигает симуляцию на dt секунд.
    public static func update(_ dt: Float) {
        MirEngineVFXUpdate(dt)
    }

    /// Отрисовывает живые частицы через зарегистрированный sink.
    public static func render() {
        MirEngineVFXRender()
    }

    /// Сбрасывает все активные эффекты.
    public static func reset() {
        MirEngineVFXReset()
    }
}
