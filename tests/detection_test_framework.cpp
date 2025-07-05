/*
 * Universal ID Reader - Cross-platform ID document scanner
 * Copyright (C) 2025 J. Keith Lawson
 *
 * Detection Test Framework
 * Validates the OpenCV document detection algorithm using synthetic test images.
 */

#include <id_reader/id_reader.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <dirent.h>
#include <sys/stat.h>

struct TestResult {
    std::string image_name;
    bool detection_success;
    float confidence;
    float processing_time_ms;
    id_reader_document_bounds_t bounds;
    std::string error_message;
};

struct TestStatistics {
    int total_images = 0;
    int successful_detections = 0;
    int failed_detections = 0;
    float average_confidence = 0.0f;
    float average_processing_time = 0.0f;
    float min_confidence = 1.0f;
    float max_confidence = 0.0f;
    float min_processing_time = 999999.0f;
    float max_processing_time = 0.0f;
};

class DetectionTestFramework {
public:
    DetectionTestFramework() : context_(nullptr) {}
    
    ~DetectionTestFramework() {
        if (context_) {
            id_reader_cleanup(context_);
        }
    }
    
    bool initialize() {
        id_reader_error_t error = id_reader_init(&context_);
        if (error != ID_READER_SUCCESS) {
            std::cerr << "Failed to initialize ID Reader: " << id_reader_error_string(error) << std::endl;
            return false;
        }
        
        // Configure detection parameters for testing
        id_reader_set_config(context_, "canny_threshold1", "50");
        id_reader_set_config(context_, "canny_threshold2", "150");
        id_reader_set_config(context_, "min_contour_area", "5000");  // Lower for test images
        id_reader_set_config(context_, "max_contour_area", "500000");
        
        std::cout << "ID Reader v" << id_reader_version_string() << " initialized" << std::endl;
        return true;
    }
    
    std::vector<TestResult> runTestSuite(const std::string& test_dir) {
        std::vector<TestResult> results;
        
        // Check if directory exists
        DIR* dir = opendir(test_dir.c_str());
        if (!dir) {
            std::cerr << "Test directory does not exist: " << test_dir << std::endl;
            return results;
        }
        
        std::cout << "Running test suite on directory: " << test_dir << std::endl;
        
        // Process all images in the directory
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_REG) { // Regular file
                std::string filename = entry->d_name;
                std::string extension = "";
                
                // Extract file extension
                size_t dot_pos = filename.find_last_of('.');
                if (dot_pos != std::string::npos) {
                    extension = filename.substr(dot_pos);
                }
                
                // Check if it's an image file
                if (extension == ".jpg" || extension == ".jpeg" || extension == ".png" || 
                    extension == ".bmp" || extension == ".tiff") {
                    std::string full_path = test_dir + "/" + filename;
                    TestResult result = testSingleImage(full_path, filename);
                    results.push_back(result);
                    
                    // Print progress
                    std::cout << "Tested: " << filename << " - " 
                             << (result.detection_success ? "SUCCESS" : "FAILED")
                             << " (confidence: " << std::fixed << std::setprecision(3) 
                             << result.confidence << ")" << std::endl;
                }
            }
        }
        
        closedir(dir);
        return results;
    }
    
    TestResult testSingleImage(const std::string& image_path, const std::string& image_name) {
        TestResult result;
        result.image_name = image_name;
        result.detection_success = false;
        result.confidence = 0.0f;
        result.processing_time_ms = 0.0f;
        
        // Load image
        cv::Mat image = cv::imread(image_path);
        if (image.empty()) {
            result.error_message = "Failed to load image";
            return result;
        }
        
        // Prepare image data for the API
        id_reader_image_t input_image;
        input_image.data = image.data;
        input_image.width = image.cols;
        input_image.height = image.rows;
        input_image.stride = image.step;
        input_image.format = ID_READER_IMAGE_FORMAT_BGR;
        
        // Measure processing time
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Process the image
        id_reader_result_t* api_result = nullptr;
        id_reader_error_t error = id_reader_process_image(context_, &input_image, &api_result);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        result.processing_time_ms = duration.count() / 1000.0f;
        
        if (error == ID_READER_SUCCESS && api_result) {
            result.detection_success = true;
            result.confidence = api_result->overall_confidence;
            result.bounds = api_result->bounds;
            
            // Free the result
            id_reader_free_result(api_result);
        } else {
            result.error_message = id_reader_error_string(error);
        }
        
        return result;
    }
    
    TestStatistics calculateStatistics(const std::vector<TestResult>& results) {
        TestStatistics stats;
        stats.total_images = results.size();
        
        if (stats.total_images == 0) {
            return stats;
        }
        
        float total_confidence = 0.0f;
        float total_processing_time = 0.0f;
        
        for (const auto& result : results) {
            if (result.detection_success) {
                stats.successful_detections++;
                total_confidence += result.confidence;
                
                stats.min_confidence = std::min(stats.min_confidence, result.confidence);
                stats.max_confidence = std::max(stats.max_confidence, result.confidence);
            } else {
                stats.failed_detections++;
            }
            
            total_processing_time += result.processing_time_ms;
            stats.min_processing_time = std::min(stats.min_processing_time, result.processing_time_ms);
            stats.max_processing_time = std::max(stats.max_processing_time, result.processing_time_ms);
        }
        
        if (stats.successful_detections > 0) {
            stats.average_confidence = total_confidence / stats.successful_detections;
        }
        
        stats.average_processing_time = total_processing_time / stats.total_images;
        
        return stats;
    }
    
    void printStatistics(const TestStatistics& stats) {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "DETECTION TEST RESULTS" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
        std::cout << "Total Images Tested: " << stats.total_images << std::endl;
        std::cout << "Successful Detections: " << stats.successful_detections 
                  << " (" << std::fixed << std::setprecision(1) 
                  << (100.0f * stats.successful_detections / stats.total_images) << "%)" << std::endl;
        std::cout << "Failed Detections: " << stats.failed_detections 
                  << " (" << std::fixed << std::setprecision(1) 
                  << (100.0f * stats.failed_detections / stats.total_images) << "%)" << std::endl;
        
        std::cout << "\nConfidence Statistics:" << std::endl;
        if (stats.successful_detections > 0) {
            std::cout << "  Average: " << std::fixed << std::setprecision(3) << stats.average_confidence << std::endl;
            std::cout << "  Minimum: " << std::fixed << std::setprecision(3) << stats.min_confidence << std::endl;
            std::cout << "  Maximum: " << std::fixed << std::setprecision(3) << stats.max_confidence << std::endl;
        } else {
            std::cout << "  No successful detections" << std::endl;
        }
        
        std::cout << "\nPerformance Statistics:" << std::endl;
        std::cout << "  Average Processing Time: " << std::fixed << std::setprecision(2) 
                  << stats.average_processing_time << " ms" << std::endl;
        std::cout << "  Fastest Processing: " << std::fixed << std::setprecision(2) 
                  << stats.min_processing_time << " ms" << std::endl;
        std::cout << "  Slowest Processing: " << std::fixed << std::setprecision(2) 
                  << stats.max_processing_time << " ms" << std::endl;
        
        std::cout << std::string(60, '=') << std::endl;
    }
    
    void saveDetailedResults(const std::vector<TestResult>& results, const std::string& output_file) {
        std::ofstream file(output_file);
        if (!file.is_open()) {
            std::cerr << "Failed to open output file: " << output_file << std::endl;
            return;
        }
        
        // CSV header
        file << "Image,Success,Confidence,ProcessingTime(ms),X1,Y1,X2,Y2,X3,Y3,X4,Y4,ErrorMessage" << std::endl;
        
        for (const auto& result : results) {
            file << result.image_name << ","
                 << (result.detection_success ? "1" : "0") << ","
                 << std::fixed << std::setprecision(4) << result.confidence << ","
                 << std::fixed << std::setprecision(2) << result.processing_time_ms << ",";
            
            if (result.detection_success) {
                file << result.bounds.x1 << "," << result.bounds.y1 << ","
                     << result.bounds.x2 << "," << result.bounds.y2 << ","
                     << result.bounds.x3 << "," << result.bounds.y3 << ","
                     << result.bounds.x4 << "," << result.bounds.y4 << ",";
            } else {
                file << ",,,,,,,,";
            }
            
            file << result.error_message << std::endl;
        }
        
        file.close();
        std::cout << "Detailed results saved to: " << output_file << std::endl;
    }
    
    void generateVisualResults(const std::vector<TestResult>& results, 
                              const std::string& test_dir, 
                              const std::string& output_dir) {
        std::cout << "Generating visual results..." << std::endl;
        
        // Create output directory
        std::string mkdir_cmd = "mkdir -p " + output_dir;
        system(mkdir_cmd.c_str());
        
        for (const auto& result : results) {
            if (!result.detection_success) continue;
            
            // Load original image
            std::string image_path = test_dir + "/" + result.image_name;
            cv::Mat image = cv::imread(image_path);
            if (image.empty()) continue;
            
            // Draw detection results
            cv::Mat result_image = image.clone();
            
            // Convert normalized coordinates to pixel coordinates
            std::vector<cv::Point> corners = {
                cv::Point(result.bounds.x1 * image.cols, result.bounds.y1 * image.rows),
                cv::Point(result.bounds.x2 * image.cols, result.bounds.y2 * image.rows),
                cv::Point(result.bounds.x3 * image.cols, result.bounds.y3 * image.rows),
                cv::Point(result.bounds.x4 * image.cols, result.bounds.y4 * image.rows)
            };
            
            // Draw the detected document boundary
            for (size_t i = 0; i < corners.size(); ++i) {
                cv::line(result_image, corners[i], corners[(i + 1) % corners.size()], 
                        cv::Scalar(0, 255, 0), 3);
                cv::circle(result_image, corners[i], 5, cv::Scalar(0, 0, 255), -1);
            }
            
            // Add confidence text
            std::string confidence_text = "Confidence: " + 
                std::to_string(result.confidence).substr(0, 5);
            cv::putText(result_image, confidence_text, cv::Point(10, 30), 
                       cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
            
            // Save result image
            std::string output_path = output_dir + "/result_" + result.image_name;
            cv::imwrite(output_path, result_image);
        }
        
        std::cout << "Visual results saved to: " << output_dir << std::endl;
    }

private:
    id_reader_context_t* context_;
};

int main(int argc, char* argv[]) {
    std::string test_dir = "test_temp";
    std::string output_dir = "test_results";
    
    if (argc > 1) {
        test_dir = argv[1];
    }
    if (argc > 2) {
        output_dir = argv[2];
    }
    
    std::cout << "Detection Test Framework" << std::endl;
    std::cout << "Testing directory: " << test_dir << std::endl;
    std::cout << "Output directory: " << output_dir << std::endl;
    
    DetectionTestFramework framework;
    if (!framework.initialize()) {
        return 1;
    }
    
    // Run the test suite
    std::vector<TestResult> results = framework.runTestSuite(test_dir);
    
    if (results.empty()) {
        std::cout << "No test images found in directory: " << test_dir << std::endl;
        return 1;
    }
    
    // Calculate and print statistics
    TestStatistics stats = framework.calculateStatistics(results);
    framework.printStatistics(stats);
    
    // Create output directory
    std::string mkdir_cmd = "mkdir -p " + output_dir;
    system(mkdir_cmd.c_str());
    
    // Save detailed results
    framework.saveDetailedResults(results, output_dir + "/detailed_results.csv");
    
    // Generate visual results
    framework.generateVisualResults(results, test_dir, output_dir + "/visual");
    
    std::cout << "\nTest framework completed successfully!" << std::endl;
    
    return 0;
}