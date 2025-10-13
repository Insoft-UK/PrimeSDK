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

class PrimeSDK {
    class func `ppl+`(i infile: URL, o outfile: URL? = nil) {
        
        let process = Process()
        process.executableURL = AppSettings.binaryURL.appendingPathComponent("ppl+")
        process.arguments = [infile.path]
        
        if AppSettings.useLib {
            process.arguments?.append("-L\(AppSettings.libPath)")
        }
        
        if AppSettings.useInclude {
            process.arguments?.append("-L\(AppSettings.includePath)")
        }
        
        if let _ = outfile {
            process.arguments?.append("-o")
            process.arguments?.append(outfile!.path)
        }
        
        let pipe = Pipe()
        process.standardOutput = pipe
        
        try? process.run()
        process.waitUntilExit()
    }
    
    class func pplmin(i infile: URL, o outfile: URL) {
        let process = Process()
        process.executableURL = AppSettings.binaryURL.appendingPathComponent("pplmin")
        process.arguments = [infile.path, "-o", outfile.path]
        
        let pipe = Pipe()
        process.standardOutput = pipe
        
        try? process.run()
        process.waitUntilExit()
    }
    
    class func hpprgm(i infile: URL, o outfile: URL? = nil) {
        let process = Process()
        process.executableURL = AppSettings.binaryURL.appendingPathComponent("hpprgm")
        if let outfile = outfile {
            process.arguments = [infile.path, "-o", outfile.path]
        } else {
            process.arguments = [infile.path]
        }
        
        let pipe = Pipe()
        process.standardOutput = pipe
        
        try? process.run()
        process.waitUntilExit()
    }
    
    class func grob(i infile: URL, o outfile: URL) {
        let process = Process()
        process.executableURL = AppSettings.binaryURL.appendingPathComponent("grob")
        process.arguments = [infile.path, "-o", outfile.path]
        
        let pipe = Pipe()
        process.standardOutput = pipe
        
        try? process.run()
        process.waitUntilExit()
    }
    
    class func pplfont(i infile: URL, o outfile: URL) {
        let process = Process()
        process.executableURL = AppSettings.binaryURL.appendingPathComponent("pplfont")
        process.arguments = [infile.path, "-o", outfile.path, "--ppl"]
        
        let pipe = Pipe()
        process.standardOutput = pipe
        
        try? process.run()
        process.waitUntilExit()
    }
    
    class func pplref(i infile: URL, o outfile: URL? = nil) {
        let process = Process()
        process.executableURL = AppSettings.binaryURL.appendingPathComponent("pplref")
        if let outfile = outfile {
            process.arguments = [infile.path, "-o", outfile.path]
        } else {
            process.arguments = [infile.path]
        }
        
        let pipe = Pipe()
        process.standardOutput = pipe
        
        try? process.run()
        process.waitUntilExit()
    }
}
