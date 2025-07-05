/*
 * Universal ID Reader - Cross-platform ID document scanner
 * Copyright (C) 2025 J. Keith Lawson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef ID_READER_H
#define ID_READER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Version information
#define ID_READER_VERSION_MAJOR 1
#define ID_READER_VERSION_MINOR 0
#define ID_READER_VERSION_PATCH 0

// Error codes
typedef enum {
    ID_READER_SUCCESS = 0,
    ID_READER_ERROR_INVALID_INPUT = -1,
    ID_READER_ERROR_MEMORY_ALLOCATION = -2,
    ID_READER_ERROR_PROCESSING_FAILED = -3,
    ID_READER_ERROR_NO_DOCUMENT_FOUND = -4,
    ID_READER_ERROR_UNSUPPORTED_FORMAT = -5,
    ID_READER_ERROR_INITIALIZATION_FAILED = -6
} id_reader_error_t;

// Document types
typedef enum {
    ID_READER_DOCUMENT_UNKNOWN = 0,
    ID_READER_DOCUMENT_DRIVERS_LICENSE = 1,
    ID_READER_DOCUMENT_PASSPORT = 2,
    ID_READER_DOCUMENT_ID_CARD = 3,
    ID_READER_DOCUMENT_CREDIT_CARD = 4
} id_reader_document_type_t;

// Country codes (ISO 3166-1 alpha-2)
typedef enum {
    ID_READER_COUNTRY_UNKNOWN = 0,
    ID_READER_COUNTRY_US = 1,
    ID_READER_COUNTRY_CA = 2,
    ID_READER_COUNTRY_GB = 3,
    ID_READER_COUNTRY_DE = 4,
    ID_READER_COUNTRY_FR = 5,
    ID_READER_COUNTRY_AU = 6
} id_reader_country_t;

// Image format
typedef enum {
    ID_READER_IMAGE_FORMAT_RGB = 0,
    ID_READER_IMAGE_FORMAT_RGBA = 1,
    ID_READER_IMAGE_FORMAT_BGR = 2,
    ID_READER_IMAGE_FORMAT_BGRA = 3,
    ID_READER_IMAGE_FORMAT_GRAYSCALE = 4
} id_reader_image_format_t;

// Image data structure
typedef struct {
    uint8_t* data;
    size_t width;
    size_t height;
    size_t stride;
    id_reader_image_format_t format;
} id_reader_image_t;

// Document bounds
typedef struct {
    float x1, y1, x2, y2, x3, y3, x4, y4;  // Four corners
    float confidence;
} id_reader_document_bounds_t;

// Extracted field data
typedef struct {
    char* name;
    char* value;
    float confidence;
    int x, y, width, height;  // Bounding box
} id_reader_field_t;

// Processing result
typedef struct {
    id_reader_document_type_t document_type;
    id_reader_country_t country;
    id_reader_document_bounds_t bounds;
    id_reader_field_t* fields;
    size_t field_count;
    float overall_confidence;
} id_reader_result_t;

// Library context (opaque)
typedef struct id_reader_context id_reader_context_t;

// Initialization and cleanup
id_reader_error_t id_reader_init(id_reader_context_t** context);
void id_reader_cleanup(id_reader_context_t* context);

// Configuration
id_reader_error_t id_reader_set_config(id_reader_context_t* context, const char* key, const char* value);
id_reader_error_t id_reader_get_config(id_reader_context_t* context, const char* key, char* value, size_t value_size);

// Document processing
id_reader_error_t id_reader_process_image(
    id_reader_context_t* context,
    const id_reader_image_t* image,
    id_reader_result_t** result
);

// Result management
void id_reader_free_result(id_reader_result_t* result);

// Utility functions
const char* id_reader_error_string(id_reader_error_t error);
const char* id_reader_document_type_string(id_reader_document_type_t type);
const char* id_reader_country_string(id_reader_country_t country);

// Version information
const char* id_reader_version_string(void);
int id_reader_version_major(void);
int id_reader_version_minor(void);
int id_reader_version_patch(void);

#ifdef __cplusplus
}
#endif

#endif // ID_READER_H