
//int mark2run=1;

#include "markEMC.h"
#include <stdio.h>
#include <stdlib.h>

MarkHal::MarkHal()
{
    int retval;

    /* STEP 1: initialise the hal component */
    comp_id= hal_init("mark");
    if (comp_id< 0) {
        rtapi_print_msg(RTAPI_MSG_ERR, "HALUI: ERROR: hal_init() failed\n");;
        return ;
    }

      /* STEP 2: allocate shared memory for hal pins data */
    halpins= (MarkHalPins *) hal_malloc(sizeof(MarkHalPins));
    if ( halpins== 0) {
        rtapi_print_msg(RTAPI_MSG_ERR, "HALUI: ERROR: hal_malloc() failed\n");
        hal_exit(comp_id);
        return;
    }

    /* STEP 3: export  pins*/
    retval = hal_pin_s32_new("mark.cvCmd", HAL_IO, &halpins->cvCmd, comp_id);
    if (retval != 0)
        return;
    *halpins->cvCmd=0;

    retval = hal_pin_u32_new("mark.ready-index", HAL_IN, &halpins->readyIndex, comp_id);
    if (retval != 0)
        return ;
    *halpins->readyIndex=0;

    retval = hal_pin_u32_new("mark.show", HAL_IO, &halpins->showWin, comp_id);
    if (retval != 0)
        return ;
    *halpins->showWin=0;

    retval = hal_pin_bit_new("mark.clear-result", HAL_IN, &halpins->clearResult, comp_id);
    if (retval != 0)
        return ;
    *halpins->clearResult=0;

    retval = hal_pin_u32_new("mark.posCmd", HAL_IO, &halpins->posCmd, comp_id);
    if (retval != 0)
        return;
    *halpins->posCmd=0;

    retval = hal_pin_float_new("mark.pos-x", HAL_OUT, &halpins->posAxis[0], comp_id);
    if (retval != 0)
        return ;

    retval = hal_pin_float_new("mark.pos-y", HAL_OUT, &halpins->posAxis[1], comp_id);
    if (retval != 0)
        return ;

    retval = hal_pin_float_new("mark.pos-z", HAL_OUT, &halpins->posAxis[2], comp_id);
    if (retval != 0)
        return ;

    retval = hal_pin_float_new("mark.pos-a", HAL_OUT, &halpins->posAxis[3], comp_id);
    if (retval != 0)
        return ;

    retval = hal_pin_float_new("mark.pos-b", HAL_OUT, &halpins->posAxis[4], comp_id);
    if (retval != 0)
        return ;

    retval = hal_pin_float_new("mark.pos-c", HAL_OUT, &halpins->posAxis[5], comp_id);
    if (retval != 0)
        return ;

    retval = hal_pin_u32_new("mark.reachCmd", HAL_IN, &halpins->reachCmd, comp_id);
    if (retval != 0)
        return;
    *halpins->reachCmd=0;

    retval = hal_pin_bit_new("mark.posValid", HAL_OUT, &halpins->posValid, comp_id);
    if (retval != 0)
        return ;
    *halpins->posValid=1;

    retval = hal_pin_bit_new("mark.watchPosValid", HAL_OUT, &halpins->watchPosValid, comp_id);
    if (retval != 0)
        return ;
    *halpins->watchPosValid=1;

    retval = hal_pin_bit_new("mark.watchHoleValid", HAL_OUT, &halpins->watchHoleValid, comp_id);
    if (retval != 0)
        return ;
    *halpins->watchHoleValid=1;

    retval = hal_pin_s32_new("mark.state", HAL_OUT, &halpins->state, comp_id);
    if (retval != 0)
        return ;
    *halpins->state=MARK_WAITING;

    retval = hal_pin_u32_new("mark.fps", HAL_OUT, &halpins->fps, comp_id);
    if (retval != 0)
        return ;

    hal_ready(comp_id);

    //run config file
    const char* halcmd="halcmd -i /home/u/cnc/configs/ppmc/ppmc.ini -f /home/u/cnc/configs/ppmc/watchDiamond.hal";
    retval=system(halcmd);
    if (retval != 0) {
        printf("ERROR: %s fail\n", halcmd);
        exit(-1);
    }
}



void MarkEmcStatus::update(){
    emc_update(NULL);
    emc_abs_act_pos(actualAxis+0, 0);
    emc_abs_act_pos(actualAxis+1, 1);
    emc_abs_act_pos(actualAxis+2, 2);
    emc_abs_act_pos(actualAxis+3, 3);
    emc_abs_act_pos(actualAxis+4, 4);
    emc_abs_cmd_pos(cmdAxis+0, 0);
    emc_abs_cmd_pos(cmdAxis+1, 1);
    emc_abs_cmd_pos(cmdAxis+2, 2);
    emc_abs_cmd_pos(cmdAxis+3, 3);
    emc_abs_cmd_pos(cmdAxis+4, 4);
    emc_program_status(&programStaut);
    emc_mode(&mode, EMC_TASK_MODE_MANUAL);

    //printf("%.3f %.3f\n",lastActualAxis[4],actualAxis[4]);

    if(fabs(lastActualAxis[0]-actualAxis[0])<0.001
        && fabs(lastActualAxis[1]-actualAxis[1])<0.001
        &&fabs(lastActualAxis[2]-actualAxis[2])<0.001
            &&fabs(lastActualAxis[3]-actualAxis[3])<0.001
            &&fabs(lastActualAxis[4]-actualAxis[4])<0.001){
        if(stopComfirm)
            hasStop=true;
        else
            stopComfirm=true;
    }
    else{
        hasStop=false;
        stopComfirm=false;
    }
    for(int i=0;i<5; i++)
        lastActualAxis[i]=actualAxis[i];
}

