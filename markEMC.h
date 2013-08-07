#ifndef MARKEMC_H
#define MARKEMC_H

#include "hal.h"
#include "rtapi.h"
#include "emcsec.hh"

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

class MarkEmcStatus
{
public:
    typedef enum {Idle,Start, runing, Pause, End} TimeState;
    typedef enum {Unhomed=0, Homeing, Homed} AxisHomeState;
    MarkEmcStatus():stopToManual(false),homing(false),homeIndex(0),
        mode(EMC_TASK_MODE_MANUAL),timeState(Idle){
        for(int i=0; i<5; i++)
            homeState[i] = Homeing;
    }
    void update();
    double cmdAxis[5];
    bool hasStop;
    bool stopToManual;
    bool homing; //homing is true,when one of the axis is unhomed
    int homeIndex; //to ensure it's the first time to call emc_home after the z axis homed.
    AxisHomeState homeState[5]; //decide to use which kind of colors to display axis
    enum EMC_TASK_INTERP_ENUM programStaut;
    enum EMC_TASK_INTERP_ENUM lastProgramStaut;
    enum EMC_TASK_MODE_ENUM mode;
    char errorMsg[256];
    char operatorText[256];
    char operatorDisplay[256];
    TimeState timeState;
private:
    //HomeState currentHomeState();
    double actualAxis[5];
    double lastActualAxis[5];
    bool stopComfirm;    
};

#endif // MARKEMC_H
