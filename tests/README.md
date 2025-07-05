# Universal ID Reader - Test Suite

This directory contains a comprehensive testing framework for validating the OpenCV document detection algorithm using synthetic test data.

## Overview

The test suite includes:

1. **Synthetic Test Generator**: Creates realistic test images without any personal information
2. **Detection Test Framework**: Validates detection algorithm performance
3. **Performance Benchmarking**: Measures processing speed and accuracy
4. **Visual Results**: Generates images showing detection results

## Quick Start

### Prerequisites

Ensure the main library is built first:
```bash
cd ..
make  # Build the main ID Reader library
cd tests
```

### Run Complete Test Suite

```bash
make run-tests
```

This will:
- Generate synthetic test images
- Run detection tests on all images
- Generate performance statistics
- Create visual results showing detections
- Save detailed CSV results

### Quick Validation

For faster testing during development:
```bash
make quick-test
```

## Test Components

### 1. Synthetic Test Generator

**File**: `synthetic_test_generator.cpp`

Generates realistic document images with various challenging conditions:

**Document Types**:
- ID Cards (85.6×53.98mm - credit card size)
- Driver's Licenses (same as ID cards)
- Passport Pages (125×88mm - B7 size)

**Test Scenarios**:
- **Basic Documents**: Clean, well-lit images
- **Rotation**: -15° to +30° angles
- **Perspective**: Simulated camera angles
- **Lighting**: Uneven illumination
- **Backgrounds**: Plain, textured, gradient
- **Blur**: Motion blur simulation

**Usage**:
```bash
./build/synthetic_test_generator [output_directory]
```

### 2. Detection Test Framework

**File**: `detection_test_framework.cpp`

Validates detection performance across all test images:

**Features**:
- Processes all images in a directory
- Measures detection success rate
- Records confidence scores
- Tracks processing times
- Generates visual results
- Exports detailed CSV data

**Usage**:
```bash
./build/detection_test_framework [test_dir] [output_dir]
```

## Test Results

### Statistics Generated

- **Success Rate**: Percentage of successful detections
- **Confidence Scores**: Average, min, max confidence values
- **Performance**: Processing times (average, min, max)
- **Error Analysis**: Breakdown of failure modes

### Output Files

**Detailed Results** (`test_results/detailed_results.csv`):
```csv
Image,Success,Confidence,ProcessingTime(ms),X1,Y1,X2,Y2,X3,Y3,X4,Y4,ErrorMessage
ID_Card_basic.jpg,1,0.8950,45.32,0.125,0.150,0.875,0.150,0.875,0.850,0.125,0.850,
```

**Visual Results** (`test_results/visual/`):
- Original images with detection boundaries overlaid
- Green lines show detected document edges
- Red circles mark corner points
- Confidence score displayed on image

## Expected Performance

Based on the OpenCV algorithm implementation:

### Success Rates (Expected)
- **Basic Documents**: 95-100%
- **Slight Rotation (±15°)**: 90-95%
- **Perspective Distortion**: 85-90%
- **Poor Lighting**: 80-85%
- **Complex Backgrounds**: 75-85%
- **Motion Blur**: 70-80%

### Performance Targets
- **Processing Time**: < 100ms per image
- **Average Confidence**: > 0.8 for successful detections
- **Memory Usage**: < 50MB peak

## Canadian Document Specific Tests

The test suite is designed to validate detection for Canadian documents:

### Ontario Driver's License Characteristics
- **Size**: Standard ID card dimensions (85.6×53.98mm)
- **Aspect Ratio**: 1.586:1
- **Border**: Clear rectangular boundary
- **Expected Detection**: High success rate (>90%)

### Canadian Passport Characteristics  
- **Size**: B7 format (125×88mm)
- **Aspect Ratio**: 1.42:1
- **Data Page**: Rectangular text area
- **Expected Detection**: Good success rate (>85%)

## Running Specific Tests

### Performance Benchmark
```bash
make benchmark
```
Runs multiple iterations to measure average performance.

### Parameter Testing
```bash
make parameter-tests
```
Tests different detection parameter configurations.

### Custom Test Directory
```bash
# Generate custom test images
./build/synthetic_test_generator my_test_dir

# Run tests on custom directory
./build/detection_test_framework my_test_dir my_results
```

## Interpreting Results

### High Success Rate (>90%)
- Algorithm is working well for the test conditions
- Ready for real-world testing with non-sensitive documents

### Medium Success Rate (70-90%)
- Algorithm needs parameter tuning
- May require preprocessing improvements
- Consider adjusting Canny thresholds or contour filtering

### Low Success Rate (<70%)
- Significant algorithm issues
- Review edge detection parameters
- Check for OpenCV version compatibility

### Performance Issues
- **High processing times (>200ms)**: Optimize algorithm or reduce image resolution
- **Low confidence scores (<0.5)**: Improve contour analysis or add preprocessing steps

## Troubleshooting

### Build Issues
```bash
# Check dependencies
make check-deps

# Validate test framework
make validate
```

### No Test Images Generated
- Check OpenCV installation
- Verify write permissions in output directory
- Run generator manually to see error messages

### Detection Framework Fails
- Ensure main library is built (`../build/libid_reader.so`)
- Check LD_LIBRARY_PATH includes `../build`
- Verify test images exist in input directory

### Low Success Rates
- Review algorithm parameters in detection_test_framework.cpp
- Try adjusting Canny thresholds or contour area limits
- Generate simpler test cases first

## Development Workflow

1. **Make Algorithm Changes**: Modify detection code in `../src/preprocessing/`
2. **Rebuild Library**: `cd .. && make`
3. **Run Quick Test**: `make quick-test`
4. **Analyze Results**: Review success rates and failure modes
5. **Iterate**: Adjust parameters and repeat

This iterative approach allows rapid development and validation of the detection algorithm without using any real identification documents.