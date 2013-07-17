

#ifndef CVM_EMCSEC_HH
#define CVM_EMCSEC_HH

#include "emc.hh"
#include "emcsecnml.hh"

typedef enum
{
    LIMITOK=0,
    HARDMIN,
    HARDMAX,
    SOFTMIN,
    SOFTMAX
}emc_limit_t;




int emc_init( int argc,  char **argv, int channel=0);
void emc_quit();

int emc_ini(char* value, const char* secstr, const char* varstr);
int emc_debug(int *debugS, int debugV);
int emc_set_wait(enum EMC_WAIT_TYPE* waitS, enum EMC_WAIT_TYPE waitC);
int emc_wait(const char* objstr);
int emc_set_timeout(double* timeoutS, double timeout);
int emc_update(const char* objstr);
int emc_time(double* time);
int emc_error(char* error_str, int update=1);
int emc_operator_text(char* str, int update=1);
int emc_operator_display(char* str, int update=1);
int emc_abs_cmd_pos(double* pos, int axis);
int emc_abs_act_pos(double* pos, int axis);
int emc_rel_cmd_pos(double* pos, int axis);
int emc_rel_act_pos(double* pos, int axis);
int emc_joint_pos(int axis, double *pos);
int emc_pos_offset(int axis, double* pos_offset);
int emc_joint_homed(int joint, int *homed);
int emc_joint_fault(int joint, int* fault);
int emc_override_limit(int *orls, int on);
int emc_joint_limit(int joint, emc_limit_t* limit);
int emc_mdi(const char* string);
int emc_program_status(enum EMC_TASK_INTERP_ENUM *stat);
int emc_program_line(int* programActiveLine);
int emc_task_plan_init();
int emc_optional_stop(int *stat, int on);
int emc_step();
int emc_program(char* file);
int emc_feed_override(int *fos, int percent);
int emc_jog_stop(int axis);
int emc_jog(int axis, double speed);
int emc_jog_incr(int axis, double speed, double incr);
int emc_jog_abs(int axis, double speed, double pos);
int emc_home(int axis);
int emc_unhome(int axis);
int emc_mode(enum EMC_TASK_MODE_ENUM *stat, enum EMC_TASK_MODE_ENUM cmd);
int emc_open(const char* file);
int emc_run(int line);
int emc_pause();
int emc_resume();
int emc_abort();
int emc_flood(int* stat, int cmd);
int emc_spindle(int* stat, int cmd);
int emc_spindle_override(int *fos, int percent);
int emc_estop(int* stat, int cmd);
int emc_machine(int* stat, int cmd);
int emc_linear_unit_conversion(enum LINEAR_UNIT_CONVERSION *stat, 
                               enum LINEAR_UNIT_CONVERSION unit);
int emc_axis_backlash(int axis, double *bls, double backlash);
int emc_axis_enable(int axis, int * stat, int val);
int emc_axis_load_comp(int axis, char* file, int type);

#endif
