// The MIT License (MIT)
//
// Copyright (c) 2025 Insoft.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the Software), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

import Cocoa

struct Theme: Codable {
    let name: String
    let type: String
    let colors: [String: String]
    let tokenColors: [TokenColor]
}

struct TokenColor: Codable {
    let scope: [String]
    let settings: TokenSettings
}

struct TokenSettings: Codable {
    let foreground: String
}


struct Grammar: Codable {
    let name: String
    let scopeName: String
    let patterns: [GrammarPattern]
}

struct GrammarPattern: Codable {
    let name: String
    let match: String
}

final class CodeEditorTextView: NSTextView {
    var theme: Theme?
    var grammar: Grammar?
    
    var colors: [String: NSColor] = [
        "Keywords": .white,
        "Operators": .white,
        "Brackets": .white,
        "Numbers": .white,
        "Strings": .white,
        "Comments": .white,
        "Backquotes": .white,
        "Preprocessor Statements": .white,
        "Functions": .white
    ]
    var editorForegroundColor = NSColor(.black)
    
    lazy var baseAttributes: [NSAttributedString.Key: Any] = {
        let font = NSFont.monospacedSystemFont(ofSize: 12, weight: .medium)
        let paragraphStyle = NSMutableParagraphStyle()
        paragraphStyle.lineSpacing = 0
        paragraphStyle.paragraphSpacing = 0
        paragraphStyle.alignment = .left
        
        return [
            .font: font,
            .foregroundColor: NSColor.textColor,
            .kern: 0,
            .ligature: 0,
            .paragraphStyle: paragraphStyle
        ]
    }()
    
    // MARK: - Initializers
    override init(frame frameRect: NSRect, textContainer container: NSTextContainer?) {
        super.init(frame: frameRect, textContainer: container)
        setupEditor()
    }
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        setupEditor()
        
        let url = Bundle.main.bundleURL.appendingPathComponent("Contents/Resources/Default (Dark).xpcolortheme")
        loadTheme(at: url)
        loadGrammar()
    }
    
    // MARK: - Setup
    private func setupEditor() {
        font = NSFont.monospacedSystemFont(ofSize: 13, weight: .regular)
        isAutomaticQuoteSubstitutionEnabled = false
        isAutomaticDataDetectionEnabled = false
        isAutomaticDashSubstitutionEnabled = false
        isAutomaticLinkDetectionEnabled = false
        isAutomaticSpellingCorrectionEnabled = false
        isAutomaticTextReplacementEnabled = false
        isContinuousSpellCheckingEnabled = false
        
        textContainerInset = NSSize(width: 8, height: 8)
        backgroundColor = NSColor.textBackgroundColor
        
        smartInsertDeleteEnabled = false
        
        // No Wrapping, Horizontal Scroll Enabled
        isHorizontallyResizable = true
        isVerticallyResizable = true
        
        if let textContainer = textContainer {
            textContainer.widthTracksTextView = false // THIS is the key line
            textContainer.containerSize = NSSize(
                width: CGFloat.greatestFiniteMagnitude,
                height: CGFloat.greatestFiniteMagnitude
            )
        }
        
        enclosingScrollView?.hasHorizontalScroller = true
        typingAttributes[.kern] = 0
        typingAttributes[.ligature] = 0
        isRichText = false
        usesFindPanel = true
    }
    
    // MARK: - Override
    
    override func replaceCharacters(in range: NSRange, with string: String) {
        super.replaceCharacters(in: range, with: string)
        applySyntaxHighlighting()
    }
    
    override func didChangeText() {
        super.didChangeText()
        replaceLastTypedOperator()
        applySyntaxHighlighting()
    }
    
    override func insertNewline(_ sender: Any?) {
        super.insertNewline(sender)
        autoIndentCurrentLine()
    }
    
    private func autoIndentCurrentLine() {
        let text = string as NSString
        let selectedRange = selectedRange()
        let cursorPosition = selectedRange.location
        
        // Find previous line range
        let prevLineRange = text.lineRange(for: NSRange(location: max(0, cursorPosition - 1), length: 0))
        let prevLine = text.substring(with: prevLineRange)
        
        // Get leading whitespace from previous line
        let indentMatch = prevLine.prefix { $0 == " " || $0 == "\t" }
        var indent = String(indentMatch)
        
        // Optional: increase indent if line ends with certain patterns
        let trimmed = prevLine.trimmingCharacters(in: .whitespacesAndNewlines)
        let increaseIndentAfter = ["then", "do", "repeat", "case"]
        if increaseIndentAfter.contains(where: { trimmed.hasSuffix($0) }) {
            indent += "  " // add one level (2 spaces)
        }
        
        // Insert the indent at the cursor position
        insertText(indent, replacementRange: selectedRange)
    }
    
    private func replaceLastTypedOperator() {
        guard let textStorage = textStorage else { return }
        
        let replacements: [(String, String)] = [
            ("!=", "≠"),
            ("<>", "≠"),
            (">=", "≥"),
            ("<=", "≤"),
            ("=>", "▶")
        ]
        
        let cursorLocation = selectedRange().location
        guard cursorLocation >= 2 else { return } // Need at least 2 chars to match most patterns
        
        let originalText = string as NSString
        
        // Check the last 2–3 characters before the cursor
        let maxLookback = 3
        let start = max(cursorLocation - maxLookback, 0)
        let range = NSRange(location: start, length: cursorLocation - start)
        let recentText = originalText.substring(with: range)
        
        textStorage.beginEditing()
        
        for (find, replace) in replacements {
            if recentText.hasSuffix(find) {
                let replaceRange = NSRange(location: cursorLocation - find.count, length: find.count)
                textStorage.replaceCharacters(in: replaceRange, with: replace)
                
                // Move cursor after replacement
                let newCursor = replaceRange.location + replace.count
                setSelectedRange(NSRange(location: newCursor, length: 0))
                break
            }
        }
        
        textStorage.endEditing()
    }
    
    func removePragma(_ string: String) -> String {
        return string
            .components(separatedBy: .newlines)
            .filter { !$0.trimmingCharacters(in: .whitespaces).hasPrefix("#pragma") }
            .joined(separator: "\n")
    }
    
    func insertCode(_ string: String) {
        if let selectedRange = selectedRanges.first as? NSRange {
            if let textStorage = textStorage {
                textStorage.replaceCharacters(in: selectedRange, with: string)
                setSelectedRange(NSRange(location: selectedRange.location + string.count, length: 0))
            }
        }
        applySyntaxHighlighting()
    }
    
    private func loadJSONString(_ url: URL) -> String? {
        do {
            let jsonString = try String(contentsOf: url, encoding: .utf8)
            return jsonString
        } catch {
            return nil
        }
    }
    
    func loadTheme(at url: URL) {
        if let jsonString = loadJSONString(url),
           let jsonData = jsonString.data(using: .utf8) {
            theme = try? JSONDecoder().decode(Theme.self, from: jsonData)
        }
        
        func colorWithKey(_ key: String) -> NSColor {
            for tokenColor in theme!.tokenColors {
                if tokenColor.scope.contains(key) {
                    return NSColor(hex: tokenColor.settings.foreground)!
                }
            }
            return editorForegroundColor
        }
        
        editorForegroundColor = NSColor(hex: (theme?.colors["editor.foreground"])!)!
        
        backgroundColor = NSColor(hex: (theme?.colors["editor.background"])!)!
        textColor = NSColor(hex: (theme?.colors["editor.foreground"])!)!
        selectedTextAttributes = [
            .backgroundColor: NSColor(hex: (theme?.colors["editor.selectionBackground"])!)!
        ]
        insertionPointColor = NSColor(hex: (theme?.colors["editor.cursor"])!)!
        
        
        colors["Keywords"] = colorWithKey("Keywords")
        colors["Operators"] = colorWithKey("Operators")
        colors["Brackets"] = colorWithKey("Brackets")
        colors["Numbers"] = colorWithKey("Numbers")
        colors["Strings"] = colorWithKey("Strings")
        colors["Comments"] = colorWithKey("Comments")
        colors["Backquote"] = colorWithKey("Backquote")
        colors["Preprocessor Statements"] = colorWithKey("Preprocessor Statements")
        colors["Functions"] = colorWithKey("Functions")
    }
    
    private func loadGrammar() {
        guard let url = Bundle.main.url(forResource: "Language", withExtension: "xpgrammar") else { return }
        
        if let jsonString = loadJSONString(url),
           let jsonData = jsonString.data(using: .utf8) {
            grammar = try? JSONDecoder().decode(Grammar.self, from: jsonData)
        }
    }
    
    // MARK: - Syntax Highlighting
    
    private func applySyntaxHighlighting() {
        guard let textStorage = textStorage else { return }
        
        let text = string as NSString
        let fullRange = NSRange(location: 0, length: text.length)
    
        // Reset all text color first
        textStorage.beginEditing()
        textStorage.setAttributes(baseAttributes, range: fullRange)
        textStorage.foregroundColor = editorForegroundColor
        
        for pattern in grammar!.patterns {
            if pattern.match.isEmpty {
                continue
            }
            let color = colors[pattern.name]!
            let regex = try! NSRegularExpression(pattern: pattern.match)
            regex.enumerateMatches(in: text as String, range: fullRange) { match, _, _ in
                if let match = match {
                    textStorage.addAttribute(.foregroundColor, value: color, range: match.range)
                }
            }
        }

        textStorage.endEditing()
    }
}
