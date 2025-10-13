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
    private let bundleURL = Bundle.main.bundleURL
    
    private var window: NSWindow?
    
    @IBOutlet weak var libPath: NSTextField!
    @IBOutlet weak var includePath: NSTextField!
    @IBOutlet weak var useLibButton: NSButton!
    @IBOutlet weak var useIncludeButton: NSButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.window?.styleMask.remove([.resizable, .miniaturizable, .fullScreen])
        
        if let window = NSApplication.shared.windows.first {
            self.window = window
        }
        
        useLibButton.state = AppSettings.useLib ? .on : .off
        useIncludeButton.state = AppSettings.useInclude ? .on: .off
        libPath.stringValue = AppSettings.libPath
        includePath.stringValue = AppSettings.includePath
    }
    

    
    @IBAction func resetInclude(_ sender: Any) {
        includePath.stringValue = bundleURL.appendingPathComponent("Contents/Developer/usr/include").path
        useIncludeButton.state = .on
    }
    
    @IBAction func resetLib(_ sender: Any) {
        libPath.stringValue = bundleURL.appendingPathComponent("Contents/Developer/usr/lib").path
        useLibButton.state = .on
    }
    
    @IBAction func okButton(_ sender: Any) {
        AppSettings.useLib = useLibButton.state == .on
        AppSettings.useInclude = useIncludeButton.state == .on
        AppSettings.libPath = libPath.stringValue
        AppSettings.includePath = includePath.stringValue
        self.view.window?.performClose(sender)
    }

    @IBAction func cancelButton(_ sender: Any) {
        self.view.window?.performClose(sender)
    }
    
//    func close(_ sender: Any) {
//        if let window = self.view.window, let parent = window.sheetParent {
//            parent.endSheet(window, returnCode: .OK) // or .cancel
//        } else {
//            self.view.window?.performClose(sender)
//        }
//    }
    
}
