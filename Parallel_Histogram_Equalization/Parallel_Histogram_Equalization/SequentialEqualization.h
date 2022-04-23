#pragma once
#include "utility.h"
#define MAX_COLOR_VALUE 256

int* makeFrequancyArray(int* image, int pixelsNumber)
{
	int* frequancyArray = new int[MAX_COLOR_VALUE] {};

	for (int i = 0; i < pixelsNumber; i++)
	{
		frequancyArray[image[i]]++;
	}
	return frequancyArray;
}

double* calculatePixelProbability(int* frequancyArray, int pixelCount)
{
	double* pixelProbability = new double[MAX_COLOR_VALUE] {};
	for (int i = 0; i < MAX_COLOR_VALUE; i++)
	{
		pixelProbability[i] = (double)frequancyArray[i] / pixelCount;
	}
	return pixelProbability;
}
double* calculateCumulativeProbability(double* pixelProbability)
{
	double* cumulativeProbability = new double[MAX_COLOR_VALUE] {};
	cumulativeProbability[0] = pixelProbability[0];
	for (int i = 1; i < MAX_COLOR_VALUE; i++)
	{
		cumulativeProbability[i] = pixelProbability[i] + cumulativeProbability[i - 1];
	}
	return cumulativeProbability;
}

int* putInRange(double* cumulativeProbability, int range)
{
	int* equalizedIntenisties = new int[MAX_COLOR_VALUE] {};
	for (int i = 1; i < MAX_COLOR_VALUE; i++)
	{
		equalizedIntenisties[i] = floor(cumulativeProbability[i] * range);
	}
	return equalizedIntenisties;
}

int* equalizeImage(int* image, int* intenisties, int pixelCount)
{
	for (int i = 0; i < pixelCount; i++)
	{
		image[i] = intenisties[image[i]];
	}
	return image;
}

int* sequentialEqualization(int* image, int width, int height)
{
	int pixelCount = width * height;
	int* frequancyArray = makeFrequancyArray(image, pixelCount);

	double* pixelProbability = calculatePixelProbability(frequancyArray, pixelCount);
	double* cumulativeProbability = calculateCumulativeProbability(pixelProbability);

	int* equalizedIntenisties = putInRange(cumulativeProbability, 240);

	int* finalImage = equalizeImage(image, equalizedIntenisties, pixelCount);

	return finalImage;
}

void sequentialRunAndClock(int* image, int width, int height, int imageIndex)
{
	int start_s, stop_s, TotalTime = 0;

	start_s = clock();

	image = sequentialEqualization(image, width, height);

	createImage(image, width, height, imageIndex);

	stop_s = clock();
	TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;

	writeResultsToFile(TotalTime, width, height);

	free(image);
}

