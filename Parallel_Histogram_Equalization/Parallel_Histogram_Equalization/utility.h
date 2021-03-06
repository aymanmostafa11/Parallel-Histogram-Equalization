#pragma once
#include <fstream>

using namespace std;

#define MAIN_PROCESSOR 0

#define TEST_N "..//Data//Input//NxN.png"
#define TEST_5N "..//Data//Input//5Nx5N.png"
#define TEST_10N "..//Data//Input//10Nx10N.png"

#define MAX_INTENSITY_VALUE 256


string TEST_PATH;
int N;

// N represents the test size (eg. NxN image, 5Nx5N image etc..)
void setTestImageN(int n)
{

	switch (n)
	{
	case 1:
		N = 1;
		TEST_PATH = TEST_N;
		break;
	case 5:
		N = 5;
		TEST_PATH = TEST_5N;
		break;
	case 10:
		N = 10;
		TEST_PATH = TEST_10N;
		break;
	default:
		N = 1;
		break;
	}
}

void writeResultsToFile(int time, int width, int height, int processorCount)
{
	ofstream file("..//Data//Output//results.txt", ios::app);
	file << "Size: (";
	file << width;
	file << " , ";
	file << height;
	file << ") , Time : ";
	file << time;
	file << "ms ,";
	file << "Processor Count : " << processorCount << "\n";
	file.close();
}


bool isMainProcessor(int rank)
{
	return rank == MAIN_PROCESSOR;
}

int* calculateDistributionCounts(int pixelsPerProcessor, int worldSize, int paddingSize)
{
	int* counts = new int[worldSize];
	counts[0] = pixelsPerProcessor;
	for (int i = 1; i < worldSize; i++)
	{
		counts[i] = pixelsPerProcessor - paddingSize;
	}
	return counts;
}
int* calculateDistributionDisplacements(int* counts, int pixelsPerProcessor, int worldSize, int paddingSize)
{
	int* displacement = new int[worldSize] {};
	for (int i = 1; i < worldSize; i++)
	{
		displacement[i] = counts[i - 1] + displacement[i - 1];
	}
	return displacement;
}


/* Kindly note the test files have only been made for the NxN picture & 240 intenisty range*/
#pragma region Testing Functions

// There's probably a cleaner way to reuse them but i couldn't be bothered

void verifyFrequancyArray(int* frequancy)
{
	cout << "Verifying Frequancy Array: \n\n";

	ifstream file("..//Data//Input//frequancy.txt");
	int expected[MAX_INTENSITY_VALUE];
	for (int i = 0; i < MAX_INTENSITY_VALUE; i++)
		file >> expected[i];
	file.close();

	for (int i = 0; i < MAX_INTENSITY_VALUE; i++)
	{
		if (frequancy[i] != expected[i])
		{
			cout << "Wrong Value at index " << i << "\nExpected : " << expected[i]
				<< ", Got : " << frequancy[i] << "\n";
			//cout << "Exiting..!\n";
			system("pause");

			//exit(-1);
		}
	}
	cout << "Verified!\n\n";

}

void verifyColorProbability(double* colorProbability)
{
	cout << "Verifying Color Probability: \n\n";

	ifstream file("..//Data//Input//colorProbability.txt");
	double expected[MAX_INTENSITY_VALUE];
	for (int i = 0; i < MAX_INTENSITY_VALUE; i++)
		file >> expected[i];
	file.close();

	for (int i = 0; i < MAX_INTENSITY_VALUE; i++)
	{
		if (abs(colorProbability[i] - expected[i]) > 0.0001)
		{
			cout << "Wrong Value at index " << i << "\nExpected : " << expected[i]
				<< ", Got : " << colorProbability[i] << "\n";
			cout << "Exiting..!\n";
			system("pause");
			exit(-1);
		}
	}
	cout << "Verfied!\n\n";

}

void verifyCumulativeProbability(double* cumulativeProbability)
{
	cout << "Verifying Cumulative Probability: \n\n";

	ifstream file("..//Data//Input//cumulativeProbability.txt");
	double expected[MAX_INTENSITY_VALUE];
	for (int i = 0; i < MAX_INTENSITY_VALUE; i++)
		file >> expected[i];
	file.close();

	for (int i = 0; i < MAX_INTENSITY_VALUE; i++)
	{
		if (abs(cumulativeProbability[i] - expected[i]) > 0.0001)
		{
			cout << "Wrong Value at index " << i << "\nExpected : " << expected[i]
				<< ", Got : " << cumulativeProbability[i] << "\n";
			cout << "Exiting..!\n";
			system("pause");
			exit(-1);
		}
	}
	cout << "Verfied!\n\n";

}

void verifyEqualizedIntenisties(int* equalizedIntenisties)
{
	double EPS = 1.0;
	cout << "Verifying Equalized Intenisties: \n\n";

	ifstream file("..//Data//Input//equalizedIntenisties.txt");
	double expected[MAX_INTENSITY_VALUE];
	for (int i = 0; i < MAX_INTENSITY_VALUE; i++)
		file >> expected[i];
	file.close();

	for (int i = 0; i < MAX_INTENSITY_VALUE; i++)
	{
		if (abs(equalizedIntenisties[i] - expected[i]) > EPS)
		{
			cout << "Wrong Value at index " << i << "\nExpected : " << expected[i]
				<< ", Got : " << equalizedIntenisties[i] << "\n";
			cout << "Exiting..!\n";
			system("pause");
			exit(-1);
		}
	}
	cout << "Verfied!\n\n";

}

void verifyFinalImage(int* finalImage, int pixelCount)
{
	double EPS = 1.0;
	cout << "Verifying Final Image: \n\n";

	ifstream file("..//Data//Input//finalImage.txt");
	int* expected = new int[pixelCount];
	for (int i = 0; i < pixelCount; i++)
		file >> expected[i];
	file.close();

	for (int i = 0; i < pixelCount; i++)
	{
		if (abs(finalImage[i] - expected[i]) > EPS)
		{
			cout << "Wrong Value at index " << i << "\nExpected : " << expected[i]
				<< ", Got : " << finalImage[i] << "\n";
			cout << "Exiting..!\n";
			system("pause");
			exit(-1);
		}
	}

	cout << "Verfied!\n\n";
}

// Not ready as a function yet

//void extractValuesToFile(string filename)
//{
//	ofstream file("..//Data//Output//filename.txt");
//	for (int i = 0; i < MAX_COLOR_VALUE; i++)
//		file << frequancyArray[i] << "\n";
//	file.close();
//}
#pragma endregion