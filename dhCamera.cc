
#include "camera.hh"
#include <stdio.h>
#include <dh-ioctl.h>
#include <hvux_def.h>
#include <grab-ng.h>


static const hv_adc_level_t adcLevel[4]=
	{ADC_LEVEL0, ADC_LEVEL1, ADC_LEVEL2,ADC_LEVEL3};

Camera::Camera(){
	resWidth=1280;
	resHeight=1024;
	img=cvCreateImage(cvSize(resWidth, resHeight), IPL_DEPTH_8U, 1);
	imgHead=cvCreateImageHeader(cvSize(resWidth, resHeight), IPL_DEPTH_8U, 1);
	assert(img && imgHead);
	hvux_handle=-1;
	cvCapture=NULL;
	sem_init(&sem, 0, 1);
	sn=0;
	
}

Camera::~Camera(){
	if(hvux_handle>=0)
		HVCloseVideo(hvux_handle);
	if(img!=NULL)
		cvReleaseImage(&img);
	if(imgHead!=NULL)
		cvReleaseImageHeader(&imgHead);
}

int Camera::open(int index){
	hvux_handle = HVOpenVideo (index);
	if (hvux_handle < 0){
		camIndex=-1;
		printf ("%s ",  HVGetErrInfo ());
		return -1;
	}
    else
        printf("open ok\n");
	camIndex=index;
	//HVGetCamInfo(hvux_handle,&info);
	return 0;
}

int Camera::start(int bufferNum){
	int res=0;
	if(hvux_handle<0)
		return -1;
	bufNum=bufferNum;
	res+=HVMSetCaptureFormat (hvux_handle, 0, 0, resWidth, resHeight, VIDEO_GRAY);
	res+=HVMStartCapture (hvux_handle, bufNum);
	return  res;
}

int Camera::stop(){
	if(hvux_handle<0)
		return -1;
	return  HVMStopCapture(hvux_handle);
}

int Camera::close(){
	int res;
	if(hvux_handle<0)
		return -1;
	res=HVCloseVideo(hvux_handle);
	hvux_handle=-1;
	return res;
}

int Camera::capture(){
	unsigned char *image_raw_buf = NULL; 
	if(hvux_handle<0)
		return -1;
	
	image_raw_buf = HVMGetNextFrame (hvux_handle); 
	if (NULL == image_raw_buf){
		printf ("ERRO:get imgae fail.\n");
		printf("%s",HVGetErrInfo ());
		//stop and then restart the capture
		HVMStopCapture (hvux_handle);
		HVMStartCapture (hvux_handle, bufNum);
		return -1;
	}
	else
	{
		imgHead->imageData=(char*)image_raw_buf;//link data 
		sem_wait(&sem);
		cvCopy(imgHead, img);
		sn++;
		sem_post(&sem);
		/*must release the buffer everytime after you get a picture*/
		if (HVMReleaseBuf (hvux_handle) < 0)
		{
			printf("%s",HVGetErrInfo ());
			return -1;
		}
	}
	return 0;
}

int  Camera::set_brightness(int val)
{
	int res=0;
	if(hvux_handle<0)
		return -1;
	res+=HVADCControl(hvux_handle, ADC_BLKLEVEL_CAL, true);
	res+=HVADCControl(hvux_handle, ADC_BLKLEVEL_CAL_REDCHANNEL, val);
	res+=HVADCControl(hvux_handle, ADC_BLKLEVEL_CAL_GREENCHANNEL1, val);
	res+=HVADCControl(hvux_handle, ADC_BLKLEVEL_CAL_GREENCHANNEL2, val);
	res+=HVADCControl(hvux_handle, ADC_BLKLEVEL_CAL_BLUECHANNEL, val);
	if(!res)
		blackLevel=val;
	return res;
}

int Camera::set_contrast(int val)
{
	int res=0;
	if(hvux_handle<0)
		return -1;
	res+=HVAGCControl(hvux_handle, AGC_GAIN, val);
	if(!res)
		gain=val;
	return res;
}

int Camera::set_exposure_time(int val){
	int res=0;
	if(hvux_handle<0)
		return -1;
	res+=HVAECControl(hvux_handle, SHUTTER_MS, val);
	if(!res)
		exposureTime=val;
	return res;
}

int Camera::set_AD_level(int val){
	int res=0;
	if(hvux_handle<0)
		return -1;
	assert(val>=0&&val<4);
	res+=HVADCControl(hvux_handle, ADC_BITS, adcLevel[val]);
	if(!res)
		adLevel=val;
	return res;
}


int Camera::set_auto_BL(int val){
	int res=0;
	if(hvux_handle<0)
		return -1;
	res+=HVADCControl(hvux_handle, ADC_BLKLEVEL_CAL, !val);
	if(!res)
		autoBL=!val;
	return res;
}

int Camera::set_hue(int){
	return 0;
}

int Camera::set_saturation(int ){
	return 0;
}

void  Camera::get_imgage(IplImage * image){
		sem_wait(&sem);
		cvCopy(img, image);
		sem_post(&sem);
}

unsigned int Camera::get_image_sn(){
	unsigned int snVal;
	sem_wait(&sem);
	snVal=sn;
	sem_post(&sem);
	return snVal;
}


