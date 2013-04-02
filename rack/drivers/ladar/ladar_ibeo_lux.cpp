/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2010 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Matthias Hentschel      <hentschel@rts.uni-hannover.de>
 *      Sebastian Smolorz       <smolorz@rts.uni-hannover.de>
 *
 */

#include <iostream>

#include "ladar_ibeo_lux.h"

//
// data structures
//

arg_table_t argTab[] = {

    { ARGOPT_OPT, "positionSys", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of the position module", { 0 } },

    { ARGOPT_OPT, "positionInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the position module", { -1 } },

    { ARGOPT_OPT, "objRecogBoundSys", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of the object recognition relay for bounding-boxes, default 0", { 0 } },

    { ARGOPT_OPT, "objRecogBoundInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the object recognition relay for bounding-boxes, default -1", { -1 } },

    { ARGOPT_OPT, "objRecogContourSys", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of the object recognition relay for contours, default 0", { 0 } },

    { ARGOPT_OPT, "objRecogContourInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the object recognition relay for contours, default -1", { -1 } },

    { ARGOPT_REQ, "ladarIp", ARGOPT_REQVAL, ARGOPT_VAL_STR,
      "IP address of the ibeo lux ladar, default '10.152.36.200'", { 0 } },

    { ARGOPT_OPT, "ladarPort", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Port of the ibeo lux ladar, default 12002", { 12002 } },

    { ARGOPT_OPT, "distanceFilter", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Filter to select the shortest distance for each angle measurement, 1=on, 0=0ff, default off", { 0 } },

    { ARGOPT_OPT, "velocityMode", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Output mode of the object velocity. 0 = relative, 1 = absolute, default = 0", { 0 } },

    { 0, "", 0, 0, "", { 0 } } // last entry
};


/*****************************************************************************
 *   !!! REALTIME CONTEXT !!!
 *
 *   moduleOn,
 *   moduleOff,
 *   moduleLoop,
 *   moduleCommand,
 *
 *   own realtime user functions
 ****************************************************************************/
int LadarIbeoLux::moduleOn(void)
{
    int             ret;
    struct timeval  recvTimeout;

    // read dynamic module parameter
    ladarIp           = getStringParam("ladarIp");
    ladarPort         = getInt32Param("ladarPort");
    distanceFilter    = getInt32Param("distanceFilter");
    velocityMode      = getInt32Param("velocityMode");

    if (positionInst >= 0)
    {
        ret = position->on();
        if (ret)
        {
            GDOS_ERROR("Can't turn on Position(%d/%d), code = %d\n",
                       positionSys, positionInst, ret);
            return ret;
        }

        ret = position->getData(&positionData, sizeof(position_data), (rack_time_t)0);
        if (ret)
        {
            GDOS_ERROR("Can't get data from Position(%d/%d), code = %d\n",
                       positionSys, positionInst, ret);
            return ret;
        }
    }
    else
    {
        memset(&positionData, 0, sizeof(position_data));
    }

    // object recognition bounding-box
    if (objRecogBoundInst >= 0)
    {
        // init objRecogBound values and send first dataMsg
        objRecogBoundData.data.recordingTime = rackTime.get();
        objRecogBoundData.data.objectNum      = 0;

        ret = workMbx.sendDataMsg(MSG_DATA, objRecogBoundMbxAdr + 1, 1, 1,
                                 &objRecogBoundData, sizeof(obj_recog_data));
        if (ret)
        {
            GDOS_WARNING("Error while sending first objRecogBound data from %x to %x (bytes %d)\n",
                         workMbx.getAdr(), objRecogBoundMbxAdr, sizeof(obj_recog_data));
        }

        GDOS_DBG_DETAIL("Turn on ObjRecog(%d/%d)\n", objRecogBoundSys, objRecogBoundInst);
        ret = objRecogBound->on();
        if (ret)
        {
            GDOS_ERROR("Can't turn on ObjRecogBound(%i/%i), code = %d\n",
                       objRecogBoundSys, objRecogBoundInst, ret);
            return ret;
        }
    }

    // object recognition contour
    if (objRecogContourInst >= 0)
    {
        // init objRecogContour values and send first dataMsg
        objRecogContourData.data.recordingTime = rackTime.get();
        objRecogContourData.data.objectNum      = 0;

        ret = workMbx.sendDataMsg(MSG_DATA, objRecogContourMbxAdr + 1, 1, 1,
                                 &objRecogContourData, sizeof(obj_recog_data));
        if (ret)
        {
            GDOS_WARNING("Error while sending first objRecogContour data from %x to %x (bytes %d)\n",
                         workMbx.getAdr(), objRecogContourMbxAdr, sizeof(obj_recog_data));
        }

        GDOS_DBG_DETAIL("Turn on ObjRecog(%d/%d)\n", objRecogContourSys, objRecogContourInst);
        ret = objRecogContour->on();
        if (ret)
        {
            GDOS_ERROR("Can't turn on ObjRecogBound(%i), code = %d\n",
                       objRecogContourSys, objRecogContourInst, ret);
            return ret;
        }
    }

    RackTask::disableRealtimeMode();


    // prepare tcp-socket
    GDOS_DBG_INFO("open network socket...\n");
    if ((tcpAddr.sin_addr.s_addr = inet_addr(ladarIp)) == INADDR_NONE)
    {
        GDOS_ERROR("Ip adress is not valid\n");
        return -1;
    }

    tcpAddr.sin_port   = htons((unsigned short)ladarPort);
    tcpAddr.sin_family = AF_INET;
    bzero(&(tcpAddr.sin_zero), 8);

    // open tcp-socket
    tcpSocket = -1;
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSocket == -1)
    {
        GDOS_ERROR("Can't create tcp socket, (%d)\n", errno);
        return -errno;
    }

    // set tcp-socket timeout
    recvTimeout.tv_sec  = 0;
    recvTimeout.tv_usec = 500000;   // 500ms
    ret = setsockopt(tcpSocket, SOL_SOCKET, SO_RCVTIMEO, &recvTimeout, sizeof(struct timeval));
    if (ret == -1)
    {
        GDOS_ERROR("Can't set timeout of tcp socket, (%d)\n", errno);
        return -errno;
    }

    // connect
    GDOS_DBG_INFO("Connect to network socket\n");
    ret = connect(tcpSocket, (struct sockaddr *)&tcpAddr, sizeof(tcpAddr));
    if (ret)
    {
        GDOS_ERROR("Can't connect to tcp socket, (%d)\n", errno);
        return -errno;
    }

    GDOS_DBG_INFO("Turn on ladar\n");

    RackTask::enableRealtimeMode();

    dataMbx.clean();

    if (positionInst >= 0)
    {
        ret = position->getContData(dataBufferPeriodTime, &dataMbx, NULL);
        if (ret)
        {
            GDOS_ERROR("Can't get continuous data from Position(%d/%d), "
                       "code = %d \n", positionSys, positionInst, ret);
            return ret;
        }
    }

    // init variables
    fracFactor        = pow(2, -32);
    dataRate          = 0;
    dataRateCounter   = 0;
    dataRateStartTime = rackTime.get();

    // has to be last command in moduleOn()
    return RackDataModule::moduleOn();
}

void LadarIbeoLux::moduleOff(void)
{
    RackDataModule::moduleOff(); // has to be first command in moduleOff()

    // turn off objRecogBound relay
    if (objRecogBoundInst >= 0)
    {
        objRecogBound->off();
    }

    // turn off objRecogContour relay
    if (objRecogContourInst >= 0)
    {
        objRecogContour->off();
    }

    // close tcp socket
    RackTask::disableRealtimeMode();
    if (tcpSocket != -1)
    {
        close(tcpSocket);
        tcpSocket = -1;
    }

    GDOS_DBG_INFO("Closed tcp socket\n");
    RackTask::enableRealtimeMode();
}

int  LadarIbeoLux::moduleLoop(void)
{
    int                         ret, i, j, l;
    int                         dx, dy, length;
    int                         dT;
    int                         layer, echo;
    int                         currId;
    uint32_t                    datalength         = 0;
    float                       angle;
    double                      scanStartTime;
    double                      scanEndTime;
    double                      ladarSendTime;
    rack_time_t                 recordingTime;
    RackMessage                 msgInfo;
    ladar_data                  *p_data            = NULL;
    ladar_ibeo_lux_scan_data    *ladarScanData     = NULL;
    ladar_ibeo_lux_obj_data     *ladarObjData      = NULL;
    ladar_ibeo_lux_obj          *ladarObj          = NULL;
    ladar_ibeo_lux_point2d      *ladarContourPoint = NULL;
    ladar_ibeo_lux_point2d      ladarContourPointOld;
    point_2d                    startPoint, endPoint;
    ladar_point                 point;

    if (positionInst >= 0)
    {
        // get position data
        ret = dataMbx.recvDataMsgIf(&positionData, sizeof(position_data), &msgInfo);
        if (!ret)
        {
            if ((msgInfo.getType() != MSG_DATA)
                || (msgInfo.getSrc() != position->getDestAdr()))
            {
                GDOS_ERROR("Received unexpected message from %n to %n type %d on "
                           "data mailbox\n", msgInfo.getSrc(), msgInfo.getDest(),
                           msgInfo.getType());

                return -EINVAL;
            }

            PositionData::parse(&msgInfo); 
        }
        else if (ret != -EWOULDBLOCK)
        {
            GDOS_ERROR("Can't receive position data on DATA_MBX, "
                       "code = %d \n", ret);
            return ret;
        }
    }

    // get datapointer from rackdatabuffer
    p_data = (ladar_data *)getDataBufferWorkSpace();

    RackTask::disableRealtimeMode();

    // read ladar header
    ret = recvLadarHeader(&ladarHeader, &recordingTime);
    if (ret)
    {
        return ret;
    }

    // calc current data rate
    dataRateCounter += ladarHeader.messageSize + 24;
    dT               = (int)recordingTime - (int)dataRateStartTime;
    if (dT > 1000)
    {
        dataRate          = dataRateCounter * 1000 / dT;
        dataRateStartTime = recordingTime;
        dataRateCounter   = 0;
    }

    GDOS_DBG_DETAIL("time %d, ladar dataType %x, messageSize %d, dataRate %d bytes/s\n",
                    recordingTime, ladarHeader.dataType, ladarHeader.messageSize, dataRate);

    // read ladar data body
    ret = recvLadarData(&ladarData, ladarHeader.messageSize);

   RackTask::enableRealtimeMode();

    // parse ladar data body
    switch (ladarHeader.dataType)
    {
        // scan data
        case LADAR_IBEO_LUX_SCAN_DATA:
            ladarScanData = (ladar_ibeo_lux_scan_data *)&ladarData;

            // calculate ladar timestamps (unit seconds)
            scanStartTime = (double)ladarScanData->scanStartTime.secondsFrac +
                                    fracFactor * (double)ladarScanData->scanStartTime.seconds;
            scanEndTime   = (double)ladarScanData->scanEndTime.secondsFrac +
                                    fracFactor * (double)ladarScanData->scanEndTime.seconds;
            ladarSendTime = (double)ladarHeader.ntpTime.seconds +
                                    fracFactor * (double)ladarHeader.ntpTime.secondsFrac;

            // recalculate recordingtime to the center of the scan
            p_data->recordingTime   = recordingTime -
                                        (int)rint((ladarSendTime - scanEndTime +
                                                 ((scanEndTime - scanStartTime) / 2.0)) * 1000.0);

            // reset pointNum for each layer buffer
            for (i = 0; i < LADAR_IBEO_LUX_LAYER_MAX; i++)
            {
                ladarWorkMsg[i].data.pointNum = 0;
            }

            // store scanpoints in layer buffer
            for (i = 0; i < ladarScanData->scanPoints; i++)
            {
                point.angle    = -(2.0 * M_PI * ladarScanData->point[i].angle) /
                                 ladarScanData->angleTicksPerRot;
                point.distance = ladarScanData->point[i].distance * 10;

                // set ladar type
                point.intensity = 0;
                point.type      = LADAR_POINT_TYPE_INVALID;

                if ((ladarScanData->point[i].flags & 0x10) == 0x10)     // object
                {
                    point.type = LADAR_POINT_TYPE_UNKNOWN;
                }
                if ((ladarScanData->point[i].flags & 0x08) == 0x08)     // dirt
                {
                    point.type = LADAR_POINT_TYPE_DIRT;
                }
                if ((ladarScanData->point[i].flags & 0x04) == 0x04)     // ground
                {
                    point.type = LADAR_POINT_TYPE_INVALID;
                }
                if ((ladarScanData->point[i].flags & 0x02) == 0x02)     // rain
                {
                    point.type = LADAR_POINT_TYPE_RAIN;
                }
                if ((ladarScanData->point[i].flags & 0x01) == 0x01)     // transparent
                {
                    point.type = LADAR_POINT_TYPE_TRANSPARENT;
                }
                if (ladarScanData->point[i].flags == 0)     // object
                {
                    point.type = LADAR_POINT_TYPE_UNKNOWN;
                }

                layer =   (ladarScanData->point[i].layerEcho & 0x0F);
                echo  =  ((ladarScanData->point[i].layerEcho & 0xF0) >> 4);
                currId = ladarWorkMsg[layer].data.pointNum;

                // distance filter activated and at least one point stored in this layer
                if ((distanceFilter == 1) && (currId > 0))
                {
                    // check if angle of current point is equal to previous point
                    if (point.angle == ladarWorkMsg[layer].point[currId - 1].angle)
                    {
                        // consider only object measurements
                        if (point.type == 0x00)
                        {
                            // replace previous point if current distance is smaller or if
                            // previous point was not an object measurement
                            if ((point.distance < ladarWorkMsg[layer].point[currId - 1].distance) ||
                                (ladarWorkMsg[layer].point[currId - 1].type != 0x00))
                            {
                                memcpy(&ladarWorkMsg[layer].point[currId - 1], &point, sizeof(ladar_point));
                            }
                        }
                    }

                    // new angle point
                    else
                    {
                        memcpy(&ladarWorkMsg[layer].point[currId], &point, sizeof(ladar_point));
                        ladarWorkMsg[layer].data.pointNum++;
                    }
                }
                else
                {
                    memcpy(&ladarWorkMsg[layer].point[currId], &point, sizeof(ladar_point));
                    ladarWorkMsg[layer].data.pointNum++;
                }
            }

            // create ladar data output
            p_data->duration        = (int)rint((scanEndTime - scanStartTime) * 1000.0);
            p_data->maxRange        = LADAR_IBEO_LUX_MAX_RANGE;
            p_data->startAngle      = -(2.0 * M_PI * ladarScanData->startAngle) /
                                        ladarScanData->angleTicksPerRot;
            p_data->endAngle        = -(2.0 * M_PI * ladarScanData->endAngle) /
                                        ladarScanData->angleTicksPerRot;
            p_data->pointNum        = 0;

            // concatenate layer data
            for (i = 0; i < LADAR_IBEO_LUX_LAYER_MAX; i++)
            {
                if (ladarWorkMsg[i].data.pointNum != 0)
                {
                    memcpy(&p_data->point[p_data->pointNum], &ladarWorkMsg[i].point[0],
                           ladarWorkMsg[i].data.pointNum * sizeof(ladar_point));
                    p_data->pointNum += ladarWorkMsg[i].data.pointNum;
                }
            }

            GDOS_DBG_DETAIL("Data recordingtime %i pointNum %i\n",
                            p_data->recordingTime, p_data->pointNum);
            datalength = sizeof(ladar_data) + sizeof(ladar_point) * p_data->pointNum;
            putDataBufferWorkSpace(datalength);
            break;

        // object data
        case LADAR_IBEO_LUX_OBJ_DATA:
            ladarObjData = (ladar_ibeo_lux_obj_data *)&ladarData;
            l            = sizeof(ladar_ibeo_lux_obj_data);

            // calculate ladar timestamps (unit seconds)
            scanStartTime = (double)ladarObjData->timestamp.seconds +
                                    fracFactor * (double)ladarObjData->timestamp.secondsFrac;
            ladarSendTime = (double)ladarHeader.ntpTime.seconds +
                                    fracFactor * (double)ladarHeader.ntpTime.secondsFrac;

            // bounding-box data
            // recalculate recordingtime to the beginning of the scan
            objRecogBoundData.data.recordingTime = recordingTime;/* -
                                                   (int)rint((ladarSendTime - scanStartTime) * 1000);*/
            memcpy(&objRecogBoundData.data.refPos, &positionData.pos, sizeof(position_3d));
            memcpy(&objRecogBoundData.data.varRefPos, &positionData.var, sizeof(position_3d));
            objRecogBoundData.data.objectNum     = 0;

            // contour data
            // recalculate recordingtime to the beginning of the scan
            objRecogContourData.data.recordingTime = recordingTime;/* -
                                                     (int)rint((ladarSendTime - scanStartTime) * 1000);*/
            memcpy(&objRecogContourData.data.refPos, &positionData.pos, sizeof(position_3d));
            memcpy(&objRecogContourData.data.varRefPos, &positionData.var, sizeof(position_3d));
            objRecogContourData.data.objectNum     = 0;

            // loop over all objects
            for (i = 0; i < ladarObjData->objNum; i++)
            {
                ladarObj = (ladar_ibeo_lux_obj *)&ladarData[l];
                l       += sizeof(ladar_ibeo_lux_obj);

                // bounding-box data
                if (objRecogBoundData.data.objectNum < OBJ_RECOG_OBJECT_MAX)
                {
                    objRecogBoundData.data.objectNum++;
                    objRecogBoundData.object[i].objectId =  ladarObj->id;
                    objRecogBoundData.object[i].type     =  (int32_t)ladarObj->classification;
                    objRecogBoundData.object[i].pos.x    =  ladarObj->objBoxCenter.x * 10;
                    objRecogBoundData.object[i].pos.y    = -ladarObj->objBoxCenter.y * 10;
                    objRecogBoundData.object[i].pos.z    =  0;
                    objRecogBoundData.object[i].pos.phi  =  0.0f;
                    objRecogBoundData.object[i].pos.psi  =  0.0f;
                    objRecogBoundData.object[i].pos.rho  = -(float)ladarObj->objBoxOrientation *
                                                                   M_PI / (180.0 * 32.0);
                    objRecogBoundData.object[i].varPos.x   = 0;
                    objRecogBoundData.object[i].varPos.y   = 0;
                    objRecogBoundData.object[i].varPos.z   = 0;
                    objRecogBoundData.object[i].varPos.phi = 0.f;
                    objRecogBoundData.object[i].varPos.psi = 0.f;
                    objRecogBoundData.object[i].varPos.rho = 0.f;

                    // velocity output with absolute velocities
                    if (velocityMode == 1)
                    {
                        if ((ladarObj->velAbs.x & 0xffff) != 0x8000)
                        {
                            objRecogBoundData.object[i].vel.x =  ladarObj->velAbs.x * 10;
                        }
                        else
                        {
                            objRecogBoundData.object[i].vel.x = 0;
                        }

                        if ((ladarObj->velAbs.y & 0xffff) != 0x8000)
                        {
                            objRecogBoundData.object[i].vel.y    = -ladarObj->velAbs.y * 10;
                        }
                        else
                        {
                            objRecogBoundData.object[i].vel.y = 0;
                        }
                    }

                    // velocity output with relative velocities
                    else
                    {
                        objRecogBoundData.object[i].vel.x =  ladarObj->velRel.x * 10;
                        objRecogBoundData.object[i].vel.y =  ladarObj->velRel.y * 10;
                    }

                    objRecogBoundData.object[i].vel.z    =  0;
                    objRecogBoundData.object[i].vel.phi  =  0.0f;
                    objRecogBoundData.object[i].vel.psi  =  0.0f;
                    objRecogBoundData.object[i].vel.rho  =  0.0f;
                    objRecogBoundData.object[i].varVel.x   = 0;
                    objRecogBoundData.object[i].varVel.y   = 0;
                    objRecogBoundData.object[i].varVel.z   = 0;
                    objRecogBoundData.object[i].varVel.phi = 0.f;
                    objRecogBoundData.object[i].varVel.psi = 0.f;
                    objRecogBoundData.object[i].varVel.rho = 0.f;
                    objRecogBoundData.object[i].dim.x    =  ladarObj->objBoxSize.x * 10;
                    objRecogBoundData.object[i].dim.y    =  ladarObj->objBoxSize.y * 10;
                    objRecogBoundData.object[i].dim.z    =  0;
                    objRecogBoundData.object[i].prob     =  0.0f;
                    objRecogBoundData.object[i].imageArea.x      = 0;
                    objRecogBoundData.object[i].imageArea.y      = 0;
                    objRecogBoundData.object[i].imageArea.width  = 0;
                    objRecogBoundData.object[i].imageArea.height = 0;
                }

                // Ibeo Lux bug: sends max number of contour points in some special environments
                // (e. g. stacked fences of grid), capture to avoid segmentation fault
                if (ladarObj->contourPointNum == 0xFFFF)
                {
                    GDOS_WARNING("Ibeo Lux bug: max number of contour points exceeded.");
                    continue;
                }

                // contour data
                for (j = 0; j < ladarObj->contourPointNum; j++)
                {
                    ladarContourPoint = (ladar_ibeo_lux_point2d *)&ladarData[l];
                    l                += sizeof(ladar_ibeo_lux_point2d);

                    if (j >= 1)
                    {
                        if (objRecogContourData.data.objectNum < OBJ_RECOG_OBJECT_MAX)
                        {
                            startPoint.x =  ladarContourPointOld.x * 10;
                            startPoint.y = -ladarContourPointOld.y * 10;
                            endPoint.x   =  ladarContourPoint->x * 10;
                            endPoint.y   = -ladarContourPoint->y * 10;

                            dx     = endPoint.x - startPoint.x;
                            dy     = endPoint.y - startPoint.y;
                            angle  = atan2((double)dy, (double)dx);
                            length = (int)sqrt((double)dx * (double)dx + (double)dy * (double)dy);

                            objRecogContourData.data.objectNum++;
                            objRecogContourData.object[j].objectId =  ladarObj->id;
                            objRecogContourData.object[j].type     =  (int32_t)ladarObj->classification;
                            objRecogContourData.object[j].pos.x    =  (startPoint.x + endPoint.x) / 2;
                            objRecogContourData.object[j].pos.y    =  (startPoint.y + endPoint.y) / 2;
                            objRecogContourData.object[j].pos.z    =  0;
                            objRecogContourData.object[j].pos.phi  =  0.0f;
                            objRecogContourData.object[j].pos.psi  =  0.0f;
                            objRecogContourData.object[j].pos.rho  =  angle;
                            objRecogContourData.object[j].varPos.x   =  0;
                            objRecogContourData.object[j].varPos.y   =  0;
                            objRecogContourData.object[j].varPos.z   =  0;
                            objRecogContourData.object[j].varPos.phi =  0.f;
                            objRecogContourData.object[j].varPos.psi =  0.f;
                            objRecogContourData.object[j].varPos.rho =  0.f;
                            objRecogContourData.object[j].vel.x    =  ladarObj->velRel.x * 10;
                            objRecogContourData.object[j].vel.y    = -ladarObj->velRel.y * 10;
                            objRecogContourData.object[j].vel.z    =  0;
                            objRecogContourData.object[j].vel.phi  =  0.0f;
                            objRecogContourData.object[j].vel.psi  =  0.0f;
                            objRecogContourData.object[j].vel.rho  =  0.0f;
                            objRecogContourData.object[j].varVel.x   =  0;
                            objRecogContourData.object[j].varVel.y   =  0;
                            objRecogContourData.object[j].varVel.z   =  0;
                            objRecogContourData.object[j].varVel.phi =  0.f;
                            objRecogContourData.object[j].varVel.psi =  0.f;
                            objRecogContourData.object[j].varVel.rho =  0.f;
                            objRecogContourData.object[j].dim.x    =  length;
                            objRecogContourData.object[j].dim.y    =  0;
                            objRecogContourData.object[j].dim.z    =  0;
                            objRecogContourData.object[j].prob     =  0.0f;
                            objRecogContourData.object[j].imageArea.x      = 0;
                            objRecogContourData.object[j].imageArea.y      = 0;
                            objRecogContourData.object[j].imageArea.width  = 0;
                            objRecogContourData.object[j].imageArea.height = 0;
                        }
                    }

                    memcpy(&ladarContourPointOld, ladarContourPoint, sizeof(ladar_ibeo_lux_point2d));
                }
            }

            // send bounding-box data
            if (objRecogBoundInst >= 0)
            {
                GDOS_DBG_DETAIL("ObjRecogBoundData recordingtime %i objectNum %i\n",
                                objRecogBoundData.data.recordingTime,
                                objRecogBoundData.data.objectNum);
                datalength =  sizeof(obj_recog_data) +
                              objRecogBoundData.data.objectNum * sizeof(obj_recog_object);

                ret = workMbx.sendDataMsg(MSG_DATA, objRecogBoundMbxAdr + 1, 1, 1,
                                         &objRecogBoundData, datalength);
                if (ret)
                {
                    GDOS_ERROR("Error while sending objRecogBound data from %x to %x (bytes %d)\n",
                               workMbx.getAdr(), objRecogBoundMbxAdr, datalength);
                    return ret;
                }
            }

            // send contour data
            if (objRecogContourInst >= 0)
            {
                GDOS_DBG_DETAIL("ObjRecogContourData recordingtime %i objectNum %i\n",
                                objRecogContourData.data.recordingTime,
                                objRecogContourData.data.objectNum);
                datalength =  sizeof(obj_recog_data) +
                              objRecogContourData.data.objectNum * sizeof(obj_recog_object);

                ret = workMbx.sendDataMsg(MSG_DATA, objRecogContourMbxAdr + 1, 1, 1,
                                         &objRecogContourData, datalength);
                if (ret)
                {
                    GDOS_ERROR("Error while sending objRecogContour data from %x to %x (bytes %d)\n",
                               workMbx.getAdr(), objRecogContourMbxAdr, datalength);
                    return ret;
                }
            }
            break;
    }

    return 0;
}

int  LadarIbeoLux::moduleCommand(RackMessage *msgInfo)
{
    // not for me -> ask RackDataModule
    return RackDataModule::moduleCommand(msgInfo);
}


int LadarIbeoLux::recvLadarHeader(ladar_ibeo_lux_header *data, rack_time_t *recordingTime)
{
    int             ret;
    unsigned int    len, i;
    uint32_t        retryNumMax = LADAR_IBEO_LUX_MESSAGE_SIZE_MAX;
    uint32_t        magicWord = 0;
    char            *magicWordBuf = (char *)&magicWord;

    // synchronize to header by reading the magic word (4 bytes)
    len = 0;
    while (len < sizeof(magicWord))
    {
        ret = recv(tcpSocket, magicWordBuf + len, sizeof(magicWord) - len, 0);
        if (ret == -1)          // error
        {
            if (errno == EAGAIN)
            {
                GDOS_ERROR("Timeout on receiving ladar header, (%d)\n", errno);
            }
            else
            {
                GDOS_ERROR("Can't receive header magic word, (%d)\n", errno);
            }
            return -errno;
        }
        if (ret == 0)           // socket closed
        {
            GDOS_ERROR("Tcp socket closed\n");
            return -1;
        }
        len += ret;
    }

    for (i = 0; i < retryNumMax; i++)
    {
        // search for magic word
        if (magicWord == LADAR_IBEO_LUX_MAGIC_WORD)
        {
            *recordingTime = rackTime.get();
            break;
        }
        magicWordBuf[0] = magicWordBuf[1];
        magicWordBuf[1] = magicWordBuf[2];
        magicWordBuf[2] = magicWordBuf[3];
        ret = recv(tcpSocket, magicWordBuf + 3, 1, 0);
        if (ret == -1)          // error
        {
            if (errno == EAGAIN)
            {
                GDOS_ERROR("Timeout on receiving ladar header, (%d)\n", errno);
            }
            else
            {
                GDOS_ERROR("Can't receive header magic word, (%d)\n", errno);
            }
            return -errno;
        }
        if (ret == 0)           // socket closed
        {
            GDOS_ERROR("Tcp socket closed\n");
            return -1;
        }
    }

    // synchronization timeout
    if (i == retryNumMax)
    {
        GDOS_ERROR("Can't synchronize to ladar header, timeout after %d attempts\n", i);
        return -ETIMEDOUT;
    }

    // receive header data
    len = 0;
    while (len < sizeof(ladar_ibeo_lux_header))
    {
        ret = recv(tcpSocket, (char*)data + len, sizeof(ladar_ibeo_lux_header) - len, 0);
        if (ret == -1)              // error
        {
            if (errno == EAGAIN)
            {
                GDOS_ERROR("Timeout on receiving ladar header, (%d)\n", errno);
            }
            else
            {
                GDOS_ERROR("Can't receive ladar header, (%d)\n", errno);
            }
            return -errno;
        }
        if (ret == 0)               // socket closed
        {
            GDOS_ERROR("Tcp socket closed\n");
            return -1;
        }
        len += ret;
    }

    // adjust byte order
    parseLadarIbeoLuxHeader(data);

    return 0;
}

int LadarIbeoLux::recvLadarData(void *data, unsigned int messageSize)
{
    int          ret;
    unsigned int len;

    // receive ladar data
    len = 0;
    while (len < messageSize)
    {
        ret = recv(tcpSocket, (char*)data + len, messageSize - len, 0);
        if (ret == -1)              // error
        {
            if (errno == EAGAIN)
            {
                GDOS_ERROR("Timeout on receiving ladar data, (%d)\n", errno);
            }
            else
            {
                GDOS_ERROR("Can't receive ladar data, (%d)\n", errno);
            }
            return -errno;
        }
        if (ret == 0)               // socket closed
        {
            GDOS_ERROR("Tcp socket closed\n");
            return -1;
        }
        len += ret;
    }

    return 0;
}

/*****************************************************************************
 *   !!! NON REALTIME CONTEXT !!!
 *
 *   moduleInit,
 *   moduleCleanup,
 *   Constructor,
 *   Destructor,
 *   main,
 *
 *   own non realtime user functions
 ****************************************************************************/

// init_flags (for init and cleanup)
#define INIT_BIT_DATA_MODULE                0
#define INIT_BIT_MBX_WORK                   1
#define INIT_BIT_MBX_DATA                   2
#define INIT_BIT_PROXY_POSITION             3
#define INIT_BIT_PROXY_OBJ_RECOG_BOUND      4
#define INIT_BIT_PROXY_OBJ_RECOG_CONTOUR    5

int LadarIbeoLux::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    // create mailbox
    ret = createMbx(&workMbx, 10, sizeof(obj_recog_data_msg),
                    MBX_IN_USERSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    ret = createMbx(&dataMbx, 10, sizeof(position_data),
                    MBX_IN_USERSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_DATA);

    // create Position Proxy
    if (positionInst >= 0)
    {
        position = new PositionProxy(&workMbx, positionSys, positionInst);
        if (!position)
        {
            ret = -ENOMEM;
            goto init_error;
        }
        initBits.setBit(INIT_BIT_PROXY_POSITION);
    }

    // create objRecogBound proxy
    if (objRecogBoundInst >= 0)
    {
        objRecogBound       = new ObjRecogProxy(&workMbx, objRecogBoundSys, objRecogBoundInst);
        if (!objRecogBound)
        {
            ret = -ENOMEM;
            goto init_error;
        }
        initBits.setBit(INIT_BIT_PROXY_OBJ_RECOG_BOUND);
    }

    // create objRecogContour proxy
    if (objRecogContourInst >= 0)
    {
        objRecogContourMbxAdr = RackName::create(objRecogContourSys, OBJ_RECOG, objRecogContourInst);
        objRecogContour       = new ObjRecogProxy(&workMbx, objRecogContourSys, objRecogContourInst);
        if (!objRecogContour)
        {
            ret = -ENOMEM;
            goto init_error;
        }
        initBits.setBit(INIT_BIT_PROXY_OBJ_RECOG_CONTOUR);
    }

    return 0;

init_error:
    LadarIbeoLux::moduleCleanup();
    return ret;
}

// non realtime context
void LadarIbeoLux::moduleCleanup(void)
{
    // call RackDataModule cleanup function (last command in cleanup)
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    // destroy position proxy
    if (initBits.testAndClearBit(INIT_BIT_PROXY_POSITION))
    {
        delete position;
    }

    // destroy objRecogContour proxy
    if (objRecogContourInst >= 0)
    {
        if (initBits.testAndClearBit(INIT_BIT_PROXY_OBJ_RECOG_CONTOUR))
        {
            delete objRecogContour;
        }
    }

    // destroy objRecogBound proxy
    if (objRecogBoundInst >= 0)
    {
        if (initBits.testAndClearBit(INIT_BIT_PROXY_OBJ_RECOG_BOUND))
        {
            delete objRecogBound;
        }
    }

    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }
}

LadarIbeoLux::LadarIbeoLux()
      : RackDataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s datatask error sleep time
                    16,               // command mailbox slots
                    48,               // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    5,                // max buffer entries
                    10)               // data buffer listener
{
    // get static module parameter
    positionSys         = getIntArg("positionSys", argTab);
    positionInst        = getIntArg("positionInst", argTab);
    objRecogBoundSys    = getIntArg("objRecogBoundSys", argTab);
    objRecogBoundInst   = getIntArg("objRecogBoundInst", argTab);
    objRecogContourSys  = getIntArg("objRecogContourSys", argTab);
    objRecogContourInst = getIntArg("objRecogContourInst", argTab);

    objRecogBoundMbxAdr = RackName::create(objRecogBoundSys, OBJ_RECOG, objRecogBoundInst);

    dataBufferMaxDataSize   = sizeof(ladar_data_msg);
    dataBufferPeriodTime    = 80; // 80 ms (12.5 per sec)
}

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "LadarIbeoLux");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    LadarIbeoLux *pInst;

    // create new LadarIbeoLux
    pInst = new LadarIbeoLux();
    if (!pInst)
    {
        printf("Can't create new LadarIbeoLux -> EXIT\n");
        return -ENOMEM;
    }

    // init
    ret = pInst->moduleInit();
    if (ret)
        goto exit_error;

    pInst->run();

    return 0;

exit_error:

    delete (pInst);
    return ret;
}
