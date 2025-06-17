// The MIT License (MIT)
//
// Copyright (c) 2023-2025 Insoft. All rights reserved.
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
import UniformTypeIdentifiers

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

extension NSColor {
    convenience init?(hex: String) {
        var hexString = hex.trimmingCharacters(in: .whitespacesAndNewlines).uppercased()
        
        // Remove "#" prefix
        if hexString.hasPrefix("#") {
            hexString.remove(at: hexString.startIndex)
        }
        
        // Handle shorthand (#RGB)
        if hexString.count == 3 {
            let r = hexString[hexString.startIndex]
            let g = hexString[hexString.index(hexString.startIndex, offsetBy: 1)]
            let b = hexString[hexString.index(hexString.startIndex, offsetBy: 2)]
            hexString = "\(r)\(r)\(g)\(g)\(b)\(b)"
        }
        
        guard hexString.count == 6,
              let rgb = Int(hexString, radix: 16) else {
            return nil
        }
        
        let red   = CGFloat((rgb >> 16) & 0xFF) / 255.0
        let green = CGFloat((rgb >> 8) & 0xFF) / 255.0
        let blue  = CGFloat(rgb & 0xFF) / 255.0
        
        self.init(red: red, green: green, blue: blue, alpha: 1.0)
    }
}

extension ViewController: NSWindowRestoration {
    static func restoreWindow(withIdentifier identifier: NSUserInterfaceItemIdentifier, state: NSCoder, completionHandler: @escaping (NSWindow?, Error?) -> Void) {
        // Restore your window here if needed
        completionHandler(nil, nil) // or provide restored window
    }
}

class ViewController: NSViewController, NSTextViewDelegate {
    let mainMenu = NSApp.mainMenu!
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
        "Preprocessor Statements": .white
    ]
    
    var currentFileURL: URL?
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
    
    var developerPath: String? {
        // For now PrimeSDK, will be used by Xprime during development.
#if !PRIMESDK
        Bundle.main.bundleURL
            .appendingPathComponent("Contents")
            .appendingPathComponent("Developer")
            .path
#else
        return "/Applications/HP/PrimeSDK"
#endif
    }
    
    
    @IBOutlet var textView: NSTextView!
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
    }
    
    
    override func viewDidLoad() {
        super.viewDidLoad()
        textView.delegate = self
        
        textView.smartInsertDeleteEnabled = false
        textView.isAutomaticQuoteSubstitutionEnabled = false
        textView.isAutomaticDashSubstitutionEnabled = false
        textView.isAutomaticTextReplacementEnabled = false
        textView.isAutomaticSpellingCorrectionEnabled = false
        textView.isAutomaticLinkDetectionEnabled = false
        
        
        // No Wrapping, Horizontal Scroll Enabled
        textView.isHorizontallyResizable = true
        textView.isVerticallyResizable = true
        
        if let textContainer = textView.textContainer {
            textContainer.widthTracksTextView = false // THIS is the key line
            textContainer.containerSize = NSSize(
                width: CGFloat.greatestFiniteMagnitude,
                height: CGFloat.greatestFiniteMagnitude
            )
        }
        
        textView.enclosingScrollView?.hasHorizontalScroller = true
        
        
        
        
        textView.typingAttributes[.kern] = 0
        textView.typingAttributes[.ligature] = 0
        
        textView.isRichText = false
        textView.usesFindPanel = true
        
        
        // Add the Line Number Ruler
        if let scrollView = textView.enclosingScrollView {
            let ruler = LineNumberRulerView(textView: textView)
            scrollView.verticalRulerView = ruler
            scrollView.hasVerticalRuler = true
            scrollView.rulersVisible = true
            
            // Force layout to avoid invisible window
            scrollView.tile()
        }
        
        if let url = Bundle.main.resourceURL?.appendingPathComponent("default.prgm+") {
            do {
                let contents = try String(contentsOf: url, encoding: .utf8)
                textView.string = contents
            } catch {
                print("Failed to open file:", error)
            }
        }
        
        loadTheme()
        loadGrammar()
        
        applySyntaxHighlighting()
    }
    
    override var representedObject: Any? {
        didSet {
            // Update the view, if already loaded.
        }
    }
    
    func textDidChange(_ notification: Notification) {
        replaceLastTypedOperator()
        applySyntaxHighlighting()
        
        if currentFileURL != nil {
            assignRevertToSavedAction()
        }
    }
    
    // MARK: - Theme
    func loadJSONString(_ url: URL) -> String? {
        do {
            let jsonString = try String(contentsOf: url, encoding: .utf8)
            return jsonString
        } catch {
            return nil
        }
    }
    
    private func loadTheme() {
        guard let url = Bundle.main.url(forResource: "Default (Dark)", withExtension: "xpcolortheme") else { return }
        
        if let jsonString = loadJSONString(url),
           let jsonData = jsonString.data(using: .utf8) {
            theme = try? JSONDecoder().decode(Theme.self, from: jsonData)
        }
        
        func colorWithKey(_ key: String) -> NSColor {
            for tokenColor in theme!.tokenColors {
                if tokenColor.scope.contains(key) {
                    return NSColor.init(hex: tokenColor.settings.foreground)!
                }
            }
            return NSColor.white
        }
        
        textView.backgroundColor = NSColor.init(hex: (theme?.colors["editor.background"])!)!
        textView.textColor = NSColor.init(hex: (theme?.colors["editor.foreground"])!)!
        textView.selectedTextAttributes = [
            .backgroundColor: NSColor.init(hex: (theme?.colors["editor.selectionBackground"])!)!
        ]
        textView.insertionPointColor = NSColor.init(hex: (theme?.colors["editor.cursor"])!)!
        
        
        colors["Keywords"] = colorWithKey("Keywords")
        colors["Operators"] = colorWithKey("Operators")
        colors["Brackets"] = colorWithKey("Brackets")
        colors["Numbers"] = colorWithKey("Numbers")
        colors["Strings"] = colorWithKey("Strings")
        colors["Comments"] = colorWithKey("Comments")
        colors["Backquote"] = colorWithKey("Backquote")
        colors["Preprocessor Statements"] = colorWithKey("Preprocessor Statements")
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
        guard let textStorage = textView.textStorage else { return }
        
        let text = textView.string as NSString
        let fullRange = NSRange(location: 0, length: text.length)
    
        // Reset all text color first
        textStorage.beginEditing()
        textStorage.setAttributes(baseAttributes, range: fullRange)
        
        for pattern in grammar!.patterns {
            let color = colors[pattern.name]!
            let regex = try! NSRegularExpression(pattern: pattern.match, options: [.caseInsensitive])
            regex.enumerateMatches(in: text as String, range: fullRange) { match, _, _ in
                if let match = match {
                    textStorage.addAttribute(.foregroundColor, value: color, range: match.range)
                }
            }
        }

        textStorage.endEditing()
    }
    
    
    
    private func replaceLastTypedOperator() {
        guard let textStorage = textView.textStorage else { return }
        
        let replacements: [(String, String)] = [
            ("!=", "≠"),
            ("<>", "≠"),
            (">=", "≥"),
            ("<=", "≤"),
            ("=>", "▶")
        ]
        
        let cursorLocation = textView.selectedRange().location
        guard cursorLocation >= 2 else { return } // Need at least 2 chars to match most patterns
        
        let originalText = textView.string as NSString
        
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
                textView.setSelectedRange(NSRange(location: newCursor, length: 0))
                break
            }
        }
        
        textStorage.endEditing()
    }
    
    override func insertNewline(_ sender: Any?) {
        super.insertNewline(sender)
        autoIndentCurrentLine()
    }
    
    private func autoIndentCurrentLine() {
        guard let textView = self.textView else { return }
        
        let text = textView.string as NSString
        let selectedRange = textView.selectedRange()
        let cursorPosition = selectedRange.location
        
        // Find previous line range
        let prevLineRange = text.lineRange(for: NSRange(location: max(0, cursorPosition - 1), length: 0))
        let prevLine = text.substring(with: prevLineRange)
        
        // Get leading whitespace from previous line
        let indentMatch = prevLine.prefix { $0 == " " || $0 == "\t" }
        var indent = String(indentMatch)
        
        // Optional: increase indent if line ends with certain patterns
        let trimmed = prevLine.trimmingCharacters(in: .whitespacesAndNewlines)
        let increaseIndentAfter = ["then", "do", "repeat", "{"]
        if increaseIndentAfter.contains(where: { trimmed.hasSuffix($0) }) {
            indent += "    " // add one level (4 spaces)
        }
        
        // Insert the indent at the cursor position
        textView.insertText(indent, replacementRange: selectedRange)
    }
    
    // MARK: - File Handling
    
    private func detectEncoding(of url: URL) -> String.Encoding? {
        do {
            let data = try Data(contentsOf: url)
            
            if data.starts(with: [0xEF, 0xBB, 0xBF]) {
                return .utf8
            } else if data.starts(with: [0xFF, 0xFE]) {
                return .utf16LittleEndian
            } else if data.starts(with: [0xFE, 0xFF]) {
                return .utf16BigEndian
            } else {
                // No BOM — assume UTF-8 as default fallback
                return .utf8
            }
        } catch {
            alert("Reading file data: \(error)")
            return nil
        }
    }
    
    func openFile() {
        let openPanel = NSOpenPanel()
        if #available(macOS 12.0, *) {
            let extensions = ["prgm", "prgm+", "ppl", "ppl+"]
            let contentTypes = extensions.compactMap { UTType(filenameExtension: $0) }
            openPanel.allowedContentTypes = contentTypes
        } else {
            openPanel.allowedFileTypes = ["prgm", "prgm+", "ppl", "ppl+"]
        }
        
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = false
        
        openPanel.begin { [weak self] result in
            guard result == .OK, let url = openPanel.url else { return }
            
            guard let encoding = self?.detectEncoding(of: url) else {
                self?.alert("Unknown encoding for \(url.pathExtension) file.")
                return
            }
            
            
            if encoding != .utf8 {
                self?.alert("Invalid encoding for \(url.pathExtension) file.")
                return
            }
            
            do {
                let contents = try String(contentsOf: url, encoding: encoding)
                self?.textView.string = contents
                self?.applySyntaxHighlighting()
                self?.currentFileURL = url
                self?.assignExportAction()
            } catch {
                self?.alert("Failed to open file: \(error)")
            }
        }
    }
    
    func saveFile() {
        guard let url = currentFileURL else {
            saveFileAs()
            return
        }
        do {
            try textView.string.write(to: url, atomically: true, encoding: .utf8)
            
            if let item = mainMenu.item(withTitle: "File")?.submenu?.item(withTitle: "Revert to Saved") {
                item.action = nil
            }
        } catch {
            alert("Failed to save file: \(error)")
        }
        
    }
    
    func saveFileAs() {
        let savePanel = NSSavePanel()
        if #available(macOS 12.0, *) {
            savePanel.allowedContentTypes = [UTType(filenameExtension: "prgm+")].compactMap { $0 }
        } else {
            savePanel.allowedFileTypes = ["prgm+"]
        }
        
        savePanel.begin { [weak self] result in
            guard result == .OK, let url = savePanel.url else { return }
            do {
                try self?.textView.string.write(to: url, atomically: true, encoding: .utf8)
                if self?.currentFileURL == url {
                    self?.removeRevertToSavedAction()
                } else {
                    self?.currentFileURL = url
                }
            } catch {
                self?.alert("Failed to save file: \(error)")
            }
        }
    }
    
    @objc private func revertToSavedDocument() {
        guard let mainMenu = NSApp.mainMenu else { return }
        
        if let url = currentFileURL {
            if let item = mainMenu.item(withTitle: "File")?.submenu?.item(withTitle: "Revert to Saved") {
                do {
                    let contents = try String(contentsOf: url, encoding: .utf8)
                    textView.string = contents
                    applySyntaxHighlighting()
                    item.action = nil
                } catch {
                    return
                }
            }
        }
    }
    
    @objc private func exportAs() {
        guard let currentFileURL = currentFileURL else {
            return
        }
        
        let extensions = ["prgm", "ppl"]
        let savePanel = NSSavePanel()
        if #available(macOS 12.0, *) {
            let contentTypes = extensions.compactMap { UTType(filenameExtension: $0) }
            savePanel.allowedContentTypes = contentTypes
        } else {
            savePanel.allowedFileTypes = extensions
        }
        
        if let path = developerPath, !FileManager.default.fileExists(atPath: path) {
            alert("Developer folder missing at: \(path)")
        }
        let toolPath = (developerPath ?? "/urs/local") + "/bin/ppl+"
        
        saveFile()
        
        savePanel.begin { [weak self] result in
            guard result == .OK, let url = savePanel.url else { return }
            
            let process = Process()
            process.executableURL = URL(fileURLWithPath: toolPath)
            process.arguments = [currentFileURL.path, "-o", url.path]
            process.currentDirectoryURL = url.deletingLastPathComponent()
            
            do {
                try process.run()
                process.waitUntilExit()
                
                if !FileManager.default.fileExists(atPath: url.path) {
                    self?.alert("Failed to export file: \(url.path)")
                }
            } catch {
                self?.alert("Failed to export file: \(error)")
            }
        }
        
    }
    
    @objc private func exportAsHpprgm() {
        guard let url = currentFileURL else {
            return
        }
        
        guard FileManager.default.fileExists(atPath: url.path) else {
            alert("Input file does not exist at: \(url.path)")
            return
        }
        
        let outputURL = url.deletingPathExtension().appendingPathExtension("hpprgm")
        let toolPath = "/Applications/HP/PrimeSDK/bin/ppl+"
        
        
        let process = Process()
        process.executableURL = URL(fileURLWithPath: toolPath)
        process.arguments = [url.path, "-o", outputURL.path]
        process.currentDirectoryURL = url.deletingLastPathComponent()
        
        do {
            try process.run()
            process.waitUntilExit()
            
            if !FileManager.default.fileExists(atPath: outputURL.path) {
                alert("Failed to export file: \(outputURL.path)")
            }
            
        } catch {
            alert("Developer Tool Missing!")
            return
        }
    }
    
    // MARK: -
    
    private func assignExportAction() {
        if let item = mainMenu.item(withTitle: "File")?.submenu?.item(withTitle: "Export")?.submenu?.item(withTitle: "Export As...") {
            item.action = #selector(exportAs)
        }
        
        if let item = mainMenu.item(withTitle: "File")?.submenu?.item(withTitle: "Export")?.submenu?.item(withTitle: "Quick Export As HPPRGM") {
            item.action = #selector(exportAsHpprgm)
        }
    }
    
    private func assignRevertToSavedAction() {
        if let item = mainMenu.item(withTitle: "File")?.submenu?.item(withTitle: "Revert to Saved") {
            item.action = #selector(revertToSavedDocument)
        }
    }
    
    private func removeRevertToSavedAction() {
        if let item = mainMenu.item(withTitle: "File")?.submenu?.item(withTitle: "Revert to Saved") {
            item.action = nil
        }
    }
    
    private func alert(_ message: String) {
        let alert = NSAlert()
        alert.messageText = "Error"
        alert.informativeText = message
        alert.runModal()
    }
    
}

