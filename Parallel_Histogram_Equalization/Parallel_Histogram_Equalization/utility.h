#pragma once
#include <fstream>

using namespace std;

#define MAIN_PROCESSOR 0

#define TEST_N "..//Data//Input//NxN.png"
#define TEST_5N "..//Data//Input//5Nx5N.png"
#define TEST_10N "..//Data//Input//10Nx10N.png"

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

void writeResultsToFile(int time, int width, int height)
{
	ofstream file("..//Data//Output//results.txt", ios::app);
	file << "Size: (";
	file << width;
	file << " , ";
	file << height;
	file << ") , Time : ";
	file << time;
	file << "ms\n";
	file.close();
}


bool isMainProcessor(int rank)
{
	return rank == MAIN_PROCESSOR;
}
