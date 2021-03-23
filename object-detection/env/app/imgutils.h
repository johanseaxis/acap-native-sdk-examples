/**
 * Copyright (C) 2021, Axis Communications AB, Lund, Sweden
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <jpeglib.h>

/**
 * brief Encode an image buffer as jpeg and store it in memory
 *
 * param image_buffer An image buffer with interleaved (if RGB) channel layout
 * param jpeg_conf A struct defining how the image is to be encoded
 * param jpeg_size The output size of the jpeg
 * param jpeg_buffer The output buffer of the jpeg
 */
void buffer_to_jpeg(unsigned char* image_buffer, struct jpeg_compress_struct* jpeg_conf,
                    unsigned long* jpeg_size, unsigned char** jpeg_buffer);


/**
 * brief Inserts common values into a jpeg configuration struct
 *
 * param width The width of the image
 * param height The height of the image
 * param channels The number of channels of the image
 * param quality The desired jpeg quality (0-100)
 * param jpeg_conf The jpeg configuration struct to modify
 */
void set_jpeg_configuration(int width, int height, int channels, int quality,
                            struct jpeg_compress_struct* jpeg_conf);


/**
 * brief Writes a memory buffer to a file
 *
 * param file_name The desired path of the output file
 * param buffer The data to be written
 * param buffer_size The size of the data to be written
 */
void jpeg_to_file(char* file_name, unsigned char* buffer, unsigned long buffer_size);

unsigned char* crop_interleaved(unsigned char* image_buffer, int image_w,
                                int image_h, int channels,
                                int crop_x, int crop_y,
                                int crop_w, int crop_h);


/**
 * brief An example of how to use the supplied utility functions
 *
 */
void test_buffer_to_jpeg_file();
