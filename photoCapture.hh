#ifndef CAPTURE_HH
#define CAPTURE_HH

#include "camera.hh"

class  ImageCapture
{
public:
	ImageCapture(int contrastVal, int brightnessVal, int exposureVal, int ADLevel, int bufferNum=2, int cameraIndex=-1);
	int start();
	int stop();
	int set_contrast(int val);
	int set_brightness (int val);
	int set_exposure_time(int val);
	int set_AD_level(int val);
	int set_auto_BL(int val);
	int set_hue(int val);
	int set_saturation(int val);
	int get_fps();
	unsigned int get_image_sn() const;
	void  get_image(IplImage* img);
	void get_image_size(int& width, int&height);
	bool camera_ok(){return !camError;};
	~ImageCapture();
	friend void* image_capture_thread(void* arg);
private:
	int recover();
	Camera* cam;
	pthread_t thread;
	int hasStop;
	int constrast;
	int brightness;
	int exposureTime;
	int adLevel;
	int autoBL;
	int bufNum;
	int camIndex;
	int run;
	int errorTimes;
	bool camError;
	int fps;
};

#endif 
