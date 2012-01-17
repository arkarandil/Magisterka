#include  "stdafx.h"
#include "EssentialSolver.h"

void EssentialSolver::calculatePositions(){
	int key;
	CvMat* transition;
	cvNamedWindow("corners1");

	original=cvQueryFrame( capture );
	//for(int i=0;i<40;i++)
		//original=cvQueryFrame( capture );
	img_sz   = cvGetSize(original);
	pyr_sz = cvSize( original->width+8, original->height/3 );
	eig_image= cvCreateImage( img_sz, IPL_DEPTH_32F, 1 );
	tmp_image = cvCreateImage( img_sz, IPL_DEPTH_32F, 1 );
	pyr[0] = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
	pyr[1] = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
	for(int i=0;i<3;i++){
		originalsGrey[i]=cvCreateImage( img_sz, IPL_DEPTH_8U, 1 );
		cornersImage[i] = cvCreateImage( img_sz, IPL_DEPTH_8U, 1 );
		
		corners[i]=new CvPoint2D32f[ NUM_CORNERS ];
		corner_count[i] = NUM_CORNERS;
	}
	cvCvtColor(original, originalsGrey[0],CV_RGB2GRAY);	
	original=cvQueryFrame( capture );
	cvCvtColor(original, originalsGrey[1],CV_RGB2GRAY);
	original=cvQueryFrame( capture );
	cvCvtColor(original, originalsGrey[2],CV_RGB2GRAY);

	cvGoodFeaturesToTrack(originalsGrey[0],eig_image,tmp_image,corners[0],&corner_count[0],0.01,10.0,0,3,0,0.04);
	cvFindCornerSubPix(originalsGrey[0],corners[0],corner_count[0],cvSize(win_size,win_size),cvSize(-1,-1),
			cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));

	cvGoodFeaturesToTrack(originalsGrey[1],eig_image,tmp_image,corners[1],&corner_count[1],0.01,10.0,0,3,0,0.04);
	cvFindCornerSubPix(originalsGrey[1],corners[1],corner_count[1],cvSize(win_size,win_size),cvSize(-1,-1),
			cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));

	cvGoodFeaturesToTrack(originalsGrey[2],eig_image,tmp_image,corners[2],&corner_count[2],0.01,10.0,0,3,0,0.04);
	cvFindCornerSubPix(originalsGrey[2],corners[2],corner_count[2],cvSize(win_size,win_size),cvSize(-1,-1),
			cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));

	cvShowImage ("corners1", originalsGrey[0]); 
	cvShowImage ("corners2", originalsGrey[1]);
	cvShowImage ("corners3", originalsGrey[2]);

	const char source[] = "<?xml version=\"1.0\"?><CameraPaths></CameraPaths>";
	size_t size = sizeof(source);
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_buffer(source, size);



	calculateFundamental(0,1,0);
	calculateFundamental(0,2,1);
	calculateFundamental(1,2,2);
	transition=calculateRotTrans(0,1,0);
	addTransitionToXML(&doc,transition);
	
	transition=calculateRotTrans(1,2,2);
	addTransitionToXML(&doc,transition);
	//calculateRotTrans(0,2,1);
	//calculateRotTrans(1,2,2);
	for(int index=3;index<NUM_VIEW;index++){

		key=cv::waitKey(30);
		//switch(key){	
		//	default:
				shiftResults();

				original=cvQueryFrame( capture );
				original=cvQueryFrame( capture );
				if(original==NULL)
 					goto finishIt;
				cvCvtColor(original, originalsGrey[2],CV_RGB2GRAY); 
				cvGoodFeaturesToTrack(originalsGrey[2],eig_image,tmp_image,corners[2],&corner_count[2],0.01,10.0,0,3,0,0.04);
				//cvFindCornerSubPix(originalsGrey[2],corners[2],corner_count[2],cvSize(win_size,win_size),cvSize(-1,-1),
						//cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
				calculateFundamental(0,2,index*2-3);
				calculateFundamental(1,2,index*2-2);
    			//transition=calculateRotTrans(1,2,index*2-2);
				addTransitionToXML(&doc,transition);
				//break;
		//}
		
		cvCopy(originalsGrey[0],cornersImage[0]);
		cvCopy(originalsGrey[1],cornersImage[1]);
		cvCopy(originalsGrey[2],cornersImage[2]);
		for(int i=0;i<3;i++)
			for(int j=0;j<corner_count[i];j++)
				cvCircle(cornersImage[i],cvPointFrom32f(corners[i][j]),8,CV_RGB(0,0,255),3,8);
		
		cvShowImage ("corners1", cornersImage[0]); 
		cvShowImage ("corners2", cornersImage[1]);
		cvShowImage ("corners3", cornersImage[2]);
	}
finishIt:
	doc.save_file("C:\\calculatedPath.xml");
		return;
}
void EssentialSolver::shiftResults(){
	cvCopy(originalsGrey[1],originalsGrey[0]); //obrazki normalnie shiftowane
	cvCopy(originalsGrey[2],originalsGrey[1]);
	memcpy(corners[0],corners[1],sizeof(CvPoint2D32f)*corner_count[1]); //cornersy  tez
	memcpy(corners[1],corners[2],sizeof(CvPoint2D32f)*corner_count[2]);
	corner_count[0]=corner_count[1];
	corner_count[1]=corner_count[2];
}

CvMat* EssentialSolver::calculateRotTrans(int indexImage1,int indexImage2,int indexFundamental){
	if(!fundamentals[indexFundamental])
		return NULL;
	CvMat* TempM=cvCreateMat(3,3,CV_32FC1);
	CvMat* Essential=cvCreateMat(3,3,CV_32FC1);

	CvMat* U=cvCreateMat(3,3,CV_32FC1);
	CvMat* S=cvCreateMat(3,3,CV_32FC1);
	CvMat* VT=cvCreateMat(3,3,CV_32FC1);

	float val[]={0.0,-1.0,0.0,
				1.0,0.0,0.0,
				0.0,0.0,1.0};
	CvMat W= cvMat(3,3,CV_32FC1,val);
	
	CvMat* WT= cvCreateMat(3,3,CV_32FC1);
	cvTranspose(&W,WT);

	float valP[]={1.0,0.0,0.0,0.0,
		0.0,1.0,0.0,0.0,
		0.0,0.0,1.0,0.0};
	CvMat P= cvMat(3,4,CV_32FC1,valP);

	CvMat* R[2];
	CvMat* t[2];
	for(int i=0;i<2;i++){
		R[i]=cvCreateMat(3,3,CV_32FC1);
		t[i]=cvCreateMat(3,1,CV_32FC1);
	}

	cvMatMul(MIntrinsicTranspose,fundamentals[indexFundamental],TempM);
	cvMatMul(TempM,MIntrinsic,Essential);
	
	cvSVD(Essential, S, U, VT,CV_SVD_V_T);
	/*cvSave("C:/Essentia1llll.xml",Essential);
	cvMatMul(U,S,TempM);
	cvMatMul(TempM,VT,R[0]);
	cvSave("C:/Essential2lll.xml",R[0]);*/

	cvMatMul(U,&W,TempM);
	cvMatMul(TempM,VT,R[0]);
	if(cvDet(R[0])<0){
		cvScale(Essential,Essential,-1);
		cvSVD(Essential, S, U, VT,CV_SVD_V_T);
		cvMatMul(U,&W,TempM);
		cvMatMul(TempM,VT,R[0]);
	}
	cvMatMul(U,WT,TempM);
	cvMatMul(TempM,VT,R[1]);
	cvSave("C:/R0.xml",R[0]);
	cvSave("C:/R1.xml",R[1]);

	for(int i=0;i<3;i++){
		*( (float*)CV_MAT_ELEM_PTR( *t[0], i, 0 ) )=CV_MAT_ELEM( *U, float, i, 2 );
		*( (float*)CV_MAT_ELEM_PTR( *t[1], i, 0 ) )=-1*CV_MAT_ELEM( *U, float, i, 2 );
	}

	CvMat* A[4];
	CvMat* ANorm[4];
	int index=0;
index_ret:
	CvPoint2D32f x=corners[indexImage1][index];
	CvPoint2D32f xp=corners[indexImage2][index];
	float valx[]={x.x,x.y,1.0};
	float valxp[]={xp.x,xp.y,1.0};

	CvMat*tempCam=cvCreateMat(3,1,CV_32FC1);

	CvMat xCam=cvMat(3,1,CV_32FC1,valx);
	CvMat xCamP=cvMat(3,1,CV_32FC1,valxp);

	cvMatMul(MIntrinsicInverse,&xCam,tempCam);
	x.x=CV_MAT_ELEM( *tempCam, float,0 ,0);
	x.y=CV_MAT_ELEM( *tempCam, float,1 ,0);
	cvSave("C:/results/x.xml",&xCam);
	cvSave("C:/results/xCam.xml",tempCam);

	cvMatMul(MIntrinsicInverse,&xCamP,tempCam);
	xp.x=CV_MAT_ELEM( *tempCam, float,0 ,0);
	xp.y=CV_MAT_ELEM( *tempCam, float,1 ,0);
	cvSave("C:/results/xP.xml",&xCamP);
	cvSave("C:/results/xCamP.xml",tempCam);
	
	char buf[64];
	//Budowanie 4 macierzy A
	for(int i=0;i<4;i++){
		A[i]=cvCreateMat(4,4,CV_32FC1);
		*( (float*)CV_MAT_ELEM_PTR( *A[i], 2, 0 ) )=xp.x*CV_MAT_ELEM( *R[i/2], float,2 ,0)-CV_MAT_ELEM( *R[i/2], float,0 ,0);
		*( (float*)CV_MAT_ELEM_PTR( *A[i], 2, 1 ) )=xp.x*CV_MAT_ELEM( *R[i/2], float,2 ,1)-CV_MAT_ELEM( *R[i/2], float,0 ,1);
		*( (float*)CV_MAT_ELEM_PTR( *A[i], 2, 2 ) )=xp.x*CV_MAT_ELEM( *R[i/2], float,2 ,2)-CV_MAT_ELEM( *R[i/2], float,0 ,2);
		*( (float*)CV_MAT_ELEM_PTR( *A[i], 2, 3 ) )=xp.x*CV_MAT_ELEM( *t[i%2], float,2 ,0)-CV_MAT_ELEM( *t[i%2], float,0 ,0);

		*( (float*)CV_MAT_ELEM_PTR( *A[i], 3, 0 ) )=xp.y*CV_MAT_ELEM( *R[i/2], float,2 ,0)-CV_MAT_ELEM( *R[i/2], float,1 ,0);
		*( (float*)CV_MAT_ELEM_PTR( *A[i], 3, 1 ) )=xp.y*CV_MAT_ELEM( *R[i/2], float,2 ,1)-CV_MAT_ELEM( *R[i/2], float,1 ,1);
		*( (float*)CV_MAT_ELEM_PTR( *A[i], 3, 2 ) )=xp.y*CV_MAT_ELEM( *R[i/2], float,2 ,2)-CV_MAT_ELEM( *R[i/2], float,1 ,2);
		*( (float*)CV_MAT_ELEM_PTR( *A[i], 3, 3 ) )=xp.y*CV_MAT_ELEM( *t[i%2], float,2 ,0)-CV_MAT_ELEM( *t[i%2], float,1 ,0);
		sprintf(buf,"C:/results/A_%d.xml",i);
		cvSave(buf,A[i]);
	}
//Macierze znormalizowane w wierszach
	float rowNorm;
	for(int i=0;i<4;i++){
		ANorm[i]=cvCreateMat(4,4,CV_32FC1);
		rowNorm=x.x*x.x+1;
		//rowNorm=sqrt(rowNorm);
		*( (float*)CV_MAT_ELEM_PTR( *ANorm[i], 0, 0 ) )=0;
		*( (float*)CV_MAT_ELEM_PTR( *ANorm[i], 0, 1 ) )=-1.0/rowNorm;
		*( (float*)CV_MAT_ELEM_PTR( *ANorm[i], 0, 2 ) )=x.x/rowNorm;
		*( (float*)CV_MAT_ELEM_PTR( *ANorm[i], 0, 3 ) )=0;

		rowNorm=x.y*x.y+1;
		//rowNorm=sqrt(rowNorm);
		*( (float*)CV_MAT_ELEM_PTR( *ANorm[i], 1, 0 ) )=0;
		*( (float*)CV_MAT_ELEM_PTR( *ANorm[i], 1, 1 ) )=-1.0/rowNorm;
		*( (float*)CV_MAT_ELEM_PTR( *ANorm[i], 1, 2 ) )=x.y/rowNorm;
		*( (float*)CV_MAT_ELEM_PTR( *ANorm[i], 1, 3 ) )=0;

		rowNorm=CV_MAT_ELEM( *A[i], float,2 ,0)*CV_MAT_ELEM( *A[i], float,2 ,0);
		rowNorm+=CV_MAT_ELEM( *A[i], float,2 ,1)*CV_MAT_ELEM( *A[i], float,2 ,1);
		rowNorm+=CV_MAT_ELEM( *A[i], float,2 ,2)*CV_MAT_ELEM( *A[i], float,2 ,2);
		rowNorm+=CV_MAT_ELEM( *A[i], float,2 ,3)*CV_MAT_ELEM( *A[i], float,2 ,3);
		//rowNorm=sqrt(rowNorm);
		*( (float*)CV_MAT_ELEM_PTR( *ANorm[i], 2, 0 ) )=CV_MAT_ELEM( *A[i], float,2 ,0)/rowNorm;
		*( (float*)CV_MAT_ELEM_PTR( *ANorm[i], 2, 1 ) )=CV_MAT_ELEM( *A[i], float,2 ,1)/rowNorm;
		*( (float*)CV_MAT_ELEM_PTR( *ANorm[i], 2, 2 ) )=CV_MAT_ELEM( *A[i], float,2 ,2)/rowNorm;
		*( (float*)CV_MAT_ELEM_PTR( *ANorm[i], 2, 3 ) )=CV_MAT_ELEM( *A[i], float,2 ,3)/rowNorm;
		
		rowNorm=CV_MAT_ELEM( *A[i], float,3 ,0)*CV_MAT_ELEM( *A[i], float,3 ,0);
		rowNorm+=CV_MAT_ELEM( *A[i], float,3 ,1)*CV_MAT_ELEM( *A[i], float,3 ,1);
		rowNorm+=CV_MAT_ELEM( *A[i], float,3 ,2)*CV_MAT_ELEM( *A[i], float,3 ,2);
		rowNorm+=CV_MAT_ELEM( *A[i], float,3 ,3)*CV_MAT_ELEM( *A[i], float,3 ,3);
		//rowNorm=sqrt(rowNorm);
		*( (float*)CV_MAT_ELEM_PTR( *ANorm[i], 3, 0 ) )=CV_MAT_ELEM( *A[i], float,3 ,0)/rowNorm;
		*( (float*)CV_MAT_ELEM_PTR( *ANorm[i], 3, 1 ) )=CV_MAT_ELEM( *A[i], float,3 ,1)/rowNorm;
		*( (float*)CV_MAT_ELEM_PTR( *ANorm[i], 3, 2 ) )=CV_MAT_ELEM( *A[i], float,3 ,2)/rowNorm;
		*( (float*)CV_MAT_ELEM_PTR( *ANorm[i], 3, 3 ) )=CV_MAT_ELEM( *A[i], float,3 ,3)/rowNorm;
		sprintf(buf,"C:/results/A_%d_Norm.xml",i);
		cvSave(buf,ANorm[i]);
	}

	CvMat* resultPoint=cvCreateMat(4,1,CV_32FC1);
	CvMat* U44=cvCreateMat(4,4,CV_32FC1);
	CvMat* S44=cvCreateMat(4,4,CV_32FC1);
	CvMat* V44=cvCreateMat(4,4,CV_32FC1);
	double dotProduct;
	double detP;
	double depth1;
	double depth2;
	int goodIndex=-1;
	CvMat* Pp=cvCreateMat(3,4,CV_32FC1);
	
	for(int i=0;i<4;i++){
		cvSVD(ANorm[i], S44, U44, V44);//VT nie jest T tylko samo V
		*( (float*)CV_MAT_ELEM_PTR( *resultPoint, 0, 0 ) )=CV_MAT_ELEM( *V44, float,0 ,3);
		*( (float*)CV_MAT_ELEM_PTR( *resultPoint, 1, 0 ) )=CV_MAT_ELEM( *V44, float,1 ,3);
		*( (float*)CV_MAT_ELEM_PTR( *resultPoint, 2, 0 ) )=CV_MAT_ELEM( *V44, float,2 ,3);
		*( (float*)CV_MAT_ELEM_PTR( *resultPoint, 3, 0 ) )=CV_MAT_ELEM( *V44, float,3 ,3);
		sprintf(buf,"C:/results/resultPoint_%d.xml",i);
		cvSave(buf,resultPoint);
		fillReturnMetrix(Pp,R[i/2],t[i%2]);
		sprintf(buf,"C:/results/PCamP_%d.xml",i);
		cvSave(buf,Pp);
		detP=cvDet(R[i/2]);
		cvMatMul(&P,resultPoint,&xCam);
		cvMatMul(Pp,resultPoint,&xCamP);
		sprintf(buf,"C:/results/xCam_%d.xml",i);
		cvSave(buf,&xCam);
		sprintf(buf,"C:/results/xCamP_%d.xml",i);
		cvSave(buf,&xCamP);
		depth1=(1*CV_MAT_ELEM( xCam, float,2,0))/CV_MAT_ELEM( *resultPoint, float,3 ,0);//bylo dzielenie
		depth2=((detP>0.0?1:-1)*CV_MAT_ELEM( xCamP, float,2,0))/(CV_MAT_ELEM( *resultPoint, float,3 ,0));//bylo dzielenie
		if(depth1>0&& depth2>0){
			std::cout<<"Jest Kurwa!!!";
			std::cout<<"Jest Kurwa!!!";
			sprintf(buf,"C:/WynikKurwa_%d.xml",i);
			cvSave(buf,resultPoint);
			//assert(goodIndex==-1);
			goodIndex=i;
		}
	}
	if(goodIndex<0){
		index++;
		goto index_ret;
	}
	CvMat* ret=cvCreateMat(4,4,CV_32FC1);
	fillReturnMetrix(ret,R[goodIndex/2],t[goodIndex%2],true);
	return ret;
	//cvSave("C:/R.txt",R[goodIndex/2]);
	//cvSave("C:/t.txt",t[goodIndex%2]);
}

void EssentialSolver::fillReturnMetrix(CvMat* ret,CvMat* R,CvMat* t,bool M44)
{
	*( (float*)CV_MAT_ELEM_PTR( *ret, 0, 0 ) )=CV_MAT_ELEM( *R, float,0 ,0);
	*( (float*)CV_MAT_ELEM_PTR( *ret, 0, 1 ) )=CV_MAT_ELEM( *R, float,0 ,1);
	*( (float*)CV_MAT_ELEM_PTR( *ret, 0, 2 ) )=CV_MAT_ELEM( *R, float,0 ,2);
	*( (float*)CV_MAT_ELEM_PTR( *ret, 0, 3 ) )=CV_MAT_ELEM( *t, float,0 ,0);

	*( (float*)CV_MAT_ELEM_PTR( *ret, 1, 0 ) )=CV_MAT_ELEM( *R, float,1 ,0);
	*( (float*)CV_MAT_ELEM_PTR( *ret, 1, 1 ) )=CV_MAT_ELEM( *R, float,1 ,1);
	*( (float*)CV_MAT_ELEM_PTR( *ret, 1, 2 ) )=CV_MAT_ELEM( *R, float,1 ,2);
	*( (float*)CV_MAT_ELEM_PTR( *ret, 1, 3 ) )=CV_MAT_ELEM( *t, float,1 ,0);

	*( (float*)CV_MAT_ELEM_PTR( *ret, 2, 0 ) )=CV_MAT_ELEM( *R, float,2 ,0);
	*( (float*)CV_MAT_ELEM_PTR( *ret, 2, 1 ) )=CV_MAT_ELEM( *R, float,2 ,1);
	*( (float*)CV_MAT_ELEM_PTR( *ret, 2, 2 ) )=CV_MAT_ELEM( *R, float,2 ,2);
	*( (float*)CV_MAT_ELEM_PTR( *ret, 2, 3 ) )=CV_MAT_ELEM( *t, float,2 ,0);
	if(M44){
		*( (float*)CV_MAT_ELEM_PTR( *ret, 3, 0 ) )=0.0f;
		*( (float*)CV_MAT_ELEM_PTR( *ret, 3, 1 ) )=0.0f;
		*( (float*)CV_MAT_ELEM_PTR( *ret, 3, 2 ) )=0.0f;
		*( (float*)CV_MAT_ELEM_PTR( *ret, 3, 3 ) )=1.0f;
	}

}

void EssentialSolver::calculateFundamental(int indexImage1,int indexImage2,int indexResult){
	char  features_found[ NUM_CORNERS ];
	float feature_errors[ NUM_CORNERS ];
	CvMat* points1;
	CvMat* points2;
	CvMat* status;
	
	int goodFeatures=0;
	cvCalcOpticalFlowPyrLK(originalsGrey[indexImage1],originalsGrey[indexImage2],
		pyr[0],pyr[1],corners[indexImage1],corners[indexImage2],
				corner_count[indexImage1],cvSize( win_size,win_size ),5,features_found,feature_errors,
				cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3 ),0);
			goodFeatures=0;
			for( int i=0; i<corner_count[indexImage1]; i++ ) {
				if( features_found[i]==0|| feature_errors[i]>550 ) {
				  continue;
				}
				goodFeatures++;
			}
			if(goodFeatures==0)
			{
				fundamentals[indexResult]=NULL;
				return;
			}
			points1 = cvCreateMat(1,goodFeatures,CV_32FC2);
			points2 = cvCreateMat(1,goodFeatures,CV_32FC2);
			status = cvCreateMat(1,goodFeatures,CV_8UC1);
			goodFeatures=0;
			for( int i=0; i<corner_count[indexImage1]; i++ ) {
				if( features_found[i]==0|| feature_errors[i]>550 ) {
				  //printf("Error is %f/n",feature_errors[i]);
				  continue;
				}
				points1->data.fl[goodFeatures*2]   = corners[indexImage1][i].x;  
				points1->data.fl[goodFeatures*2+1] = corners[indexImage1][i].y;  
				points2->data.fl[goodFeatures*2]   = corners[indexImage2][i].x;  
				points2->data.fl[goodFeatures*2+1] = corners[indexImage2][i].y;
				goodFeatures++;
			}
			cvUndistortPoints( points1, points1,MIntrinsic , MDistortion, 0, MIntrinsic  );
			cvUndistortPoints( points2, points2,MIntrinsic , MDistortion, 0, MIntrinsic );
			
			if(goodFeatures>5)
			{
				fundamentals[indexResult] = cvCreateMat(3,3,CV_32FC1);
				cvFindFundamentalMat( points1, points2,fundamentals[indexResult],CV_FM_RANSAC,1.0,0.99,status );
			}
			else
			{
				fundamentals[indexResult]=NULL;
			}
			//char buff[255];
			//sprintf(buff,"C:/F_%d.xml",indexResult);
			//cvSave(buff,fundamentals[indexResult]);
}


void EssentialSolver::addTransitionToXML(pugi::xml_document * doc,CvMat* transitionM)
{
	pugi::xml_node transition = doc->first_child().append_child("Transition");
	pugi::xml_node rotation = transition.append_child("Rotation");
	pugi::xml_node translation = transition.append_child("Translation");
	pugi::xml_node temp;
	char name[4];
	char value[32];

	for(int i=0;i<3;i++)
	{
		for(int j=0;j<3;j++)
		{
			sprintf(name, "M%d%d",i,j);
			sprintf(value, "%f",CV_MAT_ELEM( *transitionM, float,i ,j));
			temp=rotation.append_child(name);
			temp.append_child(pugi::node_pcdata).set_value(value);
		}
	}
	temp=translation.append_child("TX");
	sprintf(value, "%f",CV_MAT_ELEM( *transitionM, float,0 ,3));
	temp.append_child(pugi::node_pcdata).set_value(value);

	temp=translation.append_child("TY");
	sprintf(value, "%f",CV_MAT_ELEM( *transitionM, float,1 ,3));
	temp.append_child(pugi::node_pcdata).set_value(value);

	temp=translation.append_child("TZ");
	sprintf(value, "%f",CV_MAT_ELEM( *transitionM, float,2 ,3));
	temp.append_child(pugi::node_pcdata).set_value(value);
	doc->save_file("C:\\calculatedPath.xml");
}