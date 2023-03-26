
#include <chrono>
#include <CL/cl.h>
#include <iostream>
#include "filters.h"
#include "lodepng/lodepng.h"
#include "lodepng/lodepng.cpp"

using namespace std;

bool loadPNG(vector<unsigned char>& input, unsigned int& w, unsigned int& h, const string& fileName)
{
    return !lodepng::decode(input, w, h, fileName, LCT_RGB);
}

bool savePNG(vector<unsigned char>& input, unsigned int& w, unsigned int& h, const string& fileName)
{
    return !lodepng::encode(fileName, input, w, h, LCT_RGB);
}

int main()
{   
	vector<string> names = { "300x300", "400x400", "500x500", "600x600", "950x950", "2400x2400" };
	string format = ".png";

	vector<double> aveTimes(names.size(), 0.0);
	int iterations = 1000;

	fstream file("medFilterCL.txt");
	MedianFilter filter;

	for (int i = 0; i < names.size(); i++)
	{
		string nameFile = names[i];
		vector<unsigned char> img;
		unsigned int w, h;

		file << "Open file: " + nameFile + format + " ";
		cout << "Open file: " + nameFile + format + " ";

		if (loadPNG(img, w, h, "imageInput\\" + nameFile + format))
		{
			file << "success" << endl;
			file << "CalculationOfAverageTime: its-" << iterations << " -> ";
			cout << "success" << endl;
			cout << "CalculationOfAverageTime: its-" << iterations << " -> ";

			for (int j = 0; j < iterations; j++)
			{
				auto start = chrono::steady_clock::now();

				filter.processing(img, w, h, 16);

				auto end = chrono::steady_clock::now();
				double seconds = chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000000.0;

				aveTimes[i] += seconds;
			}

			file << "aveTimes-" << aveTimes[i] / (double)iterations << " -> fullTime-" << aveTimes[i] << endl;
			cout << "aveTimes-" << aveTimes[i] / (double)iterations << " -> fullTime-" << aveTimes[i] << endl;
		}
		else
		{
			file << "fail" << endl;
			cout << "fail" << endl;
		}
		file << endl;
		cout << endl;
	}
    
    return 0;
}