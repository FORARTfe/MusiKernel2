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

#ifndef MKCHNL_LIBMODSYNTH_H
#define	MKCHNL_LIBMODSYNTH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../libmodsynth/constants.h"
#include "../../libmodsynth/lib/amp.h"
#include "../../libmodsynth/lib/smoother-linear.h"
#include "../../libmodsynth/modules/filter/svf.h"
#include "../../libmodsynth/modules/modulation/env_follower.h"
#include "../../libmodsynth/modules/signal_routing/panner2.h"

typedef struct
{
    float current_sample0;
    float current_sample1;

    t_smoother_linear volume_smoother;
    t_smoother_linear pan_smoother;

    t_pn2_panner2 panner;
}t_mkchnl_mono_modules;

t_mkchnl_mono_modules * v_mkchnl_mono_init(float, int);

t_mkchnl_mono_modules * v_mkchnl_mono_init(float a_sr, int a_plugin_uid)
{
    t_mkchnl_mono_modules * a_mono;
    hpalloc((void**)&a_mono, sizeof(t_mkchnl_mono_modules));

    g_sml_init(&a_mono->volume_smoother, a_sr, 0.0f, -50.0f, 0.1f);
    a_mono->volume_smoother.last_value = 0.0f;
    g_sml_init(&a_mono->pan_smoother, a_sr, 100.0f, -100.0f, 0.1f);
    a_mono->pan_smoother.last_value = 0.0f;

    return a_mono;
}



#ifdef	__cplusplus
}
#endif

#endif	/* LIBMODSYNTH_H */

