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
class AppDelegate: NSObject, NSApplicationDelegate {
    @IBOutlet weak var mainMenu: NSMenu!
    
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
    }
    
    func applicationSupportsSecureRestorableState(_ app: NSApplication) -> Bool {
        return true
    }
    
    
    private func populateThemesMenu(menu: NSMenu) {
        guard let resourceURLs = Bundle.main.urls(forResourcesWithExtension: "xpcolortheme", subdirectory: nil) else {
            print("⚠️ No .xpcolortheme files found.")
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
    
    // MARK: - Interface Builder Action Handlers
    
    private func isHPConnectivityKitInstalled() -> Bool {
        let url = URL(fileURLWithPath: NSString(string: "~/Documents/HP Connectivity Kit/").expandingTildeInPath)
        var isDir: ObjCBool = false
        let exists = FileManager.default.fileExists(atPath: url.path, isDirectory: &isDir)
        return exists && isDir.boolValue
    }
    
    private func HPConnectivityKitURL() -> URL? {
        guard isHPConnectivityKitInstalled() else { return nil }
        return URL(fileURLWithPath: NSString(string: "~/Documents/HP Connectivity Kit/").expandingTildeInPath)
    }
    
    
    @IBAction func installLibraries(_ sender: Any) {
        guard isHPConnectivityKitInstalled() else { return }
        
        let destURL = URL(fileURLWithPath: NSString(string: "~/Documents/HP Connectivity Kit/Content").expandingTildeInPath)
        let srcURL = Bundle.main.bundleURL.appendingPathComponent("Contents/Developer/usr/hpprgm")
        
        for file in ["ColorSpace.hpprgm", "HP.hpprgm", "GROB.hpprgm"] {
            try? FileManager.default.copyItem(at: srcURL.appendingPathComponent(file), to: destURL)
        }
    }
    
    @IBAction func installFonts(_ sender: Any) {
        guard isHPConnectivityKitInstalled() else { return }
        
        let destURL = URL(fileURLWithPath: NSString(string: "~/Documents/HP Connectivity Kit/Content").expandingTildeInPath)
        let srcURL = Bundle.main.bundleURL.appendingPathComponent("Contents/Developer/usr/hpprgm/fonts")
      
        for file in ["CGA.hpprgm", "EGA.hpprgm", "VGA.hpprgm", "BBC.hpprgm", "ARCADE.hpprgm", "HD44780.hpprgm"] {
            try? FileManager.default.copyItem(at: srcURL.appendingPathComponent(file), to: destURL)
        }
    }

    // MARK: - Action Handlers
    
    
    @objc func handleThemeSelection(_ sender: NSMenuItem) {
        guard let vc = NSApp.mainWindow?.contentViewController as? MainViewController else { return }
        
        guard let fileURL = sender.representedObject as? URL else { return }
        vc.codeEditorTextView.loadTheme(at: fileURL)
        
        for menuItem in mainMenu.item(withTitle: "Editor")?.submenu?.item(withTitle: "Theme")!.submenu!.items ?? [] {
            menuItem.state = .off
        }
        sender.state = .on
    }
    
}
