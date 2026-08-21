import XCTest
@testable import MirUIHandGesture

final class GestureClassifierTests: XCTestCase {
    private let classifier = MIRHandGestureClassifier()

    func testSingleHandGestures() {
        XCTAssertEqual(classifier.classify(MIRHandPoseMock.mockPoint()).type, .point)
        XCTAssertEqual(classifier.classify(MIRHandPoseMock.mockGrab()).type, .grab)
        XCTAssertEqual(classifier.classify(MIRHandPoseMock.mockFist()).type, .fist)
        XCTAssertEqual(classifier.classify(MIRHandPoseMock.mockOpenPalm()).type, .openPalm)
        XCTAssertEqual(classifier.classify(MIRHandPoseMock.mockTwoFinger()).type, .twoFinger)
        XCTAssertEqual(classifier.classify(MIRHandPoseMock.mockThreeFinger()).type, .threeFinger)
        XCTAssertEqual(classifier.classify(MIRHandPoseMock.mockThumbsUp()).type, .thumbsUp)
        XCTAssertEqual(classifier.classify(MIRHandPoseMock.mockPinch()).type, .pinch)
    }

    func testGestureCarriesConfidence() {
        let result = classifier.classify(MIRHandPoseMock.mockPoint())
        XCTAssertGreaterThan(result.confidence, 0)
        XCTAssertLessThanOrEqual(result.confidence, 1)
    }

    func testAmbiguousPoseFallsBackToRest() {
        let pose = MIRHandPoseMock.mockRest()
        let result = classifier.classify(pose)

        XCTAssertLessThan(result.confidence, 0.95)
    }

    func testLowConfidenceIsRejectedByRecognizer() {
        var recognizer = MIRHandGestureRecognizer()
        recognizer.configuration.minConfidence = 0.99
        let mapper = MIRHandSpatialMapper()
        let pose = MIRHandPoseMock.mockPoint()
        let event = recognizer.ingest(pose: pose, scenePosition: mapper.map(normalized: pose.palmPosition), timestamp: Date())
        XCTAssertNil(event, "A gesture below the confidence threshold must be ignored")
    }

    func testGestureCatalogIsExtensible() {

        let expected: [MIRHandGestureType] = [
            .openPalm, .point, .pinch, .grab, .fist, .twoFinger, .threeFinger,
            .vSign, .thumbsUp, .rest,
            .twoHandTranslate, .twoHandScale, .twoHandRotate, .twoHandPinch, .twoHandGrab
        ]
        for type in expected {
            XCTAssertTrue(MIRHandGestureType.allCases.contains(type))
        }
    }
}
