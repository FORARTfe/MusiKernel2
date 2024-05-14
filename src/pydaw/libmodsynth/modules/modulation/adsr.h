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

#ifndef ADSR_H
#define	ADSR_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../constants.h"
#include "../../lib/amp.h"

#define ADSR_DB 18.0f
#define ADSR_DB_THRESHOLD -18.0f
#define ADSR_DB_THRESHOLD_LINEAR 0.125f

#define ADSR_DB_RELEASE 48.0f
#define ADSR_DB_THRESHOLD_RELEASE -48.0f
#define ADSR_DB_THRESHOLD_LINEAR_RELEASE  0.00390625f

#define ADSR_STAGE_DELAY 0
#define ADSR_STAGE_ATTACK 1
#define ADSR_STAGE_HOLD 2
#define ADSR_STAGE_DECAY 3
#define ADSR_STAGE_SUSTAIN 4
#define ADSR_STAGE_RELEASE 5
#define ADSR_STAGE_WAIT 6
#define ADSR_STAGE_OFF 7

typedef struct st_adsr
{
    float output;
    float output_db;
    int stage;  //0=a,1=d,2=s,3=r,4=inactive,6=delay,9=hold
    float a_inc;
    float d_inc;
    float s_value;
    float r_inc;

    float a_inc_db;
    float d_inc_db;
    float r_inc_db;

    float a_time;
    float d_time;
    float r_time;

    int time_counter;
    int delay_count;
    int hold_count;
    int wait_count;

    float sr;
    float sr_recip;
}t_adsr;

void v_adsr_set_a_time(t_adsr*, float);
void v_adsr_set_d_time(t_adsr*, float);
void v_adsr_set_s_value(t_adsr*, float);
void v_adsr_set_s_value_db(t_adsr*, float);
void v_adsr_set_r_time(t_adsr*, float);
void v_adsr_set_fast_release(t_adsr*);

void v_adsr_set_delay_time(t_adsr*, float);
void v_adsr_set_hold_time(t_adsr*, float);

void v_adsr_set_adsr_db(t_adsr*, float, float, float, float);
void v_adsr_set_adsr(t_adsr*, float, float, float, float);
void v_adsr_set_adsr_lin_from_db(t_adsr*, float, float, float, float);

void v_adsr_retrigger(t_adsr *);
void v_adsr_release(t_adsr *);
void v_adsr_run(t_adsr *);
void v_adsr_run_db(t_adsr *);
void v_adsr_kill(t_adsr *);

t_adsr * g_adsr_get_adsr(float);

typedef void (*fp_adsr_run)(t_adsr*);
typedef void (*fp_adsr_set)(t_adsr*, float, float, float, float);

fp_adsr_set FP_ADSR_SET[] = {
    v_adsr_set_adsr_db,
    v_adsr_set_adsr_lin_from_db
};

fp_adsr_run FP_ADSR_RUN[] = {
    v_adsr_run_db,
    v_adsr_run
};

/* void v_adsr_set_delay_time(t_adsr* a_adsr, float a_time)
 *
 * MUST BE CALLED AFTER RETRIGGER!!!!
 */
void v_adsr_set_delay_time(t_adsr* a_adsr, float a_time)
{
    if(a_time == 0.0f)
    {
        a_adsr->delay_count = 0;
    }
    else
    {
        a_adsr->stage = ADSR_STAGE_DELAY;
        a_adsr->delay_count = (int)(a_time * a_adsr->sr);
    }
}

void v_adsr_set_hold_time(t_adsr* a_adsr, float a_time)
{
    if(a_time == 0.0f)
    {
        a_adsr->hold_count = 0;
    }
    else
    {
        a_adsr->hold_count = (int)(a_time * a_adsr->sr);
    }
}


/* void v_adsr_set_a_time(
 * t_adsr* a_adsr_ptr,
 * float a_time)  //time in seconds
 *
 * Sets the envelope attack time
 */
void v_adsr_set_a_time(t_adsr*__restrict a_adsr_ptr, float a_time)
{
    if((a_adsr_ptr->a_time) == a_time)
        return;

    a_adsr_ptr->a_time = a_time;

    if(a_time <= 0.0f)
    {
        a_adsr_ptr->a_inc = 1.0f;
    }
    else
    {
        a_adsr_ptr->a_inc = (a_adsr_ptr->sr_recip) / (a_adsr_ptr->a_time);
    }

}

/* void v_adsr_set_d_time(
 * t_adsr* a_adsr_ptr,
 * float a_time) //time in seconds
 *
 * Sets the envelope decay time
 */
void v_adsr_set_d_time(t_adsr*__restrict a_adsr_ptr, float a_time)
{
    if((a_adsr_ptr->d_time) == a_time)
        return;

    if(a_time <= 0.0f)
    {
        a_adsr_ptr->d_time = .05;
    }
    else
    {
        a_adsr_ptr->d_time = a_time;
    }

    a_adsr_ptr->d_inc = ((a_adsr_ptr->sr_recip) / (a_adsr_ptr->d_time)) * -1.0f;
}

/* void v_adsr_set_r_time(
 * t_adsr* a_adsr_ptr,
 * float a_time) //time in seconds
 *
 * Sets the envelope release time
 */
void v_adsr_set_r_time(t_adsr*__restrict a_adsr_ptr, float a_time)
{
    if((a_adsr_ptr->r_time) == a_time)
        return;

    if(a_time <= 0.0f)
    {
        a_adsr_ptr->r_time = .05f;
    }
    else
    {
        a_adsr_ptr->r_time = a_time;
    }

    a_adsr_ptr->r_inc = ((a_adsr_ptr->sr_recip) / (a_adsr_ptr->r_time)) * -1.0f;
}

/* void v_adsr_set_fast_release(t_adsr* a_adsr_ptr)
 *
 * This method is for killing voices by allowing a quick fade-out
 * instead of directly stealing a voice, which should
 * allow a quick transition without a click
 * TODO:  The total time of the fadeout is not consistent
 * between different sample rates.
 */
void v_adsr_set_fast_release(t_adsr*__restrict a_adsr_ptr)
{
    a_adsr_ptr->r_time = .01f;
    a_adsr_ptr->r_inc = -.002f;
    a_adsr_ptr->r_inc_db = ((a_adsr_ptr->sr) * -0.01f) / ADSR_DB_RELEASE;
    a_adsr_ptr->stage = ADSR_STAGE_RELEASE;
}

/* void v_adsr_set_s_value(
 * t_adsr* a_adsr_ptr,
 * float a_value) //The sustain value, range: 0 to 1
 */
void v_adsr_set_s_value(t_adsr*__restrict a_adsr_ptr, float a_value)
{
    a_adsr_ptr->s_value = a_value;

    if((a_adsr_ptr->s_value) <= 0.0f)
    {
        a_adsr_ptr->s_value = .001f;
    }
}

/* void v_adsr_set_s_value_db(
 * t_adsr* a_adsr_ptr,
 * float a_value)  //The decibel value of sustain, typically -30 to 0
 */
void v_adsr_set_s_value_db(t_adsr*__restrict a_adsr_ptr, float a_value)
{
    v_adsr_set_s_value(a_adsr_ptr, f_db_to_linear_fast(a_value));
}

/* void v_adsr_set_adsr(
 * t_adsr* a_adsr_ptr,
 * float a_a, //attack
 * float a_d, //decay
 * float a_s, //sustain
 * float a_r) //release
 *
 * Set allADSR values, with a range of 0 to 1 for sustain
 */
void v_adsr_set_adsr(t_adsr*__restrict a_adsr_ptr, float a_a,
        float a_d, float a_s, float a_r)
{
    v_adsr_set_a_time(a_adsr_ptr, a_a);
    v_adsr_set_d_time(a_adsr_ptr, a_d);
    v_adsr_set_s_value(a_adsr_ptr, a_s);
    v_adsr_set_r_time(a_adsr_ptr, a_r);
}

void v_adsr_set_adsr_lin_from_db(t_adsr* a_adsr_ptr, float a_a,
        float a_d, float a_s, float a_r)
{
    a_s = f_db_to_linear_fast(a_s);
    v_adsr_set_adsr(a_adsr_ptr, a_a, a_d, a_s, a_r);
}

/* void v_adsr_set_adsr(
 * t_adsr* a_adsr_ptr,
 * float a_a, //attack
 * float a_d, //decay
 * float a_s, //sustain
 * float a_r) //release
 *
 * Set all ADSR values, with a range of -30 to 0 for sustain
 */
void v_adsr_set_adsr_db(t_adsr*__restrict a_adsr_ptr, float a_a,
        float a_d, float a_s, float a_r)
{
    v_adsr_set_a_time(a_adsr_ptr, a_a);
    v_adsr_set_d_time(a_adsr_ptr, a_d);
    v_adsr_set_s_value_db(a_adsr_ptr, a_s);
    v_adsr_set_r_time(a_adsr_ptr, a_r);

    a_adsr_ptr->a_inc_db = (a_adsr_ptr->a_inc) * ADSR_DB;
    a_adsr_ptr->d_inc_db = (a_adsr_ptr->d_inc) * (ADSR_DB);
    a_adsr_ptr->r_inc_db = (a_adsr_ptr->r_inc) * (ADSR_DB_RELEASE);
}

/* void v_adsr_retrigger(t_adsr * a_adsr_ptr)
 *
 * Reset the ADSR to the beginning of the attack phase
 */
void v_adsr_retrigger(t_adsr *__restrict a_adsr_ptr)
{
    a_adsr_ptr->stage = ADSR_STAGE_ATTACK;
    a_adsr_ptr->output = 0.0f;
    a_adsr_ptr->output_db = ADSR_DB_THRESHOLD;
    a_adsr_ptr->time_counter = 0;
    a_adsr_ptr->wait_count = 0;
}

void v_adsr_kill(t_adsr *__restrict a_adsr_ptr)
{
    a_adsr_ptr->stage = ADSR_STAGE_OFF;
    a_adsr_ptr->output = 0.0f;
    a_adsr_ptr->output_db = ADSR_DB_THRESHOLD;
}

/* void v_adsr_release(t_adsr * a_adsr_ptr)
 *
 * Set the ADSR to the release phase
 */
void v_adsr_release(t_adsr *__restrict a_adsr_ptr)
{
    if(a_adsr_ptr->stage < ADSR_STAGE_RELEASE)
    {
        a_adsr_ptr->stage = ADSR_STAGE_RELEASE;
    }
}

void g_adsr_init(t_adsr * f_result, float a_sr)
{
    f_result->sr = a_sr;
    f_result->sr_recip = 1.0f / a_sr;

    f_result->output = 0.0f;
    f_result->stage = ADSR_STAGE_OFF;

    //Set these to nonsensical values so that comparisons aren't
    //happening with invalid numbers
    f_result->a_inc = -100.5f;
    f_result->a_time = -100.5f;
    f_result->d_inc = -100.5f;
    f_result->d_time =  -100.5f;
    f_result->r_inc = -100.5f;
    f_result->r_time = -100.5f;
    f_result->s_value = -100.5f;

    f_result->output_db = -100.5f;
    f_result->a_inc_db = 0.1f;
    f_result->d_inc_db = 0.1f;
    f_result->r_inc_db = 0.1f;

    v_adsr_set_a_time(f_result, .05);
    v_adsr_set_d_time(f_result, .5);
    v_adsr_set_s_value_db(f_result, -12.0f);
    v_adsr_set_r_time(f_result, .5);

    f_result->time_counter = 0;
    f_result->delay_count = 0;
    f_result->hold_count = 0;
    f_result->wait_count = 0;
}

/* t_adsr * g_adsr_get_adsr(
 * float a_sr_recip) // 1.0f/sample_rate (TODO: use sample_rate instead)
 *
 */
t_adsr * g_adsr_get_adsr(float a_sr)
{
    t_adsr * f_result;
    hpalloc((void**)&f_result, sizeof(t_adsr));
    g_adsr_init(f_result, a_sr);
    return f_result;
}

void v_adsr_run_delay(t_adsr *self)
{
    ++self->time_counter;
    if(self->time_counter >= self->delay_count)
    {
        self->stage = ADSR_STAGE_ATTACK;
        self->time_counter = 0;
    }
}

void v_adsr_run_attack(t_adsr *self)
{
    self->output = (self->output) + (self->a_inc);
    if((self->output) >= 1.0f)
    {
        self->output = 1.0f;

        if(self->hold_count)
        {
            self->stage = ADSR_STAGE_HOLD;
        }
        else
        {
            self->stage = ADSR_STAGE_DECAY;
        }
    }
}

void v_adsr_run_attack_db(t_adsr *self)
{
    if((self->output) < ADSR_DB_THRESHOLD_LINEAR)
    {
        self->output = (self->output) + 0.005f;
    }
    else
    {
        self->output_db = (self->output_db) + (self->a_inc_db);
        self->output = f_db_to_linear_fast((self->output_db));

        if((self->output) >= 1.0f)
        {
            self->output = 1.0f;

            if(self->hold_count)
            {
                self->stage = ADSR_STAGE_HOLD;
            }
            else
            {
                self->stage = ADSR_STAGE_DECAY;
            }
        }
    }
}

void v_adsr_run_hold(t_adsr *self)
{
    ++self->time_counter;
    if(self->time_counter >= self->hold_count)
    {
        self->stage = ADSR_STAGE_DECAY;
        self->time_counter = 0;
    }
}

void v_adsr_run_decay(t_adsr *self)
{
    self->output += self->d_inc;
    if((self->output) <= (self->s_value))
    {
        self->output = self->s_value;
        self->stage = ADSR_STAGE_SUSTAIN;
    }
}

void v_adsr_run_decay_db(t_adsr *self)
{
    if((self->output) < ADSR_DB_THRESHOLD_LINEAR)
    {
        self->output += self->d_inc;
    }
    else
    {
        self->output_db += self->d_inc_db;
        self->output = f_db_to_linear_fast(self->output_db);
    }

    if((self->output) <= (self->s_value))
    {
        self->output = self->s_value;
        self->stage = ADSR_STAGE_SUSTAIN;
    }
}

void v_adsr_run_sustain(t_adsr *self)
{
    // Do nothing
}

void v_adsr_run_release(t_adsr *self)
{
    self->output += self->r_inc;
    if((self->output) <= 0.0f)
    {
        self->output = 0.0f;
        self->stage = ADSR_STAGE_WAIT;
    }
}

void v_adsr_run_release_db(t_adsr *self)
{
    if((self->output) < ADSR_DB_THRESHOLD_LINEAR_RELEASE)
    {
        self->output_db -= 0.05f;

        if(self->output_db < -96.0f)
        {
            self->stage = ADSR_STAGE_WAIT;
            self->output = 0.0f;
        }
        else
        {
            self->output = f_db_to_linear_fast(self->output_db);
        }
    }
    else
    {
        self->output_db += self->r_inc_db;
        self->output = f_db_to_linear_fast(self->output_db);
    }
}

void v_adsr_run_wait(t_adsr *self)
{
    ++self->wait_count;
    if(self->wait_count >= 1000)
    {
        self->stage = ADSR_STAGE_OFF;
    }
}


typedef void (*fn_adsr_run)(t_adsr*);

__thread fn_adsr_run ADSR_RUN[] __attribute__((aligned(CACHE_LINE_SIZE))) = {
v_adsr_run_delay, //ADSR_STAGE_DELAY 0
v_adsr_run_attack, //ADSR_STAGE_ATTACK 1
v_adsr_run_hold, //ADSR_STAGE_HOLD 2
v_adsr_run_decay, //ADSR_STAGE_DECAY 3
v_adsr_run_sustain, //ADSR_STAGE_SUSTAIN 4
v_adsr_run_release, //ADSR_STAGE_RELEASE 5
v_adsr_run_wait, //ADSR_STAGE_WAIT 6
//ADSR_STAGE_OFF 7
};

__thread fn_adsr_run ADSR_RUN_DB[] __attribute__((aligned(CACHE_LINE_SIZE))) = {
v_adsr_run_delay, //ADSR_STAGE_DELAY 0
v_adsr_run_attack_db, //ADSR_STAGE_ATTACK 1
v_adsr_run_hold, //ADSR_STAGE_HOLD 2
v_adsr_run_decay_db, //ADSR_STAGE_DECAY 3
v_adsr_run_sustain, //ADSR_STAGE_SUSTAIN 4
v_adsr_run_release_db, //ADSR_STAGE_RELEASE 5
v_adsr_run_wait, //ADSR_STAGE_WAIT 6
//ADSR_STAGE_OFF 7
};

/* void v_adsr_run(t_adsr * self)
 *
 * Run the ADSR envelope
 */
void v_adsr_run(t_adsr *self)
{
    if((self->stage) != ADSR_STAGE_OFF)
    {
        ADSR_RUN[self->stage](self);
    }
}

void v_adsr_run_db(t_adsr *self)
{
    if((self->stage) != ADSR_STAGE_OFF)
    {
        ADSR_RUN_DB[self->stage](self);
    }
}


#ifdef	__cplusplus
}
#endif

#endif	/* ADSR_H */

