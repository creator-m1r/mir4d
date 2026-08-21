import Foundation

@MainActor
public protocol MirCollaborativeDocument: AnyObject {

    func applyCollaborationOperation(_ operation: MirCollaborationOperation) throws

    func currentCollaborationSnapshot() throws -> MirProjectSnapshot
}

private struct MirLocalUndoItem {
    let entityID: String
    let kind: MirCollaborationOperation.OperationKind

    let inverseParameters: Data
}

@MainActor
public final class MirCollaborationController: ObservableObject {
    public static let shared = MirCollaborationController()

    @Published public private(set) var joined = false
    @Published public private(set) var projectID: String = ""
    @Published public private(set) var collaborators: [MirCollaborator] = []
    @Published public private(set) var operations: [MirCollaborationOperation] = []
    @Published public private(set) var conflicts: [MirConflict] = []
    @Published public private(set) var localClockCounter: UInt64 = 0
    @Published public private(set) var versions: [MirProjectVersion] = []
    @Published public private(set) var canUndo = false
    @Published public private(set) var canRedo = false

    public var document: MirCollaborativeDocument?

    private var localCollaborator: MirCollaborator?
    private var crdt = MirCollaborationCRDT()
    private var undoStack: [MirLocalUndoItem] = []
    private var redoStack: [MirLocalUndoItem] = []
    private let manager = MirServerManager.shared
    private var observers: [NSObjectProtocol] = []

    private init() {
        let center = NotificationCenter.default
        observers.append(center.addObserver(forName: .mir4DCollaborationMessageReceived, object: nil, queue: .main) { [weak self] note in
            guard let envelope = note.object as? MirCollaborationEnvelope else { return }
            MainActor.assumeIsolated { self?.receiveRemote(envelope) }
        })
        observers.append(center.addObserver(forName: .mir4DServerStatusChanged, object: nil, queue: .main) { [weak self] note in
            if let status = note.object as? MirServerConnectionStatus, status == .disconnected {
                MainActor.assumeIsolated { self?.resetPresence() }
            }
        })
    }

    deinit {
        MainActor.assumeIsolated {
            observers.forEach { NotificationCenter.default.removeObserver($0) }
        }
    }

    public func startSharedSession(projectID: String) {
        let cfg = manager.configuration
        let color = Self.color(for: cfg.currentUserID)
        let me = MirCollaborator(
            id: cfg.currentUserID.isEmpty ? UUID().uuidString : cfg.currentUserID,
            displayName: cfg.currentUserName.isEmpty ? "Инженер" : cfg.currentUserName,
            role: "Инженер",
            color: color,
            permission: .editor
        )
        self.localCollaborator = me
        self.projectID = projectID
        self.joined = true
        self.crdt = MirCollaborationCRDT()
        self.operations.removeAll()
        self.conflicts.removeAll()
        self.undoStack.removeAll()
        self.redoStack.removeAll()
        updateUndoRedoFlags()

        let active = MirCollaborator(id: me.id, displayName: me.displayName, role: me.role, color: me.color, permission: .editor, active: true)
        broadcast(.presence(active))
        broadcast(.requestSnapshot)
        notifyStateChanged()
    }

    public func leave() {
        if let me = localCollaborator {
            broadcast(.presence(MirCollaborator(id: me.id, displayName: me.displayName, role: me.role, color: me.color, permission: .editor, active: false)))
        }
        joined = false
        resetPresence()
    }

    @discardableResult
    public func applyLocal(kind: MirCollaborationOperation.OperationKind, entityID: String, parameters: Data = Data()) -> MirCollaborationOperation {
        localClockCounter &+= 1
        let authorID = localCollaborator?.id ?? "local"
        let op = MirCollaborationOperation(
            clock: MirOperationClock(counter: localClockCounter, authorID: authorID),
            entityID: entityID,
            kind: kind,
            parameters: parameters,
            authorID: authorID
        )
        recordLocal(op, inverseFor: kind, parameters: parameters)
        broadcast(.operation(op))
        return op
    }

    public func undoLastLocal() {
        guard let item = undoStack.popLast() else { return }
        let inverseKind: MirCollaborationOperation.OperationKind
        switch item.kind {
        case .create: inverseKind = .delete
        case .delete: inverseKind = .create
        case .transform: inverseKind = .transform
        default: return
        }
        let inverseOp = applyLocal(kind: inverseKind, entityID: item.entityID, parameters: item.inverseParameters)
        if inverseKind != .select { applyToDocument(inverseOp) }
        redoStack.append(item)
        updateUndoRedoFlags()
    }

    public func redoLastLocal() {
        guard let item = redoStack.popLast() else { return }
        let kind = item.kind
        let op = applyLocal(kind: kind, entityID: item.entityID, parameters: item.inverseParameters)
        if kind != .select { applyToDocument(op) }
        undoStack.append(item)
        updateUndoRedoFlags()
    }

    public func tagVersion(label: String) {
        let number = (versions.last?.number ?? 0) &+ 1
        let version = MirProjectVersion(label: label, number: number, authorID: localCollaborator?.id ?? "local")
        versions.append(version)
        notifyStateChanged()
    }

    public func simulateRemoteColleague() {
        guard joined else { return }
        let peerID = "peer-demo-\(Int.random(in: 1000...9999))"
        let peer = MirCollaborator(
            id: peerID,
            displayName: "Коллега",
            role: "Инженер",
            color: Self.color(for: peerID),
            permission: .editor,
            active: true
        )
        receiveRemote(MirCollaborationEnvelope(projectID: projectID, message: .presence(peer)))

        localClockCounter &+= 1
        let op = MirCollaborationOperation(
            clock: MirOperationClock(counter: localClockCounter, authorID: peerID),
            entityID: UUID().uuidString,
            kind: .create,
            parameters: (try? JSONEncoder().encode(MirCollaborationCreateParameters(width: 2, depth: 2, height: 2))) ?? Data(),
            authorID: peerID
        )
        receiveRemote(MirCollaborationEnvelope(projectID: projectID, message: .operation(op)))
    }

    public func receiveRemote(_ envelope: MirCollaborationEnvelope) {
        guard joined, envelope.projectID == projectID else { return }
        switch envelope.message {
        case .operation(let op):
            handleRemoteOperation(op)
        case .presence(let collaborator):
            updatePresence(collaborator)
        case .snapshot(let snapshot):
            handleSnapshot(snapshot)
        case .requestSnapshot:
            respondWithSnapshot()
        }
    }

    private func handleRemoteOperation(_ op: MirCollaborationOperation) {
        localClockCounter = max(localClockCounter, op.clock.counter)

        if let existing = crdt.state(for: op.entityID), existing.clock.authorID != op.authorID {
            conflicts.append(MirConflict(
                entityID: op.entityID,
                winnerAuthorID: op.clock >= existing.clock ? op.authorID : existing.clock.authorID,
                loserAuthorID: op.clock >= existing.clock ? existing.clock.authorID : op.authorID
            ))
        }

        let changed = crdt.apply(op)
        if changed, op.kind != .select, op.authorID != localCollaborator?.id {
            applyToDocument(op)
        }

        operations.append(op)
        if operations.count > 500 { operations.removeFirst(operations.count - 500) }
        notifyStateChanged()
    }

    private func updatePresence(_ collaborator: MirCollaborator) {
        if collaborator.active {
            collaborators.removeAll { $0.id == collaborator.id }
            collaborators.append(collaborator)
        } else {
            collaborators.removeAll { $0.id == collaborator.id }
        }
        notifyStateChanged()
    }

    private func handleSnapshot(_ snapshot: MirProjectSnapshot) {
        localClockCounter = max(localClockCounter, snapshot.baseClock.counter)
        notifyStateChanged()
    }

    private func respondWithSnapshot() {
        guard let document else { return }
        do {
            let snapshot = try document.currentCollaborationSnapshot()
            broadcast(.snapshot(snapshot))
        } catch {

        }
    }

    private func recordLocal(_ op: MirCollaborationOperation, inverseFor kind: MirCollaborationOperation.OperationKind, parameters: Data) {
        let inverseParameters: Data
        switch kind {
        case .transform:
            inverseParameters = (crdt.state(for: op.entityID)?.transform).flatMap { try? JSONEncoder().encode($0) } ?? Data()
        case .delete:
            inverseParameters = parameters
        default:
            inverseParameters = Data()
        }
        if kind != .select, kind != .rename {
            undoStack.append(MirLocalUndoItem(entityID: op.entityID, kind: kind, inverseParameters: inverseParameters))
            redoStack.removeAll()
            updateUndoRedoFlags()
        }
        _ = crdt.apply(op)
        operations.append(op)
        if operations.count > 500 { operations.removeFirst(operations.count - 500) }
        notifyStateChanged()
    }

    private func applyToDocument(_ op: MirCollaborationOperation) {
        guard let document, op.kind != .select else { return }
        try? document.applyCollaborationOperation(op)
    }

    private func resetPresence() {
        collaborators.removeAll()
        crdt = MirCollaborationCRDT()
        notifyStateChanged()
    }

    private func broadcast(_ message: MirCollaborationWireMessage) {
        guard joined else { return }
        let envelope = MirCollaborationEnvelope(projectID: projectID, message: message)
        Task {
            try? await manager.broadcastCollaboration(envelope)
        }
    }

    private func updateUndoRedoFlags() {
        canUndo = !undoStack.isEmpty
        canRedo = !redoStack.isEmpty
    }

    private func notifyStateChanged() {
        NotificationCenter.default.post(name: .mir4DCollaborationStateChanged, object: nil)
    }

    private static let palette = ["#FF6B6B", "#4DABF7", "#51CF66", "#FFD43B", "#CC5DE8", "#FF922B", "#22B8CF"]

    static func color(for id: String) -> String {
        guard !id.isEmpty else { return "#4DABF7" }
        let hash = id.utf8.reduce(0) { ($0 &* 31) &+ Int($1) }
        return palette[abs(hash) % palette.count]
    }
}
