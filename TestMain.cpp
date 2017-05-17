#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include<vector>
#include<sstream>

using namespace cv;
using namespace std;
using namespace boost::filesystem;

class Filter {
public:

	string filter;
	float sum = 0;
	int r, c;

	Mat read(string data) {
		string line;
		double count = 0;
		std::ifstream file(data);
		vector<vector<string>> array;
		while (getline(file, line))
		{
			array.push_back(vector<string>());
			istringstream ss(line);
			string value;
			while (ss >> value)
			{
				array.back().push_back(value);
			}
		}
		float total = 0;
		r = stoi(array[0][0]);
		c = stoi(array[0][1]);
		if (array[1][0] == "L")
			filter = "L";
		else
			filter = "H";
		Mat maskmatrix = Mat::zeros(cv::Size(c, r), CV_32FC1);
		for (int i = 2; i < r + 2; i++) {
			for (int j = 0; j < c; j++) {
				maskmatrix.at<float>(i - 2, j) = stoi(array[i][j]);
				if (maskmatrix.at<float>(i - 2, j)>0)
					total = total + maskmatrix.at<float>(i - 2, j);
				sum = total;
			}
		}
		return maskmatrix;
	}

	//CONVOLUTION
	Mat convolveOne(Mat mask, Mat floatImage, Mat filterimage) {
		int cx = r / 2, cy = c / 2;
		for (int i = 0; i < floatImage.rows; i++) {
			for (int j = 0; j < floatImage.cols; j++) {
				float count = 0;
				for (int s = 0; s < r; s++) {
					int	ss = r - 1 - s;
					for (int t = 0; t<c; t++) {
						int tt = c - 1 - t;

						int ii = i + (s - cx);
						int jj = j + (t - cy);

						if (ii >= 0 && ii < floatImage.rows && jj >= 0 && jj < floatImage.cols) {
							count = count + mask.at<float>(s, t)*floatImage.at<float>(ii, jj);
						}
						else
							count = count + 128 * mask.at<float>(s, t);
					}
				}
				if (filter == "L")
					filterimage.at<float>(i, j) = count / sum;
				else
					filterimage.at<float>(i, j) = count;
			}
		}
		return filterimage;
	}

	//BLURRING
	Mat blurring(Mat floatImage, Mat gausMask) {
		Mat filterImage = Mat::zeros(cv::Size(floatImage.cols, floatImage.rows), CV_32FC1);
		for (int b = 0; b < 9; b++)
			convolveOne(gausMask, floatImage, filterImage).copyTo(floatImage);

		return filterImage;
	}

	//LAPLACIAN
	Mat laplacian(Mat filteredImage, Mat lapMask) {
		Mat filter = Mat::zeros(cv::Size(filteredImage.cols, filteredImage.rows), CV_32FC1);
		convolveOne(lapMask, filteredImage, filter);
		return filter;

	}

	// ZERO CROSSING
	void zeroCrossing(float val1, float val2, Mat edge, int i, int j, int a, int b) {
		if (fabs(val1 - val2) >= 5.0) {
			if ((val1 > 0 && val2 < 0) || (val1 < 0 && val2 > 0)) {
				if (val1 < val2)
					edge.at<uchar>(i, j) = 255;
				else
					edge.at<uchar>(a, b) = 255;
			}
		}

	}

	//EDGE DETECTION
	void edgeDetection(Mat &edge, Mat lapImage) {
		for (int i = 1; i < edge.rows - 1; i++) {
			for (int j = 1; j < edge.cols - 1; j++) {
				float val1 = lapImage.at<float>(i, j);
				float val2;
				int a, b;

				val2 = lapImage.at<float>(i + 1, j);
				a = i + 1; b = j;
				zeroCrossing(val1, val2, edge, i, j, a, b);

				val2 = lapImage.at<float>(i + 1, j + 1);
				a = i + 1; b = j + 1;
				zeroCrossing(val1, val2, edge, i, j, a, b);

				val2 = lapImage.at<float>(i, j + 1);
				a = i; b = j + 1;
				zeroCrossing(val1, val2, edge, i, j, a, b);

				val2 = lapImage.at<float>(i + 1, j - 1);
				a = i + 1; b = j - 1;
				zeroCrossing(val1, val2, edge, i, j, a, b);
			}
		}
	}

};




int main(int argc, char **argv) {

	path p(argv[1]);

	string r(argv[3]);
	string lapFile = r + "/" + "laplacian.txt";
	Filter *l = new Filter();
	l->read(lapFile);
	Mat lapMask;
	l->read(lapFile).copyTo(lapMask);

	string gausFile = r + "/" + "gaussian3x3.txt";
	Filter *g = new Filter();
	g->read(gausFile);
	Mat gausMask;
	g->read(gausFile).copyTo(gausMask);



	directory_iterator end_itr;
	for (directory_iterator itr(p); itr != end_itr; ++itr) {
		string imname = itr->path().filename().string();
		string extn = itr->path().extension().string();
		if (extn == ".png" || extn == ".png") {
			Mat image = imread(itr->path().string(), IMREAD_GRAYSCALE);
			if (!image.data) {
				cout << "ERROR: Could not open or find the image!" << endl;
				return -1;
			}

			Mat floatImage = Mat::zeros(cv::Size(image.cols, image.rows), CV_32FC1);
			image.convertTo(floatImage, CV_32FC1);

			Mat filteredImage;
			g->blurring(floatImage, gausMask).copyTo(filteredImage);

			Mat lapImage;
			l->laplacian(filteredImage, lapMask).copyTo(lapImage);

			Mat edge = Mat::zeros(cv::Size(image.cols, image.rows), CV_8UC1);
			Filter *e = new Filter();
			e->edgeDetection(edge, lapImage);

			string outputpath(argv[2]);
			string output = outputpath + "/" + imname;
			imwrite(output, edge);

		}
	}
	return 0;
}