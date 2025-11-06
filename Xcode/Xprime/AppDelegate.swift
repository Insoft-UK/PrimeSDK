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
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Insert code here to initialize your application
        NSApp.appearance = NSAppearance(named: .darkAqua)
        
        UserDefaults.standard.set(false, forKey: "NSAutomaticPeriodSubstitutionEnabled")
        UserDefaults.standard.set(false, forKey: "NSAutomaticTextReplacementEnabled")
        UserDefaults.standard.set(false, forKey: "NSAutomaticQuoteSubstitutionEnabled")
        UserDefaults.standard.set(false, forKey: "NSAutomaticDashSubstitutionEnabled")
        UserDefaults.standard.synchronize()
        
        
        populateThemesMenu(menu: mainMenu)
        populateGrammarMenu(menu: mainMenu)
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
            if filename == AppSettings.selectedTheme {
                menuItem.state = .on
            }

            menu.item(withTitle: "Editor")?.submenu?.item(withTitle: "Theme")?.submenu?.addItem(menuItem)
        }
    }
    
    private func populateGrammarMenu(menu: NSMenu) {
        guard let resourceURLs = Bundle.main.urls(forResourcesWithExtension: "xpgrammar", subdirectory: nil) else {
            print("⚠️ No .xpgrammar files found.")
            return
        }
        


        
        
        for fileURL in resourceURLs {
            let name = fileURL.deletingPathExtension().lastPathComponent
            
            let menuItem = NSMenuItem(title: name, action: #selector(handleGrammarSelection(_:)), keyEquivalent: "")
            menuItem.representedObject = fileURL
            menuItem.target = self  // or another target if needed
            if name == AppSettings.selectedGrammar {
                menuItem.state = .on
            }

            menu.item(withTitle: "Editor")?.submenu?.item(withTitle: "Grammar")?.submenu?.addItem(menuItem)
        }
    }
    
    // MARK: - Interface Builder Action Handlers
    
    private func isHPConnectivityKitInstalled() -> Bool {
        let fileManager = FileManager.default
        return fileManager.fileExists(atPath: "/Applications/HP Connectivity Kit.app/Contents/MacOS/HP Connectivity Kit")
    }
    
    private func isHPPrimeVirtualCalculatorInstalled() -> Bool {
        let fileManager = FileManager.default
        return fileManager.fileExists(atPath: "/Applications/HP Prime.app/Contents/MacOS/HP Prime")
    }
    
    
    @IBAction func launchHPConnectiveKit(_ sender: Any) {
        let task = Process()
        
        task.executableURL = URL(fileURLWithPath: "/Applications/HP Connectivity Kit.app/Contents/MacOS/HP Connectivity Kit")
        
        do {
            try task.run()
        } catch {
            print("Failed to launch: \(error)")
        }
    }
    
    @IBAction func launchHPPrimeVirtualCalculator(_ sender: Any) {
        let fileManager = FileManager.default
        let homeDir = fileManager.homeDirectoryForCurrentUser
        let task = Process()
        
        if AppSettings.HPPrime == "macOS" {
            task.executableURL = URL(fileURLWithPath: "/Applications/HP Prime.app/Contents/MacOS/HP Prime")
        } else {
            task.executableURL = URL(fileURLWithPath: "/Applications/Wine.app/Contents/MacOS/wine")
            task.arguments = [homeDir.appendingPathComponent(".wine/drive_c/Program Files/HP/HP Prime Virtual Calculator/HPPrime.exe").path]
        }
        
        do {
            try task.run()
        } catch {
            print("Failed to launch: \(error)")
        }
    }
    
    @IBAction func installLibraries(_ sender: Any) {
        guard isHPConnectivityKitInstalled() else { return }
        
        let homeDir = FileManager.default.homeDirectoryForCurrentUser
        
        let destURL = homeDir.appendingPathComponent("Documents/HP Connectivity Kit/Content")
        let srcURL = URL(fileURLWithPath: "/Applications/HP/PrimeSDK/hpprgm")
        
        for file in ["ColorSpace.hpprgm", "HP.hpprgm", "GROB.hpprgm"] {
            try? FileManager.default.copyItem(at: srcURL.appendingPathComponent(file), to: destURL.appendingPathComponent(file))
        }
    }
    
    @IBAction func installFonts(_ sender: Any) {
        guard isHPConnectivityKitInstalled() else { return }
        
        let homeDir = FileManager.default.homeDirectoryForCurrentUser
        
        let destURL = homeDir.appendingPathComponent("Documents/HP Connectivity Kit/Content")
        let srcURL = URL(fileURLWithPath: "/Applications/HP/PrimeSDK/hpprgm/fonts")
        
      
        for file in ["CGA.hpprgm", "EGA.hpprgm", "VGA.hpprgm", "BBC.hpprgm", "ARCADE.hpprgm", "HD44780.hpprgm"] {
            try? FileManager.default.copyItem(at: srcURL.appendingPathComponent(file), to: destURL.appendingPathComponent(file))
        }
    }

    // MARK: - Action Handlers
    
    @objc func handleThemeSelection(_ sender: NSMenuItem) {
        guard let vc = NSApp.mainWindow?.contentViewController as? MainViewController else { return }
        
        guard let fileURL = sender.representedObject as? URL else { return }
        vc.codeEditorTextView.loadTheme(at: fileURL)
        AppSettings.selectedTheme = sender.title
        
        for menuItem in mainMenu.item(withTitle: "Editor")?.submenu?.item(withTitle: "Theme")!.submenu!.items ?? [] {
            menuItem.state = .off
        }
        sender.state = .on
    }
    
    @objc func handleGrammarSelection(_ sender: NSMenuItem) {
        guard let vc = NSApp.mainWindow?.contentViewController as? MainViewController else { return }
        
        guard let fileURL = sender.representedObject as? URL else { return }
        vc.codeEditorTextView.loadGrammar(at: fileURL)
        AppSettings.selectedGrammar = sender.title
        
        for menuItem in mainMenu.item(withTitle: "Editor")?.submenu?.item(withTitle: "Grammar")!.submenu!.items ?? [] {
            menuItem.state = .off
        }
        sender.state = .on
    }
    
    
    internal func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        switch menuItem.action {
        case #selector(launchHPConnectiveKit(_:)):
            return isHPConnectivityKitInstalled()
            
        case #selector(launchHPPrimeVirtualCalculator(_:)):
            return isHPPrimeVirtualCalculatorInstalled()
            
        default:
            break
        }
        return true
    }
}
