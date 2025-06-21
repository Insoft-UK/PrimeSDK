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

import Foundation

class TempFileManager {
    let tempDirectoryURL: URL

    init() {
        let tempRoot = FileManager.default.temporaryDirectory
        let uniqueTempDir = tempRoot.appendingPathComponent(UUID().uuidString)
        
        do {
            try FileManager.default.createDirectory(at: uniqueTempDir, withIntermediateDirectories: true)
            self.tempDirectoryURL = uniqueTempDir
        } catch {
            fatalError("Failed to create temp directory: \(error)")
        }
        
    }

    func cleanup() {
        do {
            try FileManager.default.removeItem(at: tempDirectoryURL)
        } catch {
            print("Failed to clean up temp directory: \(error)")
        }
    }
}
