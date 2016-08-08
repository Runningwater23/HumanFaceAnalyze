#pragma once
#include <vector>
using namespace std;

#include "opencv2/opencv.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/core/utility.hpp>
#include <opencv2/tracking.hpp>
using namespace cv;

#include "face.hpp"
using namespace cv::face;

#include "facedetect-dll.h"
#pragma comment(lib,"libfacedetect.lib")

#include <vector>
using namespace std;
struct POINT2D 
{
	int x;
	int y;
	POINT2D(int m, int n)
	{
		x = m;
		y = n;
	}
};
struct CTrajectory
{
	vector<Point2f> pointList;
	int markFlag;
};
class CHumanFace
{
public:
	Rect faceBox;
	Rect matchFaceBox;
	bool bMatch;
	bool bConfirmFace;
	int nCenterx;
	int nCentery;
	int nLossCount;
	int nContinousCount;
	vector<POINT2D> trajectory;

	bool bEyesDetected;
	Point leftEyePoint;
	Point rightEyePoint;

	Rect noseRect;
	bool bNoseDetected;

	Rect mouthRect;
	bool bMouthDetected;

	vector<Point2f> featPoints[2];
	vector<CTrajectory> featTrajectoies;

public:
	CHumanFace()
	{
		nContinousCount = 0;
		nLossCount = 0;
		bMatch = false;
		bConfirmFace = false;
		bEyesDetected = false;
	}
	~CHumanFace()
	{

	}
};
class CFaceAnalyze
{
public:
	Mat tempImageMerge;

	const char* face_cascade_name = "haarcascade_frontalface_alt.xml";
	const char* smile_cascade_name = "haarcascade_smile.xml";
	const char* leftEye_cascade_name = "haarcascade_mcs_lefteye.xml";
	const char* rightEye_cascade_name = "haarcascade_mcs_righteye.xml";
	const char* gender_fisherface_name = "Fisherfaces2nd.yml";
	const char* nose_cascade_name = "haarcascade_mcs_nose.xml";
	const char* mouth_cascade_name = "haarcascade_mcs_mouth.xml";

	CascadeClassifier face_cascade;
	CascadeClassifier leftEye_cascade;
	CascadeClassifier rightEye_cascade;
	CascadeClassifier nose_cascade;
	CascadeClassifier mouth_cascade;

	vector<Rect> m_facesBox;
	vector<CHumanFace> m_humanFace;
public:
	void InitParam(vector<CHumanFace> &humanFace);
	void FaceDetect(Mat& imageGray, CascadeClassifier& faceDetector, vector<Rect>& facesBox);
	void FaceDetect(Mat& imageGray, vector<Rect>& facesBox);
	void FaceTracking(Mat& prevGray, Mat& imageGray, vector<Rect>& facesBox, vector<CHumanFace>& humanFaces);
	void ExtractFeatPoints(Mat& imageGray, Rect& faceBox, vector<Point2f>& featPoints, vector<CTrajectory>& featTrajectories);
	void FeatPointsTracking(Mat& prevGray, Mat& imageGray, vector<Point2f>& prevPoints, vector<Point2f>& featPoints, vector<CTrajectory>& featTrajectory);
	bool ConsistencyOfMobile(vector<Point2f>& featPoints1, vector<Point2f>& featPoints2);
	void FacialOrgansDetect(Mat& imageGray, vector<CHumanFace>& humanFaces, Mat& imageMerge);
	void DetectBothEyes(const Mat& face, CascadeClassifier &eyeCascade1, CascadeClassifier &eyeCascade2, Point &leftEye, Point &rightEye);
	void detectLargestObject(const Mat &img, CascadeClassifier &cascade, Rect &largestObject, int scaledWidth);
	void detectObjectsCustom(const Mat &img, CascadeClassifier &cascade, vector<Rect> &objects, int scaledWidth, int flags, Size minFeatureSize, float searchScaleFactor, int minNeighbors);
	void DetectNose(Mat& face, CascadeClassifier &noseCascade, Point& eyesCenter, int eyesWidth, Rect& noseRect);
	void DetectMouth(Mat& face, CascadeClassifier &mouthCascade, Rect& noseRect, Rect& mouthRect);
public:
	void DrawFaceTrajectory(Mat& image, vector<CHumanFace>& humanFace);
	void DrawTrajectory(Mat& image, vector<POINT2D>& trajectory);
	void DrawLKMatch(Mat imgMerge, vector<CTrajectory> trajectories);
public:
	CFaceAnalyze();
	~CFaceAnalyze();
};

