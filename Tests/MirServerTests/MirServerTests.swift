import Foundation
import Testing
import MirServer

@Suite("MirServer: модели и архив проекта")
struct MirServerTests {

    @Test("MirServerConfiguration сериализуется и восстанавливается")
    func configurationRoundTrip() throws {
        let cfg = MirServerConfiguration(
            baseURL: URL(string: "https://example.com/api")!,
            webAppURL: URL(string: "https://example.com")!,
            apiToken: "secret-token",
            teamID: "team-42",
            currentUserID: "user-7",
            currentUserName: "Инженер",
            connectTimeout: 15
        )
        let data = try JSONEncoder().encode(cfg)
        let restored = try JSONDecoder().decode(MirServerConfiguration.self, from: data)
        #expect(restored.apiToken == "secret-token")
        #expect(restored.teamID == "team-42")
        #expect(restored.baseURL.absoluteString == "https://example.com/api")
        #expect(restored.connectTimeout == 15)
    }

    @Test("MirTeamMessage кодируется с iso8601 датой")
    func teamMessageCodable() throws {
        let message = MirTeamMessage(
            authorID: "u1",
            authorName: "Анна",
            text: "Привет, команде!",
            projectID: "proj-1"
        )
        let data = try JSONEncoder().encode(message)
        let decoded = try JSONDecoder().decode(MirTeamMessage.self, from: data)
        #expect(decoded.authorName == "Анна")
        #expect(decoded.text == "Привет, команде!")
        #expect(decoded.projectID == "proj-1")
        #expect(decoded.id == message.id)
    }

    @Test("MirProjectExporter упаковывает каталог проекта в архив")
    func projectArchiveRoundTrip() throws {
        let tmp = FileManager.default.temporaryDirectory
            .appendingPathComponent("mir4d-export-test-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: tmp, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: tmp) }

        let manifest = tmp.appendingPathComponent("project.mir4d.json")
        try Data(#"{"name":"Demo"}"#.utf8).write(to: manifest)
        let sub = tmp.appendingPathComponent("models")
        try FileManager.default.createDirectory(at: sub, withIntermediateDirectories: true)
        try Data("BREP-BYTES".utf8).write(to: sub.appendingPathComponent("body.stl"))

        let archiveData = try MirProjectExporter().archiveProject(at: tmp)
        let json = try #require(try JSONSerialization.jsonObject(with: archiveData) as? [String: Any])
        #expect(json["root"] is String)
        let entries = try #require(json["entries"] as? [String: [String: Any]])
        #expect(entries["project.mir4d.json"] != nil)
        #expect(entries["models/body.stl"] != nil)
        let bodyEntry = try #require(entries["models/body.stl"])
        #expect((bodyEntry["size"] as? Int) == "BREP-BYTES".utf8.count)
    }

    @Test("Статус соединения корректно сравнивается")
    func connectionStatusEquatable() {
        #expect(MirServerConnectionStatus.disconnected == .disconnected)
        #expect(MirServerConnectionStatus.connected == .connected)
        #expect(MirServerConnectionStatus.failed("a") != .failed("b"))
        #expect(MirServerConnectionStatus.failed("a") != .disconnected)
    }

    @Test("Lamport-часы упорядочивают операции детерминированно")
    func lamportClockOrdering() {
        let a = MirOperationClock(counter: 5, authorID: "u1")
        let b = MirOperationClock(counter: 5, authorID: "u2")
        let c = MirOperationClock(counter: 6, authorID: "u1")
        #expect(a < b)
        #expect(b < c)
        #expect(a < c)
    }

    @Test("Wire-конверт совместной работы сериализуется")
    func collaborationEnvelopeRoundTrip() throws {
        let op = MirCollaborationOperation(
            clock: MirOperationClock(counter: 3, authorID: "u9"),
            entityID: "body-1",
            kind: .transform,
            authorID: "u9"
        )
        let envelope = MirCollaborationEnvelope(projectID: "proj-x", message: .operation(op))
        let data = try JSONEncoder().encode(envelope)
        let restored = try JSONDecoder().decode(MirCollaborationEnvelope.self, from: data)
        #expect(restored.projectID == "proj-x")
        guard case .operation(let decoded) = restored.message else {
            Issue.record("Ожидалось сообщение .operation")
            return
        }
        #expect(decoded.entityID == "body-1")
        #expect(decoded.authorID == "u9")
    }

    private func op(_ counter: UInt64, _ author: String, _ entity: String) -> MirCollaborationOperation {
        MirCollaborationOperation(
            clock: MirOperationClock(counter: counter, authorID: author),
            entityID: entity,
            kind: .transform,
            authorID: author
        )
    }

    @Test("CRDT: конвергенция при разном порядке доставки операций")
    func crdtConvergenceDifferentOrder() {
        let a = op(1, "u1", "b1")
        let b = op(2, "u2", "b1")
        var peer1 = MirCollaborationCRDT()
        var peer2 = MirCollaborationCRDT()
        peer1.apply(a); peer1.apply(b)
        peer2.apply(b); peer2.apply(a)
        #expect(peer1.state(for: "b1")?.clock == peer2.state(for: "b1")?.clock)
    }

    @Test("CRDT: last-writer (большие Lamport-часы) побеждает")
    func crdtLastWriterWins() {
        let a = op(1, "u1", "b1")
        let b = op(5, "u1", "b1")
        var crdt = MirCollaborationCRDT()
        crdt.apply(a)
        #expect(crdt.state(for: "b1")?.clock.counter == 1)
        crdt.apply(b)
        #expect(crdt.state(for: "b1")?.clock.counter == 5)
    }

    @Test("CRDT: merge двух состояний коммутативен")
    func crdtMergeCommutative() {
        let a = op(1, "u1", "b1")
        let b = op(3, "u2", "b2")
        var crdt1 = MirCollaborationCRDT(); crdt1.apply(a); crdt1.apply(b)
        var crdt2 = MirCollaborationCRDT(); crdt2.apply(b); crdt2.apply(a)
        crdt1.merge(crdt2)
        crdt2.merge(crdt1)
        #expect(crdt1.state(for: "b1")?.clock == crdt2.state(for: "b1")?.clock)
        #expect(crdt1.state(for: "b2")?.clock == crdt2.state(for: "b2")?.clock)
    }

    @MainActor
    @Test("Контроллер: undo локальной операции применяет инверсию к документу")
    func controllerUndoAppliesInverse() throws {
        final class StubDocument: MirCollaborativeDocument {
            var applied: [MirCollaborationOperation] = []
            func applyCollaborationOperation(_ op: MirCollaborationOperation) throws { applied.append(op) }
            func currentCollaborationSnapshot() throws -> MirProjectSnapshot {
                MirProjectSnapshot(projectID: "", entities: [], baseClock: MirOperationClock())
            }
        }

        let controller = MirCollaborationController.shared
        let stub = StubDocument()
        controller.document = stub
        controller.startSharedSession(projectID: "undo-test")

        _ = controller.applyLocal(kind: .create, entityID: "e1")
        #expect(stub.applied.isEmpty)

        controller.undoLastLocal()
        #expect(stub.applied.contains { $0.kind == .delete && $0.entityID == "e1" })

        controller.leave()
        controller.document = nil
    }
}
