import Foundation
import AVFoundation
import Combine

@MainActor
final class MIR4DCapturePermissionController: ObservableObject {
    enum Kind: Equatable {
        case camera
        case microphone
    }

    enum State: Equatable {
        case notDetermined
        case authorized
        case denied
        case restricted
    }

    @Published private(set) var cameraState: State = .notDetermined
    @Published private(set) var microphoneState: State = .notDetermined
    @Published private(set) var isRequesting = false

    init() {
        refresh()
    }

    func refresh() {
        cameraState = Self.state(for: .video)
        microphoneState = Self.state(for: .audio)
    }

    func request(_ kind: Kind) async -> Bool {
        isRequesting = true
        defer {
            isRequesting = false
            refresh()
        }

        let mediaType: AVMediaType = kind == .camera ? .video : .audio
        let current = AVCaptureDevice.authorizationStatus(for: mediaType)

        if current == .authorized { return true }
        guard current == .notDetermined else { return false }

        return await AVCaptureDevice.requestAccess(for: mediaType)
    }

    func requestIfNeeded(for kind: Kind) async -> Bool {
        let mediaType: AVMediaType = kind == .camera ? .video : .audio
        guard AVCaptureDevice.authorizationStatus(for: mediaType) == .notDetermined else {
            refresh()
            return AVCaptureDevice.authorizationStatus(for: mediaType) == .authorized
        }
        return await request(kind)
    }

    var cameraReady: Bool { cameraState == .authorized }
    var microphoneReady: Bool { microphoneState == .authorized }

    private static func state(for mediaType: AVMediaType) -> State {
        switch AVCaptureDevice.authorizationStatus(for: mediaType) {
        case .authorized: return .authorized
        case .denied: return .denied
        case .restricted: return .restricted
        case .notDetermined: return .notDetermined
        @unknown default: return .restricted
        }
    }
}

extension MIR4DCapturePermissionController.State {
    var title: String {
        switch self {
        case .notDetermined: return "Требуется разрешение"
        case .authorized: return "Разрешено"
        case .denied: return "Запрещено"
        case .restricted: return "Ограничено системой"
        }
    }

    var systemImage: String {
        switch self {
        case .notDetermined: return "questionmark.circle"
        case .authorized: return "checkmark.circle.fill"
        case .denied: return "xmark.circle.fill"
        case .restricted: return "exclamationmark.triangle.fill"
        }
    }
}
