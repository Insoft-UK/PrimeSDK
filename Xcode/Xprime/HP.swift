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

fileprivate func loadHPProgramFile(_ url: URL) -> String? {
    let contents = CommandLineTool.execute("/Applications/HP/PrimeSDK/bin/hpprgm", arguments: [url.path, "-o", "/dev/stdout"])
    if let out = contents.out, !out.isEmpty {
        return contents.out
    }
    return nil
}

final class HP {
    static func isProgramFile(_ url: URL?) -> Bool {
        guard let url else { return false }
        let ext = url.pathExtension.lowercased()
        return ext == "prgm" || ext == "ppl"
    }
    
    static func isProgramPlusFile(_ url: URL?) -> Bool {
        guard let url else { return false }
        let ext = url.pathExtension.lowercased()
        return ext == "prgm+" || ext == "ppl+"
    }
    
    static func applicationDirectoryExists(atPath path: String, named name: String) -> Bool {
        var isDir: ObjCBool = false
        return FileManager.default.fileExists(atPath: "\(path)/\(name).hpappdir", isDirectory: &isDir) && isDir.boolValue
    }
    
    static func createApplicationDirectory(at url: URL, named name: String) throws {
        if applicationDirectoryExists(atPath: url.path, named: name) { return }
        
        try FileManager.default.createDirectory(
            at: url.appendingPathComponent(name).appendingPathExtension("hpappdir"),
            withIntermediateDirectories: true,
            attributes: nil
        )
        
        let folderURL = url.appendingPathComponent(name).appendingPathExtension("hpappdir")
        
        try FileManager.default.copyItem(
            at: Bundle.main.bundleURL.appendingPathComponent("Contents/Resources/template.hpapp"),
            to: folderURL.appendingPathComponent("\(name).hpapp")
        )
        
        try FileManager.default.copyItem(
            at: url.appendingPathComponent("\(name).hpprgm"),
            to: folderURL.appendingPathComponent("\(name).hpappprgm")
        )
        
        if FileManager.default.fileExists(atPath: url.appendingPathComponent("icon.png").path) {
            try FileManager.default.copyItem(
                at: url.appendingPathComponent("icon.png"),
                to: folderURL.appendingPathComponent("icon.png")
            )
        } else {
            try FileManager.default.copyItem(
                at: Bundle.main.bundleURL.appendingPathComponent("Contents/Resources/icon.png"),
                to: folderURL.appendingPathComponent("icon.png")
            )
        }
    }
    
    static func removeApplicationDirectory(at url: URL, named name: String) throws {
        try FileManager.default.removeItem(at: url.appendingPathComponent("\(name).hpappdir"))
    }
    
    static func removeArchive(at url: URL, named name: String) throws {
        try FileManager.default.removeItem(at: url.appendingPathComponent("\(name).hpappdir.zip"))
    }
    
    static func archiveApplicationDirectory(at url: URL, named name: String) -> (out: String?, err: String?)  {
        try? removeArchive(at: url, named: name)
        
        return CommandLineTool.execute(
            "/usr/bin/zip",
            arguments: [
                "-r",
                "\(name).hpappdir.zip",
                "\(name).hpappdir",
                "-x", "*.DS_Store"
            ],
            currentDirectory: url
        )
    }
    
    static func compressProgramFile(at url: URL) -> (out: String?, err: String?) {
        let tempURL = url.deletingLastPathComponent()
                         .appending(component: "~" + url.lastPathComponent)
       
        let _ = CommandLineTool.execute("/Applications/HP/PrimeSDK/bin/pplmin", arguments: [url.path, "-o", tempURL.path])
        
        if FileManager.default.fileExists(
            atPath: tempURL.path
        ) {
            let contents = CommandLineTool.execute(
                "/Applications/HP/PrimeSDK/bin/hpprgm",
                arguments: [
                    tempURL.path, "-o",
                    url.deletingPathExtension().appendingPathExtension("hpprgm").path
                ]
            )
            
            try? FileManager.default.removeItem(
                at: tempURL
            )
            return contents
        }
        
        return (nil, "Failed to find compressed program file.")
    }
    
    static func loadFile(at url: URL) -> String? {
        
        if url.pathExtension.lowercased() == "hpprgm" || url.pathExtension.lowercased() == "hpappprgm" {
            return loadHPProgramFile(url)
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
    
    static func saveFile(at url: URL, _ prgm: String) throws {
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
    
    static func installProgramFile(at url: URL) throws {
        let homeDirectory = FileManager.default.homeDirectoryForCurrentUser
        
        let source = url.deletingPathExtension().appendingPathExtension("hpprgm")
        let destination = homeDirectory.appendingPathComponent("Documents/HP Prime/Calculators/Prime").appendingPathComponent(source.lastPathComponent)
        
        do {
            if FileManager.default.fileExists(atPath: destination.path) {
                try FileManager.default.removeItem(at: destination)
            }

            try FileManager.default.copyItem(at: source, to: destination)
        }
    }
    
    static func isVirtualCalculatorRunning() -> Bool {
        if AppSettings.HPPrime != "macOS" {
            let wineApps = NSRunningApplication.runningApplications(withBundleIdentifier: "org.winehq.wine")
            return !wineApps.isEmpty
        }
        let running = NSRunningApplication.runningApplications(withBundleIdentifier: "HP Prime")
        return !running.isEmpty
    }
    
    static func launchVirtualCalculator() {
        if HP.isVirtualCalculatorRunning() {
            return
        }
        
        let homeDirectory = FileManager.default.homeDirectoryForCurrentUser
        let process = Process()
        
        process.executableURL = URL(fileURLWithPath: "/Applications/HP Prime.app/Contents/MacOS/HP Prime")
        
        if AppSettings.HPPrime != "macOS" {
            process.executableURL = URL(fileURLWithPath: "/Applications/Wine.app/Contents/MacOS/wine")
            process.arguments = [homeDirectory.appendingPathComponent(".wine/drive_c/Program Files/HP/HP Prime Virtual Calculator/HPPrime.exe").path]
        }
        
        do {
            try process.run()
        } catch {
            let alert = NSAlert()
            alert.messageText = "Failed"
            alert.informativeText = "Failed to launch: \(error)"
            alert.runModal()
            return
        }
    }

}
