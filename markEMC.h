#ifndef MARKEMC_H
#define MARKEMC_H

#include "hal.h"
#include "rtapi.h"
#include "emcsec.hh"
#include <string>
#include <ctime>

#define MARK_WAITING	1
#define MARK_DONE	0
#define MARK_BOTH_ERROR -3

typedef struct
{
    hal_s32_t* cvCmd;
    hal_bit_t* clearResult;
    hal_u32_t* readyIndex;
    hal_u32_t* showWin;
    hal_u32_t* posCmd;
    hal_float_t* posAxis[6];
    hal_u32_t* reachCmd;
    hal_bit_t* posValid;
    hal_bit_t* watchPosValid;
    hal_bit_t* watchHoleValid;
    hal_bit_t* glueHoleValid;
    hal_s32_t * state;
    hal_u32_t* fps;
    hal_bit_t* setGlue;
    hal_bit_t* pickupDiamond;
    hal_bit_t* dropDiamond;
    hal_bit_t* lightControl;
}MarkHalPins;

class MarkHal
{
public:
    MarkHal();
    int comp_id;
    MarkHalPins* halpins;
};

//Programe Run TIme Manager
class PRTManager
{
public:
    typedef enum {Idle,Start, running, Pause, Restart, End} TimeState;
    PRTManager():startTime(0),pauseTime(0),restartTime(0),endTime(0),
        allPauseTime(0), runTime(0),timeState(Idle){}
    void start();
    void run(){
        //update end time in order to caculate run time
        end();
    }

    void pause();
    void restart();
    void end();
    const std::string& get_run_time_string();
    void set_time_state(TimeState ts);
    void update_time();
private:
    //mm:ss
    std::string int_to_time_string(int sec);
    void update_time_state(TimeState ts);

    time_t startTime;
    time_t pauseTime;
    time_t restartTime;
    time_t endTime;
    time_t allPauseTime;
    time_t runTime;
    std::string runTimeStr;
    TimeState timeState;
};

class MarkEmcStatus
{
public:    
    typedef enum {Unhomed=0, Homeing, Homed} AxisHomeState;
    typedef enum {UNHOMED, Z_AXIS_HOMING, OTHER_AXIS_HOMING,WAIT_ALL_AXIS_HOMED, ALL_HOMED} HomeState;
    MarkEmcStatus();
    void update();
    double cmdAxis[5];
    bool hasStop;
    bool stopToManual;
    bool homing; //when one of the axis is unhomed, homing is true.
    HomeState currentHS; //current home state;
    AxisHomeState axisHomeState[5]; //decide to use which kind of colors to display axis value
    enum EMC_TASK_INTERP_ENUM programStaut;
    enum EMC_TASK_INTERP_ENUM lastProgramStaut;
    enum EMC_TASK_MODE_ENUM mode;
    char errorMsg[256];
    char operatorText[256];
    char operatorDisplay[256];    
    PRTManager prtManager;
private:
    void update_current_home_state();
    void update_current_time_state();
    double actualAxis[5];
    double lastActualAxis[5];
    bool stopComfirm;    
};

#endif // MARKEMC_H
