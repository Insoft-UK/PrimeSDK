import Cocoa

class LineNumberRulerView: NSRulerView {

    weak var textView: NSTextView?

    init(textView: NSTextView) {
        self.textView = textView
        super.init(scrollView: textView.enclosingScrollView!, orientation: .verticalRuler)
        self.clientView = textView
        self.ruleThickness = 40
        self.registerObservers()
    }

    required init(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    private func registerObservers() {
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(updateLines),
                                               name: NSText.didChangeNotification,
                                               object: textView)

        NotificationCenter.default.addObserver(self,
                                               selector: #selector(updateLines),
                                               name: NSView.boundsDidChangeNotification,
                                               object: textView?.enclosingScrollView?.contentView)

        textView?.enclosingScrollView?.contentView.postsBoundsChangedNotifications = true
    }

    @objc func updateLines() {
        needsDisplay = true
    }

    override func drawHashMarksAndLabels(in rect: NSRect) {
        guard let textView = self.textView,
              let layoutManager = textView.layoutManager,
              let textContainer = textView.textContainer else { return }

        // Clip to avoid drawing outside bounds
        NSGraphicsContext.current?.cgContext.clip(to: rect)

        let visibleRect = textView.enclosingScrollView?.contentView.bounds ?? textView.visibleRect
        let glyphRange = layoutManager.glyphRange(forBoundingRect: visibleRect, in: textContainer)
        
        let fullText = textView.string as NSString
        let startCharIndex = layoutManager.characterIndexForGlyph(at: glyphRange.location)
        var lineNumber = fullText.substring(to: startCharIndex).components(separatedBy: .newlines).count

        var index = glyphRange.location

        while index < NSMaxRange(glyphRange) {
            let charRange = layoutManager.characterRange(forGlyphRange: NSRange(location: index, length: 1), actualGlyphRange: nil)
            let lineRange = fullText.lineRange(for: charRange)
            let glyphRange = layoutManager.glyphRange(forCharacterRange: lineRange, actualCharacterRange: nil)

            let lineRect = layoutManager.lineFragmentUsedRect(forGlyphAt: glyphRange.location, effectiveRange: nil)
            let yPosition = lineRect.minY - visibleRect.origin.y

            let labelRect = NSRect(x: 0, y: yPosition, width: self.ruleThickness - 5, height: lineRect.height)

            let paragraphStyle = NSMutableParagraphStyle()
            paragraphStyle.alignment = .right

            let attrs: [NSAttributedString.Key: Any] = [
                .font: NSFont.monospacedDigitSystemFont(ofSize: 11, weight: .regular),
                .foregroundColor: NSColor.gray,
                .paragraphStyle: paragraphStyle
            ]

            let lineStr = "\(lineNumber)" as NSString
            lineStr.draw(in: labelRect, withAttributes: attrs)

            index = NSMaxRange(lineRange)
            lineNumber += 1
        }
    }
    
    deinit {
        NotificationCenter.default.removeObserver(self)
    }
}
