import Foundation

extension TreeNodeData: Codable {
    private enum CodingKeys: String, CodingKey { case id, name, icon, status, children }
    private enum StatusCode: String, Codable { case none, approved, inProgress, issue }

    init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        id = try c.decode(UUID.self, forKey: .id)
        name = try c.decode(String.self, forKey: .name)
        icon = try c.decode(String.self, forKey: .icon)
        children = try c.decodeIfPresent([TreeNodeData].self, forKey: .children) ?? []
        switch try c.decodeIfPresent(StatusCode.self, forKey: .status) ?? .none {
        case .none: status = .none
        case .approved: status = .approved
        case .inProgress: status = .inProgress
        case .issue: status = .issue
        }
    }

    func encode(to encoder: Encoder) throws {
        var c = encoder.container(keyedBy: CodingKeys.self)
        try c.encode(id, forKey: .id)
        try c.encode(name, forKey: .name)
        try c.encode(icon, forKey: .icon)
        try c.encode(children, forKey: .children)
        let code: StatusCode
        switch status {
        case .none: code = .none
        case .approved: code = .approved
        case .inProgress: code = .inProgress
        case .issue: code = .issue
        }
        try c.encode(code, forKey: .status)
    }
}
