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

final class PreferencesWindowController: NSWindowController {
    
    
    override func windowDidLoad() {
        super.windowDidLoad()
        applyPersistedWindowPreferences()
    }
    
    private func applyPersistedWindowPreferences() {
        guard let window = window else { return }
        
        // Style mask based on persisted preference
        if true {
            window.styleMask = [.borderless, .hudWindow]
        } else {
            window.styleMask = [.titled, .closable, .miniaturizable]
        }
        
        // Keep-on-top preference
        window.level = .floating // Keeps the window above other windows if needed
        
        // Other window flags
        window.isOpaque = true
        window.hasShadow = true
        
        // Background color
        window.backgroundColor = NSColor(white: 0.125, alpha: 1.0)
        
        // Prevent user resizing by locking min/max size to current frame and content size
        let currentSize = window.frame.size
        window.minSize = currentSize
        window.maxSize = currentSize
        
        // Disable UI affordances for resizing/zooming
        window.standardWindowButton(.zoomButton)?.isEnabled = false
      
        window.collectionBehavior.remove(.fullScreenPrimary) // if you had it
        window.tabbingMode = .disallowed
        window.zoom(nil) // no-op if not zoomed, but keeps state sane
        
        // Lock content view to a fixed size to prevent Auto Layout-driven resizing
        if let contentView = window.contentView {
            contentView.translatesAutoresizingMaskIntoConstraints = false

            // Remove any prior size constraints you may have added before (optional if not applicable)
            NSLayoutConstraint.deactivate(contentView.constraints.filter {
                $0.firstAttribute == .width || $0.firstAttribute == .height
            })

            let widthConstraint = contentView.widthAnchor.constraint(equalToConstant: window.contentRect(forFrameRect: window.frame).size.width)
            let heightConstraint = contentView.heightAnchor.constraint(equalToConstant: window.contentRect(forFrameRect: window.frame).size.height)
            widthConstraint.priority = .required
            heightConstraint.priority = .required
            NSLayoutConstraint.activate([widthConstraint, heightConstraint])
        }
        
        let contentSize = window.contentRect(forFrameRect: window.frame).size
        window.contentMinSize = contentSize
        window.contentMaxSize = contentSize
        
        window.title = "Preferences"
    }
}

