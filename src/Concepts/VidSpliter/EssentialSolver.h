#pragma once
#include  "stdafx.h"
#include <stdio.h>
#include <opencv2\opencv.hpp>
using namespace std;
#include <direct.h>
#include <stdlib.h>
#include "pugixml.hpp"

const int NUM_CORNERS = 500;
const int NUM_VIEW=1000;
class EssentialSolver{
public:
	void calculatePositions();
	void calculateFundamental(int indexImage1,int indexImage2,int indexResult);

	void shiftResults();
	CvMat* calculateRotTrans(int indexImage1,int indexImage2,int indexFundamental);
	void EssentialSolver::addTransitionToXML(pugi::xml_document* doc,CvMat* transitionM);
	void EssentialSolver::fillReturnMetrix(CvMat* ret,CvMat* R,CvMat* t,bool M44=false);
	void init(){
		MIntrinsic = (CvMat*)cvLoad("C:/Intrinsics.xml");
		MDistortion = (CvMat*)cvLoad("C:/Distortion.xml");

		MIntrinsicTranspose=cvCloneMat(MIntrinsic);
		MIntrinsicInverse=cvCloneMat(MIntrinsic);
		cvTranspose(MIntrinsic,MIntrinsicTranspose);
		cvInvert(MIntrinsic,MIntrinsicInverse);
		//cvSave("C:/MIntrinsicInverse.xml",MIntrinsicInverse);
		//capture=cvCreateFileCapture("C:\\rec.avi");
		capture=cvCreateFileCapture("C:\\video\\rec2.MOV");
		//cvSetCaptureProperty(capture,CV_CAP_PROP_POS_FRAMES,46517000);
		win_size = 20;
	}

private:

	//cvSetMouseCallback("corners1",mouse,NULL);
	CvCapture* capture;

	CvSize    img_sz;
   	int       win_size;
	CvSize pyr_sz;

	IplImage* original;
	IplImage* originalsGrey[3];

	CvPoint2D32f* corners[3];
	IplImage *cornersImage[3];
	CvMat *MIntrinsic;
	CvMat *MIntrinsicInverse;
	CvMat *MIntrinsicTranspose;
	CvMat *MDistortion;

	IplImage* eig_image;
	IplImage* tmp_image;

	IplImage* pyr[2];

	int corner_count[3];

	CvMat* fundamentals[NUM_VIEW*2-3];
};