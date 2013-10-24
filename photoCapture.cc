

#include "photoCapture.hh"
#include <stdio.h>

void* image_capture_thread(void* arg){
	const int fpsTestNum=50;
	const int maxContinueError=3;
	ImageCapture* imgCap=(ImageCapture*)arg;
	int res=0, ContinueET=0;
	Camera* cam=NULL;
	int frameNum=0;
	double frequency=cvGetTickFrequency()*1000000.;
    double t1, t0 = (double)cvGetTickCount();
	
threadBegin:
	cam=imgCap->cam;
	res=cam->capture();
	if(!res){
		frameNum++;
		ContinueET=0;
		imgCap->camError=false;
	}
	else
		ContinueET++;
	
	if(frameNum==fpsTestNum){
		t1=(double)cvGetTickCount();;
		imgCap->fps=frameNum/((t1-t0)/frequency);
		frameNum=0;
		t0=t1;
	}
	
	if(ContinueET>maxContinueError){
		ContinueET=0;
		imgCap->camError=true;
		imgCap->errorTimes++;
		imgCap->recover();
		sleep(1);
	}
	
	if(imgCap->run)
		goto threadBegin;

	return NULL;
}



ImageCapture::ImageCapture(int contrastVal, int brightnessVal, int exposureVal, int ADLevel, int bufferNum, int cameraIndex)
	:constrast(contrastVal), brightness(brightnessVal), exposureTime(exposureVal), adLevel(ADLevel), bufNum(bufferNum), camIndex(cameraIndex)
{
	cam= new Camera;
	if(bufNum<1)
		bufNum=1;
	if(bufNum>4)
		bufNum=4;
	hasStop=1;
	camError=false;
}

ImageCapture::~ImageCapture(){
	if(!hasStop)
		stop();
	delete cam;
}

int ImageCapture::start(){
	int i;
	int res=-1;
    printf("camer index = %d \n", camIndex);
	if(camIndex<0){
		for(i=0; i<4 && res; i++)
			res=cam->open(i);
	}
	else
		res=cam->open(camIndex);
	if(res)
		return -1;

	res+=cam->set_AD_level(adLevel);
	res+=cam->set_contrast(constrast);
	res+=cam->set_brightness(brightness);
	//res+=cam->set_auto_BL(autoBL);
	res+=cam->set_exposure_time(exposureTime);
	res+=cam->start(bufNum);
    if(res){
        printf("set camera error, adl=%d,constrast=%d, birghtness=%d, expos=%d, bufNum=%d\n",adLevel, constrast,
               brightness, exposureTime, bufNum);
        //return -1;
    }
	
	res=pthread_create (&thread, NULL, image_capture_thread, this);
	if(res){
		printf("thread create fail in start_capture_image()\n");
		return -1;
	}
	hasStop=0;
	errorTimes=0;
	run=1;
	return 0;
}

int ImageCapture::stop(){
	if(hasStop)
		return 0;
	run=0;
	pthread_join(thread, NULL);
	cam->stop();
	cam->close();
	hasStop=1;
	return 0;
}

int ImageCapture::set_contrast(int val){
	constrast=val;
	return cam->set_contrast(val);
}

int ImageCapture::set_brightness(int val){
	brightness=val;
	return cam->set_brightness(val);
}

int ImageCapture::set_exposure_time(int val){
	exposureTime=val;
	return cam->set_exposure_time(val);
}

int ImageCapture::set_AD_level(int val){
	adLevel=val;
	return cam->set_AD_level(val);
}

int ImageCapture::set_auto_BL(int val){
	autoBL=val;
    return cam->set_auto_BL(val);
}

int ImageCapture::set_hue(int val){
	return cam->set_hue(val);
}

int ImageCapture::set_saturation(int val){
	return cam->set_saturation(val);
}

unsigned int ImageCapture::get_image_sn() const {
	return cam->get_image_sn();
}

void ImageCapture::get_image(IplImage * img){
	cam->get_imgage(img);
}

void ImageCapture::get_image_size(int & width, int & height){
	width=cam->get_image_width();
	height=cam->get_image_height();
}

int ImageCapture::get_fps(){
	return fps;
}



int ImageCapture::recover(){
	int i, res=-1;
	
	cam->stop();
	cam->close();
	if(camIndex<0){
		for(i=0; i<4 && res; i++)
			res=cam->open(i);
	}
	else
		res=cam->open(camIndex);
	if(res)
		return -1;
	
	res+=cam->set_AD_level(adLevel);
	res+=cam->set_contrast(constrast);
	res+=cam->set_brightness(brightness);
	res+=cam->set_exposure_time(exposureTime);
	res+=cam->start(bufNum);
	if(res)
		return -1;

	return 0;
}


