// includes of OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp> 

// include of Boost libs
#include <boost/format.hpp>

// includes of STD libs
#include <iostream>
#include <conio.h>

using namespace cv;
using namespace std;

// VARIABLES NEEDED FOR CORNERS FIXING (GLOBALS)
Point patternCorners[4];
Point creasedCorners[4];
int patternCornersCounter = 0, creasedCornersCounter = 0;
bool patternCornersInitialized = false, creasedCornersInitialized = false;

void onMousePatternImage(int event, int x, int y, int flags, void* param);
void onMouseCreasedImage(int event, int x, int y, int flags, void* param);
Mat segmentateQuadrangle(int rows, int cols, Point * corners, Mat image, Point ** segmentationPoints);

int main() {
	// variables connected with photo
	string patternPhotoName, creasedPhotoName;
	int columns, rows;
	Point ** segmentationPointsPattern;
	Point ** segmentationPointCreased;

	// SETTINGS
	cout << "How many columns for segmentation?" << endl;
	cin >> columns;
	cout << "How many rows for segmentation?" << endl;
	cin >> rows;
	segmentationPointCreased = new Point*[rows];
	segmentationPointsPattern = new Point*[rows];
	for (int i = 0; i < rows; i++)
	{
		segmentationPointCreased[i] = new Point[columns];
		segmentationPointsPattern[i] = new Point[columns];
	}


	// READING THE PHOTOS
	cout << "Name of the pattern photo: " << endl;
	cin >> patternPhotoName;
	Mat patternImage = imread(str(boost::format("%1%.jpg") % patternPhotoName.data()));

	cout << "Name of the creased photo: " << endl;
	cin >> creasedPhotoName;
	Mat creasedImage = imread(str(boost::format("%1%.jpg") % creasedPhotoName.data()));

	imshow("Pattern", patternImage);
	imshow("Creased", creasedImage);
	waitKey(1);

	// OBTAINING FIXED CORNERS FOR PATTERN
	cout << "Give four corners for the pattern image." << endl;
	setMouseCallback("Pattern", onMousePatternImage, 0);
	while (!patternCornersInitialized)
	{
		Mat temporaryPattern = patternImage.clone();
		for (int i = 0; i < patternCornersCounter; i++)
		{
			circle(temporaryPattern, patternCorners[i], 5, Scalar(255, 0, 0), -1);
			putText(temporaryPattern, str(boost::format("%1%") % i), patternCorners[i], FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255));
		}
		imshow("Pattern", temporaryPattern);
		waitKey(30);
	}
	// DRAW ALL POINTS TO SEE IF EVERYTHINGS CORRECT - POSSIBLY TO OPTIMIZE
	Mat temporaryPattern = patternImage.clone();
	for (int i = 0; i < patternCornersCounter; i++)
	{
		circle(temporaryPattern, patternCorners[i], 5, Scalar(255, 0, 0), -1);
		putText(temporaryPattern, str(boost::format("%1%") % i), patternCorners[i], FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255));
	}
	segmentateQuadrangle(rows, columns, patternCorners, temporaryPattern, segmentationPointsPattern);
	imshow("Pattern", temporaryPattern);
	waitKey(30);


	// OBTAINING FIXED CORNERS FOR CREASED
	cout << "Give four corners for the creased image." << endl;
	setMouseCallback("Creased", onMouseCreasedImage, 0);
	while (!creasedCornersInitialized)
	{
		Mat temporaryCreased = creasedImage.clone();
		for (int i = 0; i < creasedCornersCounter; i++)
		{
			circle(temporaryCreased, creasedCorners[i], 5, Scalar(0, 255, 0), -1);
			putText(temporaryCreased, str(boost::format("%1%") % i), creasedCorners[i], FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255));
		}
		imshow("Creased", temporaryCreased);
		waitKey(30);
	}
	// DRAW ALL POINTS TO SEE IF EVERYTHINGS CORRECT - POSSIBLY TO OPTIMIZE
	Mat temporaryCreased = creasedImage.clone();
	for (int i = 0; i < creasedCornersCounter; i++)
	{
		circle(temporaryCreased, creasedCorners[i], 5, Scalar(0, 255, 0), -1);
		putText(temporaryCreased, str(boost::format("%1%") % i), creasedCorners[i], FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255));
	}
	segmentateQuadrangle(rows, columns, creasedCorners, temporaryCreased, segmentationPointCreased);
	imshow("Creased", temporaryCreased);
	waitKey(30);



	// GARBAGE COLLECTION
	for (int i = 0; i < rows; i++)
	{
		delete[] segmentationPointCreased[i];
		delete[] segmentationPointsPattern[i];
	}
	delete[] segmentationPointCreased;
	delete[] segmentationPointsPattern;


	system("pause");
	return 0;
}

// EVENT LISTENERS FROM OPENCV TO CORNERS FIXING
void onMousePatternImage(int event, int x, int y, int flags, void* param)
{
	if (event != CV_EVENT_LBUTTONDOWN)
		return;

	if (!patternCornersInitialized)
	{
		patternCorners[patternCornersCounter].x = x;
		patternCorners[patternCornersCounter].y = y;
		patternCornersCounter++;
		if (patternCornersCounter == 4)
			patternCornersInitialized = true;
	}
}

void onMouseCreasedImage(int event, int x, int y, int flags, void* param)
{
	if (event != CV_EVENT_LBUTTONDOWN)
		return;
	if (!creasedCornersInitialized)
	{
		creasedCorners[creasedCornersCounter].x = x;
		creasedCorners[creasedCornersCounter].y = y;
		creasedCornersCounter++;
		if (creasedCornersCounter == 4)
			creasedCornersInitialized = true;
	}
}

Mat segmentateQuadrangle(int rows, int cols, Point * corners, Mat image, Point ** rowsSegmentationPoints)
{
	// parameters for lines
	double edgesDeltaX[4], edgesDeltaY[4];
	Point * edgesSegmentationPoints[4];
	rowsSegmentationPoints = new Point*[rows + 1];
	for (int i = 0; i < (rows + 1); i++)
	{
		rowsSegmentationPoints[i] = new Point[cols + 1];
	}

	edgesSegmentationPoints[0] = new Point[cols + 1];
	edgesSegmentationPoints[1] = new Point[rows + 1];
	edgesSegmentationPoints[2] = new Point[cols + 1];
	edgesSegmentationPoints[3] = new Point[rows + 1];

	// 1) wylicz wspolrzedne punktów dzielacych wszystkie krawedzie na rowne czesci wg ilosci rows i cols
	line(image, corners[0], corners[1], Scalar(255, 0, 0));
	line(image, corners[1], corners[2], Scalar(255, 0, 0));
	line(image, corners[2], corners[3], Scalar(255, 0, 0));
	line(image, corners[3], corners[0], Scalar(255, 0, 0));

	edgesDeltaX[0] = corners[1].x - corners[0].x;
	edgesDeltaX[1] = corners[2].x - corners[1].x;
	edgesDeltaX[2] = corners[3].x - corners[2].x;
	edgesDeltaX[3] = corners[0].x - corners[3].x;

	edgesDeltaY[0] = corners[1].y - corners[0].y;
	edgesDeltaY[1] = corners[2].y - corners[1].y;
	edgesDeltaY[2] = corners[3].y - corners[2].y;
	edgesDeltaY[3] = corners[0].y - corners[3].y;

	for (int i = 0; i < cols; i++)
	{
		edgesSegmentationPoints[0][i] = Point(corners[0].x + i*edgesDeltaX[0] / cols, corners[0].y + i*edgesDeltaY[0] / cols);
		//circle(image, edgesSegmentationPoints[0][i], 5, Scalar(255, 255, 0));
	}
	edgesSegmentationPoints[0][cols] = Point(corners[1].x, corners[1].y);
	//circle(image, edgesSegmentationPoints[0][cols], 8, Scalar(0, 0, 255));

	for (int i = 0; i < rows; i++)
	{
		edgesSegmentationPoints[1][i] = Point(corners[1].x + i*edgesDeltaX[1] / rows, corners[1].y + i*edgesDeltaY[1] / rows);
		//circle(image, edgesSegmentationPoints[1][i], 5, Scalar(255, 255, 0));
	}
	edgesSegmentationPoints[1][rows] = Point(corners[2].x, corners[2].y);
	//circle(image, edgesSegmentationPoints[1][rows], 8, Scalar(0, 0, 255));

	for (int i = 0; i < cols; i++)
	{
		edgesSegmentationPoints[2][i] = Point(corners[2].x + i*edgesDeltaX[2] / cols, corners[2].y + i*edgesDeltaY[2] / cols);
		//circle(image, edgesSegmentationPoints[2][i], 5, Scalar(255, 255, 0));
	}
	edgesSegmentationPoints[2][cols] = Point(corners[3].x, corners[3].y);
	//circle(image, edgesSegmentationPoints[2][cols], 8, Scalar(0, 0, 255));

	for (int i = 0; i < rows; i++)
	{
		edgesSegmentationPoints[3][i] = Point(corners[3].x + i*edgesDeltaX[3] / rows, corners[3].y + i*edgesDeltaY[3] / rows);
		//circle(image, edgesSegmentationPoints[3][i], 5, Scalar(255, 255, 0));
	}
	edgesSegmentationPoints[3][rows] = Point(corners[0].x, corners[0].y);
	//circle(image, edgesSegmentationPoints[3][rows], 8, Scalar(0, 0, 255));

	// 2) wylicz wspolrzedne punktów znajdujacych siê miêdzy tymi punktami
	for (int i = 0; i < (rows + 1); i++)
	{
		double deltaX = edgesSegmentationPoints[1][rows - i].x - edgesSegmentationPoints[3][i].x;
		double deltaY = edgesSegmentationPoints[1][rows - i].y - edgesSegmentationPoints[3][i].y;
		for (int j = 0; j < (cols + 1); j++)
		{
			rowsSegmentationPoints[rows - i][j] = Point(edgesSegmentationPoints[3][i].x + j * deltaX / cols, edgesSegmentationPoints[3][i].y + j * deltaY / cols);
			circle(image, rowsSegmentationPoints[rows - i][j], 5, Scalar(0, 255, 0), -1);
		}
	}

	for (int i = 0; i < 4; i++)
		delete[] edgesSegmentationPoints[i];

	return image;
}