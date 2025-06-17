//
//  ViewController.swift
//  Xprime
//
//  Created by Richie on 11/06/2025.
//


import Cocoa
import UniformTypeIdentifiers

extension ViewController: NSWindowRestoration {
    static func restoreWindow(withIdentifier identifier: NSUserInterfaceItemIdentifier, state: NSCoder, completionHandler: @escaping (NSWindow?, Error?) -> Void) {
        // Restore your window here if needed
        completionHandler(nil, nil) // or provide restored window
    }
}

class ViewController: NSViewController, NSTextViewDelegate {
    let mainMenu = NSApp.mainMenu!
    
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
            enableRevertToSavedMenuItem()
        }
    }
    
    func applySyntaxHighlighting() {
        
        
        guard let textStorage = textView.textStorage else { return }
        
        let text = textView.string as NSString
        let fullRange = NSRange(location: 0, length: text.length)
        
        let blue = NSColor.init(calibratedRed: 0.330, green: 0.510, blue: 1, alpha: 1)
        let orange = NSColor.init(calibratedRed: 0.992, green: 0.561, blue: 0.247, alpha: 1)
       
        // Reset all text color first
        textStorage.beginEditing()
        textStorage.setAttributes(baseAttributes, range: fullRange)
        
        // Keywords
        let keywords = [
            "begin", "end", "return", "kill", "if", "then", "else", "xor", "or", "and", "not",
            "case", "default", "iferr", "ifte", "for", "from", "step", "downto", "to", "do",
            "while", "repeat", "until", "break", "continue", "export", "const", "local", "key",
            "var", "endif", "wend", "next", "switch", "try", "catch", "catalog", "private", "dict", "regex"
        ]
        for keyword in keywords {
            let pattern = "\\b\(keyword)\\b"
            let regex = try! NSRegularExpression(pattern: pattern, options: [.caseInsensitive])
            regex.enumerateMatches(in: text as String, range: fullRange) { match, _, _ in
                if let match = match {
                    textStorage.addAttribute(.foregroundColor, value: blue, range: match.range)
                }
            }
        }
        
        
        // Operators
        let operatorPattern = #"[▶:=+\-*/<>≠≤≥]+"#
        let operatorRegex = try! NSRegularExpression(pattern: operatorPattern)
        operatorRegex.enumerateMatches(in: text as String, range: fullRange) { match, _, _ in
            if let match = match {
                textStorage.addAttribute(.foregroundColor, value: NSColor.white, range: match.range)
            }
        }
        
        // Brackets
        let brPattern = #"\{[^}]+\}"#
        let brRegex = try! NSRegularExpression(pattern: brPattern)
        brRegex.enumerateMatches(in: text as String, range: fullRange) { match, _, _ in
            if let match = match {
                textStorage.addAttribute(.foregroundColor, value: NSColor.systemGray, range: match.range)
            }
        }
        
        
        // Numbers
        let numberPattern = #"#[\dA-F]+(:-?\d+)?h|#\d+(:-?\d+)?d|#[0-7]+(:-?\d+)?o|#[01]+(:-?\d+)?b|\b-?\d+(\.\d+)?\b"#
        let numberRegex = try! NSRegularExpression(pattern: numberPattern)
        numberRegex.enumerateMatches(in: text as String, range: fullRange) { match, _, _ in
            if let match = match {
                textStorage.addAttribute(.foregroundColor, value: orange, range: match.range)
            }
        }
        
        // Strings
        let stringPattern = #"".*?""# // double-quoted strings
        let stringRegex = try! NSRegularExpression(pattern: stringPattern)
        stringRegex.enumerateMatches(in: text as String, range: fullRange) { match, _, _ in
            if let match = match {
                textStorage.addAttribute(.foregroundColor, value: orange, range: match.range)
            }
        }
        
        // ``
        let regex = try! NSRegularExpression(pattern: #"\\`.*?`(:[#\-\dA-Fbodh]+)?"#)
        regex.enumerateMatches(in: text as String, range: fullRange) { match, _, _ in
            if let match = match {
                textStorage.addAttribute(.foregroundColor, value: orange, range: match.range)
            }
        }
        
        // Preprocessor Directives (e.g. #define, #ifdef)
        let preprocessorPattern = #"(?m)^\s*#.+"#
        let preprocessorRegex = try! NSRegularExpression(pattern: preprocessorPattern)
        preprocessorRegex.enumerateMatches(in: text as String, range: fullRange) { match, _, _ in
            if let match = match {
                textStorage.addAttribute(.foregroundColor, value: orange, range: match.range)
            }
        }
        
        // Comments
        let commentPattern = #"//.*"#  // Matches '//' followed by any characters to the end of the line
        let commentRegex = try! NSRegularExpression(pattern: commentPattern)
        commentRegex.enumerateMatches(in: text as String, range: fullRange) { match, _, _ in
            if let match = match {
                textStorage.addAttribute(.foregroundColor, value: NSColor.systemGray, range: match.range)
            }
        }
        
        
        textStorage.endEditing()
    }
    
    
    
    func replaceLastTypedOperator() {
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
    
    func autoIndentCurrentLine() {
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
    
    func detectEncoding(of url: URL) -> String.Encoding? {
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
            print("Error reading file data: \(error)")
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
                print("Unknown encoding")
                return
            }
            
            
            if encoding != .utf8 {
                print("Invalid encoding for \(url.pathExtension) file.")
                return
            }
            
            do {
                let contents = try String(contentsOf: url, encoding: encoding)
                self?.textView.string = contents
                self?.applySyntaxHighlighting()
                self?.currentFileURL = url
                self?.enableExportMenuItems()
            } catch {
                // handle error (e.g., show alert)
                print("Failed to open file:", error)
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
            print("Failed to save file:", error)
            // show alert if you want
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
                    self?.disableRevertToSavedMenuItem()
                } else {
                    self?.currentFileURL = url
                }
            } catch {
                print("Failed to save file:", error)
                // show alert if you want
            }
        }
    }
    
    @objc func revertToSavedDocument() {
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
    
    @objc func exportAs() {
        guard let currentFileURL = currentFileURL else {
            print("No input file URL.")
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
        
        if let path = developerPath, FileManager.default.fileExists(atPath: path) {
            print("Developer folder exists at: \(path)")
        }
        let toolPath = (developerPath ?? "/urs/local") + "/bin/ppl+"
        
        saveFile()
        
        savePanel.begin { result in
            guard result == .OK, let url = savePanel.url else { return }
            
            let process = Process()
            process.executableURL = URL(fileURLWithPath: toolPath)
            process.arguments = [currentFileURL.path, "-o", url.path]
            process.currentDirectoryURL = url.deletingLastPathComponent()
            
            do {
                try process.run()
                process.waitUntilExit()
                
                print("Process finished with status: \(process.terminationStatus)")
                
                if FileManager.default.fileExists(atPath: url.path) {
                    print("Output file created at: \(url.path)")
                } else {
                    print("ppl+ completed, but output file not found.")
                }
            } catch {
                print("Failed to save file:", error)
                // show alert if you want
            }
        }
        
    }
    
    @objc func exportAsHpprgm() {
        guard let url = currentFileURL else {
            print("No input file URL.")
            return
        }
        
        let outputURL = url.deletingPathExtension().appendingPathExtension("hpprgm")
        let toolPath = "/Applications/HP/PrimeSDK/bin/ppl+"
        
        guard FileManager.default.fileExists(atPath: url.path) else {
            print("Input file does not exist at: \(url.path)")
            return
        }
        
        let process = Process()
        process.executableURL = URL(fileURLWithPath: toolPath)
        process.arguments = [url.path, "-o", outputURL.path]
        process.currentDirectoryURL = url.deletingLastPathComponent()
        
        do {
            try process.run()
            process.waitUntilExit()
            
            print("Process finished with status: \(process.terminationStatus)")
            
            if FileManager.default.fileExists(atPath: outputURL.path) {
                print("Output file created at: \(outputURL.path)")
            } else {
                print("ppl+ completed, but output file not found.")
            }
            
        } catch {
            print("Failed to run ppl+ tool:", error)
            return
        }
    }
    
    
    func enableExportMenuItems() {
        if let item = mainMenu.item(withTitle: "File")?.submenu?.item(withTitle: "Export")?.submenu?.item(at: 1) {
            item.action = #selector(exportAs)
        }
        
        if let item = mainMenu.item(withTitle: "File")?.submenu?.item(withTitle: "Export")?.submenu?.item(at: 2) {
            item.action = #selector(exportAsHpprgm)
        }
    }

    func enableRevertToSavedMenuItem() {
        if let item = mainMenu.item(withTitle: "File")?.submenu?.item(withTitle: "Revert to Saved") {
            item.action = #selector(revertToSavedDocument)
        }
    }
    
    func disableRevertToSavedMenuItem() {
        if let item = mainMenu.item(withTitle: "File")?.submenu?.item(withTitle: "Revert to Saved") {
            item.action = nil
        }
    }
    
}

