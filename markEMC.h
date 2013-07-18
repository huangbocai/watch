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
    hal_s32_t * state;
    hal_u32_t* fps;
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
    MarkEmcStatus():stopToManual(false),mode(EMC_TASK_MODE_MANUAL){}
    void update();
    double cmdAxis[5];
    bool hasStop;
    bool stopToManual;
    enum EMC_TASK_INTERP_ENUM programStaut;
    enum EMC_TASK_MODE_ENUM mode;
private:
    double actualAxis[5];
    double lastActualAxis[5];
    bool stopComfirm;
};


#endif // MARKEMC_H
