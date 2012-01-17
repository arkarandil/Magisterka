// VidSpliter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <opencv2\opencv.hpp>
using namespace std;
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include "EssentialSolver.h"

int n_boards = 0; //Will be set by input list
const int board_dt = 25; //Wait 20 frames per chessboard view
int board_w;
int board_h;
int calibrate(int argc, char* argv[]) {
  if(argc != 4){
	printf("ERROR: Wrong number of input parameters\n");
	return -1;
  }
  board_w  = atoi(argv[1]);
  board_h  = atoi(argv[2]);
  n_boards = atoi(argv[3]);
  int board_n  = board_w * board_h;
  CvSize board_sz = cvSize( board_w, board_h );
  //CvCapture* capture = cvCreateCameraCapture( 0 );
  CvCapture* capture = cvCreateFileCapture("C:\\video\\calib2.MOV");
  assert( capture );
  cvNamedWindow( "Calibration" );
  //ALLOCATE STORAGE
  CvMat* image_points      = cvCreateMat(n_boards*board_n,2,CV_32FC1);
  CvMat* object_points     = cvCreateMat(n_boards*board_n,3,CV_32FC1);
  CvMat* point_counts      = cvCreateMat(n_boards,1,CV_32SC1);
  CvMat* intrinsic_matrix  = cvCreateMat(3,3,CV_32FC1);
  CvMat* distortion_coeffs = cvCreateMat(5,1,CV_32FC1);
  CvPoint2D32f* corners = new CvPoint2D32f[ board_n ];
  int corner_count;
  int successes = 0;
  int step, frame = 0;

   IplImage *image = cvQueryFrame( capture );
 IplImage *gray_image = cvCreateImage(cvGetSize(image),8,1);//subpixel
 // CAPTURE CORNER VIEWS LOOP UNTIL WE’VE GOT n_boards
 // SUCCESSFUL CAPTURES (ALL CORNERS ON THE BOARD ARE FOUND)
 //
 while(successes < n_boards) {
   //Skip every board_dt frames to allow user to move chessboard
   if(frame++ % board_dt == 0) {
      //Find chessboard corners:
      int found = cvFindChessboardCorners(
               image, board_sz, corners, &corner_count,
               CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS
      );
      //Get Subpixel accuracy on those corners
      cvCvtColor(image, gray_image, CV_BGR2GRAY);
      cvFindCornerSubPix(gray_image, corners, corner_count,
                 cvSize(11,11),cvSize(-1,-1), cvTermCriteria(
                 CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
      //Draw it
      cvDrawChessboardCorners(image, board_sz, corners,
                 corner_count, found);
      cvShowImage( "Calibration", image );
      // If we got a good board, add it to our data
      if( corner_count == board_n ) {
         step = successes*board_n;
         for( int i=step, j=0; j<board_n; ++i,++j ) {
            CV_MAT_ELEM(*image_points, float,i,0) = corners[j].x;
            CV_MAT_ELEM(*image_points, float,i,1) = corners[j].y;
            CV_MAT_ELEM(*object_points,float,i,0) = j/board_w;
            CV_MAT_ELEM(*object_points,float,i,1) = j%board_w;
            CV_MAT_ELEM(*object_points,float,i,2) = 0.0f;
         }
         CV_MAT_ELEM(*point_counts, int,successes,0) = board_n;
         successes++;
      }
   } //end skip board_dt between chessboard capture

    //Handle pause/unpause and ESC
    int c = cvWaitKey(15);
    if(c == 'p'){
       c = 0;
       while(c != 'p' && c != 27){
            c = cvWaitKey(250);
       }
     }
     if(c == 27)
        return 0;
	   image = cvQueryFrame( capture ); //Get next image
  } //END COLLECTION WHILE LOOP.
  //ALLOCATE MATRICES ACCORDING TO HOW MANY CHESSBOARDS FOUND
  CvMat* object_points2  = cvCreateMat(successes*board_n,3,CV_32FC1);
  CvMat* image_points2   = cvCreateMat(successes*board_n,2,CV_32FC1);
  CvMat* point_counts2   = cvCreateMat(successes,1,CV_32SC1);
  //TRANSFER THE POINTS INTO THE CORRECT SIZE MATRICES
  //Below, we write out the details in the next two loops. We could
  //instead have written:
  //image_points->rows = object_points->rows  = \
  //successes*board_n; point_counts->rows = successes;
  //
  for(int i = 0; i<successes*board_n; ++i) {
      CV_MAT_ELEM( *image_points2, float, i, 0) =
             CV_MAT_ELEM( *image_points, float, i, 0);
      CV_MAT_ELEM( *image_points2, float,i,1) =
             CV_MAT_ELEM( *image_points, float, i, 1);
      CV_MAT_ELEM(*object_points2, float, i, 0) =
             CV_MAT_ELEM( *object_points, float, i, 0) ;
      CV_MAT_ELEM( *object_points2, float, i, 1) =
             CV_MAT_ELEM( *object_points, float, i, 1) ;
      CV_MAT_ELEM( *object_points2, float, i, 2) =
             CV_MAT_ELEM( *object_points, float, i, 2) ;
  }
  for(int i=0; i<successes; ++i){ //These are all the same number
    CV_MAT_ELEM( *point_counts2, int, i, 0) =
             CV_MAT_ELEM( *point_counts, int, i, 0);
  }
  cvReleaseMat(&object_points);
  cvReleaseMat(&image_points);
  cvReleaseMat(&point_counts);
  // At this point we have all of the chessboard corners we need.
  // Initialize the intrinsic matrix such that the two focal
  // lengths have a ratio of 1.0
  //
  CV_MAT_ELEM( *intrinsic_matrix, float, 0, 0 ) = 1.0f;
  CV_MAT_ELEM( *intrinsic_matrix, float, 1, 1 ) = 1.0f;
  //CALIBRATE THE CAMERA!
  cvCalibrateCamera2(
      object_points2, image_points2,
      point_counts2,  cvGetSize( image ),
      intrinsic_matrix, distortion_coeffs,
      NULL, NULL,0  //CV_CALIB_FIX_ASPECT_RATIO
  );
  // SAVE THE INTRINSICS AND DISTORTIONS
  cvSave("C:/Intrinsics.xml",intrinsic_matrix);
  cvSave("C:/Distortion.xml",distortion_coeffs);
   // EXAMPLE OF LOADING THESE MATRICES BACK IN:
  CvMat *intrinsic = (CvMat*)cvLoad("C:/Intrinsics.xml");
  CvMat *distortion = (CvMat*)cvLoad("C:/Distortion.xml");
  // Build the undistort map that we will use for all
  // subsequent frames.
  //
  IplImage* mapx = cvCreateImage( cvGetSize(image), IPL_DEPTH_32F, 1 );
  IplImage* mapy = cvCreateImage( cvGetSize(image), IPL_DEPTH_32F, 1 );
  cvInitUndistortMap(
    intrinsic,
    distortion,
    mapx,
    mapy
  );
  // Just run the camera to the screen, now showing the raw and
  // the undistorted image.
  //
  cvNamedWindow( "Undistort" );
  while(image) {
    IplImage *t = cvCloneImage(image);
    cvShowImage( "Calibration", image ); // Show raw image
    cvRemap( t, image, mapx, mapy );     // Undistort image
    cvReleaseImage(&t);
    cvShowImage("Undistort", image);     // Show corrected image
    //Handle pause/unpause and ESC
    int c = cvWaitKey(15);
    if(c == 'p') {
       c = 0;
       while(c != 'p' && c != 27) {
            c = cvWaitKey(250);
       }
    }
    if(c == 27)
        break;
    image = cvQueryFrame( capture );
  }
  return 0;
}

void calibrateStereo(int nx,int ny) {
	int useUncalibrated =0;
 //f Image List
 	int displayCorners = 1;
	const int maxScale = 1;
	const float squareSize = 1.f; //Set this to your actual square size
	FILE* f = fopen("C:/list.txt", "rt");
	int i, j, lr, nframes, n = nx*ny, N = 0;
	vector<string> imageNames[2];
	vector<CvPoint3D32f> objectPoints;
	vector<CvPoint2D32f> points[2];
	vector<int> npoints;
	vector<uchar> active[2];
	vector<CvPoint2D32f> temp(n);
	CvSize imageSize = {0,0};
	// ARRAY AND VECTOR STORAGE:
	double M1[3][3], M2[3][3], D1[5], D2[5];
	double R[3][3], T[3], E[3][3], F[3][3];
	CvMat _M1 = cvMat(3, 3, CV_64F, M1 );
	CvMat _M2 = cvMat(3, 3, CV_64F, M2 );
	CvMat _D1 = cvMat(1, 5, CV_64F, D1 );
	CvMat _D2 = cvMat(1, 5, CV_64F, D2 );
	CvMat _R = cvMat(3, 3, CV_64F, R );
	CvMat _T = cvMat(3, 1, CV_64F, T );
	CvMat _E = cvMat(3, 3, CV_64F, E );
	 CvMat _F = cvMat(3, 3, CV_64F, F );
    if( displayCorners )
        cvNamedWindow( "corners", 1 );
// READ IN THE LIST OF CHESSBOARDS:
    if( !f )
    {
        fprintf(stderr, "can not open file \n");
        return;
    }
    for(i=0;;i++)
    {
        char buf[1024];
        int count = 0, result=0;
        lr = i % 2;
        vector<CvPoint2D32f>& pts = points[lr];
        if( !fgets( buf, sizeof(buf)-3, f ))
            break;
        size_t len = strlen(buf);
        while( len > 0 && isspace(buf[len-1]))
            buf[--len] = '\0';
        if( buf[0] == '#')
            continue;
        IplImage* img = cvLoadImage( buf, 0 );
        if( !img )
            break;
        imageSize = cvGetSize(img);
        imageNames[lr].push_back(buf);
    //FIND CHESSBOARDS AND CORNERS THEREIN:
        for( int s = 1; s <= maxScale; s++ )
        {
            IplImage* timg = img;
            if( s > 1 )
            {
                timg = cvCreateImage(cvSize(img->width*s,img->height*s),
                    img->depth, img->nChannels );
                cvResize( img, timg, CV_INTER_CUBIC );
            }
            result = cvFindChessboardCorners( timg, cvSize(nx, ny),
                &temp[0], &count,
                CV_CALIB_CB_ADAPTIVE_THRESH |
                CV_CALIB_CB_NORMALIZE_IMAGE);
            if( timg != img )
                cvReleaseImage( &timg );
            if( result || s == maxScale )
                for( j = 0; j < count; j++ )
            {
                temp[j].x /= s;
                temp[j].y /= s;
            }
            if( result )
                break;
        }
        if( displayCorners )
			  {
            printf("%s\n", buf);
            IplImage* cimg = cvCreateImage( imageSize, 8, 3 );
            cvCvtColor( img, cimg, CV_GRAY2BGR );
            cvDrawChessboardCorners( cimg, cvSize(nx, ny), &temp[0],
                count, result );
            cvShowImage( "corners", cimg );
			cv::waitKey(0);
            cvReleaseImage( &cimg );
        }
        else
            putchar('.');
        N = pts.size();
        pts.resize(N + n, cvPoint2D32f(0,0));
        active[lr].push_back((uchar)result);
    //assert( result != 0 );
        if( result )
        {
         //Calibration will suffer without subpixel interpolation
            cvFindCornerSubPix( img, &temp[0], count,
                cvSize(11, 11), cvSize(-1,-1),
                cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS,
                30, 0.01) );
            copy( temp.begin(), temp.end(), pts.begin() + N );
        }
        cvReleaseImage( &img );
    }
    fclose(f);
    printf("\n");
	nframes = active[0].size();//Number of good chessboads found
    objectPoints.resize(nframes*n);
    for( i = 0; i < ny; i++ )
        for( j = 0; j < nx; j++ )
        objectPoints[i*nx + j] =
        cvPoint3D32f(i*squareSize, j*squareSize, 0);
    for( i = 1; i < nframes; i++ )
        copy( objectPoints.begin(), objectPoints.begin() + n,
        objectPoints.begin() + i*n );
    npoints.resize(nframes,n);
    N = nframes*n;
    CvMat _objectPoints = cvMat(1, N, CV_32FC3, &objectPoints[0] );
    CvMat _imagePoints1 = cvMat(1, N, CV_32FC2, &points[0][0] );
    CvMat _imagePoints2 = cvMat(1, N, CV_32FC2, &points[1][0] );
    CvMat _npoints = cvMat(1, npoints.size(), CV_32S, &npoints[0] );
    cvSetIdentity(&_M1);
    cvSetIdentity(&_M2);
    cvZero(&_D1);
    cvZero(&_D2);
	 fflush(stdout);
    cvStereoCalibrate( &_objectPoints, &_imagePoints1,
        &_imagePoints2, &_npoints,
        &_M1, &_D1, &_M2, &_D2,
        imageSize, &_R, &_T, &_E, &_F,
        cvTermCriteria(CV_TERMCRIT_ITER+
        CV_TERMCRIT_EPS, 100, 1e-5),
        CV_CALIB_FIX_ASPECT_RATIO +
        CV_CALIB_ZERO_TANGENT_DIST +
        CV_CALIB_SAME_FOCAL_LENGTH );
    printf(" done\n");
// CALIBRATION QUALITY CHECK
// because the output fundamental matrix implicitly
// includes all the output information,
// we can check the quality of calibration using the
// epipolar geometry constraint: m2^t*F*m1=0
    vector<CvPoint3D32f> lines[2];
    points[0].resize(N);
    points[1].resize(N);
    _imagePoints1 = cvMat(1, N, CV_32FC2, &points[0][0] );
    _imagePoints2 = cvMat(1, N, CV_32FC2, &points[1][0] );
    lines[0].resize(N);
    lines[1].resize(N);
    CvMat _L1 = cvMat(1, N, CV_32FC3, &lines[0][0]);
    CvMat _L2 = cvMat(1, N, CV_32FC3, &lines[1][0]);
//Always work in undistorted space
    cvUndistortPoints( &_imagePoints1, &_imagePoints1,
        &_M1, &_D1, 0, &_M1 );
    cvUndistortPoints( &_imagePoints2, &_imagePoints2,
        &_M2, &_D2, 0, &_M2 );
    cvComputeCorrespondEpilines( &_imagePoints1, 1, &_F, &_L1 );
    cvComputeCorrespondEpilines( &_imagePoints2, 2, &_F, &_L2 );
    double avgErr = 0;
    for( i = 0; i < N; i++ )
    {
        double err = fabs(points[0][i].x*lines[1][i].x +
            points[0][i].y*lines[1][i].y + lines[1][i].z)
            + fabs(points[1][i].x*lines[0][i].x +
            points[1][i].y*lines[0][i].y + lines[0][i].z);
        avgErr += err;
    }
    printf( "avg err = %g\n", avgErr/(nframes*n) );
//COMPUTE AND DISPLAY RECTIFICATION
    if( 1)
    {
        CvMat* mx1 = cvCreateMat( imageSize.height,
            imageSize.width, CV_32F );
        CvMat* my1 = cvCreateMat( imageSize.height,
            imageSize.width, CV_32F );
        CvMat* mx2 = cvCreateMat( imageSize.height,
            imageSize.width, CV_32F );
        CvMat* my2 = cvCreateMat( imageSize.height,
			  imageSize.width, CV_32F );
        CvMat* img1r = cvCreateMat( imageSize.height,
            imageSize.width, CV_8U );
        CvMat* img2r = cvCreateMat( imageSize.height,
            imageSize.width, CV_8U );
        CvMat* disp = cvCreateMat( imageSize.height,
            imageSize.width, CV_16S );
        CvMat* vdisp = cvCreateMat( imageSize.height,
            imageSize.width, CV_8U );
        CvMat* pair;
        double R1[3][3], R2[3][3], P1[3][4], P2[3][4];
        CvMat _R1 = cvMat(3, 3, CV_64F, R1);
        CvMat _R2 = cvMat(3, 3, CV_64F, R2);
		if( useUncalibrated == 0 )
        {
            CvMat _P1 = cvMat(3, 4, CV_64F, P1);
            CvMat _P2 = cvMat(3, 4, CV_64F, P2);
            cvStereoRectify( &_M1, &_M2, &_D1, &_D2, imageSize,
                &_R, &_T,
                &_R1, &_R2, &_P1, &_P2, 0,
                0/*CV_CALIB_ZERO_DISPARITY*/ );
    //Precompute maps for cvRemap()
            cvInitUndistortRectifyMap(&_M1,&_D1,&_R1,&_P1,mx1,my1);
            cvInitUndistortRectifyMap(&_M2,&_D2,&_R2,&_P2,mx2,my2);
		}
		else if( useUncalibrated == 1 || useUncalibrated == 2 )
		{
            double H1[3][3], H2[3][3], iM[3][3];
            CvMat _H1 = cvMat(3, 3, CV_64F, H1);
            CvMat _H2 = cvMat(3, 3, CV_64F, H2);
            CvMat _iM = cvMat(3, 3, CV_64F, iM);
    //Just to show you could have independently used F
            if( useUncalibrated == 2 )
                cvFindFundamentalMat( &_imagePoints1,
                &_imagePoints2, &_F);
            cvStereoRectifyUncalibrated( &_imagePoints1,
                &_imagePoints2, &_F,
                imageSize,
                &_H1, &_H2, 3);
            cvInvert(&_M1, &_iM);
            cvMatMul(&_H1, &_M1, &_R1);
            cvMatMul(&_iM, &_R1, &_R1);
            cvInvert(&_M2, &_iM);
            cvMatMul(&_H2, &_M2, &_R2);
            cvMatMul(&_iM, &_R2, &_R2);
			cvInitUndistortRectifyMap(&_M1,&_D1,&_R1,&_M1,mx1,my1);
            cvInitUndistortRectifyMap(&_M2,&_D1,&_R2,&_M2,mx2,my2);
        }
        else
            assert(0);
        cvNamedWindow( "rectified", 1 );
		cvWaitKey(0) == 27;
		pair = cvCreateMat( imageSize.height*2, imageSize.width,CV_8UC3 );
		CvStereoBMState *BMState = cvCreateStereoBMState();
        assert(BMState != 0);

        BMState->speckleWindowSize=0;
		BMState->speckleRange=0;
		BMState->preFilterSize=41;
		BMState->preFilterCap=31;
		BMState->SADWindowSize=41;
		BMState->minDisparity=-64;
		BMState->numberOfDisparities=128;
		BMState->textureThreshold=10;
		BMState->uniquenessRatio=10;
		CvCapture* capture1 = cvCreateCameraCapture(-5 );
		CvCapture* capture2= cvCreateCameraCapture( -4);
		IplImage *img1= cvCreateImage( imageSize, IPL_DEPTH_8U, 1 );
		IplImage *img2=cvCreateImage( imageSize, IPL_DEPTH_8U, 1 );
		IplImage *im1;
		IplImage *im2;
		char sd[2]="a";
        for( i = 0; 1; i++ )
        {
			im1 = cvQueryFrame( capture1 );
			im2 = cvQueryFrame( capture2);
			//img1= cvCreateImage( imageSize, IPL_DEPTH_8U, 1 );
			//img2=cvCreateImage( imageSize, IPL_DEPTH_8U, 1 );
			cvCvtColor(im1,img1,CV_RGB2GRAY);
			cvCvtColor(im2, img2,CV_RGB2GRAY);
			//IplImage* img1=cvLoadImage("C:/left.jpg",0);
            //IplImage* img2=cvLoadImage("C:/right.jpg",0);
            if( img1 && img2 )
            {
                CvMat part;
                cvRemap( img1, img1r, mx1, my1 );
                cvRemap( img2, img2r, mx2, my2 );
                //if( !isVerticalStereo || useUncalibrated != 0 )
                //{
              // When the stereo camera is oriented vertically,
              // useUncalibrated==0 does not transpose the
              // image, so the epipolar lines in the rectified
              // images are vertical. Stereo correspondence
              // function does not support such a case.
				cvShowImage( "disparity1", img1r );
				cvShowImage( "disparity2", img2r );
                    cvFindStereoCorrespondenceBM( img1r, img2r, disp,
                        BMState);
                    cvNormalize( disp, vdisp, 0, 256, CV_MINMAX );
                    cvNamedWindow( "disparity" );
                    cvShowImage( "disparity", vdisp );
					CvMat* disparity_left ;
				/*CvMat* disparity_right;
				CvStereoGCState* state = cvCreateStereoGCState( 16, 2 );
				CvMat* disparity_left_visual;
				disparity_left = cvCreateMat(imageSize.height, imageSize.width, CV_16S );
				 disparity_right = cvCreateMat( imageSize.height, imageSize.width, CV_16S );
				cvFindStereoCorrespondenceGC(img1r, img2r,disparity_left, disparity_right, state, 0 );
				disparity_left_visual = cvCreateMat( imageSize.height, imageSize.width, CV_8U );
				cvConvertScale( disparity_left, disparity_left_visual, -16 );
				cvShowImage( "disparity2",disparity_left_visual );
				cvReleaseStereoGCState( &state );*/

               /* }
                //if( !isVerticalStereo )
                //{
                    cvGetCols( pair, &part, 0, imageSize.width );
                    cvCvtColor( img1r, &part, CV_GRAY2BGR );
                    cvGetCols( pair, &part, imageSize.width,
                        imageSize.width*2 );
              cvCvtColor( img2r, &part, CV_GRAY2BGR );
                    for( j = 0; j < imageSize.height; j += 16 )
                        cvLine( pair, cvPoint(0,j),
                        cvPoint(imageSize.width*2,j),
                        CV_RGB(0,255,0));
                }
                else
                {
                    cvGetRows( pair, &part, 0, imageSize.height );
                    cvCvtColor( img1r, &part, CV_GRAY2BGR );
                    cvGetRows( pair, &part, imageSize.height,
                        imageSize.height*2 );
                    cvCvtColor( img2r, &part, CV_GRAY2BGR );
                    for( j = 0; j < imageSize.width; j += 16 )
                        cvLine( pair, cvPoint(j,0),
                        cvPoint(j,im ageSize.height*2),
                        CV_RGB(0,255,0));
                }*/
                //cvShowImage( "rectified", pair );
                if( cvWaitKey(30) == 27 )
                    break;
            }
            //cvReleaseImage( &img1 );
            //cvReleaseImage( &img2 );
        }
        cvReleaseStereoBMState(&BMState);
        cvReleaseMat( &mx1 );
        cvReleaseMat( &my1 );
        cvReleaseMat( &mx2 );
        cvReleaseMat( &my2 );
        cvReleaseMat( &img1r );
        cvReleaseMat( &img2r );
        cvReleaseMat( &disp );
	}//ddddddd
}
void optFlow();
void mapDisp();
void calculatePositions();




/********************************************MAIN*************************************************************/
int main(int argc, char* argv[])
{
	CvCapture* capture1=NULL;
	CvCapture* capture2=NULL;
	cvNamedWindow ("original", CV_WINDOW_AUTOSIZE);

	//char* paths[]={"C:\\t.flv"};
	EssentialSolver solver;
	solver.init();
	solver.calculatePositions();
	return 0;
	int key;
	
	key=cv::waitKey(0);
  	switch(key){
		case 'o':
			optFlow();
			return 0;
			break;
		case 'p':
			//solver.init();
			//solver.calculatePositions();
			return 0;
			break;
		case 'c':
			calibrate(argc,argv);
			return 0;
			break;
		case 'm':
			mapDisp();
			return 0;
			break;
		case 't':
			calibrateStereo(7,7);
			return 0;
			break;
		default:
		break;
	}

	CvCapture* capture;
	char indexBuff[10];
	int index=0;
	capture=cvCreateFileCapture("C:\\r.mpeg");
	cvSetCaptureProperty(capture,CV_CAP_PROP_POS_FRAMES,46517000);
	
	int start=-1;
	int end=-1;
	int lastInputFrame;
	IplImage *original=NULL;
	IplImage *view=NULL;
	IplImage*  edges=NULL;
	IplImage*  lines=NULL;
	IplImage* cornersTemp=NULL;
	IplImage* cornersEigen=NULL;
	IplImage* greyOriginal=NULL;
	int low=30;
	int high=50;
	IplImage *r=NULL;
	IplImage *g=NULL;
	IplImage *b=NULL;
	int CORNERS_NUM=200;
	CvPoint2D32f* corners=new CvPoint2D32f[CORNERS_NUM];
	CvSeq*list;
	CvRect interest_rect=cvRect(840,270,80,80);
	char color='n';
	int gauss=3;
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX,1,1);
	 IplImage *templ;
	 int Nc,n;
	 CvSeq* first_contour;
	 CvMemStorage* strg;
	while(true){

		key=cv::waitKey(0);
		switch(key){
			case 'o'://corners
				cvCopy(greyOriginal,edges);
				//cvMorphologyEx(greyOriginal, edges,   r,NULL, CV_MOP_GRADIENT);
				cvGoodFeaturesToTrack(
					edges,
					cornersEigen,
					cornersTemp,
					corners,
					&CORNERS_NUM,
					0.01,
					20,
					NULL,
					10
				);
				for(int i=0;i<CORNERS_NUM;i++)
					cvCircle(edges,cvPointFrom32f(corners[i]),8,CV_RGB(0,0,255),3,8);
				break;
			case 'c'://contours
				strg = cvCreateMemStorage();
				first_contour  = NULL;
				Nc = cvFindContours(edges,strg,&first_contour,	sizeof(CvContour),CV_RETR_LIST);
				n=0;
				for( CvSeq* c=first_contour; c!=NULL; c=c->h_next ) {
					cvDrawContours(original,c, CV_RGB(0,255,0), CV_RGB(0,0,255), 0,  1, 8 );
				}
				cvShowImage ("contours", original);

				break;
			case 'y'://kolorowanie
				cvCopy(original,view);
				cvPyrMeanShiftFiltering(view, original, 40,15, 2);
			
			case 'u'://gauss level
				if(gauss==3)
					gauss=5;
				else
					gauss=3;
			break;
			case 'n'://template search
			case 'r':
			case 'g'://corners
			case 'b'://morphology
				color=key;
				break;
			case 'p'://template loading
				cvSetImageROI(greyOriginal,interest_rect);
				 cvCopy(greyOriginal,templ);
				 cvResetImageROI(greyOriginal);
				 cvShowImage ("sub_img", templ);
				 break;
			case 'a'://canny settings
				if(low<high)
					low++;
				break;
			case 'z'://canny settings
				if(low>0)
					low--;
				break;
			case 's'://canny settings
				if(high<256)
					high++;
				break;
			case 'x'://canny settings
				if(low<high)
					high--;
				break;
			case 'q':
				goto finish;
			case 'l'://lines detecotr on edges
				list= cvHoughLines2(edges,storage, CV_HOUGH_PROBABILISTIC,1, CV_PI/180,80,40,10);
				for( int i = 0; i < list->total; i++ ) {
					CvPoint* line = (CvPoint*)cvGetSeqElem(list,i);
					cvLine( original, line[0], line[1], CV_RGB(255,0,0), 1, 8 );
				}
				break;
			default:
				lastInputFrame=(int)cvGetCaptureProperty(capture,CV_CAP_PROP_FRAME_COUNT );
				original=cvQueryFrame( capture );
				index++;
				itoa(index,indexBuff,10);
				
				if(r==NULL){
					r=cvCreateImage(cvSize(original->width-interest_rect.width+1,original->height-interest_rect.height+1), 32, 1 );
					g=cvCreateImage( cvGetSize(original), IPL_DEPTH_8U, 1 );
					b=cvCreateImage( cvGetSize(original), IPL_DEPTH_8U, 1 );
					greyOriginal = cvCreateImage( cvGetSize(original), IPL_DEPTH_8U, 1 );
					edges= cvCreateImage( cvGetSize(original), IPL_DEPTH_8U, 1 );
					view= cvCreateImage( cvGetSize(original), IPL_DEPTH_8U, 3 );
					templ=cvCreateImage(cvSize(interest_rect.width,interest_rect.height), IPL_DEPTH_8U, 1);
					cornersTemp=cvCreateImage( cvGetSize(original), IPL_DEPTH_8U, 1 );
					cornersEigen=cvCreateImage( cvGetSize(original), IPL_DEPTH_8U, 1 );
				}
				
				break;
		}
		
		
		if(view!=NULL){	 
			cvSmooth( original,view, CV_GAUSSIAN,gauss,gauss);
			//cvSplit( view, r, g, b, NULL );
			cvCvtColor( view, greyOriginal,CV_RGB2GRAY);
			switch(color){
			case 'n':
				cvCanny(greyOriginal,edges,low,high,3);
				cvShowImage ("org", greyOriginal);
				break;
			case 'r'://template search
				cvMatchTemplate(
					greyOriginal,
					templ,
					r,
					1
				);
				cvNormalize(r,r,1,0,CV_MINMAX);
				cvShowImage ("org", r);
				break;
			case 'g'://corners
				cvShowImage ("org",greyOriginal);
				break;
			case 'b':
				cvMorphologyEx(greyOriginal, g,   r,NULL, CV_MOP_GRADIENT);
				//cvAdaptiveThreshold(g,edges,255,CV_ADAPTIVE_THRESH_MEAN_C,CV_THRESH_BINARY_INV, 150,5);
				//cvThreshold(g,edges,20,255,CV_THRESH_BINARY);
				//cvCanny(g,edges,low,high,3);
				cvShowImage ("org", g);
				
				break;
			}
 			cvPutText(edges,indexBuff,cvPoint(60,50),&font,CV_RGB(150,150,150));
			cvShowImage ("Edges", edges);
		}
		
	}
finish:
	cout<<start<<endl;
	cout<<end<<endl;
	system("PAUSE");
	return 0;
}

const int MAX_CORNERS = 500;

IplImage *corners1=NULL;
IplImage *corners2=NULL;
IplImage *original1Grey=NULL;
IplImage *original2Grey=NULL;
CvMat* fundamental_matrix;
void mouse(int ev, int x, int y, int flags, void* param){
	if( ev == CV_EVENT_LBUTTONDOWN) {
		cvCopy(original1Grey,corners1);
		cvCopy(original2Grey,corners2);
		CvMat* point=cvCreateMat(1,3,CV_32FC2);
		point->data.fl[0] = x;  
		point->data.fl[1] = y;  
		CvMat*correspondent_lines= cvCreateMat(3,3,CV_32FC1);
		cvComputeCorrespondEpilines(point,1,fundamental_matrix,correspondent_lines);
		float a= CV_MAT_ELEM( *correspondent_lines, float, 0, 0 );
		float b= CV_MAT_ELEM( *correspondent_lines, float, 1, 0 );
		float c= CV_MAT_ELEM( *correspondent_lines, float, 2, 0 );
		CvPoint p0 = cvPoint(cvRound( 0 ), cvRound( -(c/b)));
		CvPoint p1 = cvPoint(cvRound( corners1->width ),cvRound( -((c+a*corners1->width )/b)));	
		cvLine( corners2, p0, p1, CV_RGB(255,255,255),1);

		cvCircle(corners1,cvPoint(x,y),8,CV_RGB(0,0,255),3,8);
		cvShowImage ("corners1", corners1); 
		cvShowImage ("corners2", corners2);
    }
}

void mapDisp(){
	int key;
	cvNamedWindow("corners1");
	cvSetMouseCallback("corners1",mouse,NULL);
	CvCapture* capture;
	capture=cvCreateFileCapture("C:\\r.mpeg");
	cvSetCaptureProperty(capture,CV_CAP_PROP_POS_FRAMES,46517000);
	IplImage* original;
	IplImage* original2;
	CvMat *_M = (CvMat*)cvLoad("C:/Intrinsics.xml");
	CvMat *_D = (CvMat*)cvLoad("C:/Distortion.xml");
	
	CvSize    img_sz;
   	int       win_size = 10;
	IplImage* eig_image ;
	IplImage* tmp_image;
	int           corner_count = MAX_CORNERS;
	CvPoint2D32f* cornersA     = new CvPoint2D32f[ MAX_CORNERS ];
	CvPoint2D32f* cornersB     = new CvPoint2D32f[ MAX_CORNERS ];

	char  features_found[ MAX_CORNERS ];
	float feature_errors[ MAX_CORNERS ];
	
	CvSize pyr_sz;
	IplImage* pyrA;
	IplImage* pyrB;

	CvMat* points1;
	CvMat* points2;
	CvMat* status;
	
	int fm_count;
	int goodFeatures;



	CvMat* mx1;
	CvMat* my1 ;
	CvMat* mx2 ;
	CvMat* my2  ;
	CvMat *_R1 ;
	CvMat *_R2 ;

	CvMat *_H1  ;
	CvMat *_H2 ;
	CvMat *_iM;
	CvMat *pair;

	CvMat* img1r ;
    CvMat* img2r ;
    CvMat* disp;
	CvMat* vdisp ;
	CvStereoBMState *BMState = cvCreateStereoBMState();
	assert(BMState != 0);
	BMState->speckleWindowSize=0;
		BMState->speckleRange=0;
	BMState->preFilterSize=41;
	BMState->preFilterCap=31;
	BMState->SADWindowSize=41;
	BMState->minDisparity=-64;
	BMState->numberOfDisparities=128;
	BMState->textureThreshold=10;
	BMState->uniquenessRatio=10;
  	/*original=cvLoadImage("C:/right.jpg");///do zmiany
	//original2=cvLoadImage("C:/left.jpg");///do zmiany
	//img_sz   = cvGetSize(original);
	original1Grey=cvCreateImage( img_sz, IPL_DEPTH_8U, 1 );
	original2Grey=cvCreateImage( img_sz, IPL_DEPTH_8U, 1 );
	cvCvtColor(original, original1Grey,CV_RGB2GRAY);
	cvCvtColor(original2, original2Grey,CV_RGB2GRAY);*/
	CvMat* disparity_left ;
	CvMat* disparity_right;
	CvStereoGCState* state = cvCreateStereoGCState( 16, 2 );
	CvMat* disparity_left_visual;
	while(true){

		key=cv::waitKey(0);
		switch(key){
			case 'o':
				if(original1Grey==NULL)
					break;
				
				corner_count = MAX_CORNERS;
				cvGoodFeaturesToTrack(original1Grey,eig_image,tmp_image,cornersA,&corner_count,0.01,10.0,0,3,0,0.04);
				cvFindCornerSubPix(original1Grey,cornersA,corner_count,cvSize(win_size,win_size),cvSize(-1,-1),
						cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
				
				cvCalcOpticalFlowPyrLK(original1Grey,original2Grey,pyrA,pyrB,cornersA,cornersB,
					corner_count,cvSize( win_size,win_size ),5,features_found,feature_errors,
					cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3 ),0);
				goodFeatures=0;
				for( int i=0; i<corner_count; i++ ) {
					if( features_found[i]==0|| feature_errors[i]>550 ) {
					  continue;
					}
					goodFeatures++;
				}
				points1 = cvCreateMat(1,goodFeatures,CV_32FC2);
				points2 = cvCreateMat(1,goodFeatures,CV_32FC2);
				status = cvCreateMat(1,goodFeatures,CV_8UC1);
				goodFeatures=0;
				for( int i=0; i<corner_count; i++ ) {
					if( features_found[i]==0|| feature_errors[i]>550 ) {
					  //printf("Error is %f/n",feature_errors[i]);
					  continue;
					}
					points1->data.fl[goodFeatures*2]   = cornersA[i].x;  
					points1->data.fl[goodFeatures*2+1] = cornersA[i].y;  
					points2->data.fl[goodFeatures*2]   = cornersB[i].x;  
					points2->data.fl[goodFeatures*2+1] = cornersB[i].y;
					goodFeatures++;
				}
				 cvUndistortPoints( points1, points1,_M, _D, 0, _M );
				 cvUndistortPoints( points2, points2,_M, _D, 0, _M);
				fundamental_matrix = cvCreateMat(3,3,CV_32FC1);
				fm_count = cvFindFundamentalMat( points1, points2,fundamental_matrix,CV_FM_RANSAC,1.0,0.99,status );
				cvSave("C:/F.xml",fundamental_matrix);

				mx1 = cvCreateMat( original1Grey->height,
					original1Grey->width, CV_32F );
				my1 = cvCreateMat( original1Grey->height,
					original1Grey->width, CV_32F );
				mx2 = cvCreateMat( original1Grey->height,
					original1Grey->width, CV_32F );
				my2 = cvCreateMat( original1Grey->height,
					 original1Grey->width, CV_32F );
				_R1 = cvCreateMat(3, 3, CV_32F);
				_R2 = cvCreateMat(3, 3, CV_32F);

				_H1 = cvCreateMat(3, 3, CV_32F);
				_H2 = cvCreateMat(3, 3, CV_32F);
				_iM = cvCreateMat(3, 3, CV_32F);
				cvStereoRectifyUncalibrated( points1,points2, fundamental_matrix,img_sz,_H1, _H2, 3);
				cvInvert(_M, _iM);
				cvMatMul(_H1, _M, _R1);
				cvMatMul(_iM, _R1, _R1);
				cvMatMul(_H2, _M, _R2);
				cvMatMul(_iM, _R2, _R2);
				cvInitUndistortRectifyMap(_M,_D,_R1,_M,mx1,my1);
				cvInitUndistortRectifyMap(_M,_D,_R2,_M,mx2,my2);

				pair = cvCreateMat( img_sz.height, img_sz.width*2,CV_8UC3 );
				img1r = cvCreateMat( img_sz.height,img_sz.width, CV_8U );
				img2r = cvCreateMat( img_sz.height,img_sz.width, CV_8U );
				disp = cvCreateMat(img_sz.height,img_sz.width, CV_16S );
				vdisp = cvCreateMat( img_sz.height,img_sz.width, CV_8U );
				cvRemap( original1Grey, img1r, mx1, my1 );
                cvRemap( original2Grey, img2r, mx2, my2 );
				cvFindStereoCorrespondenceBM( img2r, img1r, disp,BMState);
                cvNormalize( disp, vdisp, 0, 256, CV_MINMAX );
				//cvShowImage ("m1", img1r); 
				//cvShowImage ("m2",img2r);
                cvNamedWindow( "disparity" );
                cvShowImage( "disparity", vdisp );
				/*disparity_left = cvCreateMat( img_sz.height, img_sz.width, CV_16S );
				disparity_right = cvCreateMat( img_sz.height, img_sz.width, CV_16S );
				cvFindStereoCorrespondenceGC(img1r, img2r,disparity_left, disparity_right, state, 0 );
				disparity_left_visual = cvCreateMat( img_sz.height, img_sz.width, CV_8U );
				cvConvertScale( disparity_left, disparity_left_visual, -16 );
				cvShowImage( "disparity2",disparity_left_visual );
				cvReleaseStereoGCState( &state );*/
				
			break;
			case 'q':
				goto finishIt;
			break;
			default:
				if(original2Grey!=NULL)
					cvCopy(original2Grey,original1Grey);
				original=cvQueryFrame( capture );
				if(original1Grey==NULL){
					img_sz   = cvGetSize(original);
					original1Grey=cvCreateImage( img_sz, IPL_DEPTH_8U, 1 );
					original2Grey=cvCreateImage( img_sz, IPL_DEPTH_8U, 1 );
					eig_image = cvCreateImage( img_sz, IPL_DEPTH_32F, 1 );
					tmp_image = cvCreateImage( img_sz, IPL_DEPTH_32F, 1 );
					corners1 = cvCreateImage( img_sz, IPL_DEPTH_8U, 1 );
					corners2 = cvCreateImage( img_sz, IPL_DEPTH_8U, 1 );
					pyr_sz = cvSize( original1Grey->width+8, original1Grey->height/3 );
					pyrA = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
					pyrB = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
				}
				cvCvtColor(original, original2Grey,CV_RGB2GRAY);
				break;
		}
		cvShowImage ("corners1", original1Grey); 
		cvShowImage ("corners2", original2Grey);
	}
finishIt:
		return;
}



	
	

	/*
	CvMat*correspondent_lines= cvCreateMat(3,goodFeatures,CV_32FC1);
	cvComputeCorrespondEpilines(points1,1,fundamental_matrix,correspondent_lines);
	float a,b,c;
	for( int i=0; i<goodFeatures; i++ ) {
		
		cvCopy(original1Grey,corners1);
		cvCopy(original2Grey,corners2);
		a= CV_MAT_ELEM( *correspondent_lines, float, 0, i );
		b= CV_MAT_ELEM( *correspondent_lines, float, 1, i );
		c= CV_MAT_ELEM( *correspondent_lines, float, 2, i );
		CvPoint2D32f point=cvPoint2D32f(points1->data.fl[i*2],points1->data.fl[i*2+1]);
		cvCircle(corners1,cvPointFrom32f(point),8,CV_RGB(0,0,255),3,8);

		CvPoint p0 = cvPoint(
		  cvRound( 0 ),
		  cvRound( -(c/b))
		);
		CvPoint p1 = cvPoint(
			cvRound( original1->width ),
			cvRound( -((c+a*original1->width )/b))
		);	
		cvLine( corners2, p0, p1, CV_RGB(255,255,255),1);

		cvShowImage ("corners1", corners1);
		cvShowImage ("corners2", corners2);
		key=cv::waitKey(0);
	}*/
		
void optFlow(){
	int key;
	IplImage *original=NULL;
	IplImage *originalGrey=NULL;
	IplImage *prevGrey=NULL;
	IplImage *corners=NULL;
	CvCapture* capture=NULL;
	char indexBuff[10];
	int index=0;
	capture=cvCreateFileCapture("C:\\r.mpeg");
	cvSetCaptureProperty(capture,CV_CAP_PROP_POS_FRAMES,46517000);

	CvSize    img_sz;
   	int       win_size = 10;
	IplImage* eig_image ;
	IplImage* tmp_image;
	int           corner_count = MAX_CORNERS;
	CvPoint2D32f* cornersA     = new CvPoint2D32f[ MAX_CORNERS ];
	CvPoint2D32f* cornersB     = new CvPoint2D32f[ MAX_CORNERS ];

	char  features_found[ MAX_CORNERS ];
	float feature_errors[ MAX_CORNERS ];
	
	CvSize pyr_sz;
	IplImage* pyrA;
	IplImage* pyrB;
  
	CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX,1,1);

	while(true){

		key=cv::waitKey(0);
		switch(key){
			case 's':
				if(originalGrey!=NULL)
					cvCopy(originalGrey,prevGrey);
				break;
			case 'o':
				if(prevGrey==NULL)
					break;
				corner_count = MAX_CORNERS;
				cvGoodFeaturesToTrack(prevGrey,eig_image,tmp_image,cornersA,&corner_count,0.01,10.0,0,3,0,0.04);
				cvFindCornerSubPix(prevGrey,cornersA,corner_count,cvSize(win_size,win_size),cvSize(-1,-1),
						cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
				cvCopy(prevGrey,corners);
				//for(int i=0;i<corner_count;i++)
							//cvCircle(corners,cvPointFrom32f(cornersA[i]),8,CV_RGB(0,0,255),3,8);
				cvCalcOpticalFlowPyrLK(prevGrey,originalGrey,pyrA,pyrB,cornersA,cornersB,
					corner_count,cvSize( win_size,win_size ),5,features_found,feature_errors,
					cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3 ),0);

				for( int i=0; i<corner_count; i++ ) {
					if( features_found[i]==0|| feature_errors[i]>550 ) {
					  printf("Error is %f/n",feature_errors[i]);
					  continue;
					}
					CvPoint p0 = cvPoint(
					  cvRound( cornersA[i].x ),
					  cvRound( cornersA[i].y )
					);
					CvPoint p1 = cvPoint(
					  cvRound( cornersB[i].x ),
					  cvRound( cornersB[i].y )
					);
					cvLine( corners, p0, p1, CV_RGB(255,255,255),1);
				}
				cvShowImage ("corners", corners);
				break;
			case 'q':
				goto finishFlow;

			default:
				
				original=cvQueryFrame( capture );
				index++;
				itoa(index,indexBuff,10);
				if(originalGrey==NULL){
					img_sz   = cvGetSize(original);
					originalGrey=cvCreateImage( img_sz, IPL_DEPTH_8U, 1 );
					prevGrey=cvCreateImage( img_sz, IPL_DEPTH_8U, 1 );
					eig_image = cvCreateImage( img_sz, IPL_DEPTH_32F, 1 );
					tmp_image = cvCreateImage( img_sz, IPL_DEPTH_32F, 1 );
					corners = cvCreateImage( img_sz, IPL_DEPTH_8U, 1 );
					pyr_sz = cvSize( originalGrey->width+8, originalGrey->height/3 );
					pyrA = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
					pyrB = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
				}
				cvCvtColor(original, originalGrey,CV_RGB2GRAY);
				break;
		}
		
		cvPutText(original,indexBuff,cvPoint(60,50),&font,CV_RGB(150,150,150));
		cvShowImage ("original", original);
		
	}	

finishFlow:
	return;
}

