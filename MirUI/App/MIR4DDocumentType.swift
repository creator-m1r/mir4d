import UniformTypeIdentifiers

/// Native Uniform Type Identifier for a MIR 4D project package.
extension UTType {
    static let mir4dProject = UTType(exportedAs: "app.mir4d.project")
}

/// The document declaration is also kept here so Open/Save panels use the
/// same identifier as the macOS bundle declaration.
enum MIR4DDocumentType {
    static let identifier = "app.mir4d.project"
    static let fileExtension = "mir4d"
}
