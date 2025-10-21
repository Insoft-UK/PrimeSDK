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

fileprivate func run(_ task: Process) -> String? {
    let pipe = Pipe()
    task.standardOutput = pipe
    
    do {
        try task.run()
    } catch {
        return nil
    }
    task.waitUntilExit()
    
    // Read all data from the pipe and convert to String
    let data = pipe.fileHandleForReading.readDataToEndOfFile()
    guard !data.isEmpty else { return nil }
    return String(data: data, encoding: .utf8)
}

class CommandLineTool {
    class func execute(_ command: String, arguments: [String]) -> String? {
        let task = Process()
        
        task.executableURL = Bundle.main.bundleURL.appendingPathComponent("Contents/Developer/usr/bin").appendingPathComponent(command)
        task.arguments = arguments
        
        return run(task)
    }
    
    class func `ppl+`(i infile: URL, o outfile: URL? = nil) -> String? {
        
        let process = Process()
        process.executableURL = AppPreferences.binURL.appendingPathComponent("ppl+")
        process.arguments = [infile.path]
        
        if AppPreferences.useLib {
            process.arguments?.append("-L\(AppPreferences.libPath)")
        }
        
        if AppPreferences.useInclude {
            process.arguments?.append("-L\(AppPreferences.includePath)")
        }
        
        if let _ = outfile {
            process.arguments?.append("-o")
            process.arguments?.append(outfile!.path)
        }
        
        return run(process)
    }
}

