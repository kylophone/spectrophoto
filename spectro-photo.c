#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

void column_to_PCM(FILE *sound_out, float *column_intensity, int y, int sample_rate) {
    float top_freq = (float) sample_rate / 2;
    float y_slice =  top_freq / y;
    int column_width = sample_rate / 25;
    float buf_out[column_width];
    float sample = 0;
    for (int i = 0; i < column_width; i++) {
        float envelope_multiplier = sin(M_PI * ((float) i /  column_width));
        for (int j = 0; j < y; j++) {
            float freq = top_freq - (y_slice * j);
            sample += (column_intensity[j] * (sin(2 * M_PI * freq *  ((float) i / sample_rate)) * envelope_multiplier)) / y;
            sample *= .99;
        }
        buf_out[i] = sample;
    }
    fwrite(&buf_out, sizeof(float) * column_width, 1, sound_out);
}

int get_point_index(int this_x, int this_y, int x, int n) {
    return (this_y * (x * n)) + this_x * n;
}


float get_pixel_intensity(unsigned char *data, int n) {
    int RGB_sum = 0;
    for (int i = 0; i < n; i++) {
        RGB_sum += data[i];
    }
    float intensity = ((float) RGB_sum / n) / 255;
    return intensity;
}


int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: spectrophoto <inputfile> <outputfile>.\n");
        return(-1);
    }

    const char *inputFile = argv[1];
    const char *outputFile = argv[2];
    int x,y,n;
    unsigned char *data = stbi_load(inputFile, &x, &y, &n, 1);

    if (!data) {
        fprintf(stderr, "Couldn't load image.\n"); 
        return(-1);
    }

    FILE *file = fopen(outputFile, "wb");

    if (!file) {
        fprintf(stderr, "Couldn't open file.\n"); 
        return(-1);
    }

    for (int this_x = 0; this_x < x; this_x++) {
        float this_column_intensity[y];
        for (int this_y = 0; this_y < y; this_y++) {
            int this_index = get_point_index(this_x, this_y, x, 1);
            float this_intensity = get_pixel_intensity(&data[this_index], 1);
            this_column_intensity[this_y] = this_intensity;
        }
        column_to_PCM(file, this_column_intensity, y, 48000);
    }

    fclose(file);
    stbi_image_free(data);
    return 0;
}
