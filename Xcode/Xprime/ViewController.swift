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
import UniformTypeIdentifiers

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

final class ViewController: NSViewController, NSTextViewDelegate {
    var currentURL: URL?
    
    @IBOutlet var codeEditorTextView: CodeEditorTextView!
    @IBOutlet var outputTextView: NSTextView!
    @IBOutlet var statusTextLabel: NSTextField!
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        codeEditorTextView.delegate = self
        
        // Add the Line Number Ruler
        if let scrollView = codeEditorTextView.enclosingScrollView {
            let ruler = LineNumberRulerView(textView: codeEditorTextView)
            scrollView.verticalRulerView = ruler
            scrollView.hasVerticalRuler = true
            scrollView.rulersVisible = true
            
            // Force layout to avoid invisible window
            scrollView.tile()
        }
        
        
        if let url = Bundle.main.resourceURL?.appendingPathComponent("default.prgm+") {
            codeEditorTextView.string = loadPrgmFile(url) ?? ""
        }
        
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(updateStatus),
            name: NSTextView.didChangeSelectionNotification,
            object: codeEditorTextView
        )
    }
    
    @objc private func updateStatus() {
        if let editor = codeEditorTextView {
            let text = editor.string as NSString
            let selectedRange = editor.selectedRange
            let cursorLocation = selectedRange.location
            
            // Find line number
            var lineNumber = 1
            var columnNumber = 1
            
            // Count newlines up to the cursor
            for i in 0..<cursorLocation {
                if text.character(at: i) == 10 { // '\n'
                    lineNumber += 1
                    columnNumber = 1
                } else {
                    columnNumber += 1
                }
            }
            statusTextLabel.stringValue = "Line: \(lineNumber) Col: \(columnNumber)"
        }
    }
    
    override func viewDidAppear() {
        super.viewDidAppear()
        
       
        if let window = self.view.window {
            window.representedURL = Bundle.main.resourceURL?.appendingPathComponent("default.prgm+")
            let iconButton = window.standardWindowButton(.documentIconButton)!
            iconButton.image = NSImage(named: "pplplus")
            
            window.title = "Untiled (UNSAVED)"
        }
    }
    
    
    // MARK: - Helper Functions
    func registerUndo<T: AnyObject>(
        target: T,
        oldValue: @autoclosure @escaping () -> String,
        keyPath: ReferenceWritableKeyPath<T, String>,
        undoManager: UndoManager?,
        actionName: String = "Edit"
    ) {
        guard let undoManager = undoManager else { return }
        let previousValue = oldValue()
        
        undoManager.registerUndo(withTarget: target) { target in
            let currentValue = target[keyPath: keyPath]
            self.registerUndo(target: target,
                         oldValue: currentValue,
                         keyPath: keyPath,
                         undoManager: undoManager,
                         actionName: actionName)
            target[keyPath: keyPath] = previousValue
        }
        undoManager.setActionName(actionName)
    }
    
    func registerTextViewUndo(actionName: String = "") {
        registerUndo(target: codeEditorTextView,
                     oldValue: self.codeEditorTextView.string,
                     keyPath: \NSTextView.string,
                     undoManager: codeEditorTextView.undoManager,
                     actionName: actionName)
    }
    
    func updateDocumentIconButtonImage() {
        guard let url = self.currentURL else {
            return
        }
        if let window = self.view.window {
            window.title = url.lastPathComponent
            
            window.representedURL = URL(fileURLWithPath: url.path)
            if let iconButton = window.standardWindowButton(.documentIconButton) {
                if url.pathExtension == "prgm+" {
                    iconButton.image = NSImage(named: "pplplus")
                } else {
                    iconButton.image = NSImage(named: "ppl")
                }
                iconButton.isHidden = false
            }
        }
    }
}

