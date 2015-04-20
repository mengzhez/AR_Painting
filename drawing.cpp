#include <cv.h>
#include <math.h>
#include <highgui.h>
#include <opencv2\opencv.hpp>

#define PI 3.14159265

using namespace cv;
using namespace std;

// Variables
bool drawSwitch = false;
int currentColor = 0;

class point
{
public:
	// Position
	int x;
	int y;

	// Color
	int c;

	// Discontinuity
	bool d;

	// Visiblity
	bool v;

	point()
	{
		d = false;
		v = false;
	}

	~point()
	{
	}
};

point pts[20];
point drawPts[640 * 480];
int num = 0;

// Functions
void pointRecognize(IplImage* img);
void pointSelect(IplImage* img);
bool colorCheck(IplImage* img, int y, int x, CvScalar goalColor);
IplImage* display(IplImage* img);
IplImage* colorBar(IplImage* img);
void hough(IplImage* img);

int main(int argc, char** argv)
{
	CvCapture *capture;

	// Creat Windows
	cvNamedWindow("webcam", 1); // 640*480

	// Open Camera
	if (!(capture = cvCaptureFromCAM(0)))
	{
		fprintf(stderr, "Can not open camera.\n");
		cvWaitKey(0);
		return -2;
	}

	// Show video
	IplImage *frame = cvQueryFrame(capture);
	while (1)
	{
		// Mirror the frame
		cvFlip(frame, frame, 1);

		// Draw
		pointRecognize(frame);
		pointSelect(frame);
		//hough(frame);

		// Display
		frame = display(frame);
		frame = colorBar(frame);

		// Load next frame into Main Memory
		cvShowImage("webcam", frame);
		frame = cvQueryFrame(capture);

		char k = cvWaitKey(100);
		if (k == 32)
		{
			// Spacebar (Turn on and turn off the drawing switch)
			if (drawSwitch == true)
			{
				drawSwitch = false;
				drawPts[num - 1].d = true;
			}
			else
			{
				drawSwitch = true;
			}
		}
		else if (k == 'z')
		{
			// Change color (<-);
			if (currentColor > 0 && currentColor <= 4)
			{
				currentColor--;
			}
		}
		else if (k == 'x')
		{
			// Change color (->);
			if (currentColor >= 0 && currentColor < 4)
			{
				currentColor++;
			}
		}
		else if (k == 'c')
		{
			// Clear the drawing;
			num = 0;
			drawSwitch = false;
		}
		else if (k == 27)
		{
			// ESC (Quit)
			break;
		}
	}
	cvReleaseCapture(&capture);
	cvDestroyWindow("webcam");
}

void pointRecognize(IplImage* img)
{
	int count = 0;
	int blackCount = 0;
	CvScalar red = CV_RGB(240, 130, 130);
	CvScalar black = CV_RGB(64, 64, 75);

	// Loop for all pixels of the image
	for (int i = 30; i < img->height - 30; i = i + 2)
	{
		for (int j = 50; j < img->width - 50; j = j + 2)
		{
			if (colorCheck(img, i, j, red))
			{
				int r = 20;
				for (float theta = 0; theta < 360; theta = theta + 15)
				{
					// Scan the points around the target
					int y = int(i + r*sin(theta*PI / 180));
					int x = int(j + r*cos(theta*PI / 180));
					if (y<0 || y>img->height)
					{
						y = i;
					}
					if (x<0 || x>img->width)
					{
						x = j;
					}
					if (colorCheck(img, y, x, black))
					{
						blackCount++;
						if (blackCount == 24)
						{
							pts[count].y = i;
							pts[count].x = j;
							count++;
							if (count == 20)
							{
								break;
							}
						}
					}
				}
				blackCount = 0;
			}
			else
			{
				continue;
			}
		}
	}
}

void pointSelect(IplImage* img)
{
	int count = 0;
	float avgX = 0;
	float avgY = 0;
	int r = 15;

	// Calcuate the average value of the point position
	for (int i = 0; i < 20; i++)
	{
		if (pts[i].x != 0)
		{
			count++;
			avgX = avgX + pts[i].x;
			avgX = avgX + pts[i].y;
		}
		else
		{
			break;
		}
	}
	avgX = avgX / count;
	avgY = avgY / count;
	float minD = sqrt((pts[0].x - avgX)*(pts[0].x - avgX) + (pts[0].y - avgY)*(pts[0].y - avgY));
	count = 0;
	for (int i = 1; i < count; i++)
	{
		float d = sqrt((pts[i].x - avgX)*(pts[i].x - avgX) + (pts[i].y - avgY)*(pts[i].y - avgY));
		if (d < minD)
		{
			minD = d;
			count = i;
		}
	}

	// Add to global variable
	if (num >= 2)
	{
		if (abs(drawPts[num - 1].x - pts[count].x) < 70 && abs(drawPts[num - 1].y - pts[count].y) < 70)
		{
			if (drawSwitch == true)
			{
				drawPts[num].v = true;
			}
			drawPts[num].x = pts[count].x;
			drawPts[num].y = pts[count].y;
			drawPts[num].c = currentColor;
			num++;
		}
		else
		{
			if (drawPts[num - 1].d)
			{
				if (drawSwitch == true)
				{
					drawPts[num].v = true;
				}
				drawPts[num].x = pts[count].x;
				drawPts[num].y = pts[count].y;
				drawPts[num].c = currentColor;
				num++;
			}
			else if (drawPts[num - 1].x == 0 && drawPts[num - 1].y == 0)
			{
				if (drawSwitch == true)
				{
					drawPts[num].v = true;
				}
				drawPts[num].x = pts[count].x;
				drawPts[num].y = pts[count].y;
				drawPts[num].c = currentColor;
				num++;
			}
			else
			{
				// Wrong point
			}
		}
	}
	else
	{
		if (drawSwitch == true)
		{
			drawPts[num].v = true;
		}
		drawPts[num].x = pts[count].x;
		drawPts[num].y = pts[count].y;
		drawPts[num].c = currentColor;
		num++;
	}
}

bool colorCheck(IplImage* img, int y, int x, CvScalar goalColor)
{
	if (y<0 || y>img->height || x<0 || x>img->width)
	{
		fprintf(stderr, "The selected pixel is out of window.\n");
	}
	CvScalar color = cvGet2D(img, y, x);
	float range = 60;
	if (abs(color.val[0] - goalColor.val[0]) < range && abs(color.val[1] - goalColor.val[1]) < range && abs(color.val[2] - goalColor.val[2]) < range)
	{
		return true;
	}
	else
	{
		return false;
	}
}

IplImage* display(IplImage* img)
{
	Mat image(img, 0);
	Mat pointer = imread("images/pointer.png");
	Mat mask = imread("images/pointer.png", 0);
	Mat imageROI;

	imageROI = image(Rect(drawPts[num - 1].x, drawPts[num - 1].y, pointer.cols, pointer.rows));
	pointer.copyTo(imageROI, mask);
	*img = IplImage(image);

	for (int i = 2; i < num; i++)
	{
		if (drawPts[i - 1].d)
		{
			continue;
		}
		else
		{
			if (drawPts[i].v == true && drawPts[i - 1].v == true)
			{
				CvPoint pt1;
				CvPoint pt2;
				CvScalar color;
				pt1.x = drawPts[i].x;
				pt1.y = drawPts[i].y;
				pt2.x = drawPts[i - 1].x;
				pt2.y = drawPts[i - 1].y;

				switch (drawPts[i].c)
				{
				case 0: color = CV_RGB(250, 85, 20);
					break;
				case 1: color = CV_RGB(250, 200, 55);
					break;
				case 2: color = CV_RGB(10, 185, 220);
					break;
				case 3: color = CV_RGB(135, 205, 45);
					break;
				case 4: color = CV_RGB(170, 170, 170);
					break;
				default: break;
				}

				cvLine(img, pt1, pt2, color, 3, CV_AA, 0);
			}
		}
	}
	return img;
}

IplImage* colorBar(IplImage* img)
{
	Mat image(img, 0);
	Mat imageROI;
	for (int i = 0; i < 5; i++)
	{
		Mat color;
		Mat mask;
		switch (i)
		{
		case 0: color = imread("images/red.png");
			mask = imread("images/red.png", 0);
			break;
		case 1: color = imread("images/yellow.png");
			mask = imread("images/yellow.png", 0);
			break;
		case 2: color = imread("images/blue.png");
			mask = imread("images/blue.png", 0);
			break;
		case 3: color = imread("images/green.png");
			mask = imread("images/green.png", 0);
			break;
		case 4: color = imread("images/gray.png");
			mask = imread("images/gray.png", 0);
			break;
		default: break;
		}
		imageROI = image(Rect(i * 100 + 40, img->height - 100, color.cols, color.rows));
		color.copyTo(imageROI, mask);
	}

	*img = IplImage(image);

	// Selected color
	CvPoint ctr;
	ctr.x = 40 + currentColor * 100 + 36;
	ctr.y = img->height - 100 + 36;
	cvCircle(img, ctr, 36, CV_RGB(0, 165, 230), 3, 8, 0);
	return img;
}

// ======================= Testing codes =======================

// Hough detection (not used now)
void hough(IplImage* img)
{
	// Variables
	IplImage* dst;
	IplImage* color_dst;
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* lines = 0;
	int i = 0;
	int j = 0;
	float lk[100];
	float lb[100];
	float ly[100];

	dst = cvCreateImage(cvGetSize(img), 8, 1);
	color_dst = cvCreateImage(cvGetSize(img), 8, 3);

	cvCanny(img, dst, 580, 600, 3);
	cvCvtColor(dst, color_dst, CV_GRAY2BGR);

	// Standard Hough Algorithm
	lines = cvHoughLines2(dst, storage, CV_HOUGH_STANDARD, 1, CV_PI / 180, 100, 0, 0);

	for (int i = 0; i < MIN(lines->total, 100); i++)
	{
		float* line = (float*)cvGetSeqElem(lines, i);
		float rho = line[0];
		float theta = line[1];
		CvPoint pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));

		// Describe the linear equation
		if (pt1.x - pt2.x == 0)
		{
			lk[j] = 0;
			lb[j] = 0;
			ly[j] = pt1.x;
			j++;
		}
		else
		{
			lk[j] = (float)(pt1.y - pt2.y) / (pt1.x - pt2.x);
			lb[j] = (float)(pt1.x * pt2.y - pt2.x * pt1.y) / (pt1.x - pt2.x);
			ly[j] = 0;
			if (j != 0)
			{
				for (int k = 0; k < j; k++)
				{
					if (lk[j] > lk[k] - 0.5 && lk[j] < lk[k] + 0.5 && lb[j] > lb[k] - 5 && lb[j] < lb[k] + 5)
					{
						continue;
					}
					else
					{
						cvLine(color_dst, pt1, pt2, CV_RGB(255, 0, 0), 3, CV_AA, 0);
						j++;
					}
				}
			}
			else
			{
				j++;
			}
		}
	}

	// Probabilistic Hough Algorithm
	//  lines = cvHoughLines2( dst, storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI/180, 50, 50, 10 );
	//  for( i = 0; i < lines->total; i++ )
	//  {
	//      CvPoint* line = (CvPoint*)cvGetSeqElem(lines,i);


	//// Describe the linear equation
	//if(line[0].x - line[1].x == 0)
	//{
	//	lk[j] = 0;
	//	lb[j] = 0;
	//	ly[j] = line[0].x;
	//}
	//else
	//{
	//	lk[j] = (float)(line[0].y - line[1].y) / (line[0].x - line[1].x);
	//	lb[j] = (float)(line[0].x * line[1].y - line[1].x * line[0].y) / (line[0].x - line[1].x);
	//	ly[j] = 0;
	//	if(j != 0)
	//	{
	//		for(int k = 0; k < j; k++)
	//		{
	//			if(lk[j] > lk[k]-0.3 && lk[j] < lk[k]+0.3 && lb[j] > lb[k]-5 && lb[j] < lb[k]+5)
	//			{
	//				continue;
	//			}
	//			else
	//			{
	//				cvLine( color_dst, line[0], line[1], CV_RGB(255,0,0), 3, CV_AA, 0 );
	//				j++;
	//			}
	//		}
	//	}
	//	else
	//	{
	//		j++;
	//	}
	//}
	//  }

	// Find the point of intersection
	int count = 0;
	float r = 5;
	for (int i = 30; i < img->height - 30; i = i + 2)
	{
		count = 0;
		for (int j = 50; j < img->width - 50; j = j + 2)
		{
			count = 0;
			for (int k = 0; k<MIN(lines->total, 100); k++)
			{
				if (ly[k] == 0)
				{
					if (lk[k] * j + lb[k] > i - r && lk[k] * j + lb[k] < i + r)
					{
						count++;
					}
				}
				else
				{
					if (ly[k] > j - r && ly[k] < j + r)
					{
						count++;
					}
				}

				if (count >= 3)
				{
					CvPoint center;
					center.y = i;
					center.x = j;
					CvScalar drawColor = cvScalar(230, 165, 0, 0);
					cvCircle(color_dst, center, 3, drawColor, 5, 8, 0);
					count = 0;
				}
			}
			count = 0;
		}
	}

	cvNamedWindow("Hough", 1);
	cvShowImage("Hough", color_dst);
}
