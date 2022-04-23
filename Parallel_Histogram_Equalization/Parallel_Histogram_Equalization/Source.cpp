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
#include "SequentialEqualization.h"


#using <mscorlib.dll>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
using namespace std;
using namespace msclr::interop;


bool RUNNING_PARALLEL;

int main()
{
	MPI_Init(NULL, NULL);
	int world_size, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	RUNNING_PARALLEL = world_size > 1;

	int ImageWidth = 4, ImageHeight = 4;
	int* imageData = nullptr;

	if (isMainProcessor(rank))
	{
		System::String^ imagePath;
		std::string img;

		setTestImageN(1);
		img = TEST_PATH;

		imagePath = marshal_as<System::String^>(img);
		imageData = inputImage(&ImageWidth, &ImageHeight, imagePath);
	}
	

	if (!RUNNING_PARALLEL)
	{
		sequentialRunAndClock(imageData, ImageWidth, ImageHeight, 0);
	}

	
	MPI_Finalize();
	return 0;

}



