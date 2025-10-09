// The MIT License (MIT)
//
// Copyright (c) 2025 Insoft. All rights reserved.
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
        let libPath = Bundle.main.bundleURL.appendingPathComponent("Contents/Developer/usr/lib").path
        let includePath = Bundle.main.bundleURL.appendingPathComponent("Contents/Developer/usr/include").path
        let toolURL = Bundle.main.bundleURL.appendingPathComponent("Contents/Developer/usr/bin/ppl+")
        
        let process = Process()
        process.executableURL = toolURL
        if let _ = outfile {
            process.arguments = ["-L\(libPath)", "-I\(includePath)", infile.path, "-o", outfile!.path]
        } else {
            process.arguments = ["-L\(libPath)", "-I\(includePath)", infile.path]
        }
        
        let pipe = Pipe()
        process.standardOutput = pipe
        
        try? process.run()
        process.waitUntilExit()
    }
    
    class func pplmin(i infile: URL, o outfile: URL) {
        let toolURL = Bundle.main.bundleURL.appendingPathComponent("Contents/Developer/usr/bin/pplmin")
        
        let process = Process()
        process.executableURL = toolURL
        process.arguments = [infile.path, "-o", outfile.path]
        
        let pipe = Pipe()
        process.standardOutput = pipe
        
        try? process.run()
        process.waitUntilExit()
    }
    
    class func hpprgm(i infile: URL, o outfile: URL? = nil) {
        let toolURL = Bundle.main.bundleURL.appendingPathComponent("Contents/Developer/usr/bin/hpprgm")
        
        let process = Process()
        process.executableURL = toolURL
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
        let toolURL = Bundle.main.bundleURL.appendingPathComponent("Contents/Developer/usr/bin/grob")
        
        let process = Process()
        process.executableURL = toolURL
        process.arguments = [infile.path, "-o", outfile.path]
        
        let pipe = Pipe()
        process.standardOutput = pipe
        
        try? process.run()
        process.waitUntilExit()
    }
    
    class func pplfont(i infile: URL, o outfile: URL) {
        let toolURL = Bundle.main.bundleURL.appendingPathComponent("Contents/Developer/usr/bin/pplfont")
        
        let process = Process()
        process.executableURL = toolURL
        process.arguments = [infile.path, "-o", outfile.path, "--ppl"]
        
        let pipe = Pipe()
        process.standardOutput = pipe
        
        try? process.run()
        process.waitUntilExit()
    }
    
    class func pplref(i infile: URL, o outfile: URL? = nil) {
        let process = Process()
        
        process.executableURL = Bundle.main.bundleURL.appendingPathComponent("Contents/Developer/usr/bin/pplref")
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
