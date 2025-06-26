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
        
//        guard let mainMenu = NSApp.mainMenu else { return }
//        
        
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
    
    @IBAction func embedImage(_ sender: Any) {
        if let vc = NSApp.mainWindow?.contentViewController as? ViewController {
            vc.embedImage()
        }
    }
    
    @IBAction func insertCode(_ sender: Any) {
        if let vc = NSApp.mainWindow?.contentViewController as? ViewController {
            vc.insertCode()
        }
    }
    
    @IBAction func insertTemplate(_ sender: Any) {
        func loadTextFile(at url: URL) -> String? {
            func detectEncoding(of url: URL) -> String.Encoding? {
                do {
                    let data = try Data(contentsOf: url)
                    
                    if data.starts(with: [0xEF, 0xBB, 0xBF]) {
                        return .utf8
                    }
                    if data.starts(with: [0xFF, 0xFE]) {
                        return .utf16LittleEndian
                    }
                    if data.starts(with: [0xFE, 0xFF]) {
                        return .utf16BigEndian
                    }
                    
                    // No BOM — assume UTF-8 as default fallback
                    return .utf8
                } catch {
                    return nil
                }
            }
            
            guard let encoding = detectEncoding(of: url) else { return nil }
            
            do {
                let data = try Data(contentsOf: url)
                let string = String(data: data, encoding: encoding) ?? ""
                
                return string
            } catch {
            }
            
            return nil
        }
        
        if let vc = NSApp.mainWindow?.contentViewController as? ViewController {
            if let menuItem = sender as? NSMenuItem {
                let url = Bundle.main.bundleURL.appendingPathComponent("Contents/Template/\(traceMenuItem(menuItem))/\(menuItem.title).prgm")
                
                if let contents = loadTextFile(at: url) {
                    vc.registerTextViewUndo(actionName: "Insert Template")
                    vc.insertText(contents)
                }
            }
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
}

