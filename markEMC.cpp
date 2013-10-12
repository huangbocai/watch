
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

    retval = hal_pin_bit_new("mark.glueHoleValid", HAL_OUT, &halpins->glueHoleValid, comp_id);
    if (retval != 0)
        return ;
    *halpins->glueHoleValid=1;

    retval = hal_pin_bit_new("mark.setGlue", HAL_IN, &halpins->setGlue, comp_id);
    if (retval != 0)
        return ;
    *halpins->setGlue=0;

    retval = hal_pin_bit_new("mark.glueUpDown", HAL_IN, &halpins->glueUpDown, comp_id);
    if (retval != 0)
        return ;
    *halpins->glueUpDown=0;

    retval = hal_pin_bit_new("mark.pickupDiamond", HAL_IN, &halpins->pickupDiamond, comp_id);
    if (retval != 0)
        return ;
    *halpins->pickupDiamond=0;

    retval = hal_pin_bit_new("mark.dropDiamond", HAL_IN, &halpins->dropDiamond, comp_id);
    if (retval != 0)
        return ;
    *halpins->dropDiamond=0;

    retval = hal_pin_bit_new("mark.lightControl", HAL_IN, &halpins->lightControl, comp_id);
    if (retval != 0)
        return ;
    *halpins->lightControl=0;

    retval = hal_pin_bit_new("mark.start", HAL_IN, &halpins->start, comp_id);
    if (retval != 0)
        return ;
    *halpins->start=0;

    retval = hal_pin_bit_new("mark.paused", HAL_IN, &halpins->paused, comp_id);
    if (retval != 0)
        return ;
    *halpins->paused=0;

    retval = hal_pin_bit_new("mark.stop", HAL_IN, &halpins->stop, comp_id);
    if (retval != 0)
        return ;
    *halpins->stop=0;

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


MarkEmcStatus::MarkEmcStatus():stopToManual(false),homing(false),currentHS(UNHOMED),
    mode(EMC_TASK_MODE_MANUAL){
    for(int i=0; i<5; i++)
        axisHomeState[i] = Unhomed;
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

    emc_error(errorMsg,1);
    emc_operator_text(operatorText,1);
    emc_operator_display(operatorDisplay,1);

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

    update_current_home_state();

    update_current_time_state();
}

void MarkEmcStatus::update_current_home_state()
{

    if((currentHS==UNHOMED || currentHS==ALL_HOMED) &&homing){
        emc_home(2);
        axisHomeState[2] = Homeing;
        currentHS = Z_AXIS_HOMING;
    }
    else if(currentHS == Z_AXIS_HOMING ){
        int homed=0;
        emc_joint_homed(2,&homed);
        if(homed == 1){
            axisHomeState[2] = Homed;
            emc_home(0); axisHomeState[0] = Homeing;
            emc_home(1); axisHomeState[1] = Homeing;
            emc_home(3); axisHomeState[3] = Homeing;
            emc_home(4); axisHomeState[4] = Homeing;
            currentHS = OTHER_AXIS_HOMING;
        }
    }
    else if(currentHS == OTHER_AXIS_HOMING){
        int sum = 0;
        int h[5] = {0,0,0,0,0};
        emc_joint_homed(0,h);
        emc_joint_homed(1,(h+1));
        emc_joint_homed(2,(h+2));
        emc_joint_homed(3,(h+3));
        emc_joint_homed(4,(h+4));
        for(int i=0; i<5; i++){
            if(h[i] == 1){
                axisHomeState[i] = Homed;
            }
            sum += h[i];
        }
        if(sum==5){
            homing = false;
            currentHS = ALL_HOMED;
        }
    }
}

#define READING EMC_TASK_INTERP_READING
#define PAUSED  EMC_TASK_INTERP_PAUSED
#define WAITING EMC_TASK_INTERP_WAITING
#define IDLE EMC_TASK_INTERP_IDLE

void MarkEmcStatus::update_current_time_state()
{
    if(lastProgramStaut != READING
            && programStaut == READING){
        prtManager.set_time_state(PRTManager::Start);
    }
    if(lastProgramStaut == READING &&
            programStaut == PAUSED ){
        prtManager.set_time_state(PRTManager::Pause);
    }
    if(lastProgramStaut == PAUSED&&
            programStaut == READING){
        prtManager.set_time_state(PRTManager::Restart);
    }
    if(lastProgramStaut == READING &&
            (programStaut == WAITING || programStaut == IDLE)) {
        prtManager.set_time_state(PRTManager::End);
    }
    lastProgramStaut = programStaut;

    prtManager.update_time();

}


void PRTManager::start()
{
    time(&startTime);
}

void PRTManager::pause()
{
    //struct tm* fmt;
    //char buf[128];
    time(&pauseTime);
    //fmt = localtime(&pauseTime);
    //sprintf(buf,"pause: %d:%d:%d",fmt->tm_hour,fmt->tm_min,fmt->tm_sec);
    //printf("%s\n",buf);
}

void PRTManager::restart()
{
    //struct tm* fmt;
    //char buf[128];
    time(&restartTime);
    //fmt = localtime(&restartTime);
    //sprintf(buf,"restart: %d:%d:%d",fmt->tm_hour,fmt->tm_min,fmt->tm_sec);
    //printf("%s\n",buf);
}

void PRTManager::end()
{
    time(&endTime);
    if(pauseTime == 0 && restartTime == 0){
        runTime = difftime(endTime,startTime);
        runTime -= allPauseTime;
    }
    else{
        time_t tmp1;
        tmp1 = difftime(restartTime,pauseTime);
        runTime = difftime(endTime,startTime);
        allPauseTime += tmp1;
        restartTime = 0;
        pauseTime = 0;
        //printf("run: %d,%d\n",(int)tmp1,(int)tmp2);
        runTime -= allPauseTime;
    }

}

const std::string& PRTManager::get_run_time_string()
{
    runTimeStr = int_to_time_string((int)runTime);
    return runTimeStr;
}

std::string PRTManager::int_to_time_string(int sec)
{
    int mm=0, ss=0;
    char timeBuf[24];
    if(sec>=60){
        ss = sec%60;
        mm = (sec-ss)/60;
        if(mm>=10){
            if(ss>=10){
                sprintf(timeBuf,"%d:%d",mm,ss);
            }
            else{
                sprintf(timeBuf,"%d,0%d",mm,ss);
            }
        }
        else{
            if(ss>=10){
                sprintf(timeBuf,"0%d:%d",mm,ss);
            }
            else{
                sprintf(timeBuf,"0%d:0%d",mm,ss);
            }
        }
    }
    else if(sec>=10){
        sprintf(timeBuf,"00:%d",sec);
    }
    else{
        sprintf(timeBuf,"00:0%d",sec);
    }
    return std::string(timeBuf);
}


void PRTManager::set_time_state(TimeState ts)
{
    timeState = ts;
}


void PRTManager::update_time()
{
    switch(timeState){
    case Start:
        start();
        allPauseTime = 0;
        timeState = running;
        break;
    case running:
        run();
        break;
    case Pause:
        pause();
        timeState = Idle;
        break;
    case Restart:
        restart();
        timeState = running;
        break;
    case End:
        end();
        timeState = Idle;
        break;
    case Idle:
        break;
    }

}
