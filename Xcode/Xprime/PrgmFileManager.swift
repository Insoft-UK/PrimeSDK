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

fileprivate func encodingType(_ data: inout Data, _ encoding: inout String.Encoding) {
    // Detect and remove BOM if present
    if data.count >= 2 {
        let bomLE: [UInt8] = [0xFF, 0xFE]
        let bomBE: [UInt8] = [0xFE, 0xFF]
        let firstTwo = Array(data.prefix(2))
        if firstTwo == bomLE {
            data.removeFirst(2) // UTF-16 LE BOM
            encoding = .utf16LittleEndian
        } else if firstTwo == bomBE {
            data.removeFirst(2) // UTF-16 BE BOM
            encoding = .utf16BigEndian
        }
    }
}

fileprivate func loadHpprgm(_ url: URL) -> String? {
    let contents = CommandLineTool.execute("/Applications/HP/PrimeSDK/bin/hpprgm", arguments: [url.path, "-o", "/dev/stdout"])
    if let out = contents.out, !out.isEmpty {
        return contents.out
    }
    return nil
}

final class PrgmFileManager {
    
    
    static func load(_ url: URL) -> String? {
        
        if url.pathExtension.lowercased() == "hpprgm" || url.pathExtension.lowercased() == "hpappprgm" {
            return loadHpprgm(url)
        }
        
        var encoding: String.Encoding = .utf8
        
        do {
            // Read the raw file data
            var data = try Data(contentsOf: url)
            
            encodingType(&data, &encoding)
            
            // Decode text using the chosen encoding
            if let text = String(data: data, encoding: encoding) {
                return text
            } else {
                throw NSError(domain: "FileLoadError", code: -1, userInfo: [
                    NSLocalizedDescriptionKey: "Failed to decode file text."
                ])
            }
            
        } catch {
            let alert = NSAlert()
            alert.messageText = "Error"
            alert.informativeText = "Failed to open file: \(error)"
            alert.runModal()
            return nil
        }
    }
    
    static func save(_ url: URL, _ prgm: String) throws {
        let encoding: String.Encoding = url.pathExtension == "prgm" ? .utf16LittleEndian : .utf8
        
        if encoding == .utf8 {
            try prgm.write(to: url, atomically: true, encoding: encoding)
        } else {
            // UTF-16 LE with BOM (0xFF 0xFE)
            if let body = prgm.data(using: encoding) {
                var bom = Data([0xFF, 0xFE])
                bom.append(body)
                try bom.write(to: url, options: .atomic)
            }
        }
    }
    
    
}
