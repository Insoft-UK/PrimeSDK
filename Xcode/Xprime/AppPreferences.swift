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

struct AppPreferences {
    
    private static let defaults = UserDefaults.standard
    private static let bundleURL = Bundle.main.bundleURL
    
    static let defaultHeaderSearchPath: String = "/Applications/HP/PrimeSDK/include"
    static let defaultLibarySearchPath: String = "/Applications/HP/PrimeSDK/lib"
    static let defaultSDKPath: String = "/Applications/HP/PrimeSDK"
    static let defaultHPPrime = "/Applications/HP Prime.app/Contents/MacOS/HP Prime"

    private enum Key: String {
        case librarySearchPath
        case headerSearchPath
        case selectedTheme
    }

    static var librarySearchPath: String {
        get { defaults.object(forKey: Key.librarySearchPath.rawValue) as? String ?? defaultHeaderSearchPath }
        set { defaults.set(newValue, forKey: Key.librarySearchPath.rawValue) }
    }
    
    static var headerSearchPath: String {
        get { defaults.object(forKey: Key.headerSearchPath.rawValue) as? String ?? defaultLibarySearchPath }
        set { defaults.set(newValue, forKey: Key.headerSearchPath.rawValue) }
    }
    
    static var selectedTheme: String {
        get { defaults.object(forKey: Key.selectedTheme.rawValue) as? String ?? "Default (Dark)" }
        set { defaults.set(newValue, forKey: Key.selectedTheme.rawValue) }
    }
}

