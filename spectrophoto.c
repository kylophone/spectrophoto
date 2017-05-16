#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 48000

void column_to_PCM(float *buf, float *column, int nb_samples, int height, float **sin_lut)
{
  float sample = 0.;

  for (int i = 0; i < nb_samples; i++) {
    for (int j = 0; j < height; j++) {
      sample += (column[j] * sin_lut[j][i]) / height;
    }
    sample *= .90;
    buf[i] = sample;
  }
}

int get_point_index(int i, int j, int x, int n)
{
  return (j * (x * n)) + i * n;
}

float get_pixel_intensity(unsigned char *img, int n)
{
  int RGB_sum = 0;
  for (int i = 0; i < n; i++) {
    RGB_sum += img[i];
  }
  float intensity = ((float) RGB_sum / n) / 255;
  return intensity;
}

int main(int argc, char **argv)
{
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <inputfile> <outputfile> <duration>\n", argv[0]);
    return 1;
  }

  int x,y,n;
  const char *input_file = argv[1];
  unsigned char *img = stbi_load(input_file, &x, &y, &n, 1);
  if (!img) {
    fprintf(stderr, "Couldn't load image: %s\n", input_file);
    return 1;
  }

  int duration = atoi(argv[3]);
  int nb_samples = (duration * SAMPLE_RATE) / x;
  if (nb_samples <= 0) {
    fprintf(stderr, "Duration is too low: %d\n", duration);
    return 1;
  }

  const char *output_file = argv[2];
  FILE *pcm = fopen(output_file, "wb");
  if (!pcm) {
    fprintf(stderr, "Couldn't open file: %s\n", output_file);
    return 1;
  }

  float *column = malloc(sizeof(float) * y);
  if (!column) {
    fprintf(stderr, "Couldn't allocate buffer.\n");
    return 1;
  }

  float *buf = malloc(sizeof(float) * nb_samples);
  if (!buf) {
    fprintf(stderr, "Couldn't allocate buffer.\n");
    return 1;
  }

  float **sin_lut = malloc(sizeof(float*) * y);
  if (!sin_lut) {
    fprintf(stderr, "Couldn't allocate buffer.\n");
    return 1;
  }

  for (int i = 0; i < y; i++) {
    sin_lut[i] = malloc(sizeof(float) * nb_samples);
    if (!sin_lut[i]) {
      fprintf(stderr, "Couldn't allocate buffer.\n");
      return 1;
    }
  }

  float nyquist = (float) SAMPLE_RATE / 2;
  float hz_step = nyquist / y;
  for (int i = 0; i < y; i++) {
    for (int j = 0; j < nb_samples; j++) {
      float freq = nyquist - (hz_step * i);
      float env = sin(2 * M_PI * ((float) j / nb_samples));
      sin_lut[i][j] = env * sin(2 * M_PI * freq * ((float) j / SAMPLE_RATE));
    }
  }

  for (int i = 0; i < x; i++) {
    for (int j = 0; j < y; j++) {
      int index = get_point_index(i, j, x, 1);
      float intensity = get_pixel_intensity(&img[index], 1);
      column[j] = intensity;
    }
    column_to_PCM(buf, column, nb_samples, y, sin_lut);
    fwrite(buf, sizeof(float) * nb_samples, 1, pcm);
  }

  for (int i = 0; i < y; i++)
    free(sin_lut[i]);
  free(sin_lut);

  free(buf);
  free(column);
  stbi_image_free(img);

  fclose(pcm);
  return 0;
}
