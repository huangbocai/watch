/********************************************************************
* Description: emcsh.cc
*   Extended-Tcl-based EMC automatic test interface
*
*   Derived from a work by Fred Proctor & Will Shackleford
*
* Author:
*         Reorganized by Eric H. Johnson
* License: GPL Version 2
* System: Linux
*
* Copyright (c) 2004 All rights reserved.
*
* Last change:
********************************************************************/
//#ifndef _REENTRANT
//#define _REENTRANT
//#endif


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <signal.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>

#include "hal.h"		/* access to HAL functions/definitions */
#include "rtapi.h"		/* rtapi_print_msg */
#include "rcs.hh"
#include "posemath.h"		// PM_POSE, TO_RAD
#include "emc.hh"		// EMC NML
#include "emc_nml.hh"		// EMC NML
#include "canon.hh"		// CANON_UNITS, CANON_UNITS_INCHES,MM,CM
#include "emcglb.h"		// EMC_NMLFILE, TRAJ_MAX_VELOCITY, etc.
#include "emccfg.h"		// DEFAULT_TRAJ_MAX_VELOCITY
#include "inifile.hh"		// INIFILE
#include "rcs_print.hh"
#include "timer.hh"

#include "emcsecnml.hh"

#include "emcsec.hh"

/* EMC command functions */

int emc_ini(char* value, const char* secstr, const char* varstr)
{
    IniFile inifile;
    const char *inistring;
		
    // open it
    if (inifile.Open(EMC_INIFILE) == false) {
		printf("can not open ini file\n");
		return -1;
    }

  	inistring = inifile.Find(varstr, secstr);
	if(inistring==NULL)
	{
		inifile.Close();
		return -1;
	}
	else
	{
		strcpy(value, inistring);
		inifile.Close();
		return 0;
	}
	
}


int emc_debug(int *debugS, int debugV)
{
	int debug;
    if (emcUpdateType == EMC_UPDATE_AUTO) {
	updateStatus();
    }

    if (debugS) {
	*debugS=emcStatus->debug;
	return 0;
    }
		
	debug=debugV;
	sendDebug(debug);
	EMC_DEBUG = debug;
	return 0;
}

 int emc_set_wait(enum EMC_WAIT_TYPE* waitS, enum EMC_WAIT_TYPE waitC)
{
    if (waitS) {
	switch (emcWaitType) {
	case EMC_WAIT_NONE:
	case EMC_WAIT_RECEIVED:
	case EMC_WAIT_DONE:
		*waitS=emcWaitType;
		return 0;
		break;
	default:
		*waitS=EMC_WAIT_RECEIVED;
	   return -1;
	    break;
	}
  }

   
	if (waitC==EMC_WAIT_NONE ||waitC==EMC_WAIT_RECEIVED||waitC==EMC_WAIT_DONE) 
	{
	    emcWaitType =waitC;
	    return 0;
    }
    printf("emc_set_wait: need 'none', 'received', 'done'.\n");
    return -1;
}


int emc_wait(const char* objstr)
{
	if (!strcmp(objstr, "received")) {
	    if (0 != emcCommandWaitReceived(emcCommandSerialNumber)) {
		printf( "timeout");
	    }
	    return 0;
	}
	if (!strcmp(objstr, "done")) {
	    if (0 != emcCommandWaitDone(emcCommandSerialNumber)) {
		printf( "timeout");
	    }
	    return 0;
	}
    printf( "emc_wait: need 'received' or 'done'\n");
    return -1;
}

int emc_set_timeout(double* timeoutS, double timeout)
{
  
    if (timeoutS) {
		*timeoutS=emcTimeout;
		return 0;
    }
	    emcTimeout = timeout;
	    return 0;
}

int emc_update(const char* objstr)
{

    if (objstr==NULL) {
	// no arg-- return status
	updateStatus();
	return 0;
    }
	if (!strcmp(objstr, "none")) {
	    emcUpdateType = EMC_UPDATE_NONE;
	    return 0;
	}
	if (!strcmp(objstr, "auto")) {
	    emcUpdateType = EMC_UPDATE_AUTO;
	    return 0;
	}
	return -1;
}

int emc_time(double* time)
{
   
	*time=etime();
	return 0;
}


int emc_error(char* error_str, int update)
{
	// get any new error, it's saved in global error_string[]
	if(update)
	{
		if (0 != updateError()) {
	   	 printf( "emc_error: bad status from EMC\n");
	    	return -1;
		}
	}
	strcpy(error_str, error_string);
	error_string[0]='\0';
	return 0;
}

int emc_operator_text(char* str, int update)
{
	// get any new string, it's saved in global operator_text_string[]
	if(update)
	{
		if (0 != updateError()) {
			printf( "emc_operator_text: bad status from EMC\n");
	    	return -1;
		}
	}
	// put error on result list
	strcpy(str, operator_text_string);
	operator_text_string[0]='\0';
	return 0;
}

int emc_operator_display(char* str, int update)
{
	// get any new string, it's saved in global operator_display_string[]
	if(update)
	{
		if (0 != updateError()) {
	   	 printf(
				  "emc_operator_display: bad status from EMC\n");
	    	return -1;
		}
	}
	// put error on result list
	strcpy(str, operator_display_string);
	operator_display_string[0]='\0';
	return 0;
}

int emc_estop(int* stat, int cmd)
{


    if (stat) {
	// no arg-- return status
		if (emcUpdateType == EMC_UPDATE_AUTO) {
	    	updateStatus();
		}
		if (emcStatus->task.state == EMC_TASK_STATE_ESTOP) {
	    	*stat=1;
		} else {
	    	*stat=0;
		}
		return 0;
    }

	if (cmd) {
	    sendEstop();
	    return 0;
	}
	else{
	    sendEstopReset();
	    return 0;
	}
}

int emc_machine(int* stat, int cmd)
{

    if (stat) {
	// no arg-- return status
	if (emcUpdateType == EMC_UPDATE_AUTO) {
	    updateStatus();
	}
	if (emcStatus->task.state == EMC_TASK_STATE_ON) {
	   *stat=1;
	} else {
	    *stat=0;
	}
	return 0;
    }
		
	if (cmd) {
	    sendMachineOn();
	    return 0;
	}
	else {
	    sendMachineOff();
	    return 0;
	}
}

int emc_mode(enum EMC_TASK_MODE_ENUM *stat,
	enum EMC_TASK_MODE_ENUM cmd)
{


    if (stat ){
	// no arg-- return status
	if (emcUpdateType == EMC_UPDATE_AUTO) {
	    updateStatus();
	}
	switch (emcStatus->task.mode) {
	case EMC_TASK_MODE_MANUAL:
	case EMC_TASK_MODE_AUTO:
	case EMC_TASK_MODE_MDI:
		*stat=emcStatus->task.mode;
		return 0;
	default:
	 	return -1;
	}
    }
		
	if (cmd==EMC_TASK_MODE_MANUAL) {
	    sendManual();
	    return 0;
	}
	if (cmd==EMC_TASK_MODE_AUTO) {
	    sendAuto();
	    return 0;
	}
	if (cmd==EMC_TASK_MODE_MDI) {
	    sendMdi();
	    return 0;
	}

    printf("emc_mode: need 'manual', 'auto', 'mdi'.\n");
    return -1;
}


int emc_spindle(int* stat, int cmd)
{

    if (stat) {
	//  return status
		if (emcUpdateType == EMC_UPDATE_AUTO) {
			updateStatus();
		}

		if (emcStatus->motion.spindle.direction ) {
	   		*stat=1;
		} 
		else {
	    *stat=0;
		}
		return 0;
    }
    
	if (cmd)
	    sendSpindleForward();
	else
	    sendSpindleOff();
	return 0;
 
}



int emc_flood(int* stat, int cmd)
{
    if (stat) 
	{
		// no arg-- return status
		if (emcUpdateType == EMC_UPDATE_AUTO) 
	    	updateStatus();
		if (emcStatus->io.coolant.flood == 1) 
	  	 	*stat=1;
		else 
	    	*stat=0;
		return 0;
    }
	if (cmd) 
	    sendFloodOn();
	else
	    sendFloodOff();
    return 0;
}


int emc_abs_cmd_pos(double* pos, int axis)
{
    if (emcUpdateType == EMC_UPDATE_AUTO) {
	updateStatus();
    }

    if (pos && axis>=0&&axis<=8) {
		if (axis == 0) {
	   	 *pos=convertLinearUnits(emcStatus->motion.traj.position.tran.x);
		} 
		else if (axis == 1) {
	    	*pos=convertLinearUnits(emcStatus->motion.traj.position.tran.y);
		}
		else if (axis == 2) {
	   	 *pos=convertLinearUnits(emcStatus->motion.traj.position.tran.z);
		} 
		else {
	    	if (axis == 3) {
				*pos=convertAngularUnits(emcStatus->motion.traj.position.a);
	    	} 
			else if (axis == 4) {
				*pos=convertAngularUnits(emcStatus->motion.traj.position.b);
	    	} 
			else if (axis == 5) {
				*pos=convertAngularUnits(emcStatus->motion.traj.position.c);
	    	} 
			else if (axis == 6) {
				*pos=convertAngularUnits(emcStatus->motion.traj.position.u);
			} 
			else if (axis == 7) {
				*pos=convertAngularUnits(emcStatus->motion.traj.position.v);
	    	} 
			else if (axis == 8) {
				*pos=convertAngularUnits(emcStatus->motion.traj.position.w);
	   		 } 
			else {
				*pos=0;
	    	}
		}
    } 
	else {
		printf( "emc_abs_cmd_pos: bad integer argument\n");
		return -1;
    }
	return 0;
}


 int emc_abs_act_pos(double* pos, int axis)
{

	if (emcUpdateType == EMC_UPDATE_AUTO) {
	updateStatus();
    }

    if (pos && axis>=0&&axis<=8) {
	if (axis == 0) {
	    *pos=convertLinearUnits(emcStatus->motion.traj.actualPosition.tran.x);
	} 
	else if (axis == 1) {
	    *pos=convertLinearUnits(emcStatus->motion.traj.actualPosition.tran.y);
	}
	else if (axis == 2) {
	    *pos=convertLinearUnits(emcStatus->motion.traj.actualPosition.tran.z);
	} 
	else {
	    if (axis == 3) {
			*pos=convertAngularUnits(emcStatus->motion.traj.actualPosition.a);
	    } 
		else if (axis == 4) {
			*pos=convertAngularUnits(emcStatus->motion.traj.actualPosition.b);
	    } 
		else if (axis == 5) {
			*pos=convertAngularUnits(emcStatus->motion.traj.actualPosition.c);
	    } 
		else if (axis == 6) {
			*pos=convertAngularUnits(emcStatus->motion.traj.actualPosition.u);
	    } 
		else if (axis == 7) {
			*pos=convertAngularUnits(emcStatus->motion.traj.actualPosition.v);
	    } 
		else if (axis == 8) {
			*pos=convertAngularUnits(emcStatus->motion.traj.actualPosition.w);
	    } 
		else {
			*pos=0;
	    }
	}
    } else {
	printf( "emc_abs_act_pos: bad integer argument\n");
	return -1;
    }
	return 0;
}

int emc_rel_cmd_pos(double* pos, int axis)
{
    if( !(pos && axis>=0&&axis<=8)) {
	printf("emc_rel_cmd_pos: need exactly 1 non-negative integer\n");
	return -1;
    }

    if (emcUpdateType == EMC_UPDATE_AUTO) {
	updateStatus();
    }

    if (pos && axis>=0&&axis<=8){
	if (axis == 0) {
	    *pos=convertLinearUnits(emcStatus->motion.traj.
						    position.tran.x -
						    emcStatus->task.origin.
						    tran.x -
						    emcStatus->task.toolOffset.tran.x);
	} else if (axis == 1) {
	    *pos=convertLinearUnits(emcStatus->motion.traj.
						    position.tran.y -
						    emcStatus->task.origin.
						    tran.y -
						    emcStatus->task.toolOffset.tran.y);
	} else if (axis == 2) {
	    *pos=convertLinearUnits(emcStatus->motion.traj.
						    position.tran.z -
						    emcStatus->task.origin.
						    tran.z-
						    emcStatus->task.toolOffset.tran.z);
	} else {
	    if (axis == 3) {
		    *pos=convertAngularUnits(emcStatus->motion.
							traj.position.a -
							emcStatus->task.
							origin.a -
							emcStatus->task.toolOffset.a);
	    } else if (axis == 4) {
		    *pos=convertAngularUnits(emcStatus->motion.
							traj.position.b -
							emcStatus->task.
							origin.b -
							emcStatus->task.toolOffset.b);
	    } else if (axis == 5) {
		    *pos=convertAngularUnits(emcStatus->motion.
							traj.position.c -
							emcStatus->task.
							origin.c -
							emcStatus->task.toolOffset.c);
	    } else if (axis == 6) {
		    *pos=convertAngularUnits(emcStatus->motion.
							traj.position.u-
							emcStatus->task.
							origin.u-
							emcStatus->task.toolOffset.u);
	    } else if (axis == 7) {
		  *pos=convertAngularUnits(emcStatus->motion.
							traj.position.v-
							emcStatus->task.
							origin.v-
							emcStatus->task.toolOffset.v);
	    } else if (axis == 8) {
		  *pos=convertAngularUnits(emcStatus->motion.
							traj.position.w-
							emcStatus->task.
							origin.w-
							emcStatus->task.toolOffset.w);
	    } else {
		*pos=0;
	    }
	}
    } else {
	printf( "emc_rel_cmd_pos: bad integer argument\n");
	return -1;
    }
		
    return 0;
}

int emc_rel_act_pos(double* pos, int axis)
{
	if( !(pos && axis>=0&&axis<=8)) {
	printf("emc_rel_act_pos: need exactly 1 non-negative integer\n");
	return -1;
    }

    if (emcUpdateType == EMC_UPDATE_AUTO) {
	updateStatus();
    }

    if (pos && axis>=0&&axis<=8){
	if (axis == 0) {
	    *pos=convertLinearUnits(emcStatus->motion.traj.
						    actualPosition.tran.x -
						    emcStatus->task.origin.
						    tran.x -
						    emcStatus->task.toolOffset.tran.x);
	} else if (axis == 1) {
	    *pos=convertLinearUnits(emcStatus->motion.traj.
						    actualPosition.tran.y -
						    emcStatus->task.origin.
						    tran.y -
						    emcStatus->task.toolOffset.tran.y);
	} else if (axis == 2) {
	    *pos=convertLinearUnits(emcStatus->motion.traj.
						    actualPosition.tran.z -
						    emcStatus->task.origin.
						    tran.z-
						    emcStatus->task.toolOffset.tran.z);
	} else {
	    if (axis == 3) {
		    *pos=convertAngularUnits(emcStatus->motion.
							traj.actualPosition.a -
							emcStatus->task.
							origin.a -
							emcStatus->task.toolOffset.a);
	    } else if (axis == 4) {
		    *pos=convertAngularUnits(emcStatus->motion.
							traj.actualPosition.b -
							emcStatus->task.
							origin.b -
							emcStatus->task.toolOffset.b);
	    } else if (axis == 5) {
		    *pos=convertAngularUnits(emcStatus->motion.
							traj.actualPosition.c -
							emcStatus->task.
							origin.c -
							emcStatus->task.toolOffset.c);
	    } else if (axis == 6) {
		    *pos=convertAngularUnits(emcStatus->motion.
							traj.actualPosition.u-
							emcStatus->task.
							origin.u-
							emcStatus->task.toolOffset.u);
	    } else if (axis == 7) {
		  *pos=convertAngularUnits(emcStatus->motion.
							traj.actualPosition.v-
							emcStatus->task.
							origin.v-
							emcStatus->task.toolOffset.v);
	    } else if (axis == 8) {
		  *pos=convertAngularUnits(emcStatus->motion.
							traj.actualPosition.w-
							emcStatus->task.
							origin.w-
							emcStatus->task.toolOffset.w);
	    } else {
		*pos=0;
	    }
	}
    } else {
	printf( "emc_rel_act_pos: bad integer argument\n");
	return -1;
    }
		
    return 0;
}

int emc_joint_pos(int axis, double *pos)
{
    if (!pos) {
	printf( "emc_joint_pos: need exactly pointer of posistion value.\n");
	return -1;
    }

    if (emcUpdateType == EMC_UPDATE_AUTO) {
	updateStatus();
    }

    if (axis>=0&&axis<=8) {
	*pos=emcStatus->motion.axis[axis].input;
    } else {
	printf( "emc_joint_pos: bad integer argument\n");
	return -1;
    }
    return 0;
}

int emc_pos_offset(int axis, double* pos_offset)
{
    if (!pos_offset) {
			printf( "emc_pos_offset: need exactly pointer of posistion value.\n");
			return -1;
    }

    if (emcUpdateType == EMC_UPDATE_AUTO) {
	updateStatus();
    }

    if (axis==0) {
		*pos_offset=convertLinearUnits(emcStatus->task.origin.tran.x);
    } else if (axis==1) {
		*pos_offset=convertLinearUnits(emcStatus->task.origin.tran.y);
    } else if (axis==2) {
		*pos_offset=convertLinearUnits(emcStatus->task.origin.tran.z);
    } else if (axis==3) {
		*pos_offset=convertAngularUnits(emcStatus->task.origin.a);
    } else if (axis==4) {
		*pos_offset=convertAngularUnits(emcStatus->task.origin.b);
    } else if (axis==5){
		*pos_offset=convertAngularUnits(emcStatus->task.origin.c);
    } else if (axis==6) {
		*pos_offset=convertAngularUnits(emcStatus->task.origin.u);
    } else if (axis==7) {
		*pos_offset=convertAngularUnits(emcStatus->task.origin.v);
    } else if (axis==8) {
		*pos_offset=convertAngularUnits(emcStatus->task.origin.w);
    } else {
		printf( "emc_pos_offset: bad integer argument");
		return -1;
    }
    return 0;
}

 int emc_joint_limit(int joint, emc_limit_t* limit)
{
    if (!limit) {
		printf("emc_joint_limit: need pointer of limit value\n");
		return -1;
    }

    if (emcUpdateType == EMC_UPDATE_AUTO) {
		updateStatus();
    }

	if (joint < 0 || joint >= EMC_AXIS_MAX) {
	    printf("emc_joint_limit: joint out of bounds\n");
	    return -1;
	}

	if (emcStatus->motion.axis[joint].minHardLimit) {
	    *limit=HARDMIN;
	    return 0;
	} else if (emcStatus->motion.axis[joint].minSoftLimit) {
	    *limit=SOFTMIN;
	    return 0;
	} else if (emcStatus->motion.axis[joint].maxSoftLimit) {
	    *limit=SOFTMAX;
	    return 0;
	} else if (emcStatus->motion.axis[joint].maxHardLimit) {
	    *limit=HARDMAX;
	    return 0;
	} else {
	   *limit=LIMITOK;
	    return 0;
	}
}

int emc_joint_fault(int joint, int* fault)
{

    if (!fault) {
		printf("emc_joint_fault: need int pointer of fault\n");
		return -1;
    }

    if (emcUpdateType == EMC_UPDATE_AUTO) {
		updateStatus();
    }

	if (joint < 0 || joint >= EMC_AXIS_MAX) {
	    printf(
			  "emc_joint_fault: joint out of bounds\n");
	    return -1;
	}

	if (emcStatus->motion.axis[joint].fault) {
		*fault=1;
	    return 0;
	} else {
	 	 *fault=0;
	    return 0;
	}
}

int emc_override_limit(int *orls, int on)
{
    if (orls) {
		// -- return status
		if (emcUpdateType == EMC_UPDATE_AUTO) {
	   	 updateStatus();
		}
		// motion overrides all axes at same time, so just reference index 0
		*orls=emcStatus->motion.axis[0].overrideLimits;
		return 0;
    }

	    if (on) {
			if (0 != sendOverrideLimits(0)) {
		    	printf( "emc_override_limit: can't send command\n");
			}
	    } 
		else {
			if (0 != sendOverrideLimits(-1)) {
		    	printf(
				  	"emc_override_limit: can't send command\n");
			}
	    }
	    return 0;
}

int emc_joint_homed(int joint, int *homed)
{

    if (!homed) {
	printf(
		      "emc_joint_homed: need exactly int pointer of homed value\n");
	return -1;
    }

    if (emcUpdateType == EMC_UPDATE_AUTO) {
	updateStatus();
    }

	if (joint < 0 || joint >= EMC_AXIS_MAX) {
	    printf(
			  "emc_joint_homed: joint out of bounds\n");
	    return -1;
	}

	if (emcStatus->motion.axis[joint].homed) {
	   	*homed=1;
	    return 0;
	} else {
	    *homed=0;
	    return 0;
	}

}

int emc_mdi(const char* string)
{
    if (!string) {
	printf( "emc_mdi: need command\n");
	return -1;
    }

    if (0 != sendMdiCmd(string)) {
		printf( "emc_mdi: error executing command\n");
    }
	return 0;
}

int emc_home(int axis)
{
    if (axis>=0&& axis<9){
		sendHome(axis);
		return 0;
    }

    printf( "emc_home: need axis as integer, 0..8\n");
    return -1;
}

int emc_unhome(int axis)
{

	   if (axis>=0&& axis<9){
		sendUnHome(axis);
		return 0;
    }

    printf( "emc_unhome: need axis as integer, 0..8\n");
    return -1;
}

int emc_jog_stop(int axis)
{
	if (axis>=0&& axis<9){
		if (0 != sendJogStop(axis)) 
				printf( "emc_jog_stop: can't send jog stop msg\n");		
		return 0;
    }

    printf( "emc_jog_stop: need axis as integer, 0..8\n");
    return -1;
}

int emc_jog(int axis, double speed)
{
	if (axis>=0&& axis<9){
		if (0 != sendJogCont(axis, speed)) 
				printf( "emc_jog: can't send jog msg\n");		
		return 0;
    }

    printf( "emc_jog: need axis as integer, 0..8\n");
    return -1;
}

 int emc_jog_incr(int axis, double speed, double incr)
{
	if (axis>=0&& axis<9){
    	if (0 != sendJogIncr(axis, speed, incr)) 
			printf( "emc_jog_incr: can't jog");
	return 0;
	}
   	
	printf( "emc_jog_incr: need axis as integer, 0..8\n");
    return -1;
}

  int emc_jog_abs(int axis, double speed, double pos)
  {
  	if (axis>=0&& axis<9){
    	if (0 != sendJogAbs(axis, speed, pos)) 
			printf( "emc_jog_abs: can't jog");
	return 0;
	}
   	
	printf( "emc_jog_abs: need axis as integer, 0..8\n");
    return -1;
  }

int emc_feed_override(int *fos, int percent)
{
    if (fos) {
	//-- return status
		if (emcUpdateType == EMC_UPDATE_AUTO) {
	   	 	updateStatus();
		}
	    *fos=(int)(emcStatus->motion.traj.scale * 100.0 + 0.5);
		return 0;
    }
		
	if(percent<0)
		percent=0;
	sendFeedOverride(((double) percent) / 100.0);
	return 0;
}

 int emc_spindle_override(int *fos, int percent)
{

	 if (fos) {
	//-- return status
		if (emcUpdateType == EMC_UPDATE_AUTO) {
	   	 	updateStatus();
		}
	    *fos=(int)(emcStatus->motion.traj.spindle_scale * 100.0 + 0.5);
		return 0;
    }
		
	if(percent<0)
		percent=0;
	emcWaitType=EMC_WAIT_NONE;
	sendSpindleOverride(((double) percent) / 100.0);
	emcWaitType=EMC_WAIT_RECEIVED;
	return 0;
}

int emc_task_plan_init()
{
    if (0 != sendTaskPlanInit()) {
	printf( "emc_task_plan_init: can't init interpreter\n");
	return 0;
    }
    return 0;
}

int emc_open(const char* file)
{
    if (!file) {
		printf( "emc_open: need file\n");
		return -1;
    }

    if (0 != sendProgramOpen(file)){
	printf( "emc_open: can't open file\n");
	return -1;
    }
    return 0;
}

int emc_run(int line)// line=0 means start from file beginning
{
	if(line<0)
		line=0;
	if (0 != sendProgramRun(line)) {
	    printf( "emc_run: can't execute program\n");
	    return -1;
	}
    return 0;
}

 int emc_pause()
{
    if (0 != sendProgramPause()) {
	printf( "emc_pause: can't pause program\n");
	return -1;
    }
    return 0;
}

int emc_optional_stop(int *stat, int on)
{

    if (stat) {
	// -- return status
		if (emcUpdateType == EMC_UPDATE_AUTO) {
	   	 	updateStatus();
		}
	// get the current state from the status
		*stat = emcStatus->task.optional_stop_state;
		return 0;
    }


	if(on){
		if (0 != sendSetOptionalStop(1)) {
		    printf(
				  "emc_optional_stop: can't send command\n");
		    return -1;
	    }
	}
	else{
		if (0 != sendSetOptionalStop(0)) {
		    printf(
				  "emc_optional_stop: can't send command\n");
		    return -1;
	    }
	}
		return 0;
		
}

 int emc_resume()
{
    if (0 != sendProgramResume()) {
	printf( "emc_resume: can't resume program\n");
	return -1;
    }

    return 0;
}

int emc_step()
{
    if (0 != sendProgramStep()) {
		printf( "emc_step: can't step program\n");
		return -1;
    }
    return 0;
}

int emc_abort()
{
    if (0 != sendAbort()) {
		printf( "emc_abort: can't execute program\n");
		return 1;
    }
    return 0;
}

int emc_program(char* file)
{
    if (emcUpdateType == EMC_UPDATE_AUTO) {
		updateStatus();
    }
    if (0 != emcStatus->task.file[0]) {
		strcpy(file, emcStatus->task.file);
		return 0;
    }
    else
    	return -1;
}

int emc_program_status(enum EMC_TASK_INTERP_ENUM *stat)
{
    if (emcUpdateType == EMC_UPDATE_AUTO) {
		updateStatus();
    }
	*stat=emcStatus->task.interpState;
	return 0;
}

int emc_program_line(int* programActiveLine)
{
    *programActiveLine = 0;

    if (emcUpdateType == EMC_UPDATE_AUTO) {
		updateStatus();
    }

    if (programStartLine < 0
	|| emcStatus->task.readLine < programStartLine) {
	// controller is skipping lines
	*programActiveLine = emcStatus->task.readLine;
    } else {			// controller is not skipping lines
	if (emcStatus->task.currentLine > 0) {
	    if (emcStatus->task.motionLine > 0 &&
		emcStatus->task.motionLine < emcStatus->task.currentLine) {
		// active line is the motion line, which lags
		*programActiveLine = emcStatus->task.motionLine;
	    } else {
		// active line is the current line-- no motion lag
		*programActiveLine = emcStatus->task.currentLine;
	    }
	} else {
	    // no active line at all
	    *programActiveLine = 0;
	}
    }				// end of else controller is not skipping
    // lines
    return 0;
}


 int emc_linear_unit_conversion(enum LINEAR_UNIT_CONVERSION *stat, 
	enum LINEAR_UNIT_CONVERSION unit)
{

    if (stat) {
	// no arg-- return unit setting
	switch (linearUnitConversion) {
	case LINEAR_UNITS_INCH:
	case LINEAR_UNITS_MM:
	case LINEAR_UNITS_CM:
	case LINEAR_UNITS_AUTO:
		*stat=linearUnitConversion;
		return 0;
		break;
	default:
	  	*stat=LINEAR_UNITS_MM;
		return -1;
	    break;
	}
 	}

	if(unit==LINEAR_UNITS_INCH ||unit==LINEAR_UNITS_MM||unit==LINEAR_UNITS_CM
		||unit==LINEAR_UNITS_AUTO||unit==LINEAR_UNITS_CUSTOM)
	{	
		linearUnitConversion=unit;
		return 0;
	}
	
	
    printf("emc_linear_unit_conversion: need 'inch', 'mm', 'cm', 'auto', 'custom', or no args");
    return -1;
}

 int emc_axis_backlash(int axis, double *bls, double backlash)
{

    // check axis number
    if (axis < 0 || axis >= EMC_AXIS_MAX) {
		printf("emc_axis_backlash: need axis as integer, 0..\n");
		return -1;
    }
		
    // test for get or set
    if (bls) {
		// want to get present value
		*bls=emcStatus->motion.axis[axis].backlash;
		return 0;
    } 
	else {
	sendAxisSetBacklash(axis, backlash);
	return 0;
    }
}

int emc_axis_enable(int axis, int * stat, int val)
{
    // syntax is emc_axis_output <axis> {0 | 1}


    if (axis < 0 || axis >= EMC_AXIS_MAX) {
	printf("emc_axis_enable: need axis as integer, 0..8\n");
	return -1;
    }

    if (stat) {
	if (emcUpdateType == EMC_UPDATE_AUTO) {
	    updateStatus();
		}
		*stat=(int)(emcStatus->motion.axis[axis].enabled);
		return 0;
    }
		
    // else we were given 0 or 1 to enable/disable it
    sendAxisEnable(axis, val);
    return 0;
}

int emc_axis_load_comp(int axis, char* file, int type)
{
    // syntax is emc_axis_load_comp <axis> <file>



    if (axis < 0 || axis >= EMC_AXIS_MAX) {
	printf(
		      "emc_axis_load_comp: need axis as integer, 0..8\n");
	return -1;
    }
    // now write it out
    sendAxisLoadComp(axis, file, type);
    return 0;
}



static void thisQuit()
{
	//hal_exit(cvmdata.autoRun.comp_id);
	
    EMC_NULL emc_null_msg;

    if (0 != emcStatusBuffer) {
	// wait until current message has been received
	emcCommandWaitReceived(emcCommandSerialNumber);
    }

    if (0 != emcCommandBuffer) {
	// send null message to reset serial number to original
	emc_null_msg.serial_number = saveEmcCommandSerialNumber;
	emcCommandBuffer->write(emc_null_msg);
    }
    // clean up NML buffers

    if (emcErrorBuffer != 0) {
	delete emcErrorBuffer;
	emcErrorBuffer = 0;
    }

    if (emcStatusBuffer != 0) {
	delete emcStatusBuffer;
	emcStatusBuffer = 0;
	emcStatus = 0;
    }

    if (emcCommandBuffer != 0) {
	delete emcCommandBuffer;
	emcCommandBuffer = 0;
    }
    //exit(0);
}

/*
static void sigQuit(int sig)
{
    thisQuit();
}*/

static void initGV()
{
    emcWaitType = EMC_WAIT_RECEIVED;
    emcCommandSerialNumber = 0;
    saveEmcCommandSerialNumber = 0;
    emcTimeout = 1.0;
    emcUpdateType = EMC_UPDATE_NONE;
    linearUnitConversion = LINEAR_UNITS_AUTO;
    angularUnitConversion = ANGULAR_UNITS_AUTO;
    emcCommandBuffer = 0;
    emcStatusBuffer = 0;
    emcStatus = 0;
    emcErrorBuffer = 0;
    error_string[LINELEN-1] = 0;
    operator_text_string[LINELEN-1] = 0;
    operator_display_string[LINELEN-1] = 0;
    programStartLine = 0;
}

int emc_init( int argc,  char **argv, int channel)
{	 
    initGV();
    // process command line args
    if (0 != emcGetArgs(argc, (char**)argv)) {
        printf( "error in argument list\n");
        return -1;
    }
    // get configuration information
    iniLoad(EMC_INIFILE);

    // init NML
    if (0 != tryNml(channel)) {
        printf( "can't connect to emc\n");
        thisQuit();
        return -1;
    }
    // get current serial number, and save it for restoring when we quit
    // so as not to interfere with real operator interface
    updateStatus();
    emcCommandSerialNumber = emcStatus->echo_serial_number;
    saveEmcCommandSerialNumber = emcStatus->echo_serial_number;
    // attach our quit function to SIGINT
    //signal(SIGINT, sigQuit);
    return 0;
}


void emc_quit()
{
	thisQuit();
}

