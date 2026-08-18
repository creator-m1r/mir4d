import SwiftUI

/// Floating recent-projects window for the MIR 4D start workspace.
/// Project activation is delegated to the existing command/session layer.
struct MIR4DRecentProjectsWindow: View {
    @EnvironmentObject private var appState: CADAppState
    let projects: [MIR4DRecentProject]
    let onOpen: (MIR4DRecentProject) -> Void
    let onClose: () -> Void

    @State private var query = ""

    private var filteredProjects: [MIR4DRecentProject] {
        guard !query.isEmpty else { return projects }
        return projects.filter {
            $0.name.localizedCaseInsensitiveContains(query) ||
            $0.path.localizedCaseInsensitiveContains(query)
        }
    }

    var body: some View {
        MIR4DProjectWindowShell(
            title: "Недавние проекты",
            subtitle: "Ваше инженерное пространство",
            onClose: onClose
        ) {
            VStack(spacing: 0) {
                searchField
                    .padding(16)

                Divider().overlay(Color.white.opacity(0.06))

                if filteredProjects.isEmpty {
                    emptyState
                } else {
                    ScrollView {
                        LazyVStack(spacing: 8) {
                            ForEach(filteredProjects, id: \.id) { project in
                                projectRow(project)
                            }
                        }
                        .padding(16)
                    }
                }
            }
        }
        .environmentObject(appState)
    }

    private var searchField: some View {
        HStack(spacing: 9) {
            Image(systemName: "magnifyingglass")
                .font(.system(size: 12))
                .foregroundStyle(.secondary)

            TextField("Найти проект…", text: $query)
                .textFieldStyle(.plain)
                .font(.system(size: 12))
                .foregroundStyle(.white)

            if !query.isEmpty {
                Button {
                    query = ""
                } label: {
                    Image(systemName: "xmark.circle.fill")
                        .foregroundStyle(.secondary)
                }
                .buttonStyle(.plain)
            }
        }
        .padding(.horizontal, 12)
        .frame(height: 40)
        .background(Color.white.opacity(0.045), in: RoundedRectangle(cornerRadius: 10, style: .continuous))
        .overlay {
            RoundedRectangle(cornerRadius: 10, style: .continuous)
                .stroke(Color.white.opacity(0.08), lineWidth: 1)
        }
    }

    private var emptyState: some View {
        VStack(spacing: 12) {
            Spacer()
            Image(systemName: query.isEmpty ? "folder" : "magnifyingglass")
                .font(.system(size: 30, weight: .light))
                .foregroundStyle(.cyan.opacity(0.7))
            Text(query.isEmpty ? "Проектов пока нет" : "Ничего не найдено")
                .font(.system(size: 14, weight: .semibold))
                .foregroundStyle(.white)
            Text(query.isEmpty ? "Создайте первый проект МИР 4D." : "Измените запрос поиска.")
                .font(.system(size: 11))
                .foregroundStyle(.secondary)
            Spacer()
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    private func projectRow(_ project: MIR4DRecentProject) -> some View {
        Button {
            onOpen(project)
        } label: {
            HStack(spacing: 13) {
                ZStack {
                    RoundedRectangle(cornerRadius: 10, style: .continuous)
                        .fill(Color.cyan.opacity(0.09))
                    Image(systemName: "cube.transparent")
                        .font(.system(size: 18))
                        .foregroundStyle(.cyan.opacity(0.9))
                }
                .frame(width: 46, height: 46)

                VStack(alignment: .leading, spacing: 4) {
                    Text(project.name)
                        .font(.system(size: 13, weight: .semibold))
                        .foregroundStyle(.white)
                        .lineLimit(1)
                    Text(project.path)
                        .font(.system(size: 10))
                        .foregroundStyle(.secondary)
                        .lineLimit(1)
                }

                Spacer()

                Image(systemName: "chevron.right")
                    .font(.system(size: 10, weight: .bold))
                    .foregroundStyle(.secondary)
            }
            .padding(10)
            .background(Color.white.opacity(0.025), in: RoundedRectangle(cornerRadius: 12, style: .continuous))
            .overlay {
                RoundedRectangle(cornerRadius: 12, style: .continuous)
                    .stroke(Color.white.opacity(0.06), lineWidth: 1)
            }
        }
        .buttonStyle(.plain)
    }
}
