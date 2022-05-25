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


#define ENABLE_DEBUG 0 
void parallelEqualization(int rank, int WORLD_SIZE, int* imageData,int ImageWidth, int ImageHeight, int PIXELS_COUNT, int INTENSITY_RANGE)
{

#pragma region Init
	int start_s, stop_s, TotalTime = 0;
	if (isMainProcessor(rank))
		start_s = clock();
#pragma endregion


	/*When using PIXELS_PER_PROCESSOR note that in case of non divisible pixels count
	the remainder is added to the PIXELS_PER_PROCESSOR but should only be processed by MAIN_PROCESSOR*/
	int DIVISION_REMAINDER = (PIXELS_COUNT % WORLD_SIZE);
	int PIXELS_PER_PROCESSOR = PIXELS_COUNT / WORLD_SIZE + DIVISION_REMAINDER;
	MPI_Bcast(&PIXELS_PER_PROCESSOR, 1, MPI_INT, MAIN_PROCESSOR, MPI_COMM_WORLD);
	MPI_Bcast(&DIVISION_REMAINDER, 1, MPI_INT, MAIN_PROCESSOR, MPI_COMM_WORLD);

	int INTENSITIES_PER_PROCESSOR = ceil(MAX_INTENSITY_VALUE / WORLD_SIZE);
	MPI_Bcast(&INTENSITIES_PER_PROCESSOR, 1, MPI_INT, MAIN_PROCESSOR, MPI_COMM_WORLD);

	int PADDED_SUBARRAY_SIZE = ceil((float)MAX_INTENSITY_VALUE / WORLD_SIZE);
	int PADDED_ARRAY_SIZE = PADDED_SUBARRAY_SIZE * WORLD_SIZE;




#pragma region Frequancy Array

	int* counts = new int[WORLD_SIZE];
	int* displacement = new int[WORLD_SIZE] {};

	if (isMainProcessor(rank))
	{
		counts = calculateDistributionCounts(PIXELS_PER_PROCESSOR, WORLD_SIZE, DIVISION_REMAINDER);
		displacement = calculateDistributionDisplacements(counts, PIXELS_PER_PROCESSOR, WORLD_SIZE, DIVISION_REMAINDER);
	}
	int* localImage = new int[PIXELS_PER_PROCESSOR] {};

	MPI_Scatterv(imageData, counts, displacement, MPI_INT, localImage, PIXELS_PER_PROCESSOR, MPI_INT, MAIN_PROCESSOR, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	int* localFrequancyArray;
	if (isMainProcessor(rank))
		localFrequancyArray = makeFrequancyArray(localImage, PIXELS_PER_PROCESSOR, 0);
	else
		localFrequancyArray = makeFrequancyArray(localImage, PIXELS_PER_PROCESSOR, DIVISION_REMAINDER);
	MPI_Barrier(MPI_COMM_WORLD);

	int* totalFrequancyArray = new int[MAX_INTENSITY_VALUE] {};
	MPI_Reduce(localFrequancyArray, totalFrequancyArray, MAX_INTENSITY_VALUE, MPI_INT, MPI_SUM, MAIN_PROCESSOR, MPI_COMM_WORLD);

	if (isMainProcessor(rank) && ENABLE_DEBUG)
		verifyFrequancyArray(totalFrequancyArray);
#pragma endregion

#pragma region calculate Color probability
	MPI_Bcast(&ImageHeight, 1, MPI_DOUBLE, MAIN_PROCESSOR, MPI_COMM_WORLD);
	MPI_Bcast(&ImageWidth, 1, MPI_DOUBLE, MAIN_PROCESSOR, MPI_COMM_WORLD);

	double* localProbabilites = new  double[PADDED_SUBARRAY_SIZE] {};
	int* localFrequencyArray = new  int[PADDED_SUBARRAY_SIZE];

	MPI_Scatter(totalFrequancyArray, PADDED_SUBARRAY_SIZE, MPI_INT,
		localFrequancyArray, PADDED_SUBARRAY_SIZE, MPI_INT, MAIN_PROCESSOR, MPI_COMM_WORLD);


	for (int i = 0; i < PADDED_SUBARRAY_SIZE; i++)
		localProbabilites[i] = (double)(localFrequancyArray[i] / ((double)ImageHeight * ImageWidth));




	double* colorProbability = new  double[PADDED_ARRAY_SIZE] {};

	MPI_Gather(localProbabilites, PADDED_SUBARRAY_SIZE, MPI_DOUBLE, colorProbability,
		PADDED_SUBARRAY_SIZE, MPI_DOUBLE, MAIN_PROCESSOR, MPI_COMM_WORLD);
	//verify
	if (isMainProcessor(rank) && ENABLE_DEBUG)
		verifyColorProbability(colorProbability);



#pragma endregion

#pragma region Cumulative Probability

	double* cumulativeProbability = new double[PADDED_ARRAY_SIZE];


	if (isMainProcessor(rank))
	{
		colorProbability = calculateColorProbability(totalFrequancyArray, PIXELS_COUNT);
	}

	double* localColorProbability = new double[PADDED_SUBARRAY_SIZE];


	MPI_Scatter(colorProbability, PADDED_SUBARRAY_SIZE, MPI_DOUBLE, localColorProbability, PADDED_SUBARRAY_SIZE,
		MPI_DOUBLE, MAIN_PROCESSOR, MPI_COMM_WORLD);

	double* localCumulativeProbability = calculateCumulativeProbability(localColorProbability, PADDED_SUBARRAY_SIZE);

	double prevSum = 0;
	double cumulativeSum = localCumulativeProbability[PADDED_SUBARRAY_SIZE - 1];

	MPI_Exscan(&cumulativeSum, &prevSum, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

	for (int i = 0; i < PADDED_SUBARRAY_SIZE; i++)localCumulativeProbability[i] += prevSum;

	MPI_Gather(localCumulativeProbability, PADDED_SUBARRAY_SIZE, MPI_DOUBLE, cumulativeProbability, PADDED_SUBARRAY_SIZE,
		MPI_DOUBLE, MAIN_PROCESSOR, MPI_COMM_WORLD);

	if (isMainProcessor(rank) && ENABLE_DEBUG)
		verifyCumulativeProbability(cumulativeProbability);


#pragma endregion

#pragma region Equalize Intensities 
	/* This is probably usesless beacuse the overhead of parallelizing an operatation that is applied only 256 times probably takes more
	time than just doing it sequentially */


	int* localIntensities = putInRangeV(localCumulativeProbability, INTENSITY_RANGE, PADDED_SUBARRAY_SIZE);
	int* totalIntensities = new int[PADDED_ARRAY_SIZE] {};
	MPI_Gather(localIntensities, PADDED_SUBARRAY_SIZE, MPI_INT,
		totalIntensities, PADDED_SUBARRAY_SIZE, MPI_INT, MAIN_PROCESSOR, MPI_COMM_WORLD);

	if (isMainProcessor(rank) && ENABLE_DEBUG)
		verifyEqualizedIntenisties(totalIntensities);

#pragma endregion

#pragma region apply on image

	MPI_Bcast(totalIntensities, PADDED_ARRAY_SIZE, MPI_INT, MAIN_PROCESSOR, MPI_COMM_WORLD);

	if (isMainProcessor(rank))
		localImage = equalizeImage(localImage, totalIntensities, PIXELS_PER_PROCESSOR);
	else
		localImage = equalizeImage(localImage, totalIntensities, PIXELS_PER_PROCESSOR - DIVISION_REMAINDER);

	/* Because main processor operates on different number of pixels in case of non divisble pixels count
	Gather malfunctions when provided with different number of recv buffer size than the numbers in "counts" */
	if (isMainProcessor(rank))
	{
		for (int i = 0; i < counts[0]; i++)
			imageData[i] = localImage[i];
		counts[0] = 0;
	}

	MPI_Gatherv(localImage, PIXELS_PER_PROCESSOR - DIVISION_REMAINDER, MPI_INT, imageData, counts, displacement, MPI_INT, MAIN_PROCESSOR, MPI_COMM_WORLD);

	if (isMainProcessor(rank) && ENABLE_DEBUG)
		verifyFinalImage(imageData, PIXELS_COUNT);

#pragma endregion

#pragma region Finalize
	if (isMainProcessor(rank))
	{
		stop_s = clock();
		TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;

		//createImage(imageData, ImageWidth, ImageHeight, 1);

		//writeResultsToFile(TotalTime, ImageWidth, ImageHeight, WORLD_SIZE);
	}
#pragma endregion

}