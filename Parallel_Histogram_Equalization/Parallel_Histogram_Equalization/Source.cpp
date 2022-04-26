#include <iostream>
#include <math.h>
#include <stdlib.h>
#include<string.h>
#include<msclr\marshal_cppstd.h>
#include <ctime>// include this header 
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


#define ENABLE_DEBUG 2
// prints a message from each processor where x -> message (or expression in cout form eg. foo << bar)
#define debug(x) if(ENABLE_DEBUG) cout << "Rank : " << rank << " " << x << "\n";

#define debugMainProc(rank, x) if(ENABLE_DEBUG && rank == MAIN_PROCESSOR){ cout << "Rank : " << rank << " " << x << "\n";}
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
	int INTENSITY_RANGE = 240;


	if (isMainProcessor(rank))
	{
		System::String^ imagePath;
		std::string img;

		setTestImageN(1);
		img = TEST_PATH;

		imagePath = marshal_as<System::String^>(img);
		imageData = inputImage(&ImageWidth, &ImageHeight, imagePath);
		PIXELS_COUNT = ImageHeight * ImageWidth;
	}


	if (!RUNNING_PARALLEL)
	{
		int intenistyRange = 240;
		sequentialRunAndClock(imageData, ImageWidth, ImageHeight, 0, intenistyRange);
		MPI_Finalize();
		return 0;
	}


	int PIXELS_PER_PROCESSOR = ceil(PIXELS_COUNT / WORLD_SIZE);
	MPI_Bcast(&PIXELS_PER_PROCESSOR, 1, MPI_INT, MAIN_PROCESSOR, MPI_COMM_WORLD);

	int INTENSITIES_PER_PROCESSOR = ceil(MAX_INTENSITY_VALUE / WORLD_SIZE);
	MPI_Bcast(&INTENSITIES_PER_PROCESSOR, 1, MPI_INT, MAIN_PROCESSOR, MPI_COMM_WORLD);



#pragma region Frequancy Array

	int* localImage = new int[PIXELS_PER_PROCESSOR] {};

	MPI_Scatter(imageData, PIXELS_PER_PROCESSOR, MPI_INT, localImage, PIXELS_PER_PROCESSOR, MPI_INT, MAIN_PROCESSOR, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	int* localFrequancyArray;
	localFrequancyArray = makeFrequancyArray(localImage, PIXELS_PER_PROCESSOR);
	MPI_Barrier(MPI_COMM_WORLD);

	int* totalFrequancyArray = new int[MAX_INTENSITY_VALUE] {};
	MPI_Reduce(localFrequancyArray, totalFrequancyArray, MAX_INTENSITY_VALUE, MPI_INT, MPI_SUM, MAIN_PROCESSOR, MPI_COMM_WORLD);

	if (isMainProcessor(rank) && ENABLE_DEBUG)
		verifyFrequancyArray(totalFrequancyArray);
#pragma endregion

#pragma region Equalize Intensities 

	// NOTE: currently doesn't work if number of processors is not a power of 2, shall be fixed soon (fr)


	/* This is probably usesless beacuse the overhead of parallelizing an operatation that is applied only 256 times probably takes more
	time than just doing it sequentially */

	/* Temporarily done sequentially becuase i can't work without them */
	double* cumulativeProbability = NULL;
	if (isMainProcessor(rank))
	{
		double* colorProbability = calculateColorProbability(totalFrequancyArray, PIXELS_COUNT);
		cumulativeProbability = calculateCumulativeProbability(colorProbability);
	}

	double* localCumulativeProbabilites = new double[INTENSITIES_PER_PROCESSOR] {};

	MPI_Scatter(cumulativeProbability, INTENSITIES_PER_PROCESSOR, MPI_DOUBLE,
		localCumulativeProbabilites, INTENSITIES_PER_PROCESSOR, MPI_DOUBLE, MAIN_PROCESSOR, MPI_COMM_WORLD);


	int* localIntensities = putInRangeV(localCumulativeProbabilites, INTENSITY_RANGE, INTENSITIES_PER_PROCESSOR);

	int* totalIntensities = new int[MAX_INTENSITY_VALUE];
	MPI_Gather(localIntensities, INTENSITIES_PER_PROCESSOR, MPI_INT,
		totalIntensities, INTENSITIES_PER_PROCESSOR, MPI_INT, MAIN_PROCESSOR, MPI_COMM_WORLD);

	if (isMainProcessor(rank) && ENABLE_DEBUG)
		verifyEqualizedIntenisties(totalIntensities);

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


