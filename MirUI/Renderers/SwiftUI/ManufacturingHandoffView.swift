import SwiftUI

struct ManufacturingHandoffView: View {
    @ObservedObject var appState: CADAppState
    @ObservedObject var store: ProductionWorldStore

    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            header
            deliverables
            footer
        }
        .padding(12)
        .background(.ultraThinMaterial)
        .clipShape(RoundedRectangle(cornerRadius: 10))
        .overlay {
            RoundedRectangle(cornerRadius: 10)
                .stroke(MirTheme.Colors.panelBorder.opacity(0.65), lineWidth: 1)
        }
        .padding(.horizontal, 12)
        .padding(.bottom, 6)
    }

    private var header: some View {
        HStack {
            Label(appState.ui.language == .russian ? "Выпуск и производство" : "Release & Manufacturing", systemImage: "shippingbox")
                .font(.system(size: 12, weight: .bold))
            Spacer()
            Text(store.productionReady ? "MO-2026-001" : "DRAFT")
                .font(.system(size: 9, weight: .bold, design: .monospaced))
                .foregroundStyle(store.productionReady ? MirTheme.Colors.success : MirTheme.Colors.textTertiary)
        }
    }

    private var deliverables: some View {
        HStack(spacing: 8) {
            documentCard("3D", title: "Модель", icon: "cube", ready: true)
            documentCard("DWG", title: "Чертёж", icon: "doc.text", ready: store.completion[.drawing] ?? 0 >= 1)
            documentCard("BOM", title: "Спецификация", icon: "list.bullet.rectangle", ready: store.completion[.assembly] ?? 0 >= 0.8)
            documentCard("TP", title: "Маршрут", icon: "arrow.triangle.branch", ready: store.productionReady)
        }
    }

    private func documentCard(_ code: String, title: String, icon: String, ready: Bool) -> some View {
        VStack(alignment: .leading, spacing: 4) {
            HStack {
                Image(systemName: icon)
                Spacer()
                Image(systemName: ready ? "checkmark.circle.fill" : "circle.dashed")
                    .foregroundStyle(ready ? MirTheme.Colors.success : MirTheme.Colors.textTertiary)
            }
            .font(.system(size: 12, weight: .semibold))

            Text(code)
                .font(.system(size: 12, weight: .bold, design: .monospaced))
            Text(title)
                .font(.system(size: 9))
                .foregroundStyle(MirTheme.Colors.textSecondary)
        }
        .padding(8)
        .frame(maxWidth: .infinity, minHeight: 58, alignment: .leading)
        .background(ready ? MirTheme.Colors.success.opacity(0.08) : MirTheme.Colors.panel.opacity(0.35))
        .clipShape(RoundedRectangle(cornerRadius: 8))
    }

    private var footer: some View {
        HStack(spacing: 8) {
            Text(store.drawingStatus)
                .font(.system(size: 10))
                .foregroundStyle(MirTheme.Colors.textSecondary)
                .lineLimit(1)

            Spacer()

            Button {
                store.releaseDrawing()
                appState.selectWorkbench(.drawing)
            } label: {
                Label(appState.ui.language == .russian ? "Выпустить КД" : "Release CD", systemImage: "checkmark.seal")
            }
            .buttonStyle(.bordered)
            .controlSize(.small)

            Button {
                store.createManufacturingOrder()
                appState.selectWorkbench(.collaboration)
                appState.showNotification(
                    appState.ui.language == .russian ? "Производственная заявка отправлена" : "Manufacturing request sent",
                    type: .success
                )
            } label: {
                Label(appState.ui.language == .russian ? "Отправить в производство" : "Send to Production", systemImage: "paperplane.fill")
            }
            .buttonStyle(.borderedProminent)
            .controlSize(.small)
            .disabled((store.completion[.drawing] ?? 0) < 1)
        }
    }
}
