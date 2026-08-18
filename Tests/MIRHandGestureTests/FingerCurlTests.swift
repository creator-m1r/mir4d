import XCTest
@testable import MIR4DApp

final class FingerCurlTests: XCTestCase {
    private let classifier = MIRHandGestureClassifier()

    func testOpenPalmFingersAreExtended() {
        let pose = MIRHandPoseMock.mockOpenPalm()
        for finger in MIRFinger.allCases {
            XCTAssertLessThan(classifier.curl(of: finger, in: pose), classifier.configuration.curlExtended,
                              "\(finger) should be extended in an open palm")
        }
    }

    func testFistFingersAreFolded() {
        let pose = MIRHandPoseMock.mockFist()
        for finger in MIRFinger.allCases {
            XCTAssertGreaterThan(classifier.curl(of: finger, in: pose), classifier.configuration.curlFolded,
                                 "\(finger) should be folded in a fist")
        }
    }

    func testPointHasOnlyIndexExtended() {
        let pose = MIRHandPoseMock.mockPoint()
        XCTAssertLessThan(classifier.curl(of: .index, in: pose), classifier.configuration.curlExtended)
        XCTAssertGreaterThan(classifier.curl(of: .middle, in: pose), classifier.configuration.curlFolded)
        XCTAssertGreaterThan(classifier.curl(of: .ring, in: pose), classifier.configuration.curlFolded)
        XCTAssertGreaterThan(classifier.curl(of: .little, in: pose), classifier.configuration.curlFolded)
    }

    func testCurlIsNormalised() {
        let open = MIRHandPoseMock.mockOpenPalm()
        let fist = MIRHandPoseMock.mockFist()
        for finger in MIRFinger.allCases {
            let c = classifier.curl(of: finger, in: open)
            XCTAssertGreaterThanOrEqual(c, 0)
            XCTAssertLessThanOrEqual(c, 1)
            let f = classifier.curl(of: finger, in: fist)
            XCTAssertGreaterThanOrEqual(f, 0)
            XCTAssertLessThanOrEqual(f, 1)
        }
    }
}
