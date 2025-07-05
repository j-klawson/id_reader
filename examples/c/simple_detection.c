/*
 * Universal ID Reader - Cross-platform ID document scanner
 * Copyright (C) 2025 J. Keith Lawson
 *
 * Example: Simple Document Detection (C)
 * 
 * This example demonstrates basic usage of the ID Reader library
 * from C code for document detection.
 */

#include <id_reader/id_reader.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple BMP header structure for loading test images
#pragma pack(push, 1)
typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BMPHeader;

typedef struct {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    int32_t x_pixels_per_meter;
    int32_t y_pixels_per_meter;
    uint32_t colors_used;
    uint32_t colors_important;
} BMPInfoHeader;
#pragma pack(pop)

// Simple function to load a 24-bit BMP image
uint8_t* load_bmp_image(const char* filename, int* width, int* height, int* stride) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not open file %s\n", filename);
        return NULL;
    }
    
    BMPHeader header;
    BMPInfoHeader info;
    
    if (fread(&header, sizeof(BMPHeader), 1, file) != 1 ||
        fread(&info, sizeof(BMPInfoHeader), 1, file) != 1) {
        printf("Error: Could not read BMP headers\n");
        fclose(file);
        return NULL;
    }
    
    if (header.type != 0x4D42 || info.bits_per_pixel != 24) {
        printf("Error: Only 24-bit BMP files are supported\n");
        fclose(file);
        return NULL;
    }
    
    *width = info.width;
    *height = info.height > 0 ? info.height : -info.height;
    *stride = ((*width * 3) + 3) & ~3; // 4-byte aligned
    
    uint8_t* image_data = malloc(*height * *stride);
    if (!image_data) {
        printf("Error: Could not allocate memory for image\n");
        fclose(file);
        return NULL;
    }
    
    fseek(file, header.offset, SEEK_SET);
    if (fread(image_data, *stride, *height, file) != (size_t)*height) {
        printf("Error: Could not read image data\n");
        free(image_data);
        fclose(file);
        return NULL;
    }
    
    fclose(file);
    return image_data;
}

int main(int argc, char* argv[]) {
    const char* image_path = NULL;
    
    // Use command line argument or default test pattern
    if (argc >= 2) {
        image_path = argv[1];
    }
    
    printf("Universal ID Reader - Simple Detection Example\n");
    printf("Version: %s\n", id_reader_version_string());
    printf("Copyright (C) 2025 J. Keith Lawson\n\n");
    
    // Initialize the library
    id_reader_context_t* context = NULL;
    id_reader_error_t error = id_reader_init(&context);
    if (error != ID_READER_SUCCESS) {
        printf("Error: Failed to initialize ID Reader: %s\n", id_reader_error_string(error));
        return 1;
    }
    
    printf("ID Reader initialized successfully\n");
    
    // Configure detection settings
    id_reader_set_config(context, "country", "US");
    id_reader_set_config(context, "document_type", "drivers_license");
    id_reader_set_config(context, "canny_threshold1", "50");
    id_reader_set_config(context, "canny_threshold2", "150");
    
    printf("Configuration applied\n");
    
    // Process image if provided
    if (image_path) {
        printf("Loading image: %s\n", image_path);
        
        int width, height, stride;
        uint8_t* image_data = load_bmp_image(image_path, &width, &height, &stride);
        
        if (!image_data) {
            printf("Error: Could not load image\n");
            id_reader_cleanup(context);
            return 1;
        }
        
        printf("Image loaded: %dx%d, stride: %d\n", width, height, stride);
        
        // Prepare image structure
        id_reader_image_t image;
        image.data = image_data;
        image.width = width;
        image.height = height;
        image.stride = stride;
        image.format = ID_READER_IMAGE_FORMAT_BGR; // BMP uses BGR format
        
        // Process the image
        id_reader_result_t* result = NULL;
        error = id_reader_process_image(context, &image, &result);
        
        if (error == ID_READER_SUCCESS && result) {
            printf("\n=== DETECTION RESULTS ===\n");
            printf("Document Type: %s\n", id_reader_document_type_string(result->document_type));
            printf("Country: %s\n", id_reader_country_string(result->country));
            printf("Overall Confidence: %.2f\n", result->overall_confidence);
            
            printf("\nDocument Bounds (normalized coordinates):\n");
            printf("  Top-left:     (%.3f, %.3f)\n", result->bounds.x1, result->bounds.y1);
            printf("  Top-right:    (%.3f, %.3f)\n", result->bounds.x2, result->bounds.y2);
            printf("  Bottom-right: (%.3f, %.3f)\n", result->bounds.x3, result->bounds.y3);
            printf("  Bottom-left:  (%.3f, %.3f)\n", result->bounds.x4, result->bounds.y4);
            printf("  Confidence:   %.2f\n", result->bounds.confidence);
            
            printf("\nDocument Bounds (pixel coordinates):\n");
            printf("  Top-left:     (%d, %d)\n", 
                   (int)(result->bounds.x1 * width), (int)(result->bounds.y1 * height));
            printf("  Top-right:    (%d, %d)\n", 
                   (int)(result->bounds.x2 * width), (int)(result->bounds.y2 * height));
            printf("  Bottom-right: (%d, %d)\n", 
                   (int)(result->bounds.x3 * width), (int)(result->bounds.y3 * height));
            printf("  Bottom-left:  (%d, %d)\n", 
                   (int)(result->bounds.x4 * width), (int)(result->bounds.y4 * height));
            
            if (result->field_count > 0) {
                printf("\nExtracted Fields:\n");
                for (size_t i = 0; i < result->field_count; i++) {
                    printf("  %s: %s (confidence: %.2f)\n",
                           result->fields[i].name,
                           result->fields[i].value,
                           result->fields[i].confidence);
                }
            } else {
                printf("\nNo text fields extracted (OCR not implemented yet)\n");
            }
            
            // Free the result
            id_reader_free_result(result);
            
        } else {
            printf("Error: Document detection failed: %s\n", id_reader_error_string(error));
            if (error == ID_READER_ERROR_NO_DOCUMENT_FOUND) {
                printf("Suggestion: Try adjusting detection parameters or using a clearer image\n");
            }
        }
        
        free(image_data);
        
    } else {
        printf("No image provided. Testing library functionality only.\n");
        printf("Usage: %s <image.bmp>\n", argv[0]);
        printf("\nSupported formats: 24-bit BMP files\n");
        printf("For best results, use images with clear document boundaries.\n");
    }
    
    // Cleanup
    id_reader_cleanup(context);
    printf("\nLibrary cleanup complete\n");
    
    return 0;
}