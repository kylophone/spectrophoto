#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void column_to_PCM(FILE *sound_out, float *column_intensity, int x, int y, int sample_rate) {
	float top_freq = (float) sample_rate / 2;
	float y_slice =  top_freq / x;
	int column_length = (sample_rate / 5) / 5;
	float sample = 0;
	for (int i = 0; i < column_length; i++) {
		float envelope_multiplier = sin(M_PI * ((float) i / (float) column_length));
		for (int j = 0; j < x; j++) {
			float freq = top_freq - (y_slice * j);
			sample += (column_intensity[j] * (sin(2 * M_PI * freq *  i / sample_rate) * envelope_multiplier)) / x;
		}
		//printf("%f\n", sample);
		fwrite(&sample, sizeof(float), 1, sound_out);
	}
}

int get_point_index(int this_x, int this_y, int x, int n) {
	return (this_y * (x * n)) + this_x;
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
	const char *inputFile = argv[1];
	const char *outputFile = argv[2];
	int x,y,n;
	unsigned char *data = stbi_load(inputFile, &x, &y, &n, 0);

	if (!data) {
		fprintf(stderr, "Couldn't load image.\n"); 
		return (-1);
	}

	//printf("%d %d %d\n", x, y, n);

	FILE *file = fopen(outputFile, "wb");

	for (int this_x = 0; this_x < x; this_x++) {
		float this_column_intensity[y];
		for (int this_y = 0; this_y < y; this_y++) {
			int this_index = get_point_index(this_x, this_y, x, n);
			float this_intensity = get_pixel_intensity(&data[this_index], n);
			this_column_intensity[this_y] = this_intensity;
		}
		column_to_PCM(file, this_column_intensity, x, y, 48000);
	}

	fclose(file);
	stbi_image_free(data);
	return 0;
}