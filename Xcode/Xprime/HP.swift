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

fileprivate func launchApplication(named appName: String, arguments: [String] = []) {
    switch launchApp(named: appName, arguments: arguments) {
    case .success:
        return
    case .failure(let error):
        let alert = NSAlert()
        alert.messageText = "Launch Failed"
        
        switch error {
        case .notFound:
            alert.informativeText = "The app was not found: \(appName)"
        case .invalidPath:
            alert.informativeText = "Invalid app path: \(appName)"
        case .launchFailed(let err):
            alert.informativeText = "Failed to launch: \(err.localizedDescription)"
        }
        
        alert.runModal()
        return
    }
}

//        guard let enumerator = FileManager.default.enumerator(atPath: parentURL.path) else { return }
//        for case let fileURL as URL in enumerator where fileURL.pathExtension == "hpprgm" {
//            let fileName = fileURL.deletingPathExtension().lastPathComponent
//            if fileName == name {
//                continue
//            }
//
//        }

final class HP {
    static func hpPrimeCalculatorExists(named name: String) -> Bool {
        guard !name.isEmpty else { return false }
        
        let calculatorURL = FileManager.default
            .homeDirectoryForCurrentUser
            .appendingPathComponent("Documents/HP Connectivity Kit/Calculators")
            .appendingPathComponent(name)
        
        print(calculatorURL.isDirectory)
        return calculatorURL.isDirectory
    }
    
    static var isVirtualCalculatorInstalled: Bool {
        if AppSettings.HPPrime == "macOS" {
            return FileManager.default.fileExists(atPath: "/Applications/HP Prime.app/Contents/MacOS/HP Prime")
        }
        
        let homeDirectory = FileManager.default.homeDirectoryForCurrentUser
        return FileManager.default.fileExists(atPath: homeDirectory.appendingPathComponent(".wine/drive_c/Program Files/HP/HP Prime Virtual Calculator/HPPrime.exe").path)
    }
    
    static var isConnectivityKitInstalled: Bool {
        if AppSettings.HPPrime == "macOS" {
            return FileManager.default.fileExists(atPath: "/Applications/HP Connectivity Kit.app/Contents/MacOS/HP Connectivity Kit")
        }
        let homeDirectory = FileManager.default.homeDirectoryForCurrentUser
        return FileManager.default.fileExists(atPath: homeDirectory.appendingPathComponent(".wine/drive_c/Program Files/HP/HP Connectivity Kit/ConnectivityKit.exe").path)
    }
    
    static func hpPrgmIsInstalled(named name: String, forUser user: String? = nil) -> Bool {
        let homeURL = FileManager.default.homeDirectoryForCurrentUser

        // Determine base folder
        let baseURL: URL
        if let user = user, hpPrimeCalculatorExists(named: user) {
            baseURL = homeURL
                .appendingPathComponent("Documents/HP Connectivity Kit/Calculators")
                .appendingPathComponent(user)
        } else {
            baseURL = homeURL
                .appendingPathComponent("Documents/HP Prime/Calculators/Prime")
        }
        
        return hpPrgmExists(atPath: baseURL.path, named: name)
    }
    
    static func hpAppDirectoryIsInstalled(named name: String, forUser user: String? = nil) -> Bool {
        let baseURL: URL
        if let user = user, hpPrimeCalculatorExists(named: user) {
            baseURL = FileManager.default.homeDirectoryForCurrentUser
                .appendingPathComponent("Documents/HP Connectivity Kit/Calculators")
                .appendingPathComponent(user)
        } else {
            baseURL = FileManager.default.homeDirectoryForCurrentUser
                .appendingPathComponent("Documents/HP Prime/Calculators/Prime")
        }

        let appDirURL = baseURL.appendingPathComponent(name)
        return appDirURL.isDirectory
    }
    
    static func prgmExists(atPath path: String, named name: String) -> Bool {
        let programURL = URL(fileURLWithPath: path)
            .appendingPathComponent(name)
            .appendingPathExtension("prgm")
        return FileManager.default.fileExists(atPath: programURL.path)
    }
    
    static func prgmPlusExists(atPath path: String, named name: String) -> Bool {
        let programURL = URL(fileURLWithPath: path)
            .appendingPathComponent(name)
            .appendingPathExtension("prgm+")
        return FileManager.default.fileExists(atPath: programURL.path)
    }
    
    static func hpPrgmExists(atPath path: String, named name: String) -> Bool {
        let programURL = URL(fileURLWithPath: path)
            .appendingPathComponent(name)
            .appendingPathExtension("hpprgm")
        return FileManager.default.fileExists(atPath: programURL.path)
    }
    
    static func hpAppDirExists(atPath path: String, named name: String) -> Bool {
        let dirURL = URL(fileURLWithPath: path).appendingPathComponent("\(name).hpappdir")
        var isDir: ObjCBool = false
        return FileManager.default.fileExists(atPath: dirURL.path, isDirectory: &isDir) && isDir.boolValue
    }
    
    static func hpAppDirIsComplete(atPath path: String, named name: String) -> Bool {
        guard hpAppDirExists(atPath: path, named: name) else {
            return false
        }
        
        let appDirURL = URL(fileURLWithPath: path)
            .appendingPathComponent(name)
            .appendingPathExtension("hpappdir")
        
        let files: [URL] = [
            appDirURL.appendingPathComponent("\(name).hpapp"),
            appDirURL.appendingPathComponent("\(name).hpappprgm")
        ]
        for file in files {
            if !FileManager.default.fileExists(atPath: file.path) {
                return false
            }
        }
        
        return true
    }
    
    static func createHPAppDirectory(at url: URL, named name: String) throws {
        let directoryURL = url
            .appendingPathComponent(name)
            .appendingPathExtension("hpappdir")
        
        if directoryURL.isDirectory {
            return
        }
        
        try FileManager.default.createDirectory(
            at: directoryURL,
            withIntermediateDirectories: true,
            attributes: nil
        )
        
        try FileManager.default.copyItem(
            at: Bundle.main.bundleURL.appendingPathComponent("Contents/Resources/template.hpapp"),
            to: directoryURL.appendingPathComponent("\(name).hpapp")
        )
        
        try FileManager.default.copyItem(
            at: url.appendingPathComponent("\(name).hpprgm"),
            to: directoryURL.appendingPathComponent("\(name).hpappprgm")
        )
        
        do {
            try FileManager.default.moveItem(
                at: url.appendingPathComponent("icon.png"),
                to: directoryURL.appendingPathComponent("icon.png")
            )
        } catch {
            try FileManager.default.copyItem(
                at: Bundle.main.bundleURL.appendingPathComponent("Contents/Resources/icon.png"),
                to: directoryURL.appendingPathComponent("icon.png")
            )
        }
    }
    
    static func archiveHPAppDirectory(at url: URL, named name: String, to desctinationURL: URL? = nil) -> (out: String?, err: String?)  {
        var destinationPath = "\(name).hpappdir.zip"
        
        if let desctinationURL = desctinationURL {
            try? FileManager.default.removeItem(at: desctinationURL)
            destinationPath = desctinationURL.path
        } else {
            try? FileManager.default.removeItem(at: url.appendingPathComponent("\(name).hpappdir.zip"))
        }
        
        return CommandLineTool.execute(
            "/usr/bin/zip",
            arguments: [
                "-r",
                destinationPath,
                "\(name).hpappdir",
                "-x", "*.DS_Store"
            ],
            currentDirectory: url
        )
    }
    
    static func createCompressedHPPrgm(at url: URL) -> (out: String?, err: String?) {
        guard FileManager.default.fileExists(atPath: url.path) else {
            return (nil, "")
        }
       
        // Create temp file next to the original
        let tempURL = url.deletingLastPathComponent()
            .appending(component: "~" + url.lastPathComponent)
        
        // Step 1: Minify PPL
        _ = CommandLineTool.execute(
            "/Applications/HP/PrimeSDK/bin/pplmin",
            arguments: [url.path, "-o", tempURL.path]
        )
        
        // Ensure temp file exists
        guard FileManager.default.fileExists(atPath: tempURL.path) else {
            return (nil, "Failed to create intermediate minified program.")
        }
        
        // Step 2: Compile .hpprgm
        let programURL: URL
        programURL = url
            .deletingPathExtension()
            .appendingPathExtension("hpprgm")
        let result = CommandLineTool.execute(
            "/Applications/HP/PrimeSDK/bin/hpprgm",
            arguments: [tempURL.path, "-o", programURL.path]
        )
        
        // Clean up temp file
        try? FileManager.default.removeItem(at: tempURL)
        
        return result
    }
    
    static func createHPPrgm(at url: URL) -> (out: String?, err: String?) {
        guard FileManager.default.fileExists(atPath: url.path) else {
            return (nil, "")
        }
        
        let programURL: URL
        programURL = url
            .deletingPathExtension()
            .appendingPathExtension("hpprgm")
    
        let result = CommandLineTool.execute(
            "/Applications/HP/PrimeSDK/bin/hpprgm",
            arguments: [url.path, "-o", programURL.path]
        )
        
        return result
    }
    
    static func loadHPPrgm(at url: URL) -> String? {
        
        if url.pathExtension.lowercased() == "hpprgm" || url.pathExtension.lowercased() == "hpappprgm" {
            let contents = CommandLineTool.execute("/Applications/HP/PrimeSDK/bin/hpprgm", arguments: [url.path, "-o", "/dev/stdout"])
            if let out = contents.out, !out.isEmpty {
                return contents.out
            }
            return nil
        }
        
        return loadTextFile(at: url)
    }
    
    static func saveFile(at url: URL, content prgm: String) throws {
        let encoding: String.Encoding = url.pathExtension.lowercased() == "prgm" ? .utf16LittleEndian : .utf8
        
        if encoding == .utf8 {
            try prgm.write(to: url, atomically: true, encoding: .utf8)
        } else {
            // UTF-16 LE with BOM
            guard let body = prgm.data(using: .utf16LittleEndian) else {
                throw NSError(domain: "HPFileSaveError", code: 1, userInfo: [NSLocalizedDescriptionKey: "Unable to encode string as UTF-16LE"])
            }
            var data = Data([0xFF, 0xFE]) // BOM
            data.append(body)
            try data.write(to: url, options: .atomic)
        }
    }
    
    static func installHPPrgm(at programURL: URL, forUser user: String? = nil) throws {
        let homeURL = FileManager.default.homeDirectoryForCurrentUser
        
        // Determine destination folder
        let destinationURL: URL
        
        if let user = user, hpPrimeCalculatorExists(named: user) {
            destinationURL = homeURL
                .appendingPathComponent("Documents/HP Connectivity Kit/Calculators")
                .appendingPathComponent(user)
                .appendingPathComponent(programURL.lastPathComponent)
        } else {
            destinationURL = homeURL
                .appendingPathComponent("Documents/HP Prime/Calculators/Prime")
                .appendingPathComponent(programURL.lastPathComponent)
        }

        do {
            if FileManager.default.fileExists(atPath: destinationURL.path) {
                try FileManager.default.removeItem(at: destinationURL)
            }

            try FileManager.default.copyItem(at: programURL, to: destinationURL)
        }
    }
    
    static func installHPAppDirectory(at appURL: URL, forUser user: String? = nil) throws {
        guard appURL.isDirectory else {
            return
        }
        let homeURL = FileManager.default.homeDirectoryForCurrentUser
        
        // Determine destination folder
        let destinationURL: URL
        
        if let user = user, hpPrimeCalculatorExists(named: user) {
            destinationURL = homeURL
                .appendingPathComponent("Documents/HP Connectivity Kit/Calculators")
                .appendingPathComponent(user)
                .appendingPathComponent(appURL.lastPathComponent)
        } else {
            destinationURL = homeURL
                .appendingPathComponent("Documents/HP Prime/Calculators/Prime")
                .appendingPathComponent(appURL.lastPathComponent)
        }
        

        do {
            if FileManager.default.fileExists(atPath: destinationURL.path) {
                try FileManager.default.removeItem(at: destinationURL)
            }

            try FileManager.default.copyItem(at: appURL, to: destinationURL)
        }
    }
    
    
    
    static func launchVirtualCalculator() {
        let appName = "HP Prime"
        
        if AppSettings.HPPrime == "macOS" {
            if let targetBundleIdentifier = getBundleIdentifier(forApp: appName) {
                terminateApp(withBundleIdentifier: targetBundleIdentifier)
            }
            
            launchApplication(named: appName + ".app")
            return
        }
        
        if isProcessRunning("HP Prime Virtual Calculator") {
            killProcess(named: "HP Prime Virtual Calculator")
        }
        
        launchApplication(
            named: "/Applications/Wine.app/Contents/MacOS/wine",
            arguments: [
                FileManager
                    .default
                    .homeDirectoryForCurrentUser
                    .appendingPathComponent(".wine/drive_c/Program Files/HP/HP Prime Virtual Calculator/HPPrime.exe")
                    .path
            ])
    }
    
    static func launchConnectivityKit() {
        let appName = "HP Connectivity Kit"
        
        if AppSettings.HPPrime == "macOS" {
            launchApplication(named: appName + ".app")
            return
        }
        
        launchApplication(
            named: "/Applications/Wine.app/Contents/MacOS/wine",
            arguments: [
                FileManager
                    .default
                    .homeDirectoryForCurrentUser
                    .appendingPathComponent(".wine/drive_c/Program Files/HP/HP Connectivity Kit/ConnectivityKit.exe")
                    .path
            ])
    }
}
