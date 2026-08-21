import Foundation

extension Notification.Name {
    public static let mir4DServerStatusChanged = Notification.Name("MIR4D.Server.StatusChanged")
    public static let mir4DTeamMessageReceived = Notification.Name("MIR4D.Server.TeamMessageReceived")
    public static let mir4DTeamUpdated = Notification.Name("MIR4D.Server.TeamUpdated")
    public static let mir4DProjectExported = Notification.Name("MIR4D.Server.ProjectExported")
    public static let mir4DServerError = Notification.Name("MIR4D.Server.Error")

    public static let mir4DCollaborationMessageReceived = Notification.Name("MIR4D.Collaboration.MessageReceived")
    public static let mir4DCollaborationStateChanged = Notification.Name("MIR4D.Collaboration.StateChanged")
    public static let mir4DCollaborationConflict = Notification.Name("MIR4D.Collaboration.Conflict")
}
