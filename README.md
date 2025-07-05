# Universal ID Reader

A cross-platform application for reading and extracting data from credit card-sized identification documents including driver's licenses and passports from multiple countries.

## Overview

The Universal ID Reader provides real-time ID document scanning and data extraction capabilities with a focus on privacy and offline processing. The application runs on iOS, Android, macOS, and Windows platforms.

## Key Features

- **Cross-Platform**: Works on iOS, Android, macOS, and Windows
- **Privacy-First**: All processing performed locally on device
- **Real-Time Scanning**: Fast document detection and data extraction
- **Multi-Country Support**: Supports ID documents from multiple countries
- **Offline Processing**: No internet connection required
- **High Accuracy**: Target accuracy of >95% for standard documents

## Technology Stack

- **Framework**: Flutter with native plugins for camera and ML integration
- **Computer Vision**: OpenCV for image preprocessing and document detection
- **OCR Engine**: Tesseract (open source) with cloud API fallback options
- **Machine Learning**: TensorFlow Lite for document classification and field detection

## Architecture

The application is built with five core layers:

1. **Image Capture & Preprocessing** - Camera integration and image enhancement
2. **Document Classification** - ML-based document type and country detection
3. **Data Extraction** - OCR processing with country-specific optimizations
4. **Data Validation & Parsing** - Field validation and confidence scoring
5. **Security & Privacy** - Local processing and data encryption

## Supported Documents

### Initial Release Target Countries
- United States (Driver's License, Passport)
- Canada (Driver's License, Passport)
- United Kingdom (Driver's License, Passport)
- European Union (ID Cards, Passports)
- Australia (Driver's License, Passport)

## Getting Started

### Prerequisites

- Flutter SDK 3.0+
- Xcode (for iOS/macOS development)
- Android Studio (for Android development)
- Visual Studio (for Windows development)
- OpenCV 4.5+
- Tesseract 5.0+
- TensorFlow Lite 2.8+

### Installation

1. Clone this repository:
```bash
git clone https://github.com/j-klawson/id_reader.git
cd id_reader
```

2. Install Flutter dependencies:
```bash
flutter pub get
```

3. Set up native development environments for your target platforms

4. Run the application:
```bash
flutter run
```

### Building for Different Platforms

```bash
# Android
flutter build apk

# iOS
flutter build ios

# macOS
flutter build macos

# Windows
flutter build windows
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

- Camera with auto-focus capability
- Minimum 2GB RAM for mobile devices
- GPU acceleration support preferred

## Development Status

This project is currently in the planning and foundation phase. See [TODO.md](TODO.md) for the complete development roadmap covering all 10 phases from foundation through deployment.

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