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


final class SettingsViewController: NSViewController {
    
    
    @IBOutlet weak var librarySearchPath: NSTextField!
    @IBOutlet weak var headerSearchPath: NSTextField!
    @IBOutlet weak var macOS: NSButton!
    @IBOutlet weak var Wine: NSButton!
    @IBOutlet weak var compressHPPRGM: NSButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
    
        librarySearchPath.stringValue = AppSettings.librarySearchPath
        headerSearchPath.stringValue = AppSettings.headerSearchPath
        
        validateUserInterfaceItems()
            
        if AppSettings.HPPrime == "macOS" {
            macOS.state = .on
            Wine.state = .off
        } else {
            macOS.state = .off
            Wine.state = .on
        }
        
        compressHPPRGM.state = AppSettings.compressHPPRGM ? .on : .off
    }
    
    @IBAction func defaultHeaderSearchPath(_ sender: Any) {
        headerSearchPath.stringValue = AppSettings.defaultHeaderSearchPath
    }
    
    @IBAction func defaultLibarySearchPath(_ sender: Any) {
        librarySearchPath.stringValue = AppSettings.defaultLibarySearchPath
    }
    
    @IBAction func close(_ sender: Any) {
        AppSettings.librarySearchPath = librarySearchPath.stringValue
        AppSettings.headerSearchPath = headerSearchPath.stringValue
        self.view.window?.close()
    }

    @IBAction func cancel(_ sender: Any) {
        self.view.window?.close()
    }
    
    
    @IBAction func HPPrime(_ sender: NSButton) {
        AppSettings.HPPrime = sender.title
    }
    
    @IBAction func compressHPPRGM(_ sender: NSButton) {
        AppSettings.compressHPPRGM = sender.state == .on
    }
    
    private func validateUserInterfaceItems() {
        let fileManager = FileManager.default
        if !fileManager.fileExists(atPath: "/Applications/Wine.app/Contents/MacOS/wine") {
            macOS.isEnabled = false
            Wine.isEnabled = false
        }
    }
}
