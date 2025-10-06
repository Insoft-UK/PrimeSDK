//
//  SettingsController.swift
//  Xprime
//
//  Created by Richie on 06/10/2025.
//

import Cocoa

final class SettingsWindowController: NSWindowController {
    
    
    override func windowDidLoad() {
        super.windowDidLoad()

        window?.styleMask = .borderless
        window?.styleMask.insert([.titled])
        window?.level = .floating // Keeps the window above other windows if needed
        window?.isOpaque = true
        window?.hasShadow = true
        window?.backgroundColor = NSColor(white: 0.125, alpha: 1.0)
    }
    
}
