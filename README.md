# Universal ID Reader

A cross-platform C++ library for reading and extracting data from credit card-sized identification documents including driver's licenses and passports from multiple countries.

## Overview

The Universal ID Reader is a C++ library that provides real-time ID document scanning and data extraction capabilities with a focus on privacy and offline processing. The library can be integrated into applications on iOS, Android, macOS, and Windows platforms.

## Key Features

- **Cross-Platform C++ Library**: Can be integrated into iOS, Android, macOS, and Windows applications
- **Privacy-First**: All processing performed locally on device
- **Real-Time Scanning**: Fast document detection and data extraction
- **Multi-Country Support**: Supports ID documents from multiple countries
- **Offline Processing**: No internet connection required
- **High Accuracy**: Target accuracy of >95% for standard documents
- **Easy Integration**: Simple C API for binding to other languages

## Technology Stack

- **Core Language**: C++17 with C API bindings
- **Computer Vision**: OpenCV for image preprocessing and document detection
- **OCR Engine**: Tesseract (open source) with cloud API fallback options
- **Machine Learning**: TensorFlow Lite for document classification and field detection
- **Build System**: CMake for cross-platform builds

## Architecture

The C++ library is built with five core layers:

1. **Image Preprocessing** - Image enhancement and document boundary detection
2. **Document Classification** - ML-based document type and country detection
3. **Data Extraction** - OCR processing with country-specific optimizations
4. **Data Validation & Parsing** - Field validation and confidence scoring
5. **C API Layer** - Simple C interface for cross-platform integration

## Supported Documents

### Initial Release Target Countries
- Canada (Driver's License, Passport)
- United States (Driver's License, Passport)
- United Kingdom (Driver's License, Passport)
- European Union (ID Cards, Passports)
- Australia (Driver's License, Passport)

#### System Dependencies
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- OpenCV 4.5+ with development headers
- pkg-config (Linux/macOS)
- Make or CMake 3.15+

#### Installing OpenCV

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install libopencv-dev pkg-config build-essential
```

**macOS (Homebrew):**
```bash
brew install opencv pkg-config
```

**Windows:**
Download OpenCV from https://opencv.org/releases/ and set environment variables.

#### Optional Dependencies
- Tesseract 5.0+ (for OCR - future implementation)
- TensorFlow Lite 2.8+ (for ML classification - future implementation)

### Quick Start Build

1. Clone this repository:
```bash
git clone https://github.com/j-klawson/id_reader.git
cd id_reader
```

2. Build using Make (recommended for development):
```bash
make                    # Build shared and static libraries
make examples          # Build example applications
make test              # Run tests (if available)
```

3. Or build using CMake:
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

#### Using Make

```bash
# Debug build
make BUILD_TYPE=Debug

# Release build (default)
make BUILD_TYPE=Release

# Use different compiler
make CXX=clang++

# Build specific targets
make all               # Build libraries
make examples          # Build examples
make tests             # Build test suite
make install           # Install to system
make clean             # Clean build artifacts
```

#### Using CMake

```bash
# Linux/macOS
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# macOS (Universal Binary)
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
make -j$(nproc)

# Windows (Visual Studio)
mkdir build && cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release

# Cross-compile for iOS
cmake .. -DCMAKE_TOOLCHAIN_FILE=cmake/ios.toolchain.cmake -DPLATFORM=OS64
make -j$(nproc)

# Cross-compile for Android
cmake .. -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a
make -j$(nproc)
```

### Installation

Install to system directories:
```bash
sudo make install      # Using Make
# or
sudo make install      # Using CMake from build directory
```

Uninstall:
```bash
sudo make uninstall
```

### Testing the Build

#### Quick Validation
Run the example applications:
```bash
# After building with Make
./build/document_detector_example path/to/your/image.jpg

# Or the C example
./build/simple_detection_c path/to/your/image.bmp
```

#### Comprehensive Testing
Run the synthetic test suite to validate detection performance:
```bash
cd tests
make run-tests
```

This will:
- Generate synthetic test images (Canadian ID cards, driver's licenses, passports)
- Test detection across various challenging conditions
- Provide performance statistics and visual results
- Export detailed CSV analysis

**Quick Test**: For faster validation during development:
```bash
cd tests
make quick-test
```

### Troubleshooting

**OpenCV not found:**
```bash
# Check OpenCV installation
pkg-config --modversion opencv4
# or
pkg-config --modversion opencv

# Set PKG_CONFIG_PATH if needed
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
```

**Compiler errors:**
- Ensure C++17 support: `g++ --version` (GCC 7+) or `clang++ --version` (Clang 5+)
- Update compiler: `sudo apt install g++-9` (Ubuntu) or `brew install llvm` (macOS)

**Library linking issues:**
```bash
# Update library cache (Linux)
sudo ldconfig

# Check library installation
ls -la /usr/local/lib/libid_reader*
```

## Privacy & Security

- **Local Processing**: All data processing happens on-device
- **No Cloud Storage**: Personal information never leaves the device
- **Secure Deletion**: Processed images are securely deleted
- **Privacy Compliance**: Built with GDPR, CCPA, and other privacy regulations in mind
- **User Consent**: Transparent data handling with user control

## Hardware Requirements

- Minimum 2GB RAM 
- GPU acceleration support preferred (OpenCL/CUDA for processing acceleration)
- Camera integration handled by host application

See [TODO.md](TODO.md) for the complete development roadmap covering all 10 phases from foundation through deployment.

## Testing and Validation

The project includes a comprehensive testing framework using synthetic data:

**Test Suite Features**:
- Synthetic document generation (no real ID data)
- Canadian document specifications (ID cards, driver's licenses, passports)
- Performance benchmarking and validation
- Visual result generation
- Automated success rate analysis

**Run Tests**:
```bash
cd tests && make run-tests
```

**Test Results**: Located in `tests/test_results/` with CSV data and visual outputs.

**Latest Test Results** (54 synthetic Canadian documents):
- 🎯 **88.9% Success Rate** (48/54 successful detections)
- ⚡ **0.62ms Average Processing** (real-time capable)
- 🏆 **96.4% Average Confidence** (0.964/1.0)
- 🇨🇦 **Canadian Document Performance**:
  - Ontario Driver's License simulation: 90-95% success
  - Canadian Passport simulation: 85-90% success
  - ID Card simulation: 95%+ success

## Integration Examples

### iOS (Swift)
```swift
import Foundation

// Swift wrapper around C API
class IDReader {
    private var context: OpaquePointer?
    
    init() throws {
        guard id_reader_init(&context) == ID_READER_SUCCESS else {
            throw IDReaderError.initializationFailed
        }
    }
    
    func processImage(_ image: UIImage) -> IDReaderResult? {
        // Implementation using C API
    }
    
    deinit {
        id_reader_cleanup(context)
    }
}
```

### Android (Kotlin)
```kotlin
class IDReader {
    private external fun nativeInit(): Long
    private external fun nativeProcessImage(context: Long, imageData: ByteArray): IDReaderResult?
    private external fun nativeCleanup(context: Long)
    
    companion object {
        init {
            System.loadLibrary("id_reader")
        }
    }
}
```

### Windows (C#)
```csharp
using System;
using System.Runtime.InteropServices;

public class IDReader : IDisposable {
    [DllImport("id_reader.dll")]
    private static extern int id_reader_init(out IntPtr context);
    
    [DllImport("id_reader.dll")]
    private static extern void id_reader_cleanup(IntPtr context);
    
    // Implementation using P/Invoke
}
```

## License

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)

This project is licensed under the GNU General Public License v2.0.

## Acknowledgments

- OpenCV community for computer vision tools
- Tesseract OCR project
- Flutter team for cross-platform framework
- Contributors and testers from the developer community