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

extension MainViewController: NSWindowRestoration {
    static func restoreWindow(withIdentifier identifier: NSUserInterfaceItemIdentifier, state: NSCoder, completionHandler: @escaping (NSWindow?, Error?) -> Void) {
        // Restore your window here if needed
        completionHandler(nil, nil) // or provide restored window
    }
}

final class MainViewController: NSViewController, NSTextViewDelegate, NSToolbarItemValidation, NSMenuItemValidation {
    @IBOutlet weak var toolbar: NSToolbar!
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
    
    
    
    
    
    // MARK: - Interface Builder Action Handlers
    
    @IBAction func openDocument(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? MainViewController else { return }
        
        let openPanel = NSOpenPanel()
        let extensions = ["prgm", "prgm+"]
        let contentTypes = extensions.compactMap { UTType(filenameExtension: $0) }
        
        openPanel.allowedContentTypes = contentTypes
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = false
        
        openPanel.begin { result in
            guard result == .OK, let url = openPanel.url else { return }
            
            if let contents = loadPrgmFile(url) {
                vc.codeEditorTextView.string = contents
                vc.currentURL = url
                vc.updateDocumentIconButtonImage()
            }
        }
    }
    
    @IBAction func saveDocument(_ sender: Any) {
        guard let url = currentURL else {
            saveDocumentAs(sender)
            return
        }
        
        do {
            try savePrgmFile(url, codeEditorTextView.string)
            currentURL = url
        } catch {
            let alert = NSAlert()
            alert.messageText = "Error"
            alert.informativeText = "Failed to save file: \(error)"
            alert.runModal()
        }
    }
    
    @IBAction func saveDocumentAs(_ sender: Any) {
        let savePanel = NSSavePanel()
        let extensions = ["prgm", "prgm+"]
        let contentTypes = extensions.compactMap { UTType(filenameExtension: $0) }
        
        savePanel.allowedContentTypes = contentTypes
        savePanel.nameFieldStringValue = "Untitled.prgm+"
        
        savePanel.begin { result in
            guard result == .OK, let url = savePanel.url else { return }

            do {
                try savePrgmFile(url, self.codeEditorTextView.string)
                self.currentURL = url
            } catch {
                let alert = NSAlert()
                alert.messageText = "Error"
                alert.informativeText = "Failed to save file: \(error)"
                alert.runModal()
            }
        }
    
    }
    
    @IBAction func exportAsHpprgm(_ sender: Any) {
        guard let url = currentURL else { return }
        
        let savePanel = NSSavePanel()
        savePanel.allowedContentTypes = ["hpprgm"].compactMap { UTType(filenameExtension: $0) }
        savePanel.nameFieldStringValue = url.deletingPathExtension().lastPathComponent + ".hpprgm"
        savePanel.begin { result in
            guard result == .OK, let outURL = savePanel.url else { return }
            
            if let content = CommandLineTool.execute("hpprgm", arguments: [url.path, "-o", outURL.path]) {
                self.outputTextView.string = content
            }
        }
    }
    
    @IBAction func revertDocumentSaved(_ sender: Any) {
        if let contents = loadPrgmFile(currentURL!) {
            codeEditorTextView.string = contents
            updateDocumentIconButtonImage()
        }
    }
    
    @IBAction func run(_ sender: Any) {
        build(sender)
        guard let url = currentURL,
           FileManager.default.fileExists(atPath: url.path) else
        {
            return
        }
        let prgm = url.deletingPathExtension().appendingPathExtension("prgm")
        if FileManager.default.fileExists(atPath: prgm.path) {
            if let content = CommandLineTool.execute("hpprgm", arguments: [prgm.path]) {
                outputTextView.string = content
                let destURL = URL(fileURLWithPath: NSString(string: "~/Documents/HP Prime/Calculators/Prime").expandingTildeInPath)
                if !destURL.hasDirectoryPath {
                    return
                }
                let srcURL = prgm.deletingLastPathComponent().appendingPathComponent("hpprgm")
                try? FileManager.default.copyItem(atPath: srcURL.path, toPath: destURL.path)
                
                let task = Process()
                task.launchPath = "/Applications/HP Prime.app/Contents/MacOS/HP Prime"
                task.launch()
            }
        }
        
    }
    
    @IBAction func buildForRunning(_ sender: Any) {
        saveDocument(sender)
        
        if let url = currentURL,
           FileManager.default.fileExists(atPath: url.path)
        {
            if let content = CommandLineTool.`ppl+`(i: url) {
                outputTextView.string = content
            }
            let prgm = url.deletingPathExtension().appendingPathExtension("prgm")
            if FileManager.default.fileExists(atPath: prgm.path) {
                _ = CommandLineTool.execute("hpprgm", arguments: [prgm.path])
            }
        }
    }
    
    @IBAction func build(_ sender: Any) {
        saveDocument(sender)
        
        if let url = currentURL,
           FileManager.default.fileExists(atPath: url.path)
        {
            if let content = CommandLineTool.`ppl+`(i: url) {
                outputTextView.string = content
            }
        }
    }
    
    
    @IBAction func embedImage(_ sender: Any) {
        let openPanel = NSOpenPanel()
        let extensions = ["bmp", "png"]
        let contentTypes = extensions.compactMap { UTType(filenameExtension: $0) }
        
        openPanel.allowedContentTypes = contentTypes
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = false
        
        openPanel.begin { result in
            guard result == .OK, let url = openPanel.url else { return }
            
            if let contents = CommandLineTool.execute("grob", arguments: [url.path, "-o", "/dev/stdout"]) {
                self.registerTextViewUndo(actionName: "Insert Code")
                self.codeEditorTextView.insertCode(contents)
            }
        }
    }
    
    @IBAction func embedFont(_ sender: Any) {
        let openPanel = NSOpenPanel()
        let extensions = ["h"]
        let contentTypes = extensions.compactMap { UTType(filenameExtension: $0) }
        
        openPanel.allowedContentTypes = contentTypes
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = false
        
        openPanel.begin { result in
            guard result == .OK, let url = openPanel.url else { return }
            
            if let contents = CommandLineTool.execute("pplfont", arguments: [url.path, "-o", "/dev/stdout"]) {
                self.registerTextViewUndo(actionName: "Embeded Adafruit GFX font")
                self.codeEditorTextView.insertCode(contents)
            }
        }
    }
    
    @IBAction func insertCode(_ sender: Any) {
        guard let url = currentURL else { return }
        
        let openPanel = NSOpenPanel()
        var extensions = ["prgm"]
        if url.pathExtension == "prgm+" {
            extensions.append("prgm+")
        }
        let contentTypes = extensions.compactMap { UTType(filenameExtension: $0) }
        
        openPanel.allowedContentTypes = contentTypes
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = false
        
        openPanel.begin { result in
            guard result == .OK, let url = openPanel.url else { return }
            
            if let contents = loadPrgmFile(url) {
                self.registerTextViewUndo(actionName: "Insert")
                self.codeEditorTextView.insertCode(self.codeEditorTextView.removePragma(contents))
            }
        }
    }
    
    @IBAction func insertTemplate(_ sender: Any) {
        func traceMenuItem(_ item: NSMenuItem) -> String {
            if let parentMenu = item.menu {
                print("Item '\(item.title)' is in menu: \(parentMenu.title)")
                
                // Try to find the parent NSMenuItem that links to this menu
                for superitem in parentMenu.supermenu?.items ?? [] {
                    if superitem.submenu == parentMenu {
                        return superitem.title
                    }
                }
            }
            return ""
        }
        
        guard let menuItem = sender as? NSMenuItem else { return }
        
        let url = Bundle.main.bundleURL.appendingPathComponent("Contents/Template/\(traceMenuItem(menuItem))/\(menuItem.title).prgm")
        
        if let contents = loadPrgmFile(url) {
            registerTextViewUndo(actionName: "Template")
            codeEditorTextView.insertCode(contents)
        }
    }
    
    @IBAction func archive(_ sender: Any) {
        
    }
    
    @IBAction func reformatCode(_ sender: Any) {
        if let contents = CommandLineTool.pplref(i: currentURL!) {
            registerTextViewUndo(actionName: "Reformat Code")
            codeEditorTextView.string = contents
        }
    }
    
    func validateToolbarItem(_ item: NSToolbarItem) -> Bool {
        // Enable or disable toolbar items as needed. For now, always enable.
        switch item.action {
        case #selector(build(_:)):
            if let url = currentURL, url.pathExtension == "prgm+" {
                return true
            }
            return false
            
        case #selector(exportAsHpprgm(_:)):
            if let url = currentURL, url.pathExtension == "prgm" {
                return true
            }
            return false
            
         default :
            break
        }
        return true
    }
    
    @MainActor
    func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        switch menuItem.action {
        case #selector(run(_:)), #selector(build(_:)):
            if let url = currentURL, url.pathExtension == "prgm+" {
                return true
            }
            return false
            
        case #selector(archive(_:)), #selector(reformatCode(_:)), #selector(exportAsHpprgm(_:)):
            if let url = currentURL, url.pathExtension == "prgm" {
                return true
            }
            return false
            
        case #selector(embedImage(_:)), #selector(embedFont(_:)), #selector(revertDocumentSaved(_:)):
            if let _ = currentURL {
                return true
            }
            return false
        
        default:
            break
        }
        return true
    }
}

