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

#include "../../include/pydaw_plugin.h"
#include "synth.h"
#include "../../libmodsynth/lib/lms_math.h"

static fp_get_wavpool_item_from_host wavpool_get_func;

static inline void v_euphoria_slow_index(t_euphoria*);

static void cleanupSampler(PYFX_Handle instance)
{
    t_euphoria *plugin = (t_euphoria *)instance;
    free(plugin);
}

static void v_euphoria_set_cc_map(PYFX_Handle instance, char * a_msg)
{
    t_euphoria *plugin = (t_euphoria *)instance;
    v_generic_cc_map_set(&plugin->cc_map, a_msg);
}

static void euphoriaPanic(PYFX_Handle instance)
{
    t_euphoria *plugin = (t_euphoria *)instance;
    int f_i = 0;
    while(f_i < EUPHORIA_POLYPHONY)
    {
        v_adsr_kill(&plugin->data[f_i]->adsr_amp);
        ++f_i;
    }
}

static void v_euphoria_on_stop(PYFX_Handle instance)
{
    t_euphoria *plugin = (t_euphoria *)instance;
    int f_i = 0;
    while(f_i < EUPHORIA_POLYPHONY)
    {
        v_euphoria_poly_note_off(plugin->data[f_i], 0);
        ++f_i;
    }
    plugin->sv_pitch_bend_value = 0.0f;
}

static void euphoriaConnectBuffer(PYFX_Handle instance, int a_index,
        float * DataLocation, int a_is_sidechain)
{
    if(a_is_sidechain)
    {
        return;
    }

    t_euphoria *plugin = (t_euphoria *) instance;

    switch(a_index)
    {
        case 0:
            plugin->output[0] = DataLocation;
            break;
        case 1:
            plugin->output[1] = DataLocation;
            break;
        default:
            assert(0);
            break;
    }
}

static void connectPortSampler(PYFX_Handle instance, int port,
			       PYFX_Data * data)
{
    t_euphoria *plugin = (t_euphoria *) instance;

    if(port < EUPHORIA_LAST_REGULAR_CONTROL_PORT)
    {
        switch (port)
        {
            case EUPHORIA_ATTACK:
                plugin->attack = data;
                break;
            case EUPHORIA_DECAY:
                plugin->decay = data;
                break;
            case EUPHORIA_SUSTAIN:
                plugin->sustain = data;
                break;
            case EUPHORIA_RELEASE:
                plugin->release = data;
                break;
            case EUPHORIA_FILTER_ATTACK:
                plugin->attack_f = data;
                break;
            case EUPHORIA_FILTER_DECAY:
                plugin->decay_f = data;
                break;
            case EUPHORIA_FILTER_SUSTAIN:
                plugin->sustain_f = data;
                break;
            case EUPHORIA_FILTER_RELEASE:
                plugin->release_f = data;
                break;
            case EUPHORIA_MASTER_VOLUME:
                plugin->master_vol = data;
                break;
            case EUPHORIA_MASTER_GLIDE:
                plugin->master_glide = data;
                break;
            case EUPHORIA_MASTER_PITCHBEND_AMT:
                plugin->master_pb_amt = data;
                break;
            case EUPHORIA_PITCH_ENV_TIME:
                plugin->pitch_env_time = data;
                break;
            case EUPHORIA_LFO_FREQ:
                plugin->lfo_freq = data;
                break;
            case EUPHORIA_LFO_TYPE:
                plugin->lfo_type = data;
                break;

            case EUPHORIA_FX0_KNOB0: plugin->pfx_mod_knob[0][0] = data; break;
            case EUPHORIA_FX0_KNOB1: plugin->pfx_mod_knob[0][1] = data; break;
            case EUPHORIA_FX0_KNOB2: plugin->pfx_mod_knob[0][2] = data; break;
            case EUPHORIA_FX1_KNOB0: plugin->pfx_mod_knob[1][0] = data; break;
            case EUPHORIA_FX1_KNOB1: plugin->pfx_mod_knob[1][1] = data; break;
            case EUPHORIA_FX1_KNOB2: plugin->pfx_mod_knob[1][2] = data; break;
            case EUPHORIA_FX2_KNOB0: plugin->pfx_mod_knob[2][0] = data; break;
            case EUPHORIA_FX2_KNOB1: plugin->pfx_mod_knob[2][1] = data; break;
            case EUPHORIA_FX2_KNOB2: plugin->pfx_mod_knob[2][2] = data; break;
            case EUPHORIA_FX3_KNOB0: plugin->pfx_mod_knob[3][0] = data; break;
            case EUPHORIA_FX3_KNOB1: plugin->pfx_mod_knob[3][1] = data; break;
            case EUPHORIA_FX3_KNOB2: plugin->pfx_mod_knob[3][2] = data; break;

            case EUPHORIA_FX0_COMBOBOX: plugin->fx_combobox[0] = data; break;
            case EUPHORIA_FX1_COMBOBOX: plugin->fx_combobox[1] = data; break;
            case EUPHORIA_FX2_COMBOBOX: plugin->fx_combobox[2] = data; break;
            case EUPHORIA_FX3_COMBOBOX: plugin->fx_combobox[3] = data; break;
            //End from Modulex
            /*PolyFX mod matrix port connections*/
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL0: plugin->polyfx_mod_matrix[0][0][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL1: plugin->polyfx_mod_matrix[0][0][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL2: plugin->polyfx_mod_matrix[0][0][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL0: plugin->polyfx_mod_matrix[0][1][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL1: plugin->polyfx_mod_matrix[0][1][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC1CTRL2: plugin->polyfx_mod_matrix[0][1][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL0: plugin->polyfx_mod_matrix[0][2][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL1: plugin->polyfx_mod_matrix[0][2][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC2CTRL2: plugin->polyfx_mod_matrix[0][2][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL0: plugin->polyfx_mod_matrix[0][3][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL1: plugin->polyfx_mod_matrix[0][3][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC3CTRL2: plugin->polyfx_mod_matrix[0][3][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL0: plugin->polyfx_mod_matrix[1][0][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL1: plugin->polyfx_mod_matrix[1][0][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC0CTRL2: plugin->polyfx_mod_matrix[1][0][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL0: plugin->polyfx_mod_matrix[1][1][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL1: plugin->polyfx_mod_matrix[1][1][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC1CTRL2: plugin->polyfx_mod_matrix[1][1][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL0: plugin->polyfx_mod_matrix[1][2][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL1: plugin->polyfx_mod_matrix[1][2][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC2CTRL2: plugin->polyfx_mod_matrix[1][2][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL0: plugin->polyfx_mod_matrix[1][3][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL1: plugin->polyfx_mod_matrix[1][3][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC3CTRL2: plugin->polyfx_mod_matrix[1][3][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL0: plugin->polyfx_mod_matrix[2][0][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL1: plugin->polyfx_mod_matrix[2][0][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC0CTRL2: plugin->polyfx_mod_matrix[2][0][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL0: plugin->polyfx_mod_matrix[2][1][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL1: plugin->polyfx_mod_matrix[2][1][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC1CTRL2: plugin->polyfx_mod_matrix[2][1][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL0: plugin->polyfx_mod_matrix[2][2][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL1: plugin->polyfx_mod_matrix[2][2][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC2CTRL2: plugin->polyfx_mod_matrix[2][2][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL0: plugin->polyfx_mod_matrix[2][3][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL1: plugin->polyfx_mod_matrix[2][3][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC3CTRL2: plugin->polyfx_mod_matrix[2][3][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL0: plugin->polyfx_mod_matrix[3][0][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL1: plugin->polyfx_mod_matrix[3][0][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC0CTRL2: plugin->polyfx_mod_matrix[3][0][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL0: plugin->polyfx_mod_matrix[3][1][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL1: plugin->polyfx_mod_matrix[3][1][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC1CTRL2: plugin->polyfx_mod_matrix[3][1][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL0: plugin->polyfx_mod_matrix[3][2][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL1: plugin->polyfx_mod_matrix[3][2][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC2CTRL2: plugin->polyfx_mod_matrix[3][2][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL0: plugin->polyfx_mod_matrix[3][3][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL1: plugin->polyfx_mod_matrix[3][3][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC3CTRL2: plugin->polyfx_mod_matrix[3][3][2] = data; break;

            //keyboard tracking
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC4CTRL0: plugin->polyfx_mod_matrix[0][4][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC4CTRL1: plugin->polyfx_mod_matrix[0][4][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC4CTRL2: plugin->polyfx_mod_matrix[0][4][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC4CTRL0: plugin->polyfx_mod_matrix[1][4][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC4CTRL1: plugin->polyfx_mod_matrix[1][4][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC4CTRL2: plugin->polyfx_mod_matrix[1][4][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC4CTRL0: plugin->polyfx_mod_matrix[2][4][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC4CTRL1: plugin->polyfx_mod_matrix[2][4][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC4CTRL2: plugin->polyfx_mod_matrix[2][4][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC4CTRL0: plugin->polyfx_mod_matrix[3][4][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC4CTRL1: plugin->polyfx_mod_matrix[3][4][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC4CTRL2: plugin->polyfx_mod_matrix[3][4][2] = data; break;

            //velocity tracking
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC5CTRL0: plugin->polyfx_mod_matrix[0][5][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC5CTRL1: plugin->polyfx_mod_matrix[0][5][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST0SRC5CTRL2: plugin->polyfx_mod_matrix[0][5][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC5CTRL0: plugin->polyfx_mod_matrix[1][5][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC5CTRL1: plugin->polyfx_mod_matrix[1][5][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST1SRC5CTRL2: plugin->polyfx_mod_matrix[1][5][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC5CTRL0: plugin->polyfx_mod_matrix[2][5][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC5CTRL1: plugin->polyfx_mod_matrix[2][5][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST2SRC5CTRL2: plugin->polyfx_mod_matrix[2][5][2] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC5CTRL0: plugin->polyfx_mod_matrix[3][5][0] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC5CTRL1: plugin->polyfx_mod_matrix[3][5][1] = data; break;
            case EUPHORIA_PFXMATRIX_GRP0DST3SRC5CTRL2: plugin->polyfx_mod_matrix[3][5][2] = data; break;

            //End PolyFX mod matrix
            case EUPHORIA_LFO_PITCH: plugin->lfo_pitch = data; break;
            default:
                break;
        }
    }
    else if((port >= EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MAX))
    {
        plugin->samples[(port - EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MIN)].basePitch = data;
    }
    else if((port >= EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MIN) &&
            (port < EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MAX))
    {
        plugin->samples[(port - EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MIN)].low_note = data;
    }
    else if((port >= EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MIN) &&
            (port < EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MAX))
    {
        plugin->samples[(port - EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MIN)].high_note = data;
    }
    else if((port >= LMS_SAMPLE_VOLUME_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_VOLUME_PORT_RANGE_MAX))
    {
        plugin->samples[(port - LMS_SAMPLE_VOLUME_PORT_RANGE_MIN)].sample_vol = data;
    }
    else if((port >= EUPHORIA_SAMPLE_START_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_START_PORT_RANGE_MAX))
    {
        plugin->samples[(port - EUPHORIA_SAMPLE_START_PORT_RANGE_MIN)].sampleStarts = data;
    }
    else if((port >= EUPHORIA_SAMPLE_END_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_END_PORT_RANGE_MAX))
    {
        plugin->samples[(port - EUPHORIA_SAMPLE_END_PORT_RANGE_MIN)].sampleEnds = data;
    }
    else if((port >= EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MAX))
    {
        plugin->samples[(port - EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MIN)].sample_vel_sens = data;
    }
    else if((port >= EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MAX))
    {
        plugin->samples[(port - EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MIN)].sample_vel_low = data;
    }
    else if((port >= EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MAX))
    {
        plugin->samples[(port - EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MIN)].sample_vel_high = data;
    }
    else if((port >= EUPHORIA_PITCH_PORT_RANGE_MIN) &&
            (port < EUPHORIA_PITCH_PORT_RANGE_MAX))
    {
        plugin->samples[(port - EUPHORIA_PITCH_PORT_RANGE_MIN)].sample_pitch = data;
    }
    else if((port >= EUPHORIA_TUNE_PORT_RANGE_MIN) &&
            (port < EUPHORIA_TUNE_PORT_RANGE_MAX))
    {
        plugin->samples[(port - EUPHORIA_TUNE_PORT_RANGE_MIN)].sample_tune = data;
    }
    else if((port >= EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MAX))
    {
        plugin->samples[(port - EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MIN)].sample_interpolation_mode = data;
    }

    else if((port >= EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MAX))
    {
        plugin->samples[(port - EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MIN)].sampleLoopStarts = data;
    }
    else if((port >= EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MAX))
    {
        plugin->samples[(port - EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MIN)].sampleLoopEnds = data;
    }
    else if((port >= EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MAX))
    {
        plugin->samples[(port - EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MIN)].sampleLoopModes = data;
    }
    else if((port >= EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MIN)][0][0] = data;
    }
    else if((port >= EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MIN)][0][1] = data;
    }
    else if((port >= EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MIN)][0][2] = data;
    }
    else if((port >= EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MAX))
    {
        plugin->mfx_comboboxes[(port - EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MIN)][0] = data;
    }
    //MonoFX1
    else if((port >= EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MIN)][1][0] = data;
    }
    else if((port >= EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MIN)][1][1] = data;
    }
    else if((port >= EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MIN)][1][2] = data;
    }
    else if((port >= EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MAX))
    {
        plugin->mfx_comboboxes[(port - EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MIN)][1] = data;
    }
    //MonoFX2
    else if((port >= EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MIN)][2][0] = data;
    }
    else if((port >= EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MIN)][2][1] = data;
    }
    else if((port >= EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MIN)][2][2] = data;
    }
    else if((port >= EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MAX))
    {
        plugin->mfx_comboboxes[(port - EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MIN)][2] = data;
    }
    //MonoFX3
    else if((port >= EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MIN)][3][0] = data;
    }
    else if((port >= EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MIN)][3][1] = data;
    }
    else if((port >= EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MAX))
    {
        plugin->mfx_knobs[(port - EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MIN)][3][2] = data;
    }
    else if((port >= EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MIN) &&
            (port < EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MAX))
    {
        plugin->mfx_comboboxes[(port - EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MIN)][3] = data;
    }

    else if((port >= EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MIN) &&
            (port < EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MAX))
    {
        plugin->sample_mfx_groups[(port - EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MIN)] = data;
    }
    else if(port >= EUPHORIA_NOISE_AMP_MIN && port < EUPHORIA_NOISE_AMP_MAX)
    {
        plugin->samples[(port - EUPHORIA_NOISE_AMP_MIN)].noise_amp = data;
    }
    else if(port >= EUPHORIA_NOISE_TYPE_MIN && port < EUPHORIA_NOISE_TYPE_MAX)
    {
        plugin->samples[(port - EUPHORIA_NOISE_TYPE_MIN)].noise_type = data;
    }
    else if(port >= EUPHORIA_SAMPLE_FADE_IN_MIN &&
            port < EUPHORIA_SAMPLE_FADE_IN_MAX)
    {
        plugin->samples[(port - EUPHORIA_SAMPLE_FADE_IN_MIN)].sampleFadeInEnds = data;
    }
    else if(port >= EUPHORIA_SAMPLE_FADE_OUT_MIN &&
            port < EUPHORIA_SAMPLE_FADE_OUT_MAX)
    {
        plugin->samples[(port - EUPHORIA_SAMPLE_FADE_OUT_MIN)].sampleFadeOutStarts = data;
    }
    else if(port >= EUPHORIA_FIRST_EQ_PORT &&
            port < EUPHORIA_LAST_EQ_PORT)
    {
        int f_port = port - EUPHORIA_FIRST_EQ_PORT;
        int f_instance = f_port / 18;
        int f_diff = f_port % 18;
        v_eq6_connect_port(
            &plugin->mono_modules->mfx[f_instance].eqs, f_diff, data);
    }
    else if(port == EUPHORIA_LFO_PITCH_FINE)
    {
        plugin->lfo_pitch_fine = data;
    }
    else if(port == EUPHORIA_MIN_NOTE)
    {
        plugin->min_note = data;
    }
    else if(port == EUPHORIA_MAX_NOTE)
    {
        plugin->max_note = data;
    }
    else if(port == EUPHORIA_MASTER_PITCH)
    {
        plugin->master_pitch = data;
    }
    else if(port == EUPHORIA_ADSR_LIN_MAIN)
    {
        plugin->adsr_lin_main = data;
    }

}

static void v_euphoria_load(PYFX_Handle instance,
        PYFX_Descriptor * Descriptor, char * a_file_path)
{
    t_euphoria *plugin_data = (t_euphoria*)instance;
    pydaw_generic_file_loader(instance, Descriptor,
        a_file_path, plugin_data->port_table, &plugin_data->cc_map);
}

static void v_euphoria_set_port_value(PYFX_Handle Instance,
        int a_port, float a_value)
{
    t_euphoria *plugin_data = (t_euphoria*)Instance;
    plugin_data->port_table[a_port] = a_value;
}

static PYFX_Handle instantiateSampler(PYFX_Descriptor * descriptor,
        int s_rate,
        fp_get_wavpool_item_from_host a_host_wavpool_func,
        int a_plugin_uid, fp_queue_message a_queue_func)
{
    wavpool_get_func = a_host_wavpool_func;
    t_euphoria *plugin_data;

    hpalloc((void**)&plugin_data, sizeof(t_euphoria));

    plugin_data->descriptor = descriptor;
    plugin_data->voices = g_voc_get_voices(EUPHORIA_POLYPHONY,
            EUPHORIA_POLYPHONY_THRESH);

    plugin_data->plugin_uid = a_plugin_uid;

    plugin_data->current_sample = 0;
    plugin_data->loaded_samples_count = 0;
    plugin_data->amp = 1.0f;
    plugin_data->i_slow_index = EUPHORIA_SLOW_INDEX_COUNT;

    plugin_data->smp_pit_ratio = g_pit_ratio();

    t_euphoria_sample * f_sample;

    register int f_i = 0;
    while(f_i < EUPHORIA_MAX_SAMPLE_COUNT)
    {
        f_sample = &plugin_data->samples[f_i];
        f_sample->sampleStarts = NULL;
        f_sample->sampleEnds = NULL;
        f_sample->basePitch = NULL;
        f_sample->low_note = NULL;
        f_sample->high_note = NULL;
        f_sample->sample_vol = NULL;
        f_sample->sample_amp = 1.0f;
        f_sample->sampleEndPos = 0.0f;
        f_sample->sample_last_interpolated_value = 0.0f;
        f_sample->adjusted_base_pitch = 60.0f;
        f_sample->wavpool_items = NULL;
        f_sample->noise_func_ptr = f_run_noise_off;
        f_sample->noise_linamp = 1.0f;
        f_sample->noise_type = NULL;
        f_sample->noise_amp = NULL;

        ++f_i;
    }

    f_i = 0;

    while(f_i < EUPHORIA_POLYPHONY)
    {
        plugin_data->data[f_i] = g_euphoria_poly_init(s_rate);
        ++f_i;
    }

    plugin_data->sv_pitch_bend_value = 0.0f;
    plugin_data->sv_last_note = -1.0f;
    plugin_data->mono_modules = g_euphoria_mono_init(s_rate);
    plugin_data->sampleNo = 0;

    plugin_data->port_table = g_pydaw_get_port_table(
            (void**)plugin_data, descriptor);

    v_cc_map_init(&plugin_data->cc_map);

    return (PYFX_Handle) plugin_data;
}


static inline int check_sample_bounds(t_euphoria * plugin_data, int n)
{
    t_euphoria_sample * f_sample =
        &plugin_data->samples[plugin_data->current_sample];
    t_int_frac_read_head * f_read_head =
        &plugin_data->data[n]->samples[
            (plugin_data->current_sample)].sample_read_heads;

    if (f_read_head->whole_number >= f_sample->sampleEndPos)
    {
        if(((int)(*(f_sample->sampleLoopModes))) > 0)
        {
            //TODO:  write a special function that either maintains
            //the fraction, or else wraps the negative interpolation back
            // to where it was before the loop happened, to avoid clicks
            v_ifh_retrigger(f_read_head,
                (EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2 +
                (f_sample->sampleLoopStartPos)));// 0.0f;

            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 0;
    }
}

static int calculate_ratio_sinc(t_euphoria *__restrict plugin_data, int n)
{
    t_euphoria_sample * f_sample =
        &plugin_data->samples[plugin_data->current_sample];
    t_int_frac_read_head * f_read_head =
        &plugin_data->data[n]->samples[
            (plugin_data->current_sample)].sample_read_heads;
    plugin_data->ratio =
        f_pit_midi_note_to_ratio_fast(f_sample->adjusted_base_pitch,
            plugin_data->data[n]->base_pitch, plugin_data->smp_pit_ratio) *
            f_sample->wavpool_items->ratio_orig;

    v_ifh_run(f_read_head, plugin_data->ratio);

    return check_sample_bounds(plugin_data, n);
}

static int calculate_ratio_linear(t_euphoria *__restrict plugin_data, int n)
{
    return calculate_ratio_sinc(plugin_data, n);
}

static int calculate_ratio_none(t_euphoria *__restrict plugin_data, int n)
{
    ++plugin_data->data[n]->samples[
        (plugin_data->current_sample)].sample_read_heads.whole_number;
    return check_sample_bounds(plugin_data, n);
}

static void run_sampler_interpolation_sinc(
        t_euphoria *__restrict plugin_data, int n, int ch)
{
    t_euphoria_sample * f_sample =
        &plugin_data->samples[plugin_data->current_sample];
    t_int_frac_read_head * f_read_head =
        &plugin_data->data[n]->samples[
            (plugin_data->current_sample)].sample_read_heads;

    f_sample->sample_last_interpolated_value =
        f_sinc_interpolate2(&plugin_data->mono_modules->sinc_interpolator,
        f_sample->wavpool_items->samples[ch],
        f_read_head->whole_number, f_read_head->fraction);
}


static void run_sampler_interpolation_linear(
        t_euphoria *__restrict plugin_data, int n, int ch)
{
    t_euphoria_sample * f_sample =
        &plugin_data->samples[plugin_data->current_sample];
    t_int_frac_read_head * f_read_head =
        &plugin_data->data[n]->samples[
            (plugin_data->current_sample)].sample_read_heads;

    f_sample->sample_last_interpolated_value =
        f_cubic_interpolate_ptr_ifh(
            f_sample->wavpool_items->samples[ch],
            f_read_head->whole_number, f_read_head->fraction);
}


static void run_sampler_interpolation_none(
        t_euphoria *__restrict plugin_data, int n, int ch)
{
    t_euphoria_sample * f_sample =
        &plugin_data->samples[plugin_data->current_sample];
    t_int_frac_read_head * f_read_head =
        &plugin_data->data[n]->samples[
            (plugin_data->current_sample)].sample_read_heads;

    f_sample->sample_last_interpolated_value =
        f_sample->wavpool_items-> samples[ch][(f_read_head->whole_number)];
}

/* void add_sample_lms_euphoria(t_euphoria *__restrict plugin_data,
 *                                                      int n) //voice number
*/
static void add_sample_lms_euphoria(t_euphoria * plugin_data, int n)
{
    t_voc_single_voice * f_poly_voice = &plugin_data->voices->voices[n];

    if((f_poly_voice->on) > (plugin_data->sampleNo))
    {
        return;
    }

    int ch;
    t_euphoria_sample * f_sample = NULL;
    t_int_frac_read_head * f_read_head = NULL;
    t_euphoria_pfx_sample * f_pfx_sample = NULL;
    t_euphoria_poly_voice * f_voice = plugin_data->data[n];

    //Run things that aren't per-channel like envelopes

    f_voice->adsr_run_func(&f_voice->adsr_amp);

    if(f_voice->adsr_amp.stage == ADSR_STAGE_OFF)
    {
        f_poly_voice->n_state = note_state_off;
        return;
    }

    v_adsr_run(&f_voice->adsr_filter);

    //Run the glide module
    f_rmp_run_ramp(&f_voice->ramp_env);
    f_rmp_run_ramp(&f_voice->glide_env);

    //Set and run the LFO
    v_lfs_set(&f_voice->lfo1,  (*(plugin_data->lfo_freq)) * .01);
    v_lfs_run(&f_voice->lfo1);

    f_voice->base_pitch = (f_voice->glide_env.output_multiplied)
            +  (plugin_data->mono_modules->pitchbend_smoother.last_value *
            (*(plugin_data->master_pb_amt)))
            + (f_voice->last_pitch) + ((f_voice->lfo1.output) *
            (*plugin_data->lfo_pitch + (*plugin_data->lfo_pitch_fine * 0.01f)));

    if((f_poly_voice->off == plugin_data->sampleNo) &&
        (f_voice->adsr_amp.stage < ADSR_STAGE_RELEASE))
    {
        if(f_poly_voice->n_state == note_state_killed)
        {
            v_euphoria_poly_note_off(f_voice, 1);
        }
        else
        {
            v_euphoria_poly_note_off(f_voice, 0);
        }
    }

    plugin_data->sample[0] = 0.0f;
    plugin_data->sample[1] = 0.0f;
    f_voice->modulex_current_sample[0] = 0.0f;
    f_voice->modulex_current_sample[1] = 0.0f;

    int i_ls;

    //Calculating and summing all of the interpolated samples for this note
    for(i_ls = 0; i_ls < f_voice->sample_indexes_count; ++i_ls)
    {
        plugin_data->current_sample = f_voice->sample_indexes[i_ls];
        f_pfx_sample = &f_voice->samples[plugin_data->current_sample];
        f_sample = &plugin_data->samples[plugin_data->current_sample];

        if(f_sample->ratio_function_ptr(plugin_data, n) == 1)
        {
            continue;
        }

        float f_fade_vol = 1.0f;
        f_read_head = &f_pfx_sample->sample_read_heads;

        if(f_read_head->whole_number < f_pfx_sample->sample_fade_in_end_sample)
        {
            float f_fade_in_inc = f_pfx_sample->sample_fade_in_inc;
            float f_start_pos = f_sample->sampleStartPos;
            float f_read_head_pos = (float)(f_read_head->whole_number);

            f_fade_vol =  (f_read_head_pos - f_start_pos) * f_fade_in_inc;
            f_fade_vol = (f_fade_vol * 18.0f) - 18.0f;
            f_fade_vol = f_db_to_linear_fast(f_fade_vol);
        }
        else if(f_read_head->whole_number >
                f_pfx_sample->sample_fade_out_start_sample)
        {
            float f_sample_end_pos = f_sample->sampleEndPos;
            float f_read_head_pos = (float)(f_read_head->whole_number);
            float f_fade_out_dec = f_pfx_sample->sample_fade_out_dec;

            f_fade_vol = (f_sample_end_pos - f_read_head_pos) * f_fade_out_dec;
            f_fade_vol = (f_fade_vol * 18.0f) - 18.0f;
            f_fade_vol = f_db_to_linear_fast(f_fade_vol);
        }

        f_voice->noise_sample =
            f_sample->noise_func_ptr(
            &plugin_data->mono_modules->white_noise1[(f_voice->noise_index)])
            * f_sample->noise_linamp;

        for(ch = 0; ch < f_sample->wavpool_items->channels; ++ch)
        {
            f_sample->interpolation_mode(plugin_data, n, ch);

            plugin_data->sample[ch] +=
                f_sample->sample_last_interpolated_value * f_fade_vol;

            plugin_data->sample[ch] += (f_voice->noise_sample);

            plugin_data->sample[ch] =
                plugin_data->sample[ch] * f_sample->sample_amp *
                f_pfx_sample->vel_sens_output;

            f_voice->modulex_current_sample[ch] += (plugin_data->sample[ch]);

            if((f_sample->wavpool_items->channels) == 1)
            {
                f_voice->modulex_current_sample[1] += plugin_data->sample[0];
                break;
            }
        }
    }

    t_euphoria_pfx_group * f_pfx_group;
    int f_dst;
    //Modular PolyFX, processed from the index created during note_on
    register int i_dst = 0;
    while(i_dst < f_voice->active_polyfx_count)
    {
        f_dst = f_voice->active_polyfx[(i_dst)];
        f_pfx_group = &f_voice->effects[f_dst];

        v_mf3_set(&f_pfx_group->multieffect,
            *(plugin_data->pfx_mod_knob[f_dst][0]),
            *(plugin_data->pfx_mod_knob[f_dst][1]),
            *(plugin_data->pfx_mod_knob[f_dst][2]));

        int f_mod_test = 0;

        while(f_mod_test < f_pfx_group->polyfx_mod_counts)
        {
            v_mf3_mod_single(&f_pfx_group->multieffect,
                *(f_voice->modulator_outputs[
                    (f_voice->polyfx_mod_src_index[f_dst][f_mod_test])]),
                (f_voice->polyfx_mod_matrix_values[f_dst][f_mod_test]),
                (f_voice->polyfx_mod_ctrl_indexes[f_dst][f_mod_test])
                );
            ++f_mod_test;
        }

        f_pfx_group->fx_func_ptr(&f_pfx_group->multieffect,
            (f_voice->modulex_current_sample[0]),
            (f_voice->modulex_current_sample[1]));

        f_voice->modulex_current_sample[0] = f_pfx_group->multieffect.output0;
        f_voice->modulex_current_sample[1] = f_pfx_group->multieffect.output1;
        ++i_dst;
    }

    plugin_data->mono_fx_buffers[
        (plugin_data->sample_mfx_groups_index[(plugin_data->current_sample)])][0] +=
            (f_voice->modulex_current_sample[0]) * (f_voice->adsr_amp.output) *
            (plugin_data->amp);
    plugin_data->mono_fx_buffers[
        plugin_data->sample_mfx_groups_index[(plugin_data->current_sample)]][1]
            +=
            (f_voice->modulex_current_sample[1]) * (f_voice->adsr_amp.output) *
            (plugin_data->amp);
}

static inline void v_euphoria_slow_index(t_euphoria* plugin_data)
{
    t_euphoria_sample * f_sample = NULL;
    plugin_data->i_slow_index = 0;
    plugin_data->monofx_channel_index_count = 0;

    register int i = 0;
    register int i3;

    while(i < EUPHORIA_MONO_FX_GROUPS_COUNT)
    {
        plugin_data->monofx_channel_index_tracker[i] = 0;
        ++i;
    }

    i = 0;
    while(i < (plugin_data->loaded_samples_count))
    {
        f_sample = &plugin_data->samples[plugin_data->loaded_samples[i]];

        int f_mono_fx_group =
            (int)(*plugin_data->sample_mfx_groups[
                plugin_data->loaded_samples[i]]);

        if((plugin_data->monofx_channel_index_tracker[f_mono_fx_group]) == 0)
        {
            plugin_data->monofx_channel_index_tracker[f_mono_fx_group] = 1;
            plugin_data->monofx_channel_index[
                plugin_data->monofx_channel_index_count] = f_mono_fx_group;
            ++plugin_data->monofx_channel_index_count;

            i3 = 0;
            while(i3 < EUPHORIA_MONO_FX_COUNT)
            {
                plugin_data->mono_modules->mfx[f_mono_fx_group].fx_func_ptr[i3]
                    = g_mf3_get_function_pointer(
                    (int)(*plugin_data->mfx_comboboxes[f_mono_fx_group][i3]));
                ++i3;
            }
        }

        int f_index = (int)(*f_sample->noise_type);
        //Get the noise function pointer
        f_sample->noise_func_ptr = fp_get_noise_func_ptr(f_index);

        if(f_index > 0)
        {
            f_sample->noise_linamp = f_db_to_linear_fast(*f_sample->noise_amp);
        }
        ++i;
    }
}

static void v_euphoria_process_midi_event(
    t_euphoria * plugin_data, t_pydaw_seq_event * a_event)
{
    t_euphoria_sample * f_sample = NULL;
    t_int_frac_read_head * f_read_head = NULL;
    t_euphoria_poly_voice * f_voice = NULL;
    t_euphoria_pfx_sample * f_pfx_sample = NULL;

    int f_note = 60;
    int f_min_note = (int)*plugin_data->min_note;
    int f_max_note = (int)*plugin_data->max_note;

    if (a_event->type == PYDAW_EVENT_NOTEON)
    {
        f_note = a_event->note;

        if (a_event->velocity > 0)
        {
            if(a_event->note > f_max_note ||
                a_event->note < f_min_note)
            {
                return;
            }
            int f_voice_num = i_pick_voice(
                plugin_data->voices, f_note,
                plugin_data->sampleNo, a_event->tick);
            f_voice = plugin_data->data[f_voice_num];
            f_voice->velocities = a_event->velocity;

            int f_adsr_main_lin = (int)(*plugin_data->adsr_lin_main);
            f_voice->adsr_run_func = FP_ADSR_RUN[f_adsr_main_lin];

            f_voice->keyboard_track = ((float)(a_event->note)) * 0.007874016f;
            f_voice->velocity_track =
                ((float)(a_event->velocity)) * 0.007874016f;

            f_voice->sample_indexes_count = 0;

            //Figure out which samples to play and stash all relevant values
            register int i = 0;
            int f_smp;
            while(i  < (plugin_data->loaded_samples_count))
            {
                f_smp = plugin_data->loaded_samples[i];
                f_sample = &plugin_data->samples[f_smp];
                f_pfx_sample = &f_voice->samples[f_smp];
                f_read_head = &f_pfx_sample->sample_read_heads;

                if((f_note >= ((int)(*f_sample->low_note))) &&
                (f_note <= ((int)(*f_sample->high_note))) &&
                (f_voice->velocities <= ((int)(*f_sample->sample_vel_high))) &&
                (f_voice->velocities >= ((int)(*f_sample->sample_vel_low))))
                {
                    f_voice->sample_indexes[f_voice->sample_indexes_count] =
                        (f_smp);
                    ++f_voice->sample_indexes_count;

                    plugin_data->sample_mfx_groups_index[(f_smp)] =
                        (int)(*(plugin_data->sample_mfx_groups[(f_smp)]));

                    f_sample->sampleStartPos =
                        (EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2 +
                        ((f_sample->wavpool_items->length) *
                        ((*f_sample->sampleStarts)) * .001));

                    f_sample->sampleLoopStartPos =
                        (EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2 +
                        ((f_sample->wavpool_items->length) *
                        ((*f_sample->sampleLoopStarts) * .001)));

                    if(((int)(*f_sample->sampleLoopModes)) == 0)
                    {
                        f_sample->sampleEndPos =
                            (EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2 +
                            ((int)((float)(f_sample->wavpool_items->length
                            - 5)) * (*f_sample->sampleEnds * .001)));
                    }
                    else
                    {
                        f_sample->sampleEndPos =
                            (EUPHORIA_SINC_INTERPOLATION_POINTS_DIV2 +
                            ((int)((float)(f_sample->wavpool_items->length - 5))
                            * (*f_sample->sampleLoopEnds * .001)));
                    }

                    if((f_sample->sampleEndPos) >
                        ((float)(f_sample->wavpool_items->length)))
                    {
                        f_sample->sampleEndPos =
                            (float)(f_sample->wavpool_items->length);
                    }

                    //get the fade in values
                    f_pfx_sample->sample_fade_in_end_sample =
                            (int)((*f_sample->sampleFadeInEnds) * 0.001f *
                            (float)(f_sample->wavpool_items->length));

                    if(f_pfx_sample->sample_fade_in_end_sample <
                            (f_sample->sampleStartPos))
                    {
                        f_pfx_sample->sample_fade_in_end_sample =
                            (f_sample->sampleStartPos);
                    }
                    else if(f_pfx_sample->sample_fade_in_end_sample >
                            (f_sample->sampleEndPos))
                    {
                        f_pfx_sample->sample_fade_in_end_sample =
                            (f_sample->sampleEndPos);
                    }

                    if(f_pfx_sample->sample_fade_in_end_sample >
                            (f_sample->sampleStartPos))
                    {
                            f_pfx_sample->sample_fade_in_inc = 1.0f /
                                (f_pfx_sample->sample_fade_in_end_sample -
                                    (f_sample->sampleStartPos));
                    }
                    else
                    {
                        f_pfx_sample->sample_fade_in_inc = 1.0f;
                    }

                    //get the fade out values
                    f_pfx_sample->sample_fade_out_start_sample =
                            (int)((*f_sample->sampleFadeOutStarts) * 0.001f *
                            (float)(f_sample->wavpool_items->length));

                    if(f_pfx_sample->sample_fade_out_start_sample <
                            (f_sample->sampleStartPos))
                    {
                        f_pfx_sample->sample_fade_out_start_sample =
                            (f_sample->sampleStartPos);
                    }
                    else if(f_pfx_sample->sample_fade_out_start_sample >
                            (f_sample->sampleEndPos))
                    {
                        f_pfx_sample->sample_fade_out_start_sample =
                            (f_sample->sampleEndPos);
                    }

                    if(f_pfx_sample->sample_fade_out_start_sample <
                            (f_sample->sampleEndPos))
                    {
                            f_pfx_sample->sample_fade_out_dec = 1.0f /
                                (f_sample->sampleEndPos -
                                f_pfx_sample->sample_fade_out_start_sample);
                    }
                    else
                    {
                        f_pfx_sample->sample_fade_out_dec = 1.0f;
                    }


                    //end fade stuff

                    f_sample->adjusted_base_pitch =
                        (*f_sample->basePitch) - (*f_sample->sample_pitch) -
                        ((*f_sample->sample_tune) * .01f);

                    v_ifh_retrigger(f_read_head,
                        (f_sample->sampleStartPos)); // 0.0f;

                    f_pfx_sample->vel_sens_output =
                            (1.0f - (((float)(a_event->velocity) -
                            (*f_sample->sample_vel_low))
                            /
                            ((float)(*f_sample->sample_vel_high) -
                            (*f_sample->sample_vel_low))))
                            * (*f_sample->sample_vel_sens) * -1.0f;

                    f_pfx_sample->vel_sens_output = f_db_to_linear(
                        f_pfx_sample->vel_sens_output);

                    f_sample->sample_amp = f_db_to_linear(
                        (*f_sample->sample_vol));

                    switch((int)(*f_sample->sample_interpolation_mode))
                    {
                        case 0:
                            f_sample->interpolation_mode =
                                run_sampler_interpolation_sinc;
                            f_sample->ratio_function_ptr =
                                calculate_ratio_sinc;
                            break;
                        case 1:
                            f_sample->interpolation_mode =
                                run_sampler_interpolation_linear;
                            f_sample->ratio_function_ptr =
                                calculate_ratio_linear;
                            break;
                        case 2:
                            f_sample->interpolation_mode =
                                run_sampler_interpolation_none;
                            f_sample->ratio_function_ptr =
                                calculate_ratio_none;
                            break;
                        default:
                            printf("Error, invalid interpolation mode %i\n",
                                ((int)(*f_sample->sample_interpolation_mode)));
                    }
                }
                ++i;
            }

            f_voice->active_polyfx_count = 0;
            //Determine which PolyFX have been enabled
            register int i_dst, i_src, i_ctrl;
            i_dst = 0;
            while((i_dst) < EUPHORIA_MODULAR_POLYFX_COUNT)
            {
                int f_pfx_combobox_index =
                    (int)(*plugin_data->fx_combobox[(i_dst)]);
                f_voice->effects[i_dst].fx_func_ptr =
                        g_mf3_get_function_pointer(f_pfx_combobox_index);
                f_voice->effects[i_dst].fx_reset_ptr =
                        g_mf3_get_reset_function_pointer(f_pfx_combobox_index);

                f_voice->effects[i_dst].fx_reset_ptr(
                    &f_voice->effects[i_dst].multieffect);

                if(f_pfx_combobox_index != 0)
                {
                    f_voice->active_polyfx[(f_voice->active_polyfx_count)] =
                        (i_dst);
                    ++f_voice->active_polyfx_count;
                }
                ++i_dst;
            }

            i_dst = 0;
            int f_dst = 0;

            while((i_dst) < (f_voice->active_polyfx_count))
            {
                f_dst = f_voice->active_polyfx[i_dst];
                f_voice->effects[
                    f_voice->active_polyfx[i_dst]].polyfx_mod_counts = 0;

                i_src = 0;
                while((i_src) < EUPHORIA_MODULATOR_COUNT)
                {
                    i_ctrl = 0;
                    while((i_ctrl) < EUPHORIA_CONTROLS_PER_MOD_EFFECT)
                    {
                        if((*plugin_data->polyfx_mod_matrix[
                            f_voice->active_polyfx[i_dst]][i_src][i_ctrl]) != 0)
                        {
                            f_voice->polyfx_mod_ctrl_indexes[
                                f_dst][
                                f_voice->effects[f_dst].polyfx_mod_counts] =
                                i_ctrl;
                            f_voice->polyfx_mod_src_index[f_dst][
                                (f_voice->effects[f_dst].polyfx_mod_counts)] =
                                i_src;
                            f_voice->polyfx_mod_matrix_values[f_dst][
                                f_voice->effects[f_dst].polyfx_mod_counts] =
                                (*(plugin_data->polyfx_mod_matrix[
                                    f_dst][i_src][i_ctrl])) * .01;

                            ++f_voice->effects[f_dst].polyfx_mod_counts;
                        }
                        ++i_ctrl;
                    }
                    ++i_src;
                }
                ++i_dst;
            }

            f_voice->noise_index =
                    (plugin_data->mono_modules->noise_current_index);
            ++plugin_data->mono_modules->noise_current_index;

            if((plugin_data->mono_modules->noise_current_index) >=
                    EUPHORIA_NOISE_COUNT)
            {
                plugin_data->mono_modules->noise_current_index = 0;
            }

            plugin_data->amp = f_db_to_linear_fast(*(plugin_data->master_vol));

            f_voice->note_f =
                (float)f_note + (float)(*plugin_data->master_pitch);

            f_voice->target_pitch = f_voice->note_f;

            if(plugin_data->sv_last_note < 0.0f)
            {
                f_voice->last_pitch = (f_voice->note_f);
            }
            else
            {
                f_voice->last_pitch = (plugin_data->sv_last_note);
            }

            v_rmp_retrigger_glide_t(
                &f_voice->glide_env, (*(plugin_data->master_glide) * .01),
                (f_voice->last_pitch), (f_voice->target_pitch));

            /*Retrigger ADSR envelopes and LFO*/
            v_adsr_retrigger(&f_voice->adsr_amp);
            v_adsr_retrigger(&f_voice->adsr_filter);
            v_lfs_sync(&f_voice->lfo1, 0.0f, *(plugin_data->lfo_type));

            float f_attack_a = (*(plugin_data->attack) * .01);
            f_attack_a *= f_attack_a;
            float f_decay_a = (*(plugin_data->decay) * .01);
            f_decay_a *= f_decay_a;
            float f_release_a = (*(plugin_data->release) * .01);
            f_release_a *= f_release_a;
            FP_ADSR_SET[f_adsr_main_lin](&f_voice->adsr_amp,
                f_attack_a, f_decay_a, (*(plugin_data->sustain)),
                f_release_a);

            float f_attack_f = (*(plugin_data->attack_f) * .01);
            f_attack_f *= f_attack_f;
            float f_decay_f = (*(plugin_data->decay_f) * .01);
            f_decay_f *= f_decay_f;
            float f_release_f = (*(plugin_data->release_f) * .01);
            f_release_f *= f_release_f;
            v_adsr_set_adsr(&f_voice->adsr_filter,
                f_attack_f, f_decay_f, (*(plugin_data->sustain_f) * .01),
                f_release_f);

            /*Retrigger the pitch envelope*/
            v_rmp_retrigger(&f_voice->ramp_env,
                    (*(plugin_data->pitch_env_time) * .01), 1.0f);

            /*Set the last_note property, so the next note can
             * glide from it if glide is turned on*/
            plugin_data->sv_last_note = f_voice->note_f;
        }
        else
        {
            v_voc_note_off(plugin_data->voices, a_event->note,
                    plugin_data->sampleNo, a_event->tick);
        }
    }
    else if (a_event->type == PYDAW_EVENT_NOTEOFF )
    {
        f_note = a_event->note;
        v_voc_note_off(plugin_data->voices, a_event->note,
                plugin_data->sampleNo, a_event->tick);
    }
    else if (a_event->type == PYDAW_EVENT_CONTROLLER)
    {
        assert(a_event->param >= 1 && a_event->param < 128);

        v_plugin_event_queue_add(&plugin_data->midi_queue,
            PYDAW_EVENT_CONTROLLER, a_event->tick,
            a_event->value, a_event->param);
    }
    else if (a_event->type == PYDAW_EVENT_PITCHBEND)
    {
        v_plugin_event_queue_add(&plugin_data->midi_queue,
            PYDAW_EVENT_PITCHBEND, a_event->tick,
            a_event->value * 0.00012207f, 0);
    }
}


static void v_run_lms_euphoria(
        PYFX_Handle instance, int sample_count,
        struct ShdsList * midi_events, struct ShdsList *atm_events)
{
    t_euphoria *plugin_data = (t_euphoria*)instance;

    t_pydaw_seq_event **events = (t_pydaw_seq_event**)midi_events->data;
    int event_count = midi_events->len;

    register int f_i, i2, i3;
    int midi_event_pos = 0;
    t_plugin_event_queue_item * f_midi_item;

    ++plugin_data->i_slow_index;

    v_plugin_event_queue_reset(&plugin_data->midi_queue);

    if((plugin_data->i_slow_index) >= EUPHORIA_SLOW_INDEX_COUNT)
    {
        v_euphoria_slow_index(plugin_data);
    }

    for(f_i = 0; f_i < event_count; ++f_i)
    {
        v_euphoria_process_midi_event(plugin_data, events[f_i]);
    }

    v_plugin_event_queue_reset(&plugin_data->atm_queue);

    t_pydaw_seq_event * ev_tmp;
    for(f_i = 0; f_i < atm_events->len; ++f_i)
    {
        ev_tmp = (t_pydaw_seq_event*)atm_events->data[f_i];
        v_plugin_event_queue_add(
            &plugin_data->atm_queue, ev_tmp->type,
            ev_tmp->tick, ev_tmp->value, ev_tmp->port);
    }

    float f_temp_sample0, f_temp_sample1;

    int f_monofx_index = 0;

    for(i2 = 0; i2 < (plugin_data->monofx_channel_index_count); ++i2)
    {
        f_monofx_index = (plugin_data->monofx_channel_index[i2]);
        v_eq6_set(&plugin_data->mono_modules->mfx[f_monofx_index].eqs);
    }

    for(f_i = 0; f_i < sample_count; ++f_i)
    {
        while(1)
        {
            f_midi_item = v_plugin_event_queue_iter(
                &plugin_data->midi_queue, f_i);
            if(!f_midi_item)
            {
                break;
            }

            if(f_midi_item->type == PYDAW_EVENT_PITCHBEND)
            {
                plugin_data->sv_pitch_bend_value = f_midi_item->value;
            }
            else if(f_midi_item->type == PYDAW_EVENT_CONTROLLER)
            {
                v_cc_map_translate(
                    &plugin_data->cc_map, plugin_data->descriptor,
                    plugin_data->port_table,
                    f_midi_item->port, f_midi_item->value);
            }

            ++midi_event_pos;
        }

        v_plugin_event_queue_atm_set(
            &plugin_data->atm_queue, f_i, plugin_data->port_table);

        v_sml_run(&plugin_data->mono_modules->pitchbend_smoother,
                (plugin_data->sv_pitch_bend_value));

        for(i2 = 0; i2 < (plugin_data->monofx_channel_index_count); ++i2)
        {
            plugin_data->mono_fx_buffers[
                (plugin_data->monofx_channel_index[i2])][0] = 0.0f;
            plugin_data->mono_fx_buffers[
                (plugin_data->monofx_channel_index[i2])][1] = 0.0f;
        }

        for(i2 = 0; i2 < EUPHORIA_POLYPHONY; ++i2)
        {
            if(((plugin_data->data[i2]->adsr_amp.stage) != ADSR_STAGE_OFF) &&
                (plugin_data->data[i2]->sample_indexes_count > 0))
            {
                add_sample_lms_euphoria(plugin_data, i2);
            }
        }

        for(i2 = 0; i2 < (plugin_data->monofx_channel_index_count); ++i2)
        {
            f_monofx_index = (plugin_data->monofx_channel_index[i2]);

            f_temp_sample0 = (plugin_data->mono_fx_buffers[f_monofx_index][0]);
            f_temp_sample1 = (plugin_data->mono_fx_buffers[f_monofx_index][1]);

            for(i3 = 0; i3 < EUPHORIA_MONO_FX_COUNT; ++i3)
            {
                v_mf3_set(
                    &plugin_data->mono_modules->mfx[
                        f_monofx_index].multieffect[i3],
                    (*(plugin_data->mfx_knobs[f_monofx_index][i3][0])),
                    (*(plugin_data->mfx_knobs[f_monofx_index][i3][1])),
                    (*(plugin_data->mfx_knobs[f_monofx_index][i3][2])));
                plugin_data->mono_modules->mfx[f_monofx_index].fx_func_ptr[i3](
                    &plugin_data->mono_modules->mfx[
                        f_monofx_index].multieffect[i3],
                    f_temp_sample0, f_temp_sample1);

                f_temp_sample0 =
                    (plugin_data->mono_modules->mfx[
                        f_monofx_index].multieffect[i3].output0);
                f_temp_sample1 =
                    (plugin_data->mono_modules->mfx[
                        f_monofx_index].multieffect[i3].output1);
            }

            v_eq6_run(&plugin_data->mono_modules->mfx[f_monofx_index].eqs,
                f_temp_sample0, f_temp_sample1);

            plugin_data->output[0][f_i] +=
                plugin_data->mono_modules->mfx[f_monofx_index].eqs.output0;
            plugin_data->output[1][f_i] +=
                plugin_data->mono_modules->mfx[f_monofx_index].eqs.output1;
        }
        ++plugin_data->sampleNo;
    }
}

static char *c_euphoria_load_all(t_euphoria *plugin_data, char *paths,
        pthread_spinlock_t * a_spinlock)
{
    int f_index = 0;
    int f_samples_loaded_count = 0;
    int f_current_string_index = 0;
    int f_total_index = 0;

    t_wav_pool_item * f_wavpool_items[EUPHORIA_MAX_SAMPLE_COUNT];
    int f_loaded_samples[EUPHORIA_MAX_SAMPLE_COUNT];

    int f_i = 0;
    while(f_i < EUPHORIA_MAX_SAMPLE_COUNT)
    {
        f_wavpool_items[f_i] = 0;
        f_loaded_samples[f_i] = 0;
        ++f_i;
    }

    char * f_result_string = (char*)malloc(sizeof(char) * 2048);

    while (f_samples_loaded_count < EUPHORIA_MAX_SAMPLE_COUNT)
    {
        if(paths[f_index] == EUPHORIA_FILES_STRING_DELIMITER ||
            paths[f_index] == '\0')
        {
            f_result_string[f_current_string_index] = '\0';

            if(f_current_string_index == 0)
            {
                f_wavpool_items[f_total_index] = 0;
            }
            else
            {
                int f_uid = atoi(f_result_string);
                t_wav_pool_item * f_wavpool_item = wavpool_get_func(f_uid);
                f_wavpool_items[f_total_index] = f_wavpool_item;
                f_loaded_samples[f_samples_loaded_count] = f_total_index;
                ++f_samples_loaded_count;
            }

            f_current_string_index = 0;
            ++f_total_index;

            if(paths[f_index] == '\0')
            {
                break;
            }
        }
        else
        {
            f_result_string[f_current_string_index] = paths[f_index];
            ++f_current_string_index;
        }

        ++f_index;
    }

    free(f_result_string);

    if(a_spinlock)
    {
        pthread_spin_lock(a_spinlock);
    }

    f_i = 0;

    while(f_i < f_samples_loaded_count)
    {
        plugin_data->loaded_samples[f_i] = f_loaded_samples[f_i];
        ++f_i;
    }

    plugin_data->loaded_samples_count = f_samples_loaded_count;

    f_i = 0;

    while(f_i < EUPHORIA_MAX_SAMPLE_COUNT)
    {
        plugin_data->samples[f_i].wavpool_items = f_wavpool_items[f_i];
        ++f_i;
    }

    //Force a re-index before the next sample period
    plugin_data->i_slow_index = EUPHORIA_SLOW_INDEX_COUNT;

    if(a_spinlock)
    {
        pthread_spin_unlock(a_spinlock);
    }

    return NULL;
}

void v_euphoria_configure(PYFX_Handle instance, char *key,
        char *value, pthread_spinlock_t * a_spinlock)
{
    t_euphoria *plugin_data = (t_euphoria *)instance;

    if (!strcmp(key, "load"))
    {
        c_euphoria_load_all(plugin_data, value, a_spinlock);
    }
    else
    {
        printf("ERROR: Euphoria unrecognized configure key %s\n", key);
    }
}

PYFX_Descriptor *euphoria_PYFX_descriptor()
{
    PYFX_Descriptor *f_result = pydaw_get_pyfx_descriptor(EUPHORIA_PORT_COUNT);

    pydaw_set_pyfx_port(f_result, EUPHORIA_ATTACK, 0.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_DECAY, 50.0f, 10.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_SUSTAIN, 0.0f, -60, 0);
    pydaw_set_pyfx_port(f_result, EUPHORIA_RELEASE, 50.0f, 10.0f, 400.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FILTER_ATTACK, 10.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FILTER_DECAY, 50.0f, 10.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FILTER_SUSTAIN, 100.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FILTER_RELEASE, 50.0f, 10.0f, 400.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_MASTER_VOLUME, -6.0f, -24, 24);
    pydaw_set_pyfx_port(f_result, EUPHORIA_MASTER_GLIDE, 0.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_MASTER_PITCHBEND_AMT, 18.0f, 1, 36);
    pydaw_set_pyfx_port(f_result, EUPHORIA_PITCH_ENV_TIME, 100.0f, 1.0f, 600.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_LFO_FREQ, 200.0f, 10, 1600);
    pydaw_set_pyfx_port(f_result, EUPHORIA_LFO_TYPE, 0.0f, 0, 2);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX0_KNOB0, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX0_KNOB1, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX0_KNOB2, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX0_COMBOBOX, 0.0f, 0, MULTIFX3KNOB_MAX_INDEX);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX1_KNOB0, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX1_KNOB1, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX1_KNOB2, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX1_COMBOBOX, 0.0f, 0, MULTIFX3KNOB_MAX_INDEX);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX2_KNOB0, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX2_KNOB1, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX2_KNOB2, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX2_COMBOBOX, 0.0f, 0, MULTIFX3KNOB_MAX_INDEX);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX3_KNOB0, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX3_KNOB1, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX3_KNOB2, 64.0f, 0, 127);
    pydaw_set_pyfx_port(f_result, EUPHORIA_FX3_COMBOBOX, 0.0f, 0, MULTIFX3KNOB_MAX_INDEX);
    pydaw_set_pyfx_port(f_result, EUPHORIA_MASTER_PITCH, 0.0f, -36.0f, 36.0f);

    int f_i = EUPHORIA_PFXMATRIX_GRP0DST0SRC0CTRL0;

    while(f_i <= EUPHORIA_PFXMATRIX_GRP0DST3SRC5CTRL2)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, -100.0f, 100.0f);
        ++f_i;
    }

    pydaw_set_pyfx_port(f_result, EUPHORIA_LFO_PITCH, 0.0f, -36.0f, 36.0f);

    f_i = EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MIN;

    while(f_i < EUPHORIA_SAMPLE_PITCH_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 60.0f, 0, 120);
        ++f_i;
    }

    while(f_i < EUPHORIA_PLAY_PITCH_LOW_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0, 120);
        ++f_i;
    }

    while(f_i < EUPHORIA_PLAY_PITCH_HIGH_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 120.0f, 0, 120);
        ++f_i;
    }

    while(f_i < EUPHORIA_SAMPLE_VOLUME_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, -50, 36);
        ++f_i;
    }

    while(f_i < EUPHORIA_SAMPLE_START_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0.0f, 1000.0f);
        ++f_i;
    }

    while(f_i < EUPHORIA_SAMPLE_END_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 1000.0f, 0.0f, 1000.0f);
        ++f_i;
    }

    while(f_i < EUPHORIA_SAMPLE_VEL_SENS_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 10.0f, 0.0f, 20.0f);
        ++f_i;
    }

    while(f_i < EUPHORIA_SAMPLE_VEL_LOW_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 1.0f, 1.0f, 127.0f);
        ++f_i;
    }

    while(f_i < EUPHORIA_SAMPLE_VEL_HIGH_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 128.0f, 1.0f, 128.0f);
        ++f_i;
    }

    while(f_i < EUPHORIA_PITCH_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, -36.0f, 36.0f);
        ++f_i;
    }

    while(f_i < EUPHORIA_TUNE_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, -100.0f, 100.0f);
        ++f_i;
    }

    while(f_i < EUPHORIA_SAMPLE_INTERPOLATION_MODE_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 1.0f, 0.0f, 3.0f);
        ++f_i;
    }

    while(f_i < EUPHORIA_SAMPLE_LOOP_START_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0.0f, 1000.0f);
        ++f_i;
    }

    while(f_i < EUPHORIA_SAMPLE_LOOP_END_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 1000.0f, 0.0f, 1000.0f);
        ++f_i;
    }

    while(f_i < EUPHORIA_SAMPLE_LOOP_MODE_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0.0f, 1.0f);
        ++f_i;
    }

    while(f_i < EUPHORIA_MONO_FX0_KNOB0_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0.0f, 127.0f);
        ++f_i;
    }

    while(f_i < EUPHORIA_MONO_FX0_KNOB1_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        ++f_i;
    }

    while(f_i < EUPHORIA_MONO_FX0_KNOB2_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        ++f_i;
    }

    while(f_i < EUPHORIA_MONO_FX0_COMBOBOX_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0, MULTIFX3KNOB_MAX_INDEX);
        ++f_i;
    }

    while(f_i < EUPHORIA_MONO_FX1_KNOB0_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        ++f_i;
    }

    while(f_i < EUPHORIA_MONO_FX1_KNOB1_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        ++f_i;
    }

    while(f_i < EUPHORIA_MONO_FX1_KNOB2_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        ++f_i;
    }

    while(f_i < EUPHORIA_MONO_FX1_COMBOBOX_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0, MULTIFX3KNOB_MAX_INDEX);
        ++f_i;
    }

    while(f_i < EUPHORIA_MONO_FX2_KNOB0_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        ++f_i;
    }

    while(f_i < EUPHORIA_MONO_FX2_KNOB1_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        ++f_i;
    }

    while(f_i < EUPHORIA_MONO_FX2_KNOB2_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        ++f_i;
    }

    while(f_i < EUPHORIA_MONO_FX2_COMBOBOX_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0, MULTIFX3KNOB_MAX_INDEX);
        ++f_i;
    }

    while(f_i < EUPHORIA_MONO_FX3_KNOB0_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        ++f_i;
    }

    while(f_i < EUPHORIA_MONO_FX3_KNOB1_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        ++f_i;
    }

    while(f_i < EUPHORIA_MONO_FX3_KNOB2_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 64.0f, 0, 127);
        ++f_i;
    }

    while(f_i < EUPHORIA_MONO_FX3_COMBOBOX_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0, MULTIFX3KNOB_MAX_INDEX);
        ++f_i;
    }

    while(f_i < EUPHORIA_SAMPLE_MONO_FX_GROUP_PORT_RANGE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0, (EUPHORIA_MAX_SAMPLE_COUNT - 1));
        ++f_i;
    }

    while(f_i < EUPHORIA_NOISE_AMP_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, -30.0f, -60.0f, 0.0f);
        ++f_i;
    }

    while(f_i < EUPHORIA_NOISE_TYPE_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0.0f, 2.0f);
        ++f_i;
    }

    while(f_i < EUPHORIA_SAMPLE_FADE_IN_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, 0.0f, 1000.0f);
        ++f_i;
    }

    while(f_i < EUPHORIA_SAMPLE_FADE_OUT_MAX)
    {
        pydaw_set_pyfx_port(f_result, f_i, 1000.0f, 0.0f, 1000.0f);
        ++f_i;
    }

    f_i = EUPHORIA_FIRST_EQ_PORT;

    int f_i2 = 0;

    while(f_i2 < EUPHORIA_MAX_SAMPLE_COUNT)
    {
        int f_i3 = 0;
        while(f_i3 < 6)
        {
            pydaw_set_pyfx_port(f_result, f_i, (f_i3 * 18.0f) + 24.0f, 20.0f, 120.0f);
            ++f_i;

            pydaw_set_pyfx_port(f_result, f_i, 300.0f, 100.0f, 600.0f);
            ++f_i;

            pydaw_set_pyfx_port(f_result, f_i, 0.0f, -240.0f, 240.0f);
            ++f_i;

            ++f_i3;
        }

        ++f_i2;
    }

    pydaw_set_pyfx_port(f_result, EUPHORIA_LFO_PITCH_FINE, 0.0f, -100.0f, 100.0f);

    pydaw_set_pyfx_port(f_result, EUPHORIA_MIN_NOTE, 0.0f, 0.0f, 120.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_MAX_NOTE, 120.0f, 0.0f, 120.0f);
    pydaw_set_pyfx_port(f_result, EUPHORIA_ADSR_LIN_MAIN, 1.0f, 0.0f, 1.0f);

    f_result->cleanup = cleanupSampler;
    f_result->connect_port = connectPortSampler;
    f_result->connect_buffer = euphoriaConnectBuffer;
    f_result->instantiate = instantiateSampler;
    f_result->panic = euphoriaPanic;
    f_result->load = v_euphoria_load;
    f_result->set_port_value = v_euphoria_set_port_value;
    f_result->set_cc_map = v_euphoria_set_cc_map;

    f_result->API_Version = 1;
    f_result->configure = v_euphoria_configure;
    f_result->run_replacing = v_run_lms_euphoria;
    f_result->offline_render_prep = NULL;
    f_result->on_stop = v_euphoria_on_stop;

    return f_result;
}


