# Universal ID Reader - AI Assistant Guide

## Project Overview
This is a cross-platform application for reading and extracting data from credit card-sized identification documents (driver's licenses, passports) from multiple countries. The app runs on iOS, Android, macOS, and Windows with a focus on privacy and offline processing.

## Technology Stack
- **Framework**: Flutter with native plugins
- **Computer Vision**: OpenCV for image preprocessing and document detection
- **OCR**: Tesseract (open source) with cloud API fallback options
- **Machine Learning**: TensorFlow Lite for document classification and field detection
- **Platforms**: iOS, Android, macOS, Windows

## Key Architecture Layers
1. Image Capture & Preprocessing - Camera integration and image enhancement
2. Document Classification - ML-based document type and country detection
3. Data Extraction - OCR processing with country-specific optimizations
4. Data Validation & Parsing - Field validation and confidence scoring
5. Security & Privacy - Local processing and data encryption

## Development Environment
- Flutter SDK 3.0+
- Xcode (for iOS/macOS development)
- Android Studio (for Android development)
- Visual Studio (for Windows development)
- OpenCV 4.5+
- Tesseract 5.0+
- TensorFlow Lite 2.8+

## Commands to Run
```bash
# Install dependencies
flutter pub get

# Run the app
flutter run

# Build for different platforms
flutter build apk          # Android
flutter build ios          # iOS
flutter build macos        # macOS
flutter build windows      # Windows
```

## Privacy & Security Focus
- All processing performed locally on device
- No cloud storage of personal information
- Secure deletion of processed images
- GDPR, CCPA compliance required

## Performance Targets
- Document detection: < 500ms
- OCR processing: < 2 seconds
- Total processing time: < 5 seconds
- Accuracy target: > 95% for standard documents

## Current Status
The project is in the planning phase. See TODO.md for the complete 10-phase development roadmap covering foundation, computer vision, ML classification, OCR, data processing, security, UI/UX, optimization, testing, and deployment.

## Target Countries (Initial Release)
- United States (Driver's License, Passport)
- Canada (Driver's License, Passport)
- United Kingdom (Driver's License, Passport)
- European Union (ID Cards, Passports)
- Australia (Driver's License, Passport)