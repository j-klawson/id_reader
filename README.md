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
- United States (Driver's License, Passport)
- Canada (Driver's License, Passport)
- United Kingdom (Driver's License, Passport)
- European Union (ID Cards, Passports)
- Australia (Driver's License, Passport)

## Getting Started

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.15+
- OpenCV 4.5+
- Tesseract 5.0+
- TensorFlow Lite 2.8+
- pkg-config (Linux/macOS)

### Installation

1. Clone this repository:
```bash
git clone https://github.com/j-klawson/id_reader.git
cd id_reader
```

2. Create build directory and configure:
```bash
mkdir build
cd build
cmake ..
```

3. Build the library:
```bash
make -j$(nproc)  # Linux/macOS
# or
cmake --build . --config Release  # Windows
```

4. Install the library:
```bash
make install  # Linux/macOS
# or
cmake --install . --config Release  # Windows
```

### Building for Different Platforms

```bash
# Linux
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# macOS
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
make -j$(nproc)

# Windows (Visual Studio)
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release

# Cross-compile for iOS
cmake .. -DCMAKE_TOOLCHAIN_FILE=cmake/ios.toolchain.cmake -DPLATFORM=OS64
make -j$(nproc)

# Cross-compile for Android
cmake .. -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a
make -j$(nproc)
```

## Performance Targets

- Document detection: < 500ms
- OCR processing: < 2 seconds
- Total processing time: < 5 seconds
- Accuracy target: > 95% for standard documents

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

## Usage Example

```c
#include <id_reader/id_reader.h>
#include <stdio.h>

int main() {
    id_reader_context_t* context = NULL;
    id_reader_result_t* result = NULL;
    
    // Initialize the library
    if (id_reader_init(&context) != ID_READER_SUCCESS) {
        printf("Failed to initialize ID Reader\n");
        return -1;
    }
    
    // Configure the library
    id_reader_set_config(context, "country", "US");
    id_reader_set_config(context, "document_type", "drivers_license");
    
    // Process an image (image data not shown)
    id_reader_image_t image = {
        .data = image_data,
        .width = 1920,
        .height = 1080,
        .stride = 1920 * 3,
        .format = ID_READER_IMAGE_FORMAT_RGB
    };
    
    if (id_reader_process_image(context, &image, &result) == ID_READER_SUCCESS) {
        printf("Document Type: %s\n", id_reader_document_type_string(result->document_type));
        printf("Country: %s\n", id_reader_country_string(result->country));
        printf("Confidence: %.2f\n", result->overall_confidence);
        
        // Print extracted fields
        for (size_t i = 0; i < result->field_count; i++) {
            printf("%s: %s (%.2f)\n", 
                   result->fields[i].name, 
                   result->fields[i].value, 
                   result->fields[i].confidence);
        }
        
        // Free the result
        id_reader_free_result(result);
    }
    
    // Cleanup
    id_reader_cleanup(context);
    return 0;
}
```

## Language Bindings

The library provides a C API that can be easily bound to other languages:

- **Swift (iOS/macOS)**: Native integration with iOS/macOS applications
- **Kotlin (Android)**: JNI bindings for Android applications  
- **C# (Windows/.NET)**: P/Invoke bindings for Windows applications
- **JavaScript (Node.js)**: FFI bindings for Node.js applications
- **Python**: ctypes or cffi bindings for Python applications

## Development Status

This project is currently in the planning and foundation phase. See [TODO.md](TODO.md) for the complete development roadmap covering all 10 phases from foundation through deployment.

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

## Contributing

We welcome contributions! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct and the process for submitting pull requests.

## License

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)

This project is licensed under the GNU General Public License v2.0.

## Acknowledgments

- OpenCV community for computer vision tools
- Tesseract OCR project
- Flutter team for cross-platform framework
- Contributors and testers from the developer community