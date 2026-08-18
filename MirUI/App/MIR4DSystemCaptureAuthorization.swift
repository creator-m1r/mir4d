import Foundation
import AVFoundation

@MainActor
final class MIR4DSystemCaptureAuthorization: ObservableObject {
    enum Status: Equatable {
        case notDetermined
        case authorized
        case denied
        case restricted
    }

    @Published private(set) var camera: Status = .notDetermined
    @Published private(set) var microphone: Status = .notDetermined

    func refresh() {
        camera = map(AVCaptureDevice.authorizationStatus(for: .video))
        microphone = map(AVCaptureDevice.authorizationStatus(for: .audio))
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

    var canUseCamera: Bool { camera == .authorized }
    var canUseMicrophone: Bool { microphone == .authorized }

    private func map(_ status: AVAuthorizationStatus) -> Status {
        switch status {
        case .authorized: return .authorized
        case .denied: return .denied
        case .restricted: return .restricted
        case .notDetermined: return .notDetermined
        @unknown default: return .restricted
        }
    }
}
