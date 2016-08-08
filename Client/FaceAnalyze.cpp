#include "stdafx.h"
#include "FaceAnalyze.h"


CFaceAnalyze::CFaceAnalyze()
{
	if (!face_cascade.load(face_cascade_name))
	{
	
	}
	if (!leftEye_cascade.load(leftEye_cascade_name))
	{

	}
	if (!rightEye_cascade.load(rightEye_cascade_name))
	{

	}
	if (!nose_cascade.load(nose_cascade_name));
	{

	}
	if (!mouth_cascade.load(mouth_cascade_name))
	{

	}
}

CFaceAnalyze::~CFaceAnalyze()
{
}
void CFaceAnalyze::InitParam(vector<CHumanFace> &humanFace)
{
	for (int i = 0; i < humanFace.size(); i++)
	{
		humanFace[i].bMatch = false;
		std::swap(humanFace[i].featPoints[0], humanFace[i].featPoints[1]);
	}
}
void CFaceAnalyze::FaceDetect(Mat& imageGray, CascadeClassifier& faceDetector, vector<Rect>& facesBox)
{
	facesBox.clear();
	int minFaceWidth = imageGray.cols >> 3;
	if (!faceDetector.empty())
	{
		equalizeHist(imageGray, imageGray);
		faceDetector.detectMultiScale(imageGray, facesBox, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
		int k = 0;
		for (size_t i = 0; i < facesBox.size(); i++)
		{
			if (facesBox[i].width>minFaceWidth)
			{
				facesBox[k++] = facesBox[i];
			}
		}
		facesBox.resize(k);
	}
}
void CFaceAnalyze::FaceDetect(Mat& imageGray, vector<Rect>& facesBox)
{
	int * pResults = NULL;
	//pResults = facedetect_multiview_reinforce((unsigned char*)(imageGray.ptr(0)), imageGray.cols, imageGray.rows, imageGray.step,1.2f, 5, 24);
	//pResults = facedetect_frontal((unsigned char*)(imageGray.ptr(0)), imageGray.cols, imageGray.rows, imageGray.step, 1.2f, 3, 24);
	//pResults = facedetect_multiview((unsigned char*)(imageGray.ptr(0)), imageGray.cols, imageGray.rows, imageGray.step,1.2f, 5, 24);
	pResults = facedetect_frontal_tmp((unsigned char*)(imageGray.ptr(0)), imageGray.cols, imageGray.rows, imageGray.step, 1.2f, 5, 24);
	facesBox.clear();
	//print the detection results
	for (int i = 0; i < (pResults ? *pResults : 0); i++)
	{
		short * p = ((short*)(pResults + 1)) + 6 * i;
		int x = p[0];
		int y = p[1];
		int w = p[2];
		int h = p[3];
		int neighbors = p[4];
		int angle = p[5];

		Rect tempRect;
		tempRect.x = x;
		tempRect.y = y;
		tempRect.width = w;
		tempRect.height = h;
		
		facesBox.push_back(tempRect);

	}
}
void CFaceAnalyze::FaceTracking(Mat& prevGray, Mat& imageGray, vector<Rect>& facesBox, vector<CHumanFace>& humanFaces)
{
	InitParam(humanFaces);
	int centerx, centery;
	int nHistroyFaceNum = humanFaces.size();
	int xMaxDistance = imageGray.cols >> 4;
	int yMaxDistance = imageGray.rows >> 4;
	for (int i = 0; i < facesBox.size(); i++)
	{
		centerx = facesBox[i].x + facesBox[i].width / 2;
		centery = facesBox[i].y + facesBox[i].height / 2;
		bool bMatch = false;
		for (int nHistoryFaceNo = 0; nHistoryFaceNo < nHistroyFaceNum; nHistoryFaceNo++)
		{
			int index = humanFaces[nHistoryFaceNo].trajectory.size() - 1;
			int matchCenterx = humanFaces[nHistoryFaceNo].trajectory[index].x;
			int matchCentery = humanFaces[nHistoryFaceNo].trajectory[index].y;
			if (abs(centerx - matchCenterx) < xMaxDistance&&abs(centery - matchCentery) < yMaxDistance)
			{
				bMatch = true;
				if (!humanFaces[nHistoryFaceNo].bMatch)
				{
					humanFaces[nHistoryFaceNo].matchFaceBox = facesBox[i];
					humanFaces[nHistoryFaceNo].bMatch = true;
				}
				else
				{
					int currentDis = abs(centerx - matchCenterx) + abs(centery - matchCentery);
					centerx = humanFaces[nHistoryFaceNo].matchFaceBox.x + humanFaces[nHistoryFaceNo].matchFaceBox.width >> 1;
					centery = humanFaces[nHistoryFaceNo].matchFaceBox.y + humanFaces[nHistoryFaceNo].matchFaceBox.height >> 1;
					int matchedDis = abs(centerx - matchCenterx) + abs(centery - matchCentery);
					if (currentDis < matchedDis)
					{
						humanFaces[nHistoryFaceNo].matchFaceBox = facesBox[i];
						break;
					}
				}
			}
		}
		if (!bMatch)
		{
			CHumanFace newHumanFace;
			newHumanFace.faceBox = facesBox[i];
			newHumanFace.trajectory.push_back(POINT2D(facesBox[i].x + facesBox[i].width/2, facesBox[i].y + facesBox[i].height/2));
			newHumanFace.nContinousCount = 1;
			newHumanFace.nLossCount = 0;
			humanFaces.push_back(newHumanFace);
		}
	}
	//ÌØÕ÷µã¸ú×Ù
	for (int nHistoryNo = nHistroyFaceNum - 1; nHistoryNo >= 0; nHistoryNo--)
	{
		humanFaces[nHistoryNo].nLossCount++;
		if (humanFaces[nHistoryNo].bMatch)
		{
			Rect matchBox = humanFaces[nHistoryNo].matchFaceBox;
			humanFaces[nHistoryNo].faceBox = matchBox;
			humanFaces[nHistoryNo].trajectory.push_back(POINT2D(matchBox.x + matchBox.width/2, matchBox.y + matchBox.height/2));
			humanFaces[nHistoryNo].nLossCount = 0;
			humanFaces[nHistoryNo].nContinousCount++;

			if (humanFaces[nHistoryNo].bConfirmFace)
			{
				if (humanFaces[nHistoryNo].featPoints[0].size() > 5)
				{
					FeatPointsTracking(prevGray, imageGray, humanFaces[nHistoryNo].featPoints[0], humanFaces[nHistoryNo].featPoints[1], humanFaces[nHistoryNo].featTrajectoies);
				}
				else
				{
					ExtractFeatPoints(imageGray, humanFaces[nHistoryNo].faceBox, humanFaces[nHistoryNo].featPoints[1], humanFaces[nHistoryNo].featTrajectoies);
				}
			}
		}
		else
		{
			humanFaces[nHistoryNo].nLossCount++;
			humanFaces[nHistoryNo].nContinousCount = 0;
		}
	}
	int k = 0;
	for (int i = 0; i < humanFaces.size(); i++)
	{
		if (humanFaces[i].nLossCount<2)
		{
			humanFaces[k++] = humanFaces[i];
		}
	}
	if (k) humanFaces.resize(k);
	else humanFaces.clear();
}
void CFaceAnalyze::DrawFaceTrajectory(Mat& image, vector<CHumanFace>& humanFace)
{
	for (int i = 0; i < humanFace.size(); i++)
	{
		//if (humanFace[i].bConfirmFace)
		{
			DrawTrajectory(image, humanFace[i].trajectory);
		}

	}
}
void CFaceAnalyze::DrawTrajectory(Mat& image, vector<POINT2D>& trajectory)
{
	if (trajectory.size() > 1)
	{
		for (int i = 1; i < trajectory.size(); i++)
		{
			cv::line(image, Point(trajectory[i - 1].x, trajectory[i - 1].y), Point(trajectory[i].x, trajectory[i].y), cv::Scalar(0, 0, 255), 2);
		}
	}
}
void CFaceAnalyze::DrawLKMatch(Mat imgMerge, vector<CTrajectory> trajectories)
{
	int cols = imgMerge.cols / 2;
	int colorCount = 1;
	int r, g, b;
	for (int i = 0; i < trajectories.size(); i++)
	{
		if (trajectories[i].pointList.size()>1)
		{
			int index1 = trajectories[i].pointList.size() - 1;
			int index2 = trajectories[i].pointList.size() - 2;
			Point2f pt1 = trajectories[i].pointList[index1];
			Point2f pt2 = trajectories[i].pointList[index2];
			r = colorCount % 2;
			g = (colorCount >> 1) % 2;;
			b = (colorCount >> 2) % 2;
			colorCount++;
			line(imgMerge, Point(pt1.x + cols, pt1.y), Point(pt2.x, pt2.y), Scalar(b * 255, g * 255, r * 255), 1);
		}
	}
}
void CFaceAnalyze::ExtractFeatPoints(Mat& imageGray, Rect& faceBox, vector<Point2f>& featPoints, vector<CTrajectory>& featTrajectories)
{
	static int MAX_COUNT = 100;
	static TermCriteria termcrit(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03);
	static Size subPixWinSize(10, 10), winSize(31, 31);
	
	Mat imgROI;
	imageGray(faceBox).copyTo(imgROI);
	goodFeaturesToTrack(imgROI, featPoints, MAX_COUNT, 0.01, 10, Mat(), 3, 0, 0.04);
	cornerSubPix(imgROI, featPoints, subPixWinSize, Size(-1, -1), termcrit);

	CTrajectory tempTrajectory;
	tempTrajectory.markFlag = 0;
	featTrajectories.clear();
	for (int i = 0; i < featPoints.size(); i++)
	{
		featPoints[i].x += faceBox.x;
		featPoints[i].y += faceBox.y;
		tempTrajectory.pointList.clear();
		tempTrajectory.pointList.push_back(featPoints[i]);
		featTrajectories.push_back(tempTrajectory);
	}

}
void CFaceAnalyze::FeatPointsTracking(Mat& prevGray, Mat& imageGray, vector<Point2f>& prevPoints, vector<Point2f>& featPoints, vector<CTrajectory>& featTrajectory)
{
	static TermCriteria termcrit(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03);
	static Size subPixWinSize(10, 10), winSize(31, 31);
	vector<uchar> status;
	vector<float> err;
	calcOpticalFlowPyrLK(prevGray, imageGray, prevPoints, featPoints, status, err, winSize, 3, termcrit, 0, 0.001);
	int i, k;
	for (i = 0, k = 0; i < featPoints.size(); i++)
	{
		circle(tempImageMerge, featPoints[i], 3, Scalar(255, 0, 0), 2);
		circle(tempImageMerge, prevPoints[i], 3, Scalar(0, 0, 255), 2);
		//if (status[i]) continue;

		featPoints[k] = featPoints[i];
		featTrajectory[k] = featTrajectory[i];
		featTrajectory[k].pointList.push_back(featPoints[i]);
		k++;
	}
	featPoints.resize(k);
	featTrajectory.resize(k);
}
bool CFaceAnalyze::ConsistencyOfMobile(vector<Point2f>& featPoints, vector<Point2f>& prevPoints)
{
	int nSize = featPoints.size();
	if (nSize)
	{
		float xAveDis = 0;
		float yAveDis = 0;
		for (int i = 0; i < nSize; i++)
		{
			xAveDis += featPoints[i].x - prevPoints[i].x;
			yAveDis += featPoints[i].y - prevPoints[i].y;
		}
		xAveDis /= nSize;
		yAveDis /= nSize;
		float xDis_1norm = 0;
		float yDis_1norm = 0;
		for (int i = 0; i < nSize; i++)
		{
			xDis_1norm += abs(featPoints[i].x - prevPoints[i].x - xAveDis);
			yDis_1norm += abs(featPoints[i].y - prevPoints[i].y - yAveDis);
		}
		xDis_1norm /= nSize;
		yDis_1norm /= nSize;
		if (xDis_1norm < 5 && yDis_1norm < 5)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}
void CFaceAnalyze::FacialOrgansDetect(Mat& imageGray, vector<CHumanFace>& humanFaces, Mat& imageMerge)
{
	static TermCriteria termcrit(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03);
	for (int i = 0; i < humanFaces.size(); i++)
	{
		humanFaces[i].bEyesDetected = false;
		humanFaces[i].bNoseDetected = false;
		humanFaces[i].bMouthDetected = false;

		Rect tempRect = humanFaces[i].faceBox;
		//tempRect.x -= (tempRect.width >> 4);
		//tempRect.width += (tempRect.width >> 3);
		//tempRect.y -= (tempRect.height >> 3);
		//tempRect.height += (tempRect.height >> 2);
		if (tempRect.x>0 && tempRect.y > 0 && (tempRect.x + tempRect.width) < imageGray.cols && (tempRect.y + tempRect.height) < imageGray.rows)
		{
			Mat imgROI = imageGray(tempRect);
			//¼ì²âÑÛ¾¦
			Point eyesCenter(-1, -1);
			int eyesWidth = -1;
			if (!rightEye_cascade.empty() && !rightEye_cascade.empty())
			{
				Point leftEye(-1,-1), rightEye(-1,-1);
				DetectBothEyes(imgROI, leftEye_cascade, rightEye_cascade, leftEye,rightEye);
				if (leftEye.x>0 && rightEye.x>0)
				{
					eyesCenter.x = (leftEye.x + rightEye.x) / 2;
					eyesCenter.y = (leftEye.y + rightEye.y) / 2;
					eyesWidth = abs(rightEye.x - leftEye.x);

					humanFaces[i].leftEyePoint.x = leftEye.x + tempRect.x;
					humanFaces[i].leftEyePoint.y = leftEye.y + tempRect.y;

					humanFaces[i].rightEyePoint.x = rightEye.x + tempRect.x;
					humanFaces[i].rightEyePoint.y = rightEye.y + tempRect.y;

					humanFaces[i].bEyesDetected = true;

				}
			}
			//¼ì²â±Ç×Ó
			Rect noseRect(-1, -1, -1, -1);
			if (!nose_cascade.empty()&&eyesCenter.x!=-1)
			{		
				DetectNose(imgROI, nose_cascade, eyesCenter, eyesWidth, noseRect);
				if (noseRect.x != -1)
				{          
					humanFaces[i].noseRect = noseRect;
					humanFaces[i].noseRect.x += tempRect.x;
					humanFaces[i].noseRect.y += tempRect.y;
					humanFaces[i].bNoseDetected = true;
				}
			}
			//¼ì²â×ì°Í
			Rect mouthRect(-1, -1, -1, -1);
			if (!mouth_cascade.empty()&&noseRect.x!=-1)
			{
				vector<Rect> mouthList;
				DetectMouth(imgROI, mouth_cascade, noseRect, mouthRect);

				if (mouthRect.x != -1)
				{
					mouthRect.x += tempRect.x;
					mouthRect.y += tempRect.y;
					humanFaces[i].mouthRect = mouthRect;
					humanFaces[i].bMouthDetected = true;
				}
			}
			//È·ÈÏÊÇÈËÁ³
			if (!humanFaces[i].bConfirmFace&&humanFaces[i].bEyesDetected&&humanFaces[i].bNoseDetected)
			{
				humanFaces[i].bConfirmFace = true;
				
				ExtractFeatPoints(imageGray, humanFaces[i].faceBox, humanFaces[i].featPoints[1], humanFaces[i].featTrajectoies);
			}
		}	
	}
}
void CFaceAnalyze::DetectBothEyes(const Mat& face, CascadeClassifier &eyeCascade1, CascadeClassifier &eyeCascade2, Point &leftEye, Point &rightEye)
{
	// For default eye.xml or eyeglasses.xml: Finds both eyes in roughly 40% of detected faces, but does not detect closed eyes.	
	const float EYE_SX = 0.16f;
	const float EYE_SY = 0.26f;
	const float EYE_SW = 0.30f;
	const float EYE_SH = 0.28f;

	int leftX = cvRound(face.cols * EYE_SX);
	int topY = cvRound(face.rows * EYE_SY);
	int widthX = cvRound(face.cols * EYE_SW);
	int heightY = cvRound(face.rows * EYE_SH);
	int rightX = cvRound(face.cols * (1.0 - EYE_SX - EYE_SW));  // Start of right-eye corner

	Mat topLeftOfFace = face(Rect(leftX, topY, widthX, heightY));
	Mat topRightOfFace = face(Rect(rightX, topY, widthX, heightY));
	Rect leftEyeRect, rightEyeRect;

	// Search the left region, then the right region using the 1st eye detector.
	detectLargestObject(topLeftOfFace, eyeCascade1, leftEyeRect, topLeftOfFace.cols);
	detectLargestObject(topRightOfFace, eyeCascade1, rightEyeRect, topRightOfFace.cols);

	// If the eye was not detected, try a different cascade classifier.
	if (leftEyeRect.width <= 0 && !eyeCascade2.empty()) {
		detectLargestObject(topLeftOfFace, eyeCascade2, leftEyeRect, topLeftOfFace.cols);
	}
	//else
	//    cout << "1st eye detector LEFT SUCCESS" << endl;

	// If the eye was not detected, try a different cascade classifier.
	if (rightEyeRect.width <= 0 && !eyeCascade2.empty()) {
		detectLargestObject(topRightOfFace, eyeCascade2, rightEyeRect, topRightOfFace.cols);
	}
	//else
	//    cout << "1st eye detector RIGHT SUCCESS" << endl;

	if (leftEyeRect.width > 0) 
	{   // Check if the eye was detected.
		leftEyeRect.x += leftX;    // Adjust the left-eye rectangle because the face border was removed.
		leftEyeRect.y += topY;
		leftEye = Point(leftEyeRect.x + leftEyeRect.width / 2, leftEyeRect.y + leftEyeRect.height / 2);
	}
	else 
	{
		leftEye = Point(-1, -1);    // Return an invalid point
	}

	if (rightEyeRect.width > 0) 
	{ // Check if the eye was detected.
		rightEyeRect.x += rightX; // Adjust the right-eye rectangle, since it starts on the right side of the image.
		rightEyeRect.y += topY;  // Adjust the right-eye rectangle because the face border was removed.
		rightEye = Point(rightEyeRect.x + rightEyeRect.width / 2, rightEyeRect.y + rightEyeRect.height / 2);
	}
	else 
	{
		rightEye = Point(-1, -1);    // Return an invalid point
	}
}
void CFaceAnalyze::detectLargestObject(const Mat &img, CascadeClassifier &cascade, Rect &largestObject, int scaledWidth)
{
	// Only search for just 1 object (the biggest in the image).
	int flags = CASCADE_FIND_BIGGEST_OBJECT;// | CASCADE_DO_ROUGH_SEARCH;
	// Smallest object size.
	Size minFeatureSize = Size(10, 10);
	// How detailed should the search be. Must be larger than 1.0.
	float searchScaleFactor = 1.1f;
	// How much the detections should be filtered out. This should depend on how bad false detections are to your system.
	// minNeighbors=2 means lots of good+bad detections, and minNeighbors=6 means only good detections are given but some are missed.
	int minNeighbors = 3;

	// Perform Object or Face Detection, looking for just 1 object (the biggest in the image).
	vector<Rect> objects;
	detectObjectsCustom(img, cascade, objects, scaledWidth, flags, minFeatureSize, searchScaleFactor, minNeighbors);
	if (objects.size() > 0) {
		// Return the only detected object.
		largestObject = (Rect)objects.at(0);
	}
	else {
		// Return an invalid rect.
		largestObject = Rect(-1, -1, -1, -1);
	}
}
void CFaceAnalyze::detectObjectsCustom(const Mat &img, CascadeClassifier &cascade, vector<Rect> &objects, int scaledWidth, int flags, Size minFeatureSize, float searchScaleFactor, int minNeighbors)
{
	// If the input image is not grayscale, then convert the BGR or BGRA color image to grayscale.
	Mat gray;
	if (img.channels() == 3) 
	{
		cvtColor(img, gray, CV_BGR2GRAY);
	}
	else if (img.channels() == 4) 
	{
		cvtColor(img, gray, CV_BGRA2GRAY);
	}
	else 
	{
		// Access the input image directly, since it is already grayscale.
		gray = img;
	}

	// Possibly shrink the image, to run much faster.
	Mat inputImg;
	float scale = img.cols / (float)scaledWidth;
	if (img.cols > scaledWidth) 
	{
		// Shrink the image while keeping the same aspect ratio.
		int scaledHeight = cvRound(img.rows / scale);
		resize(gray, inputImg, Size(scaledWidth, scaledHeight));
	}
	else
	{
		// Access the input image directly, since it is already small.
		inputImg = gray;
	}

	// Standardize the brightness and contrast to improve dark images.
	Mat equalizedImg;
	equalizeHist(inputImg, equalizedImg);

	// Detect objects in the small grayscale image.
	cascade.detectMultiScale(equalizedImg, objects, searchScaleFactor, minNeighbors, flags, minFeatureSize);

	// Enlarge the results if the image was temporarily shrunk before detection.
	if (img.cols > scaledWidth) 
	{
		for (int i = 0; i < (int)objects.size(); i++) 
		{
			objects[i].x = cvRound(objects[i].x * scale);
			objects[i].y = cvRound(objects[i].y * scale);
			objects[i].width = cvRound(objects[i].width * scale);
			objects[i].height = cvRound(objects[i].height * scale);
		}
	}

	// Make sure the object is completely within the image, in case it was on a border.
	for (int i = 0; i < (int)objects.size(); i++) 
	{
		if (objects[i].x < 0)
			objects[i].x = 0;
		if (objects[i].y < 0)
			objects[i].y = 0;
		if (objects[i].x + objects[i].width > img.cols)
			objects[i].x = img.cols - objects[i].width;
		if (objects[i].y + objects[i].height > img.rows)
			objects[i].y = img.rows - objects[i].height;
	}

	// Return with the detected face rectangles stored in "objects".
}

void CFaceAnalyze::DetectNose(Mat& face, CascadeClassifier &noseCascade, Point& eyesCenter, int eyesWidth, Rect& noseRect)
{
	const float NOSE_SX = 0.30f;
	const float NOSE_SY = 0.30f;
	const float NOSE_SW = 0.40f;
	const float NOSE_SH = 0.50f;

	int leftX = cvRound(face.cols * NOSE_SX); 
	int topY = cvRound(face.rows * NOSE_SY);
	int widthX = cvRound(face.cols * NOSE_SW);
	int heightY = cvRound(face.rows * NOSE_SH);

	Mat centerOfFace = face(Rect(leftX, topY, widthX, heightY));
	Mat equalizedImg;
	equalizeHist(centerOfFace, equalizedImg);
	vector<Rect> noseList;
	noseCascade.detectMultiScale(equalizedImg, noseList, 1.1, 3, 0 | CV_HAAR_SCALE_IMAGE, Size(10, 10));
	
	if (noseList.size())
	{
		Point nosePoint;
		eyesCenter.x -= leftX;
		eyesCenter.y -= topY;
		if (noseList.size() == 1)
		{
			nosePoint.x = noseList[0].x + noseList[0].width / 2;
			//nosePoint.y = noseList[0].y + noseList[0].height / 2;
			if (abs(nosePoint.x - eyesCenter.x) < (eyesWidth>>1 ))
			{
				noseRect = noseList[0];
				noseRect.x += leftX;
				noseRect.y += topY;
			}
		}
		else
		{
			int offsetDis, minOffsetDis = 999;
			for (int i = 0; i < noseList.size(); i++)
			{
				nosePoint.x = noseList[i].x + noseList[i].width / 2;
				//nosePoint.y = noseList[i].y + noseList[i].height / 2;
				offsetDis = abs(nosePoint.x - eyesCenter.x);
				if (offsetDis < minOffsetDis)
				{
					minOffsetDis = offsetDis;
					noseRect = noseList[i];
					noseRect.x += leftX;
					noseRect.y += topY;
				}
			}
		}
	}
}
void CFaceAnalyze::DetectMouth(Mat& face, CascadeClassifier &mouthCascade, Rect& noseRect, Rect& mouthRect)
{
	const float MOUTH_SX = 0.30f;
	const float MOUTH_SY = 0.60f;
	const float MOUTH_SW = 0.40f;
	const float MOUTH_SH = 0.40f;

	int leftX = cvRound(face.cols * MOUTH_SX);
	int topY = cvRound(face.rows * MOUTH_SY);
	int widthX = cvRound(face.cols * MOUTH_SW);
	int heightY = cvRound(face.rows * MOUTH_SH);

	Mat bottomOfFace = face(Rect(leftX, topY, widthX, heightY));
	Mat equalizedImg;
	equalizeHist(bottomOfFace, equalizedImg);

	vector<Rect> mouthList;
	mouthCascade.detectMultiScale(equalizedImg, mouthList, 1.1, 3, 0 | CV_HAAR_SCALE_IMAGE, Size(10, 10));

	Point nosePoint = Point(noseRect.x + noseRect.width / 2, noseRect.y + noseRect.height / 2);
	nosePoint.x -= leftX;
	nosePoint.y -= topY;
	for (int i = 0; i < mouthList.size(); i++)
	{
		Point mouthPoint = Point(mouthList[i].x + mouthList[i].width / 2, mouthList[i].y + mouthList[i].height / 2);
		if (mouthPoint.y - nosePoint.y>(noseRect.height>>1)
			&& abs(mouthPoint.y-nosePoint.y) < (noseRect.height<<1)
			&& abs(mouthPoint.x - nosePoint.x) < (noseRect.width >> 2))
		{
			mouthRect = mouthList[i];
			mouthList.resize(1);
			mouthRect.x += leftX;
			mouthRect.y += topY;
			break;
		}
	}
}