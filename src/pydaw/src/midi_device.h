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

#ifndef MIDI_DEVICE_H
#define	MIDI_DEVICE_H

#include <assert.h>
#include "../include/pydaw_plugin.h"
#include <sys/time.h>
#include <time.h>
#include <portmidi.h>

#define MK_MAX_MIDI_DEVICE_COUNT 8

#define MIDI_EVENT_BUFFER_SIZE 1024
#define MIDI_CONTROLLER_COUNT 128

//low-level MIDI stuff
#define MIDI_NOTE_OFF       0x80
#define MIDI_NOTE_ON        0x90
#define MIDI_CC             0xB0
#define MIDI_PITCH_BEND     0xE0
#define MIDI_EOX            0xF7

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct
{
    int loaded;
    PmStream *f_midi_stream;
    PmError f_midi_err;
    PmDeviceID f_device_id;
    int instanceEventCounts;
    t_pydaw_seq_event instanceEventBuffers[MIDI_EVENT_BUFFER_SIZE];
    PmEvent portMidiBuffer[MIDI_EVENT_BUFFER_SIZE];
    t_pydaw_seq_event midiEventBuffer[MIDI_EVENT_BUFFER_SIZE];
    int midiEventReadIndex;
    int midiEventWriteIndex;
    char name[256];
}t_midi_device;

typedef struct
{
    int count;
    t_midi_device devices[MK_MAX_MIDI_DEVICE_COUNT];
}t_midi_device_list;

void midiDeviceRead(t_midi_device *, float, unsigned long);
void midiPoll(t_midi_device * self);

#ifdef	__cplusplus
}
#endif

int midiDeviceInit(t_midi_device * self, char * f_midi_device_name)
{
    self->instanceEventCounts = 0;
    self->midiEventReadIndex = 0;
    self->midiEventWriteIndex = 0;
    self->loaded = 0;
    self->f_device_id = pmNoDevice;
    self->name[0] = '\0';
    sprintf(self->name, "%s", f_midi_device_name);

    if(strcmp(f_midi_device_name, "None"))
    {
        int f_i;
        int f_count = Pm_CountDevices();

        for(f_i = 0; f_i < f_count; ++f_i)
        {
            const PmDeviceInfo * f_device = Pm_GetDeviceInfo(f_i);
            if(f_device->input && !strcmp(f_device->name, f_midi_device_name))
            {
                self->f_device_id = f_i;
                break;
            }
        }

        if(self->f_device_id == pmNoDevice)
        {
            return 1;
        }

        printf("Opening MIDI device ID: %i\n", self->f_device_id);
        self->f_midi_err = Pm_OpenInput(
            &self->f_midi_stream, self->f_device_id, NULL,
            MIDI_EVENT_BUFFER_SIZE, NULL, NULL);

        if(self->f_midi_err != pmNoError)
        {
            return 2;
        }
    }

    self->loaded = 1;

    return 0;
}

void midiDeviceClose(t_midi_device * self)
{
    Pm_Close(self->f_midi_stream);
}

void midiReceive(
    t_midi_device * self, unsigned char status, unsigned char control,
    char value)
{
    unsigned char channel = status & 0x0F;
    unsigned char opCode = status & 0xF0;
    if (opCode >= 0xF0)
    {
        opCode = status;
    }

    //int twoBytes = 1;
    struct timeval tv;

    if(self->midiEventReadIndex == self->midiEventWriteIndex + 1)
    {
        printf("Warning: MIDI event buffer overflow!  "
                "ignoring incoming event\n");
        return;
    }

    switch (opCode)
    {
        case MIDI_PITCH_BEND:
        {
            //twoBytes = 0;
            int f_pb_val = ((value << 7) | control) - 8192;
            v_pydaw_ev_set_pitchbend(
                &self->midiEventBuffer[self->midiEventWriteIndex],
                channel, f_pb_val);
            ++self->midiEventWriteIndex;
            //printf("MIDI PITCHBEND status %i ch %i, value %i\n",
            //      status, channel+1, f_pb_val);
        }
            break;
        case MIDI_NOTE_OFF:
            v_pydaw_ev_set_noteoff(
                &self->midiEventBuffer[self->midiEventWriteIndex],
                channel, control, value);
            ++self->midiEventWriteIndex;
            /*printf("MIDI NOTE_OFF status %i (ch %i, opcode %i), ctrl %i, "
                    "val %i\n", status, channel+1, (status & 255)>>4, control,
                    value);*/
            break;
        case MIDI_NOTE_ON:
            if(value == 0)
            {
                v_pydaw_ev_set_noteoff(
                    &self->midiEventBuffer[self->midiEventWriteIndex],
                    channel, control, value);
            }
            else
            {
                v_pydaw_ev_set_noteon(
                    &self->midiEventBuffer[self->midiEventWriteIndex],
                    channel, control, value);
            }
            ++self->midiEventWriteIndex;
            /*printf("MIDI NOTE_ON status %i (ch %i, opcode %i), ctrl %i, "
                    "val %i\n", status, channel+1, (status & 255)>>4, control,
                    value);*/
            break;
        //case MIDI_AFTERTOUCH:
        case MIDI_CC:
            v_pydaw_ev_set_controller(
                &self->midiEventBuffer[self->midiEventWriteIndex],
                channel, control, value);
            ++self->midiEventWriteIndex;
            /*printf("MIDI CC status %i (ch %i, opcode %i), ctrl %i, "
                    "val %i\n", status, channel+1, (status & 255)>>4, control,
                    value);*/
            break;
        default:
            //twoBytes = 0;
            /*message = QString("MIDI status 0x%1")
                 .arg(QString::number(status, 16).toUpper());*/
            break;
    }

    /* At this point we change the event timestamp so that its
   real-time field contains the actual time at which it was
   received and processed (i.e. now).  Then in the audio
   callback we use that to calculate frame offset. */

    gettimeofday(&tv, NULL);
    self->midiEventBuffer[self->midiEventWriteIndex].tv_sec = tv.tv_sec;
    self->midiEventBuffer[self->midiEventWriteIndex].tv_nsec =
        tv.tv_usec * 1000L;

    if(self->midiEventWriteIndex >= MIDI_EVENT_BUFFER_SIZE)
    {
        self->midiEventWriteIndex = 0;
    }
}


void midiPoll(t_midi_device * self)
{
    PmError f_poll_result;
    int f_bInSysex = 0;
    unsigned char status;
    //int f_cReceiveMsg_index = 0;
    int i;

    f_poll_result = Pm_Poll(self->f_midi_stream);
    if(f_poll_result < 0)
    {
        printf("Portmidi error %s\n", Pm_GetErrorText(f_poll_result));
    }
    else if(f_poll_result > 0)
    {
        int numEvents = Pm_Read(self->f_midi_stream, self->portMidiBuffer,
                MIDI_EVENT_BUFFER_SIZE);

        if (numEvents < 0)
        {
            printf("PortMidi error: %s\n", Pm_GetErrorText((PmError)numEvents));
        }
        else if(numEvents > 0)
        {
            for (i = 0; i < numEvents; i++)
            {
                status = Pm_MessageStatus(self->portMidiBuffer[i].message);

                if ((status & 0xF8) == 0xF8) {
                    // Handle real-time MIDI messages at any time
                    midiReceive(self, status, 0, 0);
                }

                reprocessMessage:

                if (!f_bInSysex) {
                    if (status == 0xF0) {
                        f_bInSysex = 1;
                        status = 0;
                    } else {
                        //unsigned char channel = status & 0x0F;
                        unsigned char note = Pm_MessageData1(
                                self->portMidiBuffer[i].message);
                        unsigned char velocity = Pm_MessageData2(
                                self->portMidiBuffer[i].message);
                        midiReceive(self, status, note, velocity);
                    }
                }

                if(f_bInSysex)
                {
                    // Abort (drop) the current System Exclusive message if a
                    //  non-realtime status byte was received
                    if (status > 0x7F && status < 0xF7)
                    {
                        f_bInSysex = 0;
                        //f_cReceiveMsg_index = 0;
                        printf("Buggy MIDI device: SysEx interrupted!\n");
                        goto reprocessMessage;    // Don't lose the new message
                    }

                    // Collect bytes from PmMessage
                    int data = 0;
                    int shift;
                    for (shift = 0; shift < 32 && (data != MIDI_EOX);
                            shift += 8)
                    {
                        if ((data & 0xF8) == 0xF8)
                        {
                            // Handle real-time messages at any time
                            midiReceive(self, data, 0, 0);
                        }
                        else
                        {
                            //m_cReceiveMsg[m_cReceiveMsg_index++] = data =
                            //    (portMidiBuffer[i].message >> shift) & 0xFF;
                        }
                    }

                    // End System Exclusive message if the EOX byte was received
                    if (data == MIDI_EOX)
                    {
                        f_bInSysex = 0;
                        printf("Dropping MIDI message in if "
                                "(data == MIDI_EOX)\n");
                        //const char* buffer =
                                //reinterpret_cast<const char*>(m_cReceiveMsg);
                        //receive(QByteArray::fromRawData(buffer,
                        //        m_cReceiveMsg_index));
                        //f_cReceiveMsg_index = 0;
                    } //if (data == MIDI_EOX)
                } //if(f_bInSysex)
            } //for (i = 0; i < numEvents; i++)
        } //else if(numEvents > 0)
    } //else if(f_poll_result > 0)
}


void midiDeviceRead(t_midi_device * self, float sample_rate,
        unsigned long framesPerBuffer)
{
    struct timeval tv, evtv, diff;
    long framediff;

    gettimeofday(&tv, NULL);

    self->instanceEventCounts = 0;

    while(self->midiEventReadIndex != self->midiEventWriteIndex)
    {
	t_pydaw_seq_event *ev =
            &self->midiEventBuffer[self->midiEventReadIndex];

        /*
        if (!v_pydaw_ev_is_channel_type(ev))
        {
            midiEventReadIndex++
            //discard non-channel oriented messages
            continue;
        }
        */

        /* Stop processing incoming MIDI if an instance's event buffer is
         * full. */
	if (self->instanceEventCounts == MIDI_EVENT_BUFFER_SIZE)
            break;

	/* Each event has a real-time timestamp indicating when it was
	 * received (set by midi_callback).  We need to calculate the
	 * difference between then and the start of the audio callback
	 * (held in tv), and use that to assign a frame offset, to
	 * avoid jitter.  We should stop processing when we reach any
	 * event received after the start of the audio callback. */

	evtv.tv_sec = ev->tv_sec;
	evtv.tv_usec = ev->tv_nsec / 1000;

	if (evtv.tv_sec > tv.tv_sec ||
	    (evtv.tv_sec == tv.tv_sec && evtv.tv_usec > tv.tv_usec))
        {
	    break;
	}

	diff.tv_sec = tv.tv_sec - evtv.tv_sec;
	if (tv.tv_usec < evtv.tv_usec)
        {
	    --diff.tv_sec;
	    diff.tv_usec = tv.tv_usec + 1000000 - evtv.tv_usec;
	}
        else
        {
	    diff.tv_usec = tv.tv_usec - evtv.tv_usec;
	}

	framediff =
	    diff.tv_sec * sample_rate +
	    ((diff.tv_usec / 1000) * sample_rate) / 1000 +
	    ((diff.tv_usec - 1000 * (diff.tv_usec / 1000)) * sample_rate)
            / 1000000;

	if (framediff >= framesPerBuffer) framediff = framesPerBuffer - 1;
	else if (framediff < 0) framediff = 0;

	ev->tick = framesPerBuffer - framediff - 1;
        int f_max_tick = framesPerBuffer - 1;

        if(ev->tick < 0)
        {
            ev->tick = 0;
        }
        else if(ev->tick > f_max_tick)
        {
            ev->tick = f_max_tick;
        }

	if (ev->type == PYDAW_EVENT_CONTROLLER)
        {
	    int controller = ev->param;

	    if (controller == 0)
            {
                // bank select MSB

	    }
            else if (controller == 32)
            {
                // bank select LSB
	    }
            else if (controller > 0 && controller < MIDI_CONTROLLER_COUNT)
            {
                self->instanceEventBuffers[self->instanceEventCounts] = *ev;
                ++self->instanceEventCounts;
	    }
            else
            {
                assert(0);
            }
	}
        else
        {
            self->instanceEventBuffers[self->instanceEventCounts] = *ev;
            ++self->instanceEventCounts;
	}

        ++self->midiEventReadIndex;
        if(self->midiEventReadIndex >= MIDI_EVENT_BUFFER_SIZE)
        {
            self->midiEventReadIndex = 0;
        }
    }

    assert(self->instanceEventCounts < MIDI_EVENT_BUFFER_SIZE);
}

#endif	/* MIDI_DEVICE_H */

