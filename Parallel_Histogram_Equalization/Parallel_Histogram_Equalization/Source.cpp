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
		//img = TEST_PATH;
		img = "..//Data//Input//5N.png";
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

	int start_s, stop_s, TotalTime = 0;
	if (isMainProcessor(rank))
		start_s = clock();

	for (int i = 0; i < 10; i++)
	{
		parallelEqualization(rank, WORLD_SIZE, imageData, ImageWidth, ImageHeight, PIXELS_COUNT, INTENSITY_RANGE);
		
	}
	if (isMainProcessor(rank))
	{
		stop_s = clock();
		TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
		writeResultsToFile(TotalTime, ImageWidth, ImageHeight, WORLD_SIZE);
	}

	//cleanUp(localFrequancyArray, localImage, imageData, totalFrequancyArray);
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

