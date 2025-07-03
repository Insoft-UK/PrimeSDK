// The MIT License (MIT)
//
// Copyright (c) 2025 Insoft. All rights reserved.
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

@main
class AppDelegate: NSObject, NSApplicationDelegate {
    @IBOutlet weak var mainMenu: NSMenu!
    let tempManager = TempFileManager()
    
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
        tempManager.cleanup()
    }
    
    func applicationSupportsSecureRestorableState(_ app: NSApplication) -> Bool {
        return true
    }
    
    
    @IBAction func openDocument(_ sender: Any) {
        if let vc = NSApp.mainWindow?.contentViewController as? ViewController {
            vc.openFile()
        }
    }
    
    @IBAction func saveDocument(_ sender: Any) {
        if let vc = NSApp.mainWindow?.contentViewController as? ViewController {
            vc.saveFile()
        }
    }
    
    @IBAction func saveDocumentAs(_ sender: Any) {
        if let vc = NSApp.mainWindow?.contentViewController as? ViewController {
            vc.saveFileAs()
        }
    }
    
    @IBAction func installLibraries(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }
        
        guard let tmpURL = vc.tempDirectoryURL else { return }
        let destURL = URL(fileURLWithPath: NSString(string: "~/Documents/HP Connectivity Kit/Content").expandingTildeInPath)
        let srcURL = Bundle.main.bundleURL.appendingPathComponent("Contents/Developer/usr/prgm")
        
        let names: [String] = ["ColorSpace", "HP", "GROB"]
        for name in names {
            if vc.minifier == true {
                PrimeSDK.pplmin(i: srcURL.appendingPathComponent("\(name).prgm"), o: tmpURL.appendingPathComponent("\(name).prgm"))
                PrimeSDK.hpprgm(i: tmpURL.appendingPathComponent("\(name).prgm"), o: destURL.appendingPathComponent("\(name).hpprgm"))
            } else {
                PrimeSDK.hpprgm(i: srcURL.appendingPathComponent("\(name).prgm"), o: destURL.appendingPathComponent("\(name).hpprgm"))
            }
        }
    }
    
    @IBAction func installFonts(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }
       
        let destURL = URL(fileURLWithPath: NSString(string: "~/Documents/HP Connectivity Kit/Content").expandingTildeInPath)
        let srcURL = Bundle.main.bundleURL.appendingPathComponent("Contents/Developer/usr/prgm/fonts")
        var contents: String = ""
        
        for file in ["CGA.prgm", "EGA.prgm", "VGA.prgm", "BBC.prgm", "ARCADE.prgm", "HD44780.prgm"] {
            if let prgm = vc.loadPrgmFile(srcURL.appendingPathComponent(file)) {
                contents.append(prgm)
            }
        }
        
        try? contents.write(to: destURL.appendingPathComponent("Fonts.prgm"), atomically: true, encoding: .utf16LittleEndian)
        PrimeSDK.hpprgm(i: destURL.appendingPathComponent("Fonts.prgm"), o: destURL.appendingPathComponent("Fonts.hpprgm"))
        try? FileManager.default.removeItem(at: destURL.appendingPathComponent("Fonts.prgm"))
    }
    
    @IBAction func embedImage(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }
        vc.embedImage()
    }
    
    @IBAction func embedFont(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }
        vc.embedFont()
    }
    
    @IBAction func insertCode(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }
        vc.insertCode()
    }
    
    @IBAction func minifier(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }
        guard let menuItem = sender as? NSMenuItem else { return }
        
        if menuItem.state == .on {
            menuItem.state = .off
            vc.minifier = false
        } else {
            menuItem.state = .on
            vc.minifier = true
        }
    }
    
    @IBAction func insertTemplate(_ sender: Any) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }
        guard let menuItem = sender as? NSMenuItem else { return }
        
        let url = Bundle.main.bundleURL.appendingPathComponent("Contents/Template/\(traceMenuItem(menuItem))/\(menuItem.title).prgm")
        
        if let contents = vc.loadPrgmFile(url) {
            vc.registerTextViewUndo(actionName: "Template")
            vc.insertString(contents)
            vc.applySyntaxHighlighting()
        }
    }
    
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
    
    func populateThemesMenu(menu: NSMenu) {
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
    
    @objc func handleThemeSelection(_ sender: NSMenuItem) {
        guard let vc = NSApp.mainWindow?.contentViewController as? ViewController else { return }
        
        guard let fileURL = sender.representedObject as? URL else { return }
        vc.loadTheme(at: fileURL)
        vc.applySyntaxHighlighting()
        
        for menuItem in mainMenu.item(withTitle: "Editor")?.submenu?.item(withTitle: "Theme")!.submenu!.items ?? [] {
            menuItem.state = .off
        }
        sender.state = .on
    }
}

