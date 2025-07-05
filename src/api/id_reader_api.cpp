/*
 * Universal ID Reader - Cross-platform ID document scanner
 * Copyright (C) 2025 J. Keith Lawson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "id_reader/id_reader.h"
#include "../preprocessing/document_detection/document_detector.h"
#include <opencv2/opencv.hpp>
#include <memory>
#include <string>
#include <map>

struct id_reader_context {
    std::unique_ptr<id_reader::preprocessing::DocumentDetector> detector;
    std::map<std::string, std::string> config;
    
    id_reader_context() {
        detector = std::make_unique<id_reader::preprocessing::DocumentDetector>();
    }
};

extern "C" {

const char* id_reader_version_string(void) {
    return "1.0.0";
}

int id_reader_version_major(void) {
    return ID_READER_VERSION_MAJOR;
}

int id_reader_version_minor(void) {
    return ID_READER_VERSION_MINOR;
}

int id_reader_version_patch(void) {
    return ID_READER_VERSION_PATCH;
}

const char* id_reader_error_string(id_reader_error_t error) {
    switch (error) {
        case ID_READER_SUCCESS:
            return "Success";
        case ID_READER_ERROR_INVALID_INPUT:
            return "Invalid input";
        case ID_READER_ERROR_MEMORY_ALLOCATION:
            return "Memory allocation failed";
        case ID_READER_ERROR_PROCESSING_FAILED:
            return "Processing failed";
        case ID_READER_ERROR_NO_DOCUMENT_FOUND:
            return "No document found";
        case ID_READER_ERROR_UNSUPPORTED_FORMAT:
            return "Unsupported format";
        case ID_READER_ERROR_INITIALIZATION_FAILED:
            return "Initialization failed";
        default:
            return "Unknown error";
    }
}

const char* id_reader_document_type_string(id_reader_document_type_t type) {
    switch (type) {
        case ID_READER_DOCUMENT_DRIVERS_LICENSE:
            return "Driver's License";
        case ID_READER_DOCUMENT_PASSPORT:
            return "Passport";
        case ID_READER_DOCUMENT_ID_CARD:
            return "ID Card";
        case ID_READER_DOCUMENT_CREDIT_CARD:
            return "Credit Card";
        case ID_READER_DOCUMENT_UNKNOWN:
        default:
            return "Unknown";
    }
}

const char* id_reader_country_string(id_reader_country_t country) {
    switch (country) {
        case ID_READER_COUNTRY_US:
            return "United States";
        case ID_READER_COUNTRY_CA:
            return "Canada";
        case ID_READER_COUNTRY_GB:
            return "United Kingdom";
        case ID_READER_COUNTRY_DE:
            return "Germany";
        case ID_READER_COUNTRY_FR:
            return "France";
        case ID_READER_COUNTRY_AU:
            return "Australia";
        case ID_READER_COUNTRY_UNKNOWN:
        default:
            return "Unknown";
    }
}

id_reader_error_t id_reader_init(id_reader_context_t** context) {
    if (!context) {
        return ID_READER_ERROR_INVALID_INPUT;
    }
    
    try {
        *context = new id_reader_context();
        return ID_READER_SUCCESS;
    } catch (const std::exception&) {
        return ID_READER_ERROR_MEMORY_ALLOCATION;
    }
}

void id_reader_cleanup(id_reader_context_t* context) {
    delete context;
}

id_reader_error_t id_reader_set_config(id_reader_context_t* context, const char* key, const char* value) {
    if (!context || !key || !value) {
        return ID_READER_ERROR_INVALID_INPUT;
    }
    
    try {
        context->config[key] = value;
        
        // Apply configuration to detector if relevant
        if (std::string(key) == "canny_threshold1") {
            double threshold = std::stod(value);
            context->detector->setCannyThresholds(threshold, 150); // Keep default for threshold2
        } else if (std::string(key) == "canny_threshold2") {
            double threshold = std::stod(value);
            context->detector->setCannyThresholds(50, threshold); // Keep default for threshold1
        } else if (std::string(key) == "min_contour_area") {
            double area = std::stod(value);
            context->detector->setContourAreaRange(area, 500000); // Keep default for max
        } else if (std::string(key) == "max_contour_area") {
            double area = std::stod(value);
            context->detector->setContourAreaRange(10000, area); // Keep default for min
        }
        
        return ID_READER_SUCCESS;
    } catch (const std::exception&) {
        return ID_READER_ERROR_PROCESSING_FAILED;
    }
}

id_reader_error_t id_reader_get_config(id_reader_context_t* context, const char* key, char* value, size_t value_size) {
    if (!context || !key || !value || value_size == 0) {
        return ID_READER_ERROR_INVALID_INPUT;
    }
    
    auto it = context->config.find(key);
    if (it == context->config.end()) {
        return ID_READER_ERROR_INVALID_INPUT;
    }
    
    const std::string& config_value = it->second;
    if (config_value.length() >= value_size) {
        return ID_READER_ERROR_INVALID_INPUT;
    }
    
    strcpy(value, config_value.c_str());
    return ID_READER_SUCCESS;
}

id_reader_error_t id_reader_process_image(
    id_reader_context_t* context,
    const id_reader_image_t* image,
    id_reader_result_t** result) {
    
    if (!context || !image || !result || !image->data) {
        return ID_READER_ERROR_INVALID_INPUT;
    }
    
    try {
        // Convert input image to OpenCV Mat
        cv::Mat cv_image;
        int cv_type;
        
        switch (image->format) {
            case ID_READER_IMAGE_FORMAT_RGB:
                cv_type = CV_8UC3;
                cv_image = cv::Mat(image->height, image->width, cv_type, image->data, image->stride);
                cv::cvtColor(cv_image, cv_image, cv::COLOR_RGB2BGR);
                break;
            case ID_READER_IMAGE_FORMAT_RGBA:
                cv_type = CV_8UC4;
                cv_image = cv::Mat(image->height, image->width, cv_type, image->data, image->stride);
                cv::cvtColor(cv_image, cv_image, cv::COLOR_RGBA2BGR);
                break;
            case ID_READER_IMAGE_FORMAT_BGR:
                cv_type = CV_8UC3;
                cv_image = cv::Mat(image->height, image->width, cv_type, image->data, image->stride);
                break;
            case ID_READER_IMAGE_FORMAT_BGRA:
                cv_type = CV_8UC4;
                cv_image = cv::Mat(image->height, image->width, cv_type, image->data, image->stride);
                cv::cvtColor(cv_image, cv_image, cv::COLOR_BGRA2BGR);
                break;
            case ID_READER_IMAGE_FORMAT_GRAYSCALE:
                cv_type = CV_8UC1;
                cv_image = cv::Mat(image->height, image->width, cv_type, image->data, image->stride);
                break;
            default:
                return ID_READER_ERROR_UNSUPPORTED_FORMAT;
        }
        
        // Detect document bounds
        id_reader::preprocessing::DocumentBounds bounds;
        if (!context->detector->detectDocument(cv_image, bounds)) {
            return ID_READER_ERROR_NO_DOCUMENT_FOUND;
        }
        
        // Create result structure
        *result = new id_reader_result_t();
        (*result)->document_type = ID_READER_DOCUMENT_UNKNOWN; // Will be determined by classification
        (*result)->country = ID_READER_COUNTRY_UNKNOWN; // Will be determined by classification
        (*result)->bounds.x1 = bounds.x1;
        (*result)->bounds.y1 = bounds.y1;
        (*result)->bounds.x2 = bounds.x2;
        (*result)->bounds.y2 = bounds.y2;
        (*result)->bounds.x3 = bounds.x3;
        (*result)->bounds.y3 = bounds.y3;
        (*result)->bounds.x4 = bounds.x4;
        (*result)->bounds.y4 = bounds.y4;
        (*result)->bounds.confidence = bounds.confidence;
        (*result)->fields = nullptr;
        (*result)->field_count = 0;
        (*result)->overall_confidence = bounds.confidence;
        
        return ID_READER_SUCCESS;
        
    } catch (const std::exception&) {
        return ID_READER_ERROR_PROCESSING_FAILED;
    }
}

void id_reader_free_result(id_reader_result_t* result) {
    if (!result) {
        return;
    }
    
    // Free field data
    if (result->fields) {
        for (size_t i = 0; i < result->field_count; ++i) {
            if (result->fields[i].name) {
                delete[] result->fields[i].name;
            }
            if (result->fields[i].value) {
                delete[] result->fields[i].value;
            }
        }
        delete[] result->fields;
    }
    
    delete result;
}

} // extern "C"