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
        Bundle.main.bundleURL
            .appendingPathComponent("Contents")
            .appendingPathComponent("developer")
            .path
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
        
        applySyntaxHighlighting()
    }
    
    override var representedObject: Any? {
        didSet {
            // Update the view, if already loaded.
        }
    }
    
    func textDidChange(_ notification: Notification) {
//        replaceOperators()
        applySyntaxHighlighting()
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
        let keywords = ["export", "begin", "default", "until", "switch", "case", "and", "or", "xor", "catalog", "local", "var", "if", "then", "else", "do", "while", "repeat", "return", "break", "end", "endif", "wend", "to", "downto", "step", "ifer", "try", "catch", "const"]
        for keyword in keywords {
            let pattern = "\\b\(keyword)\\b"
            let regex = try! NSRegularExpression(pattern: pattern, options: [.caseInsensitive])
            regex.enumerateMatches(in: text as String, range: fullRange) { match, _, _ in
                if let match = match {
                    textStorage.addAttribute(.foregroundColor, value: orange, range: match.range)
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
        let brPattern = #"[\[\](){}]+"#
        let brRegex = try! NSRegularExpression(pattern: brPattern)
        brRegex.enumerateMatches(in: text as String, range: fullRange) { match, _, _ in
            if let match = match {
                textStorage.addAttribute(.foregroundColor, value: blue, range: match.range)
            }
        }
        
        
        // Numbers
        let numberPattern = #"#[\dA-F]+(:-?\d+)?h|#\d+(:-?\d+)?d|#[0-7]+(:-?\d+)?o|#[01]+(:-?\d+)?b|\b-?\d+(\.\d+)?\b"#
        let numberRegex = try! NSRegularExpression(pattern: numberPattern)
        numberRegex.enumerateMatches(in: text as String, range: fullRange) { match, _, _ in
            if let match = match {
                textStorage.addAttribute(.foregroundColor, value: blue, range: match.range)
            }
        }
        
        // Strings
        let stringPattern = #"".*?""# // double-quoted strings
        let stringRegex = try! NSRegularExpression(pattern: stringPattern)
        stringRegex.enumerateMatches(in: text as String, range: fullRange) { match, _, _ in
            if let match = match {
                textStorage.addAttribute(.foregroundColor, value: blue, range: match.range)
            }
        }
        
        // Strings
        let regex = try! NSRegularExpression(pattern: #"\\`.*?`(:[#\-\dA-Fbodh]+)?"#)
        regex.enumerateMatches(in: text as String, range: fullRange) { match, _, _ in
            if let match = match {
                textStorage.addAttribute(.foregroundColor, value: blue, range: match.range)
            }
        }
        
        // Preprocessor Directives (e.g. #define, #ifdef)
        let preprocessorPattern = #"(?m)^\s*#.+"#
        let preprocessorRegex = try! NSRegularExpression(pattern: preprocessorPattern)
        preprocessorRegex.enumerateMatches(in: text as String, range: fullRange) { match, _, _ in
            if let match = match {
                textStorage.addAttribute(.foregroundColor, value: NSColor.systemGray, range: match.range)
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
    
    
    
    func replaceOperators() {
        guard let textStorage = textView.textStorage else { return }
        
        let replacements: [(String, String)] = [
            ("!=", "≠"),
            ("<>", "≠"),
            (">=", "≥"),
            ("<=", "≤"),
            ("=>", "▶")
        ]
        
        var newCursorLocation = textView.selectedRange().location
        let originalText = textView.string as NSString
        
        textStorage.beginEditing()
        
        var totalOffset = 0
        
        for (find, replace) in replacements {
            let pattern = NSRegularExpression.escapedPattern(for: find)
            let regex = try! NSRegularExpression(pattern: pattern)
            
            let matches = regex.matches(in: originalText as String, range: NSRange(location: 0, length: originalText.length))
            
            for match in matches.reversed() {
                let range = match.range
                let adjustedRange = NSRange(location: range.location + totalOffset, length: range.length)
                textStorage.replaceCharacters(in: adjustedRange, with: replace)
                
                let offsetDelta = replace.count - range.length
                totalOffset += offsetDelta
                
                // If replacement happened before or at the cursor, adjust it
                if adjustedRange.location <= newCursorLocation {
                    newCursorLocation += offsetDelta
                }
            }
        }
        
        textStorage.endEditing()
        
        // Update the selection to avoid jump or newline
        textView.setSelectedRange(NSRange(location: newCursorLocation, length: 0))
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
            let extensions = ["prgm", "prgm+", "ppl", "ppl+", "pp"]
            let contentTypes = extensions.compactMap { UTType(filenameExtension: $0) }
            openPanel.allowedContentTypes = contentTypes
        } else {
            openPanel.allowedFileTypes = ["prgm", "prgm+", "ppl", "ppl+", "pp"]
        }
        
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = false
        
        openPanel.begin { [weak self] result in
            guard result == .OK, let url = openPanel.url else { return }
            
            guard let encoding = self?.detectEncoding(of: url) else {
                print("Unknown encoding")
                return
            }
            
            //            if url.pathExtension == "ppl" || url.pathExtension == "ppl+" {
            //                if encoding != .utf8 {
            //                    print("Invalid encoding for \(url.pathExtension) file.")
            //                    return
            //                }
            //            }
            //
            //            if url.pathExtension == "prgm" || url.pathExtension == "prgm+" {
            //                if encoding != .utf16LittleEndian {
            //                    print("Invalid encoding for \(url.pathExtension) file.")
            //                    return
            //                }
            //            }
            
            do {
                let contents = try String(contentsOf: url, encoding: encoding)
                self?.textView.string = contents
                self?.applySyntaxHighlighting()
                self?.currentFileURL = url
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
                self?.currentFileURL = url
            } catch {
                print("Failed to save file:", error)
                // show alert if you want
            }
        }
    }
    
    func exportAsPrgm() {
        guard let url = currentFileURL else {
            print("No input file URL.")
            return
        }
        
        if let path = developerPath, FileManager.default.fileExists(atPath: path) {
            print("Developer folder exists at: \(path)")
        }
        
        let outputURL = url.deletingPathExtension().appendingPathExtension("prgm")
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
    
    func exportAsHpprgm() {
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
    
}

