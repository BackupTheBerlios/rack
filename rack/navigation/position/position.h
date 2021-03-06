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
 *      Oliver Wulf <wulf@rts.uni-hannover.de>
 *
 */
#ifndef __POSITION_H__
#define __POSITION_H__

#include <main/rack_data_module.h>
#include <main/position_tool.h>
#include <navigation/position_proxy.h>
#include <navigation/odometry_proxy.h>

// define module class
#define MODULE_CLASS_ID                 POSITION

#define POSITION_REFERENCE_WGS84        0
#define POSITION_REFERENCE_GK           1
#define POSITION_REFERENCE_UTM          2



/**
 *
 * @ingroup modules_position
 */
class Position : public RackDataModule {
    private:
        int                 odometrySys;
        int                 odometryInst;
        unsigned int        updateInterpol;
        double              offsetLatitude;
        double              offsetLongitude;
        int                 scaleLatitude;
        int                 scaleLongitude;
        double              offsetNorthing;
        double              offsetEasting;
        int                 utmZone;
        position_utm_band   utmBand;
        int                 positionReference;
        int                 autoOffset;
        double              odometryStdDevX;
        double              odometryStdDevY;
        double              odometryStdDevRho;
        double              stdDevX, stdDevY, stdDevRho;

        int                 offset;
        RackMutex           refPosMtx;
        position_3d         refOdo;
        double              sinRefOdo, cosRefOdo;
        position_3d         oldPos;

        position_3d         interpolDiff;           // for update interpolation
        position_3d         positionStdDeviation;
        int                 stdDeviationUpdate;
        rack_time_t         interpolStartTime;
        position_3d         refPos;
        position_3d         refPosI;
        position_3d         oldOdometryPos;
        double              sinRefPosI, cosRefPosI;
        PositionTool        *positionTool;

        // mailboxes
        RackMailbox         odometryMbx;
        RackMailbox         workMbx;

        // proxies
        OdometryProxy*      odometry;

    protected:
        // -> realtime context
        int     moduleOn(void);
        int     moduleLoop(void);
        void    moduleOff(void);
        int     moduleCommand(RackMessage *msgInfo);

        void    wgs84ToPos(position_wgs84_data *posWgs84Data, position_data *posData);
        void    posToWgs84(position_data *posData, position_wgs84_data *posWgs84Data);
        void    getPosition(position_3d* odo, position_3d* pos);

        // -> non realtime context
        void    moduleCleanup(void);

    public:
        // constructor und destructor
        Position();
        ~Position() {};

        // -> non realtime context
        int  moduleInit(void);
};

#endif // __POSITION_H__
