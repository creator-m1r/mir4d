import Foundation
import AVFoundation
import Combine

@MainActor
final class MIR4DSystemCaptureAuthorizationViewModel: ObservableObject {
    enum Status: Equatable {
        case notDetermined
        case authorized
        case denied
        case restricted

        var title: String {
            switch self {
            case .notDetermined: return "Требуется разрешение"
            case .authorized: return "Системный доступ разрешён"
            case .denied: return "Доступ запрещён системой"
            case .restricted: return "Доступ ограничен системой"
            }
        }
    }

    @Published private(set) var camera: Status = .notDetermined
    @Published private(set) var microphone: Status = .notDetermined

    func refresh() {
        camera = status(for: .video)
        microphone = status(for: .audio)
    }

    func requestCamera() async -> Bool {
        let granted = await AVCaptureDevice.requestAccess(for: .video)
        refresh()
        return granted
    }

    func requestMicrophone() async -> Bool {
        let granted = await AVCaptureDevice.requestAccess(for: .audio)
        refresh()
        return granted
    }

    private func status(for mediaType: AVMediaType) -> Status {
        switch AVCaptureDevice.authorizationStatus(for: mediaType) {
        case .notDetermined: return .notDetermined
        case .authorized: return .authorized
        case .denied: return .denied
        case .restricted: return .restricted
        @unknown default: return .restricted
        }
    }
}
