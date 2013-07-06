


#ifndef CAMERA_HH
#define CAMERA_HH

#include <cv.h>
#include <highgui.h>
#include <semaphore.h>

class Camera
{
public:
	Camera();
	int open(int index);
	int close();
	int start(int bufferNum=4);
	int stop();
	int capture();
	int set_contrast(int val);
	int set_brightness(int val);
	int set_exposure_time(int val);
	int set_AD_level(int val);
	int set_auto_BL(int val);
	int set_hue(int val);
	int set_saturation(int val);
	void  get_imgage(IplImage* image);
	unsigned int get_image_sn();
	int get_image_width()const {return resWidth;};
	int get_image_height()const {return resHeight;};
	~Camera();
private:
	int camIndex;
	int resWidth;
	int resHeight;
	int bufNum;
	int hvux_handle;
	CvCapture* cvCapture;
	unsigned int sn;
	IplImage* img;
	IplImage* imgHead;
	sem_t sem;
	//struct hv_cam_info info;
	int blackLevel, gain, exposureTime;
	int adLevel, autoBL;
};

#endif
