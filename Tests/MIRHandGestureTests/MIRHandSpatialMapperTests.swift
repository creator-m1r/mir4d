import XCTest
import simd
@testable import MirUIHandGesture

final class MIRHandSpatialMapperTests: XCTestCase {
    func testMirrorAndFlipOrigin() {
        let mapper = MIRHandSpatialMapper()

        let p = mapper.map(normalized: SIMD3(0, 0, 0))
        XCTAssertEqual(p.x, (1.0 - 0.5) * mapper.volume.width, accuracy: 1e-9)
        XCTAssertEqual(p.y, (1.0 - 0.5) * mapper.volume.height, accuracy: 1e-9)
        XCTAssertEqual(p.z, mapper.volume.depthCenter, accuracy: 1e-9)
    }

    func testNoMirror() {
        var mapper = MIRHandSpatialMapper()
        mapper.mirrorX = false
        let p = mapper.map(normalized: SIMD3(0, 0, 0))
        XCTAssertEqual(p.x, (0.0 - 0.5) * mapper.volume.width, accuracy: 1e-9)
    }

    func testCenterProjectsToOrigin() {
        let mapper = MIRHandSpatialMapper()
        let p = mapper.map(normalized: SIMD3(0.5, 0.5, 0))
        XCTAssertEqual(p.x, 0, accuracy: 1e-9)
        XCTAssertEqual(p.y, 0, accuracy: 1e-9)
    }

    func testDepthIsClamped() {
        let mapper = MIRHandSpatialMapper()
        let far = mapper.map(normalized: SIMD3(0.5, 0.5, 5))
        let near = mapper.map(normalized: SIMD3(0.5, 0.5, -5))
        XCTAssertEqual(far.z, mapper.volume.depthCenter + mapper.volume.depth * 0.5, accuracy: 1e-9)
        XCTAssertEqual(near.z, mapper.volume.depthCenter - mapper.volume.depth * 0.5, accuracy: 1e-9)
    }

    func testDepthRange() {
        let mapper = MIRHandSpatialMapper()
        let mid = mapper.map(normalized: SIMD3(0.5, 0.5, 1))
        XCTAssertEqual(mid.z, mapper.volume.depthCenter + mapper.volume.depth * 0.5, accuracy: 1e-9)
    }
}
