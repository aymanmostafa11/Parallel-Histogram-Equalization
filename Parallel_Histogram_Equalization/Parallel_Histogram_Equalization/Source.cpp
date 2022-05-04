#include <iostream>
#include <math.h>
#include <stdlib.h>
#include<string.h>
#include<msclr\marshal_cppstd.h>
#include <ctime> 
#include <mpi.h>
#include <stdio.h>
#pragma once


#include "utility.h"
#include "imageUtility.h"
#include "SequentialEqualization.h"


#using <mscorlib.dll>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
using namespace std;
using namespace msclr::interop;


#define MAX_INTENSITY_VALUE 256

#define MAIN_PROCESSOR 0
bool RUNNING_PARALLEL;


#define ENABLE_DEBUG 0
// prints a message from each processor where x -> message (or expression in cout form eg. foo << bar)
#define debug(x) if(ENABLE_DEBUG) {cout << "Rank : " << rank << " " << x << endl;}

#define debugMainProc(rank, x) if(ENABLE_DEBUG && rank == MAIN_PROCESSOR){ cout << "Rank : " << rank << " " << x << endl;}
#define PAUSE(rank) {if (ENABLE_DEBUG && rank == MAIN_PROCESSOR){ char tmp; cin>>tmp;} MPI_Barrier(MPI_COMM_WORLD);}

void cleanUp(int* localFrequancyArray, int* localImage, int* imageData, int* totalFrequancyArray);


int main()
{
	MPI_Init(NULL, NULL);
	int WORLD_SIZE, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &WORLD_SIZE);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	RUNNING_PARALLEL = WORLD_SIZE > 1;

	int ImageWidth = 4, ImageHeight = 4;
	int PIXELS_COUNT = 0;
	int* imageData = nullptr;
	int INTENSITY_RANGE = 250;


	if (isMainProcessor(rank))
	{
		System::String^ imagePath;
		std::string img;

		setTestImageN(10);
		img = TEST_PATH;

		imagePath = marshal_as<System::String^>(img);
		imageData = inputImage(&ImageWidth, &ImageHeight, imagePath);
		PIXELS_COUNT = ImageHeight * ImageWidth;
	}


	if (!RUNNING_PARALLEL)
	{
		int intenistyRange = 240;
		sequentialRunAndClock(imageData, ImageWidth, ImageHeight, 0, intenistyRange, WORLD_SIZE);
		MPI_Finalize();
		return 0;
	}
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

		createImage(imageData, ImageWidth, ImageHeight, 1);

		writeResultsToFile(TotalTime, ImageWidth, ImageHeight, WORLD_SIZE);
	}
#pragma endregion

	cleanUp(localFrequancyArray, localImage, imageData, totalFrequancyArray);
	MPI_Finalize();
	return 0;

}

void cleanUp(int* localFrequancyArray, int* localImage, int* imageData, int* totalFrequancyArray)
{
	delete localFrequancyArray;
	delete localImage;
	delete imageData;
	delete totalFrequancyArray;
}

