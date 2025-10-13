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

import Foundation
import AppKit

struct AppSettings {
    private static let defaults = UserDefaults.standard
    private static let bundleURL = Bundle.main.bundleURL

    private enum Key: String {
        case libPath
        case includePath
        case binaryURL
        case useLib
        case useInclude
    }


    static var libPath: String {
        get { defaults.object(forKey: Key.libPath.rawValue) as? String ?? bundleURL.appendingPathComponent("Contents/Developer/usr/lib").path }
        set { defaults.set(newValue, forKey: Key.libPath.rawValue) }
    }
    
    static var includePath: String {
        get { defaults.object(forKey: Key.includePath.rawValue) as? String ?? bundleURL.appendingPathComponent("Contents/Developer/usr/include").path }
        set { defaults.set(newValue, forKey: Key.includePath.rawValue) }
    }
    
    static var binaryURL: URL {
        get { defaults.object(forKey: Key.binaryURL.rawValue) as? URL ?? bundleURL.appendingPathComponent("Contents/Developer/usr/bin") }
        set { defaults.set(newValue, forKey: Key.binaryURL.rawValue) }
    }
    
    static var useLib: Bool {
        get { defaults.object(forKey: Key.useLib.rawValue) as? Bool ?? true }
        set { defaults.set(newValue, forKey: Key.useLib.rawValue) }
    }
    
    static var useInclude: Bool {
        get { defaults.object(forKey: Key.useInclude.rawValue) as? Bool ?? true }
        set { defaults.set(newValue, forKey: Key.useInclude.rawValue) }
    }
    
    static func resetLib() {
        defaults.set(bundleURL.appendingPathComponent("Contents/Developer/usr/lib").path, forKey: Key.libPath.rawValue)
        defaults.set(true, forKey: Key.useLib.rawValue)
    }
    
    static func resetInclude() {
        defaults.set(bundleURL.appendingPathComponent("Contents/Developer/usr/include").path, forKey: Key.includePath.rawValue)
        defaults.set(true, forKey: Key.useInclude.rawValue)
    }
}

extension NSColor {
    static func fromHex(_ hex: String) -> NSColor {
        var hexSanitized = hex.trimmingCharacters(in: .whitespacesAndNewlines).uppercased()
        if hexSanitized.hasPrefix("#") { hexSanitized.removeFirst() }

        var rgb: UInt64 = 0
        Scanner(string: hexSanitized).scanHexInt64(&rgb)

        let r, g, b: CGFloat
        switch hexSanitized.count {
        case 6:
            r = CGFloat((rgb & 0xFF0000) >> 16) / 255.0
            g = CGFloat((rgb & 0x00FF00) >> 8) / 255.0
            b = CGFloat(rgb & 0x0000FF) / 255.0
            return NSColor(calibratedRed: r, green: g, blue: b, alpha: 1.0)
        default:
            return NSColor(calibratedWhite: 0.125, alpha: 1.0)
        }
    }

    var hexString: String {
        guard let rgb = usingColorSpace(.deviceRGB) else { return "#202020" }
        let r = Int(rgb.redComponent * 255.0)
        let g = Int(rgb.greenComponent * 255.0)
        let b = Int(rgb.blueComponent * 255.0)
        return String(format: "#%02X%02X%02X", r, g, b)
    }
}
