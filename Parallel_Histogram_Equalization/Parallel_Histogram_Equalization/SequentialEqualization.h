#pragma once
#include "utility.h"
#define MAX_INTENSITY_VALUE 256

int* makeFrequancyArray(int* image, int pixelsNumber)
{
	int* frequancyArray = new int[MAX_INTENSITY_VALUE] {};

	for (int i = 0; i < pixelsNumber; i++)
	{
		frequancyArray[image[i]]++;
	}
	return frequancyArray;
}

/* This overload is made to account for non divisble pixels to processor counts*/
int* makeFrequancyArray(int* image, int pixelsNumber, int paddingSize)
{
	int* frequancyArray = new int[MAX_INTENSITY_VALUE] {};

	for (int i = 0; i < pixelsNumber - paddingSize; i++)
	{
		frequancyArray[image[i]]++;
	}
	return frequancyArray;
}

double* calculateColorProbability(int* frequancyArray, int pixelCount)
{
	double* colorProbability = new double[MAX_INTENSITY_VALUE] {};
	for (int i = 0; i < MAX_INTENSITY_VALUE; i++)
	{
		colorProbability[i] = (double)frequancyArray[i] / pixelCount;
	}
	return colorProbability;
}
double* calculateCumulativeProbability(double* colorProbability)
{
	double* cumulativeProbability = new double[MAX_INTENSITY_VALUE] {};
	cumulativeProbability[0] = colorProbability[0];
	for (int i = 1; i < MAX_INTENSITY_VALUE; i++)
	{
		cumulativeProbability[i] = colorProbability[i] + cumulativeProbability[i - 1];
	}
	return cumulativeProbability;
}

double* calculateCumulativeProbability(double* colorProbability, int size)
{
	double* cumulativeProbability = new double[size] {};
	cumulativeProbability[0] = colorProbability[0];
	for (int i = 1; i < size; i++)
	{
		cumulativeProbability[i] = colorProbability[i] + cumulativeProbability[i - 1];
	}
	return cumulativeProbability;
}

int* putInRange(double* cumulativeProbability, int range)
{
	int* equalizedIntenisties = new int[MAX_INTENSITY_VALUE] {};
	for (int i = 0; i < MAX_INTENSITY_VALUE; i++)
	{
		equalizedIntenisties[i] = floor(cumulativeProbability[i] * range);
	}
	return equalizedIntenisties;
}

int* putInRangeV(double* cumulativeProbability, int range, int NUMBER_OF_INTENSITY_VALUES)
{
	int* equalizedIntenisties = new int[NUMBER_OF_INTENSITY_VALUES] {};
	for (int i = 0; i < NUMBER_OF_INTENSITY_VALUES; i++)
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

int* sequentialEqualization(int* image, int width, int height, int intenistyRange)
{
	int pixelCount = width * height;

	int* frequancyArray = makeFrequancyArray(image, pixelCount);

	double* colorProbability = calculateColorProbability(frequancyArray, pixelCount);
	double* cumulativeProbability = calculateCumulativeProbability(colorProbability);

	int* equalizedIntenisties = putInRange(cumulativeProbability, intenistyRange);

	int* finalImage = equalizeImage(image, equalizedIntenisties, pixelCount);
	return finalImage;
}

void sequentialRunAndClock(int* image, int width, int height, int imageIndex, int intenistyRange, int processorCount)
{
	int start_s, stop_s, TotalTime = 0;

	start_s = clock();

	image = sequentialEqualization(image, width, height, intenistyRange);


	stop_s = clock();
	TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;

	createImage(image, width, height, imageIndex);
	writeResultsToFile(TotalTime, width, height, processorCount);

	free(image);
}

