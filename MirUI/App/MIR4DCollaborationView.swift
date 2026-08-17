import SwiftUI
import AppKit
import MirServer

/// Панель «Совместная работа»: редактирование одного проекта командой инженеров.
struct MIR4DCollaborationView: View {
    @StateObject private var collab = MirCollaborationController.shared
    @State private var projectIDInput: String = ""
    @State private var serverStatusLabel: String = MirServerManager.shared.status.label
    @State private var lastCreatedBodyID: String = ""
    @State private var lastError: String?
    @State private var importMessage: String?

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            header
            Divider()
            sessionControls
            Divider()
            presenceSection
            MIR4DCollaborationPresenceOverlay(collaborators: collab.collaborators)
            Divider()
            operationsSection
            if !collab.versions.isEmpty {
                Divider()
                versionsSection
            }
            if !collab.conflicts.isEmpty {
                Divider()
                conflictsSection
            }
            if let error = lastError {
                Text("Ошибка: \(error)").font(.caption).foregroundStyle(.red).lineLimit(3)
            }
            if let msg = importMessage {
                Text(msg).font(.caption).foregroundStyle(.green).lineLimit(3)
            }
        }
        .padding(16)
        .frame(minWidth: 560, minHeight: 720)
        .onAppear { collab.document = MIR4DModelRuntime.shared }
        .toolbar {
            ToolbarItemGroup {
                Button("Отменить") { collab.undoLastLocal() }
                    .disabled(!collab.canUndo)
                Button("Повторить") { collab.redoLastLocal() }
                    .disabled(!collab.canRedo)
                Button("Версия") { collab.tagVersion(label: "Ручная версия \(collab.versions.count + 1)") }
                    .disabled(!collab.joined)
                Button("Импорт с сервера") { importProjectFromServer() }
                Button("Симулировать коллегу") { collab.simulateRemoteColleague() }
                    .disabled(!collab.joined)
            }
        }
        .onReceive(NotificationCenter.default.publisher(for: .mir4DServerStatusChanged)) { note in
            if let s = note.object as? MirServerConnectionStatus {
                serverStatusLabel = s.label
            }
        }
    }

    private var header: some View {
        HStack {
            Text("Совместная работа").font(.headline)
            Spacer()
            Text(serverStatusLabel).font(.caption).foregroundStyle(.secondary)
        }
    }

    private var sessionControls: some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack {
                TextField("ID общего проекта", text: $projectIDInput)
                    .textFieldStyle(.roundedBorder)
                Button("Подключиться") { collab.startSharedSession(projectID: projectIDInput) }
                    .disabled(projectIDInput.isEmpty || collab.joined)
            }
            HStack {
                Button("Опубликовать текущий проект") { shareCurrentProject() }
                    .disabled(MirServerManager.shared.status != .connected)
                Button("Выйти") { collab.leave() }
                    .disabled(!collab.joined)
            }
            if collab.joined {
                Text("Активный общий проект: \(collab.projectID)")
                    .font(.caption).foregroundStyle(.green)
            }
        }
    }

    private var presenceSection: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("Участники (presence)").font(.subheadline).bold()
            if collab.collaborators.isEmpty {
                Text("Нет активных участников.").font(.caption).foregroundStyle(.secondary)
            } else {
                ForEach(collab.collaborators) { member in
                    HStack {
                        Circle().fill(mirColor(member.color)).frame(width: 10, height: 10)
                        Text(member.displayName).font(.callout)
                        Spacer()
                        Text(member.active ? "в сети" : "вышел").font(.caption).foregroundStyle(.secondary)
                    }
                }
            }
        }
    }

    private var operationsSection: some View {
        VStack(alignment: .leading, spacing: 6) {
            HStack {
                Text("Журнал операций").font(.subheadline).bold()
                Spacer()
                Text("часы: \(collab.localClockCounter)").font(.caption).foregroundStyle(.secondary)
            }
            HStack {
                Button("Создать примитив (синхронизировать)") {
                    let bodyID = MIR4DModelRuntime.shared.addBox(width: 1, depth: 1, height: 1)
                    lastCreatedBodyID = bodyID.uuidString
                    _ = try? collab.applyLocal(
                        kind: .create,
                        entityID: bodyID.uuidString,
                        parameters: JSONEncoder().encode(MirCollaborationCreateParameters(width: 1, depth: 1, height: 1))
                    )
                }
                .disabled(!collab.joined)
                Button("Переместить (синхронизировать)") {
                    guard let id = UUID(uuidString: lastCreatedBodyID) else { return }
                    var matrix = MirCollaborationTransform().matrix
                    matrix[3] += 2.0
                    let transform = MirCollaborationTransform(matrix: matrix)
                    MIR4DModelRuntime.shared.applyCollaborationTransform(bodyID: id, matrix: matrix)
                    _ = try? collab.applyLocal(
                        kind: .transform,
                        entityID: lastCreatedBodyID,
                        parameters: JSONEncoder().encode(transform)
                    )
                }
                .disabled(!collab.joined || lastCreatedBodyID.isEmpty)
            }
            ScrollView(.vertical, showsIndicators: true) {
                VStack(alignment: .leading, spacing: 3) {
                    ForEach(Array(collab.operations.suffix(40).reversed())) { op in
                        HStack {
                            Text(op.kind.rawValue).font(.caption).bold()
                            Text(op.entityID.prefix(8)).font(.caption)
                            Spacer()
                            Text(op.authorID).font(.caption2).foregroundStyle(.secondary)
                        }
                    }
                }
            }
            .frame(maxHeight: 160)
        }
    }

    private var versionsSection: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("Версии проекта").font(.subheadline).bold()
            ForEach(Array(collab.versions.suffix(8).reversed())) { version in
                HStack {
                    Text("v\(version.number): \(version.label)").font(.caption)
                    Spacer()
                    Text(version.at, style: .time).font(.caption2).foregroundStyle(.secondary)
                }
            }
        }
    }

    private var conflictsSection: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("Конфликты (разрешены CRDT)").font(.subheadline).bold().foregroundStyle(.orange)
            ForEach(Array(collab.conflicts.suffix(10).reversed())) { conflict in
                Text("Сущность \(conflict.entityID.prefix(8)): победитель \(conflict.winnerAuthorID), проигравший \(conflict.loserAuthorID)")
                    .font(.caption)
            }
        }
    }

    // MARK: - Действия

    private func shareCurrentProject() {
        guard let url = MIR4DProjectSession.shared.projectURL else {
            lastError = "Сначала откройте проект MIR 4D"
            return
        }
        Task {
            do {
                let payload = try MirProjectExporter().archiveProject(at: url)
                let id = MIR4DProjectSession.shared.projectUUID?.uuidString ?? url.lastPathComponent
                let result = await MirServerManager.shared.exportProject(
                    projectID: id,
                    projectName: MIR4DProjectSession.shared.projectName,
                    format: "mir4d",
                    payload: payload,
                    message: "Опубликовано для совместной работы"
                )
                if let result {
                    await MainActor.run { collab.startSharedSession(projectID: result.projectID) }
                } else {
                    await MainActor.run { lastError = "Экспорт не выполнен" }
                }
            } catch {
                await MainActor.run { lastError = String(describing: error) }
            }
        }
    }

    private func importProjectFromServer() {
        guard !projectIDInput.isEmpty else {
            lastError = "Укажите ID проекта для импорта"
            return
        }
        Task {
            do {
                let archive = try await MirServerManager.shared.fetchProjectArchive(projectID: projectIDInput)
                let destination = FileManager.default.temporaryDirectory
                    .appendingPathComponent("MIR4DImport_\(projectIDInput)")
                let url = try MirProjectImporter().unpack(archive, into: destination)
                await MainActor.run {
                    importMessage = "Импортировано в: \(url.path)"
                    lastError = nil
                }
            } catch {
                await MainActor.run { lastError = String(describing: error) }
            }
        }
    }
}

/// Оверлей присутствия: плавающие метки участников с их курсорами.
struct MIR4DCollaborationPresenceOverlay: View {
    let collaborators: [MirCollaborator]

    var body: some View {
        GeometryReader { geo in
            ForEach(collaborators.filter { $0.active }) { member in
                if let cursor = member.cursor {
                    let x = CGFloat(cursor.x) * geo.size.width
                    let y = CGFloat(cursor.y) * geo.size.height
                    label(member)
                        .position(x: x, y: y)
                }
            }
        }
        .frame(height: 64)
        .background(Color.black.opacity(0.03))
        .cornerRadius(6)
    }

    private func label(_ member: MirCollaborator) -> some View {
        HStack(spacing: 4) {
            Circle().fill(mirColor(member.color)).frame(width: 8, height: 8)
            Text(member.displayName).font(.caption2).padding(3)
        }
        .background(mirColor(member.color).opacity(0.2))
        .cornerRadius(4)
    }
}

private func mirColor(_ hex: String) -> Color {
    var hexSanitized = hex.trimmingCharacters(in: .whitespacesAndNewlines)
    hexSanitized = hexSanitized.replacingOccurrences(of: "#", with: "")
    var rgb: UInt64 = 0
    Scanner(string: hexSanitized).scanHexInt64(&rgb)
    let r = Double((rgb & 0xFF0000) >> 16) / 255
    let g = Double((rgb & 0x00FF00) >> 8) / 255
    let b = Double(rgb & 0x0000FF) / 255
    return Color(red: r, green: g, blue: b)
}
