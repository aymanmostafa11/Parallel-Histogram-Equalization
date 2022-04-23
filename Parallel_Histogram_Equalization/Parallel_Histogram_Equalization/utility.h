#pragma once
#include <fstream>

#using <mscorlib.dll>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

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

int* inputImage(int* w, int* h, System::String^ imagePath) //put the size of image in w & h
{
	int* input;


	int OriginalImageWidth, OriginalImageHeight;

	//*********************************************************Read Image and save it to local arrayss*************************	
	//Read Image and save it to local arrayss

	System::Drawing::Bitmap BM(imagePath);

	OriginalImageWidth = BM.Width;
	OriginalImageHeight = BM.Height;
	*w = BM.Width;
	*h = BM.Height;
	/*int *Red = new int[BM.Height * BM.Width];
	int *Green = new int[BM.Height * BM.Width];
	int *Blue = new int[BM.Height * BM.Width];*/
	input = new int[BM.Height*BM.Width];
	for (int i = 0; i < BM.Height; i++)
	{
		for (int j = 0; j < BM.Width; j++)
		{
			System::Drawing::Color c = BM.GetPixel(j, i);

			/*Red[i * BM.Width + j] = c.R;
			Blue[i * BM.Width + j] = c.B;
			Green[i * BM.Width + j] = c.G;*/

			input[i*BM.Width + j] = ((c.R + c.B + c.G) / 3); //gray scale value equals the average of RGB values

		}

	}
	return input;
}


void createImage(int* image, int width, int height, int index)
{
	System::Drawing::Bitmap MyNewImage(width, height);


	for (int i = 0; i < MyNewImage.Height; i++)
	{
		for (int j = 0; j < MyNewImage.Width; j++)
		{
			//i * OriginalImageWidth + j
			if (image[i*width + j] < 0)
			{
				image[i*width + j] = 0;
			}
			if (image[i*width + j] > 255)
			{
				image[i*width + j] = 255;
			}
			System::Drawing::Color c = System::Drawing::Color::FromArgb(image[i*MyNewImage.Width + j], image[i*MyNewImage.Width + j], image[i*MyNewImage.Width + j]);
			MyNewImage.SetPixel(j, i, c);
		}
	}
	MyNewImage.Save("..//Data//Output//Res" + index + "_" + N + "N" + ".png");
	cout << "result Image '" << index << "' Saved " << endl;
}
