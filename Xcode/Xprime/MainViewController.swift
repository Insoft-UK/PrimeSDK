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
    @IBOutlet var outputScrollView: NSScrollView!
    
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
        
        NotificationCenter.default.addObserver(
            forName: NSText.didChangeNotification,
            object: codeEditorTextView,
            queue: .main
        ) { [weak self] _ in
            self?.documentIsModified = true
        }
      
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
            
            window.title = "Untitled (UNSAVED)"
        }
    }
    
   
    var documentIsModified: Bool = false {
        didSet {
            if let window = self.view.window {
                if documentIsModified {
                    if let url = currentURL {
                        window.title = url.lastPathComponent + " — Edited"
                    }
                } else {
                    // When saved, show current file name or default title
                    if let url = currentURL {
                        window.title = url.lastPathComponent
                    } else {
                        window.title = "Untitled"
                    }
                }
            }
        }
    }
    
    
    // MARK: - Helper Functions
    private func registerUndo<T: AnyObject>(
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
    
    private func registerTextViewUndo(actionName: String = "") {
        registerUndo(target: codeEditorTextView,
                     oldValue: self.codeEditorTextView.string,
                     keyPath: \NSTextView.string,
                     undoManager: codeEditorTextView.undoManager,
                     actionName: actionName)
    }
    
    private func updateDocumentIconButtonImage() {
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
    
    @IBAction func discloseOutputTextView(_ sender: Any) {
        outputScrollView.isHidden.toggle()
    }
    
    @IBAction func openDocument(_ sender: Any) {
        let openPanel = NSOpenPanel()
        let extensions = ["prgm", "prgm+"]
        let contentTypes = extensions.compactMap { UTType(filenameExtension: $0) }
        
        openPanel.allowedContentTypes = contentTypes
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = false
        
        openPanel.begin { result in
            guard result == .OK, let url = openPanel.url else { return }
            
            if let contents = loadPrgmFile(url) {
                self.codeEditorTextView.string = contents
                self.currentURL = url
                self.updateDocumentIconButtonImage()
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
            self.documentIsModified = false
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
                self.documentIsModified = false
            } catch {
                let alert = NSAlert()
                alert.messageText = "Error"
                alert.informativeText = "Failed to save file: \(error)"
                alert.runModal()
            }
        }
    
    }
    
    /*
     • Saves the current PPL source file.
     
     • Builds an .hpprgm executable package from the
       generated .prgm file to a given destination.
     */
    @IBAction func exportAsHpprgm(_ sender: Any) {
        guard let url = currentURL else { return }
        
        saveDocument(sender)
        
        let savePanel = NSSavePanel()
        savePanel.allowedContentTypes = ["hpprgm"].compactMap { UTType(filenameExtension: $0) }
        savePanel.nameFieldStringValue = url.deletingPathExtension().lastPathComponent + ".hpprgm"
        savePanel.begin { result in
            guard result == .OK, let outURL = savePanel.url else { return }
            
            let contents = CommandLineTool.execute("/Applications/HP/PrimeSDK/bin/hpprgm", arguments: [url.path, "-o", outURL.path])
            if let out = contents.out, !out.isEmpty {
                self.outputTextView.string = out
            }
            self.outputTextView.string = contents.err ?? ""
        }
    }
    
    @IBAction func revertDocumentToSaved(_ sender: Any) {
        if let contents = loadPrgmFile(currentURL!) {
            codeEditorTextView.string = contents
            self.documentIsModified = false
            updateDocumentIconButtonImage()
        }
    }
    
    /*
     • Saves the current PPL+ source file.
     
     • Preprocesses the PPL+ (.prgm+) source into
       standard PPL (.prgm) format.
       NOTE: Only if PPL+
     
     • Builds an .hpprgm executable package from the
       generated .prgm file.
     */
    @IBAction func run(_ sender: Any) {
        guard let url = currentURL,
           FileManager.default.fileExists(atPath: url.path) else
        {
            return
        }
        
        buildForRunning(sender)
        runWithoutBuilding(sender)
    }
    
    /*
     • Saves the current PPL+ source file.
     
     • Preprocesses the PPL+ (.prgm+) source into
       standard PPL (.prgm) format.
     
     • Builds an .hpprgm executable package from the
       generated .prgm file.
     */
    @IBAction func buildForRunning(_ sender: Any) {
        build(sender)
        
        if let prgmURL = currentURL?.deletingPathExtension().appendingPathExtension("prgm") {
            outputTextView.string = "🧱 Building for running...\n\n"
            let contents = CommandLineTool.execute("/Applications/HP/PrimeSDK/bin/hpprgm", arguments: [prgmURL.path])
            if let out = contents.out, !out.isEmpty {
                self.outputTextView.string = out
            }
            outputTextView.string = contents.err ?? ""
        }
    }
    
    @IBAction func runWithoutBuilding(_ sender: Any) {
        guard let url = currentURL else { return }
            
        outputTextView.string = "🧱 Running without building...\n\n"
        
        let destURL = URL(fileURLWithPath: NSString(string: "~/Documents/HP Prime/Calculators/Prime").expandingTildeInPath)
        if !destURL.hasDirectoryPath {
            return
        }
        let srcURL = url.deletingPathExtension().appendingPathExtension("hpprgm")
        try? FileManager.default.copyItem(atPath: srcURL.path, toPath: destURL.path)
        
        let task = Process()
        task.launchPath = "/Applications/HP Prime.app/Contents/MacOS/HP Prime"
        task.launch()
    }
    
    /*
     • Saves the current PPL+ source file.
     
     • Preprocesses the PPL+ (.prgm+) source into
       standard PPL (.prgm) format.
     */
    @IBAction func build(_ sender: Any) {
        saveDocument(sender)
        
        if let url = currentURL,
           FileManager.default.fileExists(atPath: url.path)
        {
            outputTextView.string = "🧱 Building...\n\n"
            let contents = CommandLineTool.`ppl+`(i: url)
            outputTextView.string = contents.err ?? ""
            
        }
    }
    
    
    @IBAction func importImage(_ sender: Any) {
        let openPanel = NSOpenPanel()
        let extensions = ["bmp", "png"]
        let contentTypes = extensions.compactMap { UTType(filenameExtension: $0) }
        
        openPanel.allowedContentTypes = contentTypes
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = false
        
        openPanel.begin { result in
            guard result == .OK, let url = openPanel.url else { return }
            
            let contents = CommandLineTool.execute("/Applications/HP/PrimeSDK/bin/grob", arguments: [url.path, "-o", "/dev/stdout"])
            if let out = contents.out, !out.isEmpty {
                self.registerTextViewUndo(actionName: "Insert Code")
                self.codeEditorTextView.insertCode(out)
            }
            self.outputTextView.string += contents.err ?? ""
        }
    }
    
    @IBAction func importAdafruitGFXFont(_ sender: Any) {
        let openPanel = NSOpenPanel()
        let extensions = ["h"]
        let contentTypes = extensions.compactMap { UTType(filenameExtension: $0) }
        
        openPanel.allowedContentTypes = contentTypes
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = false
        
        openPanel.begin { result in
            guard result == .OK, let url = openPanel.url else { return }
            
            let contents = CommandLineTool.execute("/Applications/HP/PrimeSDK/bin/pplfont", arguments: [url.path, "-o", "/dev/stdout", "--ppl"])
            if let out = contents.out, !out.isEmpty {
                self.registerTextViewUndo(actionName: "Embeded Adafruit GFX font")
                self.codeEditorTextView.insertCode(contents.out ?? "")
            }
            self.outputTextView.string += contents.err ?? ""
        }
    }
    
    @IBAction func importCode(_ sender: Any) {
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
                self.registerTextViewUndo(actionName: "Imported Code")
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
        guard let url = currentURL else { return }
        
        buildForRunning(sender)
        
        let appName = url.deletingPathExtension().lastPathComponent
        let hpappdirURL = url.deletingPathExtension().appendingPathExtension("hpappdir")
        
        do {
            try FileManager.default.createDirectory(atPath: hpappdirURL.path, withIntermediateDirectories: true)
        } catch {
            outputTextView.string += "❌ Failed to create directory \(appName).hpappdir\n"
            return
        }
        
        outputTextView.string += "🗯️ Created \(appName).hpappdir\n"
        
        try? FileManager.default.copyItem(at: url.deletingPathExtension().appendingPathExtension("hpprgm"), to: hpappdirURL.appendingPathComponent("\(appName).hpappprgm"))
        try? FileManager.default.copyItem(at: Bundle.main.bundleURL.appendingPathComponent("Contents/Resources/template.hpapp"), to: hpappdirURL.appendingPathComponent("\(appName).hpapp"))
        try? FileManager.default.copyItem(at: Bundle.main.bundleURL.appendingPathComponent("Contents/Resources/icon.png"), to: hpappdirURL.appendingPathComponent("icon.hpapp"))
        
        let zipResult = CommandLineTool.execute("/usr/bin/zip", arguments: ["-j", "-r", "\(hpappdirURL.path).zip", hpappdirURL.path, "-x", "*.DS_Store"])
        if let out = zipResult.out, !out.isEmpty {
            self.outputTextView.string += out
        }
        self.outputTextView.string += zipResult.err ?? ""
    }
    
    @IBAction func reformatCode(_ sender: Any) {
        let contents = CommandLineTool.execute("/Applications/HP/PrimeSDK/bin/pplref", arguments: [currentURL!.path, "-o", "/dev/stdout"])
        if let out = contents.out, !out.isEmpty {
            registerTextViewUndo(actionName: "Reformat Code")
            codeEditorTextView.string = out
        }
        self.outputTextView.string += contents.err ?? ""
    }
    
    // MARK: - Validation for Toolbar Items
    
    internal func validateToolbarItem(_ item: NSToolbarItem) -> Bool {
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
    
    // MARK: - Validation for Menu Items
    
    internal func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        switch menuItem.action {
        case #selector(run(_:)), #selector(build(_:)), #selector(buildForRunning(_:)), #selector(archive(_:)):
            if let url = currentURL, url.pathExtension == "prgm+" {
                return true
            }
            return false
            
        case #selector(reformatCode(_:)), #selector(exportAsHpprgm(_:)):
            if let url = currentURL, url.pathExtension == "prgm" {
                return true
            }
            return false
            
        case #selector(runWithoutBuilding(_:)):
            if let url = currentURL, FileManager.default.fileExists(atPath: url.deletingPathExtension().appendingPathExtension("hpprgm").path) {
                return true
            }
            return false
            
        case #selector(revertDocumentToSaved(_:)):
            return documentIsModified
        
        default:
            break
        }
        return true
    }
}

