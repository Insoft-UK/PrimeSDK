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

@main
class AppDelegate: NSObject, NSApplicationDelegate, NSMenuItemValidation {
    @IBOutlet weak var mainMenu: NSMenu!

    fileprivate func populateThemesMenu(menu: NSMenu) {
        guard let resourceURLs = Bundle.main.urls(forResourcesWithExtension: "xpcolortheme", subdirectory: nil) else {
            print("No .xpcolortheme files found.")
            return
        }

        for fileURL in resourceURLs {
            let filename = fileURL.deletingPathExtension().lastPathComponent

            let menuItem = NSMenuItem(title: filename, action: #selector(handleThemeSelection(_:)), keyEquivalent: "")
            menuItem.representedObject = fileURL
            menuItem.target = self  // or another target if needed
            if filename == "Default (Dark)" {
                menuItem.state = .on
            }

            menu.item(withTitle: "Editor")?.submenu?.item(withTitle: "Theme")?.submenu?.addItem(menuItem)
        }
    }
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Insert code here to initialize your application
        NSApp.appearance = NSAppearance(named: .darkAqua)
        
        UserDefaults.standard.set(false, forKey: "NSAutomaticPeriodSubstitutionEnabled")
        UserDefaults.standard.set(false, forKey: "NSAutomaticTextReplacementEnabled")
        UserDefaults.standard.set(false, forKey: "NSAutomaticQuoteSubstitutionEnabled")
        UserDefaults.standard.set(false, forKey: "NSAutomaticDashSubstitutionEnabled")
        UserDefaults.standard.synchronize()
        
        populateThemesMenu(menu: mainMenu)
    }
    
    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
//        tempManager.cleanup()
    }
    
    func applicationSupportsSecureRestorableState(_ app: NSApplication) -> Bool {
        return true
    }
    
    // MARK: - Interface Builder Action Handlers
    
    @IBAction func openDocument(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }
        
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
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }
        guard let url = vc.currentURL else {
            saveDocumentAs(sender)
            return
        }
        
        do {
            try savePrgmFile(url, vc.codeEditorTextView.string)
        } catch {
            let alert = NSAlert()
            alert.messageText = "Error"
            alert.informativeText = "Failed to save file: \(error)"
            alert.runModal()
        }
    }
    
    @IBAction func saveDocumentAs(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }

        let savePanel = NSSavePanel()
        let extensions = ["prgm", "prgm+"]
        let contentTypes = extensions.compactMap { UTType(filenameExtension: $0) }
        
        savePanel.allowedContentTypes = contentTypes
        savePanel.nameFieldStringValue = "Untitled.prgm+"
        
        savePanel.begin { result in
            guard result == .OK, let url = savePanel.url else { return }

            do {
                try savePrgmFile(url, vc.codeEditorTextView.string)
                vc.updateDocumentIconButtonImage()
            } catch {
                let alert = NSAlert()
                alert.messageText = "Error"
                alert.informativeText = "Failed to save file: \(error)"
                alert.runModal()
            }
        }
    
    }
    
    @IBAction private func exportAsHpprgm(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }
        guard let url = vc.currentURL else { return }
        
        let savePanel = NSSavePanel()
        savePanel.allowedContentTypes = ["hpprgm"].compactMap { UTType(filenameExtension: $0) }
        savePanel.nameFieldStringValue = url.deletingPathExtension().lastPathComponent + ".hpprgm"
        savePanel.begin { result in
            guard result == .OK, let outURL = savePanel.url else { return }
            
            _ = CommandLineTool.hpprgm(i: url, o: outURL)
            if !FileManager.default.fileExists(atPath: outURL.path) {
                let alert = NSAlert()
                alert.messageText = "Error"
                alert.informativeText = "Failed to export file: \(outURL.path)"
                alert.runModal()
            }
        }
    }
    
    @IBAction private func revertDocumentSaved(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }

        if let contents = loadPrgmFile(vc.currentURL!) {
            vc.codeEditorTextView.string = contents
            vc.updateDocumentIconButtonImage()
        }
    }
    
    @IBAction func run(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }

        build(sender)
        vc.exportToHpPrimeEmulator()
    }
    
    @IBAction func buildForRunning(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }

        saveDocument(sender)
        
        if let url = vc.currentURL,
           FileManager.default.fileExists(atPath: url.path)
        {
            CommandLineTool.`ppl+`(i: url)
            let prgm = url.deletingPathExtension().appendingPathExtension("prgm")
            if FileManager.default.fileExists(atPath: prgm.path) {
                CommandLineTool.hpprgm(i: prgm)
            }
        }
    }
    
    @IBAction func build(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }

        saveDocument(sender)
        
        if let url = vc.currentURL,
           FileManager.default.fileExists(atPath: url.path)
        {
            CommandLineTool.`ppl+`(i: url)
            
            let prgm = url.deletingPathExtension().appendingPathExtension("prgm")
            if let contents = loadPrgmFile(prgm) {
                vc.codeEditorTextView.string = contents
                vc.currentURL = prgm
                vc.updateDocumentIconButtonImage()
            }
        }
    }
    
    
    @IBAction func installLibraries(_ sender: Any) {
        let destURL = URL(fileURLWithPath: NSString(string: "~/Documents/HP Connectivity Kit/Content").expandingTildeInPath)
        let srcURL = Bundle.main.bundleURL.appendingPathComponent("Contents/Developer/usr/hpprgm")
        
        for file in ["ColorSpace.hpprgm", "HP.hpprgm", "GROB.hpprgm"] {
            try? FileManager.default.copyItem(at: srcURL.appendingPathComponent(file), to: destURL)
        }
    }
    
    @IBAction func installFonts(_ sender: Any) {
        let destURL = URL(fileURLWithPath: NSString(string: "~/Documents/HP Connectivity Kit/Content").expandingTildeInPath)
        let srcURL = Bundle.main.bundleURL.appendingPathComponent("Contents/Developer/usr/hpprgm/fonts")
      
        for file in ["CGA.hpprgm", "EGA.hpprgm", "VGA.hpprgm", "BBC.hpprgm", "ARCADE.hpprgm", "HD44780.hpprgm"] {
            try? FileManager.default.copyItem(at: srcURL.appendingPathComponent(file), to: destURL)
        }
    }
    
    @IBAction func embedImage(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }
        
        let openPanel = NSOpenPanel()
        let extensions = ["bmp", "png"]
        let contentTypes = extensions.compactMap { UTType(filenameExtension: $0) }
        
        openPanel.allowedContentTypes = contentTypes
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = false
        
        openPanel.begin { result in
            guard result == .OK, let url = openPanel.url else { return }
            
            if let contents = CommandLineTool.grob(i: url) {
                vc.registerTextViewUndo(actionName: "Insert Code")
                vc.codeEditorTextView.insertCode(contents)
            }
        }
    }
    
    @IBAction func embedFont(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }

        let openPanel = NSOpenPanel()
        let extensions = ["h"]
        let contentTypes = extensions.compactMap { UTType(filenameExtension: $0) }
        
        openPanel.allowedContentTypes = contentTypes
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = false
        
        openPanel.begin { result in
            guard result == .OK, let url = openPanel.url else { return }
            
            if let contents = CommandLineTool.grob(i: url) {
                vc.registerTextViewUndo(actionName: "Embeded Adafruit GFX font")
                vc.codeEditorTextView.insertCode(contents)
            }
        }
    }
    
    @IBAction func insertCode(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }
        guard let url = vc.currentURL else { return }
        
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
                vc.registerTextViewUndo(actionName: "Insert")
                vc.codeEditorTextView.insertCode(vc.codeEditorTextView.removePragma(contents))
            }
        }
    }
    
    @IBAction func insertTemplate(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }
        guard let menuItem = sender as? NSMenuItem else { return }
        
        let url = Bundle.main.bundleURL.appendingPathComponent("Contents/Template/\(traceMenuItem(menuItem))/\(menuItem.title).prgm")
        
        if let contents = loadPrgmFile(url) {
            vc.registerTextViewUndo(actionName: "Template")
            vc.codeEditorTextView.insertCode(contents)
        }
    }
    
    @IBAction func archive(_ sender: Any) {
        guard let _ = NSApp.mainWindow?.contentViewController as? ViewController else { return }
        
    }
    
    @IBAction func reformatCode(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }
        
        if let contents = CommandLineTool.pplref(i: vc.currentURL!) {
            vc.registerTextViewUndo(actionName: "Reformat Code")
            vc.codeEditorTextView.string = contents
        }
    }

    // MARK: - Action Handlers
    
    private func traceMenuItem(_ item: NSMenuItem) -> String {
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
    
    
    @objc func handleThemeSelection(_ sender: NSMenuItem) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }
        
        guard let fileURL = sender.representedObject as? URL else { return }
        vc.codeEditorTextView.loadTheme(at: fileURL)
        
        for menuItem in mainMenu.item(withTitle: "Editor")?.submenu?.item(withTitle: "Theme")!.submenu!.items ?? [] {
            menuItem.state = .off
        }
        sender.state = .on
    }
    
    
    func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return true }
        
        switch menuItem.action {
        case #selector(run(_:)), #selector(build(_:)):
            if let url = vc.currentURL, url.pathExtension == "prgm+" {
                return true
            }
            return false
            
        case #selector(archive(_:)), #selector(reformatCode(_:)), #selector(exportAsHpprgm(_:)):
            if let url = vc.currentURL, url.pathExtension == "prgm" {
                return true
            }
            return false
            
        case #selector(embedImage(_:)), #selector(embedFont(_:)), #selector(revertDocumentSaved(_:)):
            if let _ = vc.currentURL {
                return true
            }
            return false
        
        default:
            break
        }
        return true
    }
    
}
