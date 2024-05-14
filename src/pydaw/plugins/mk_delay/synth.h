/*
This file is part of the MusiKernel project, Copyright MusiKernel Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef MKDELAY_SYNTH_H
#define	MKDELAY_SYNTH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../include/pydaw_plugin.h"
#include "libmodsynth.h"

#define MKDELAY_FIRST_CONTROL_PORT 0
#define MKDELAY_DELAY_TIME  0
#define MKDELAY_FEEDBACK  1
#define MKDELAY_DRY  2
#define MKDELAY_WET  3
#define MKDELAY_DUCK  4
#define MKDELAY_CUTOFF 5
#define MKDELAY_STEREO 6

#define MKDELAY_LAST_CONTROL_PORT 6
/* must be 1 + highest value above
 * CHANGE THIS IF YOU ADD OR TAKE AWAY ANYTHING*/
#define MKDELAY_COUNT 7

typedef struct
{
    PYFX_Data *output0;
    PYFX_Data *output1;

    PYFX_Data *delay_time;
    PYFX_Data *feedback;
    PYFX_Data *dry;
    PYFX_Data *wet;
    PYFX_Data *duck;
    PYFX_Data *cutoff;
    PYFX_Data *stereo;

    float fs;
    t_mkdelay_mono_modules * mono_modules;

    int midi_event_types[200];
    int midi_event_ticks[200];
    float midi_event_values[200];
    int midi_event_ports[200];
    int midi_event_count;
    t_plugin_event_queue atm_queue;
    int plugin_uid;
    fp_queue_message queue_func;

    float * port_table;
    t_plugin_cc_map cc_map;
    PYFX_Descriptor * descriptor;
} t_mkdelay;

#ifdef	__cplusplus
}
#endif

#endif	/* MKDELAY_SYNTH_H */

