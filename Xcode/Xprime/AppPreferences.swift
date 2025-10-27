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
    
    static let defaultIncludePath: String = "/Applications/HP/PrimeSDK/include"
    static let defaultLibaryPath: String = "/Applications/HP/PrimeSDK/lib"

    private enum Key: String {
        case libPath
        case includePath
        case useLib
        case useInclude
    }


    static var libPath: String {
        get { defaults.object(forKey: Key.libPath.rawValue) as? String ?? defaultLibaryPath }
        set { defaults.set(newValue, forKey: Key.libPath.rawValue) }
    }
    
    static var includePath: String {
        get { defaults.object(forKey: Key.includePath.rawValue) as? String ?? defaultIncludePath }
        set { defaults.set(newValue, forKey: Key.includePath.rawValue) }
    }
    
    static var useLib: Bool {
        get { defaults.object(forKey: Key.useLib.rawValue) as? Bool ?? true }
        set { defaults.set(newValue, forKey: Key.useLib.rawValue) }
    }
    
    static var useInclude: Bool {
        get { defaults.object(forKey: Key.useInclude.rawValue) as? Bool ?? true }
        set { defaults.set(newValue, forKey: Key.useInclude.rawValue) }
    }
}

