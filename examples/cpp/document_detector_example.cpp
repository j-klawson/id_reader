/*
 * Universal ID Reader - Cross-platform ID document scanner
 * Copyright (C) 2025 J. Keith Lawson
 *
 * Example: Document Detection
 * 
 * This example demonstrates how to use the ID Reader library
 * to detect documents in images using C++.
 */

#include <id_reader/id_reader.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return -1;
    }
    
    std::string image_path = argv[1];
    
    // Load image using OpenCV
    cv::Mat image = cv::imread(image_path);
    if (image.empty()) {
        std::cerr << "Error: Could not load image from " << image_path << std::endl;
        return -1;
    }
    
    std::cout << "Loaded image: " << image.cols << "x" << image.rows << std::endl;
    
    // Initialize ID Reader library
    id_reader_context_t* context = nullptr;
    id_reader_error_t error = id_reader_init(&context);
    if (error != ID_READER_SUCCESS) {
        std::cerr << "Error initializing ID Reader: " << id_reader_error_string(error) << std::endl;
        return -1;
    }
    
    std::cout << "ID Reader v" << id_reader_version_string() << " initialized successfully" << std::endl;
    
    // Configure detection parameters
    id_reader_set_config(context, "canny_threshold1", "50");
    id_reader_set_config(context, "canny_threshold2", "150");
    id_reader_set_config(context, "min_contour_area", "10000");
    
    // Prepare image data
    id_reader_image_t input_image;
    input_image.data = image.data;
    input_image.width = image.cols;
    input_image.height = image.rows;
    input_image.stride = image.step;
    input_image.format = ID_READER_IMAGE_FORMAT_BGR;
    
    // Process the image
    id_reader_result_t* result = nullptr;
    error = id_reader_process_image(context, &input_image, &result);
    
    if (error == ID_READER_SUCCESS && result) {
        std::cout << "Document detection successful!" << std::endl;
        std::cout << "Document Type: " << id_reader_document_type_string(result->document_type) << std::endl;
        std::cout << "Country: " << id_reader_country_string(result->country) << std::endl;
        std::cout << "Overall Confidence: " << result->overall_confidence << std::endl;
        
        std::cout << "Document Bounds (normalized coordinates):" << std::endl;
        std::cout << "  Top-left: (" << result->bounds.x1 << ", " << result->bounds.y1 << ")" << std::endl;
        std::cout << "  Top-right: (" << result->bounds.x2 << ", " << result->bounds.y2 << ")" << std::endl;
        std::cout << "  Bottom-right: (" << result->bounds.x3 << ", " << result->bounds.y3 << ")" << std::endl;
        std::cout << "  Bottom-left: (" << result->bounds.x4 << ", " << result->bounds.y4 << ")" << std::endl;
        std::cout << "  Confidence: " << result->bounds.confidence << std::endl;
        
        // Draw detection results on image
        cv::Mat result_image = image.clone();
        
        // Convert normalized coordinates to pixel coordinates
        std::vector<cv::Point> corners = {
            cv::Point(result->bounds.x1 * image.cols, result->bounds.y1 * image.rows),
            cv::Point(result->bounds.x2 * image.cols, result->bounds.y2 * image.rows),
            cv::Point(result->bounds.x3 * image.cols, result->bounds.y3 * image.rows),
            cv::Point(result->bounds.x4 * image.cols, result->bounds.y4 * image.rows)
        };
        
        // Draw the detected document boundary
        for (size_t i = 0; i < corners.size(); ++i) {
            cv::line(result_image, corners[i], corners[(i + 1) % corners.size()], cv::Scalar(0, 255, 0), 3);
            cv::circle(result_image, corners[i], 5, cv::Scalar(0, 0, 255), -1);
        }
        
        // Add confidence text
        std::string confidence_text = "Confidence: " + std::to_string(result->bounds.confidence);
        cv::putText(result_image, confidence_text, cv::Point(10, 30), 
                   cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
        
        // Save result image
        std::string output_path = "detection_result.jpg";
        cv::imwrite(output_path, result_image);
        std::cout << "Detection result saved to: " << output_path << std::endl;
        
        // Display image if possible (comment out if running headless)
        try {
            cv::imshow("Original Image", image);
            cv::imshow("Detection Result", result_image);
            std::cout << "Press any key to close windows..." << std::endl;
            cv::waitKey(0);
            cv::destroyAllWindows();
        } catch (const cv::Exception& e) {
            std::cout << "Display not available (running headless): " << e.what() << std::endl;
        }
        
        // Free the result
        id_reader_free_result(result);
        
    } else {
        std::cerr << "Document detection failed: " << id_reader_error_string(error) << std::endl;
        if (error == ID_READER_ERROR_NO_DOCUMENT_FOUND) {
            std::cerr << "Try adjusting the detection parameters or using a clearer image." << std::endl;
        }
    }
    
    // Cleanup
    id_reader_cleanup(context);
    
    return error == ID_READER_SUCCESS ? 0 : -1;
}