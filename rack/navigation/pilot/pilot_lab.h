/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 */

#ifndef __PILOT_LAB_H__
#define __PILOT_LAB_H__

#include <main/rack_data_module.h>

#include <drivers/chassis_proxy.h>
#include <navigation/pilot_proxy.h>
#include <navigation/position_proxy.h>
#include <main/pilot_tool.h>
#include <perception/scan2d_proxy.h>

// define module class
#define MODULE_CLASS_ID             PILOT

// scan_2d data message (use max message size)
typedef struct {
    scan2d_data   data;
    scan_point    point[SCAN2D_POINT_MAX];
} __attribute__((packed)) scan2d_data_msg;

// pilot data message (use no splines)
typedef struct {
     pilot_data        data;
     polar_spline      spline[0];
} __attribute__((packed)) pilot_data_msg;



/**
 * Pilot Lab
 *
 * @ingroup modules_pilot
 */
class PilotLab : public RackDataModule {
      private:

        // external module parameter
        int                 chassisSys;
        int                 chassisInst;
        int                 positionSys;
        int                 positionInst;
        int                 scan2dSys;
        int                 scan2dInst;
        int                 speedMax;
        float               omegaMax;
        int                 varDistance;
        float               varRho;
        int                 distanceMin;

        // mailboxes
        RackMailbox         workMbx;                // communication
        RackMailbox         scan2dDataMbx;          // scan2d data

        // proxies
        ChassisProxy*       chassis;
        PositionProxy*      position;
        Scan2dProxy*        scan2d;

        // data structures
        chassis_param_data  chasParData;
        position_data       positionData;
        scan2d_data_msg     scan2dMsg;
        pilot_dest_data     pilotDest;

        // variables
        int                 comfortDistance;
        int                 pilotState;
        int                 speed;
        float               omega;

      protected:
        // -> realtime context
        int      moduleOn(void);
        int      moduleLoop(void);
        void     moduleOff(void);
        int      moduleCommand(RackMessage *msgInfo);
        
        float getAngle(int x1, int y1, int x2, int y2);
        int   getDistance(int x1, int y1, int x2, int y2);

        float controlOmega(int speed, int dCurr, int dSet, float rhoCurr, float rhoSet);

        // -> non realtime context
        void     moduleCleanup(void);

      public:
        // constructor und destructor
        PilotLab();
        ~PilotLab() {};

        // -> non realtime context
        int  moduleInit(void);
};

#endif // __PILOT_LAB_H__
