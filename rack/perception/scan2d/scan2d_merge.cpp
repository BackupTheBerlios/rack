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
#include "scan2d_merge.h"

//
// data structures
//

arg_table_t argTab[] = {

    { ARGOPT_OPT, "odometrySys", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of the odometry module", { 0 } },

    { ARGOPT_OPT, "odometryInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the odometry module", { 0 } },

    { ARGOPT_OPT, "positionSys", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of the position module", { 0 } },

    { ARGOPT_OPT, "positionInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the position module", { -1 } },

    { ARGOPT_OPT, "scan2dSysA", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of a scan2d module", { 0 } },

    { ARGOPT_REQ, "scan2dInstA", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of a scan2d module", { -1 } },

    { ARGOPT_OPT, "scan2dSysB", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of a scan2d module", { 0 } },

    { ARGOPT_OPT, "scan2dInstB", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of a scan2d module", { -1 } },

    { ARGOPT_OPT, "scan2dSysC", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of a scan2d module", { 0 } },

    { ARGOPT_OPT, "scan2dInstC", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of a scan2d module", { -1 } },

    { ARGOPT_OPT, "scan2dSysD", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of a scan2d module", { 0 } },

    { ARGOPT_OPT, "scan2dInstD", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of a scan2d module", { -1 } },

    { 0, "", 0, 0, "", { 0 } } // last entry
};


/*******************************************************************************
 *   !!! REALTIME CONTEXT !!!
 *
 *   moduleOn,
 *   moduleOff,
 *   moduleLoop,
 *   moduleCommand,
 *   initDataBuffer
 *
 *   own realtime user functions
 ******************************************************************************/
int  Scan2dMerge::moduleOn(void)
{
    int ret, k, i;

    // turn on odometry
    GDOS_DBG_INFO("Turn on Odometry(%d/%d)\n", odometrySys, odometryInst);
    ret = odometry->on();
    if (ret)
    {
        GDOS_ERROR("Can't turn on Odometry(%d/%d), code = %d\n",
                   odometrySys, odometryInst, ret);
        return ret;
    }

    // turn on position if required
    if (positionInst >= 0)
    {
        GDOS_DBG_INFO("Turn on Position(%d/%d)\n", positionSys, positionInst);
        ret = position->on();
        if (ret)
        {
            GDOS_ERROR("Can't turn on Position(%d/%d), code = %d\n",
                       positionSys, positionInst, ret);
            return ret;
        }
    }

    // turn on scan2d modules
    for (k = 0; k < SCAN2D_SENSOR_NUM_MAX; k++)
    {
        if (scan2dInst[k] >= 0)
        {
            GDOS_DBG_INFO("Turn on Scan2d(%d/%d)\n", scan2dSys[k], scan2dInst[k]);
            ret = scan2d[k]->on();
            if (ret)
            {
                GDOS_ERROR("Can't turn on Scan2d(%i/%i), code = %d\n",
                           scan2dSys[k], scan2dInst[k], ret);
                return ret;
            }
        }
    }


    // get continuous data from odometry
    ret = odometry->getContData(0, &dataMbx, &dataBufferPeriodTime);
    if (ret)
    {
        GDOS_ERROR("Can't get continuous data from Odometry(%i/%i), code = %d\n",
                   odometrySys, odometryInst, ret);
        return ret;
    }

    // get continuous data from scan2d modules
    for (k = 0; k < SCAN2D_SENSOR_NUM_MAX; k++)
    {
        if (scan2dInst[k] >= 0)
        {
            ret = scan2d[k]->getContData(0, &dataMbx, NULL);
            if (ret)
            {
                GDOS_ERROR("Can't get continuous data from Scan2d(%i/%i), "
                           "code = %d\n", scan2dSys[k], scan2dInst[k], ret);
                return ret;
            }
        }

        for (i = 0; i < SCAN2D_SECTOR_NUM_MAX; i++)
        {
            scanBuffer[k][i].data.pointNum = 0;
        }
        scan2dTimeout[k]            = 0;
    }

    return RackDataModule::moduleOn();  // has to be last command in moduleOn();
}

void Scan2dMerge::moduleOff(void)
{
    int     k;

    RackDataModule::moduleOff();        // has to be first command in moduleOff();

    odometry->stopContData(&dataMbx);

    for (k = 0; k < SCAN2D_SENSOR_NUM_MAX; k++)
    {
        if (scan2dInst[k] >= 0)
        {
            scan2d[k]->stopContData(&dataMbx);
        }
    }
}


// realtime context
int  Scan2dMerge::moduleLoop(void)
{
    RackMessage     dataInfo;
    odometry_data   *odoData   = NULL;
    scan2d_data     *scanData  = NULL;
    scan2d_data     *mergeData = NULL;
    int             ret;
    int             i, j, k, l;
    int             x, y;
    int             posDiffX, posDiffY;
    double          sinRho, cosRho;
    int             curSector;

    // receive data
    ret = dataMbx.peekTimed(1000000000llu, &dataInfo); // 1s
    if (ret)
    {
        GDOS_ERROR("Can't receive data on MBX, code = %d\n", ret);
        return ret;
    }

    if (dataInfo.getType() == MSG_DATA)
    {
        // store new scan2d data message from scan2dInst 0, 1, 2, ...
        for (k = 0; k < SCAN2D_SENSOR_NUM_MAX; k++)
        {
            if (scan2dInst[k] >= 0)
            {
                // message received
                if ((dataInfo.getSrc() == RackName::create(scan2dSys[k], SCAN2D, scan2dInst[k])) &&
                    (dataInfo.getType() == MSG_DATA))
                {
                    // message parsing
                    scanData = Scan2dData::parse(&dataInfo);

                    if (scanData->sectorNum > SCAN2D_SECTOR_NUM_MAX)
                    {
                        GDOS_ERROR("Sector num exceeds SCAN2D_SECTOR_NUM_MAX %i\n", SCAN2D_SECTOR_NUM_MAX);

                        dataMbx.peekEnd();
                        return -EOVERFLOW;
                    }
                    scan2dSectorNum[k] = scanData->sectorNum;
                    curSector = scanData->sectorIndex;

                    ret = odometry->getData(&odometryBuffer[k][curSector],
                                            sizeof(odometry_data),
                                            scanData->recordingTime);
                    if (ret)
                    {
                        GDOS_ERROR("Can't get data from Odometry(%i/%i), code = %d\n",
                                   odometrySys, odometryInst, ret);
                        dataMbx.peekEnd();
                        return ret;
                    }

                    sinRho = sin(odometryBuffer[k][curSector].pos.rho);
                    cosRho = cos(odometryBuffer[k][curSector].pos.rho);

                    j = 0;

                    for (i = 0; i < scanData->pointNum; i++)
                    {
                        scanBuffer[k][curSector].point[j].x  = (int)(scanData->point[i].x *
                                                          cosRho) -
                                                    (int)(scanData->point[i].y *
                                                          sinRho);
                        scanBuffer[k][curSector].point[j].y  = (int)(scanData->point[i].x *
                                                          sinRho) +
                                                    (int)(scanData->point[i].y *
                                                          cosRho);
                        scanBuffer[k][curSector].point[j].z  = scanData->point[i].z;
                        scanBuffer[k][curSector].point[j].type      = scanData->point[i].type;
                        scanBuffer[k][curSector].point[j].segment   = scanData->point[i].segment;
                        scanBuffer[k][curSector].point[j].intensity = scanData->point[i].intensity;

                        j++;
                    }

                    scanBuffer[k][curSector].data.recordingTime = scanData->recordingTime;
                    scanBuffer[k][curSector].data.duration      = scanData->duration;
                    scanBuffer[k][curSector].data.maxRange      = scanData->maxRange;
                    scanBuffer[k][curSector].data.pointNum      = j;
                    scan2dTimeout[k] = 0;

                    GDOS_DBG_DETAIL("Buffer Scan2D(%i/%i) recordingtime %i "
                                    "pointNum %i x %i y %i\n", scan2dSys[k], scan2dInst[k],
                                    scanData->recordingTime,
                                    scanData->pointNum,
                                    odometryBuffer[k][curSector].pos.x,
                                    odometryBuffer[k][curSector].pos.y);
                }
            }
        }


        // new odometry data, build new scan2d_data_msg
        if ((dataInfo.getSrc() == RackName::create(odometrySys, ODOMETRY, odometryInst)) &&
            (dataInfo.getType() == MSG_DATA))
        {
            odoData = OdometryData::parse(&dataInfo);

            // get datapointer from rackdatabuffer
            mergeData = (scan2d_data *)getDataBufferWorkSpace();

            mergeData->recordingTime = odoData->recordingTime;
            mergeData->duration      = dataBufferPeriodTime;
            mergeData->maxRange      = scanBuffer[0][0].data.maxRange;
            mergeData->sectorNum     = 1;
            mergeData->sectorIndex   = 0;
            mergeData->pointNum      = 0;

            // reference position
            if (positionInst >= 0)
            {
                ret = position->getData(&positionData, sizeof(position_data),
                                        mergeData->recordingTime);
                if (ret)
                {
                    GDOS_ERROR("Can't get data from Position(%i/%i), code = %d\n",
                               positionSys, positionInst, ret);
                    dataMbx.peekEnd();
                    return ret;
                }
            }
            else
            {
                memset(&positionData, 0, sizeof(position_data));
            }
            mergeData->refPos.x   = positionData.pos.x;
            mergeData->refPos.y   = positionData.pos.y;
            mergeData->refPos.z   = positionData.pos.z;
            mergeData->refPos.phi = positionData.pos.phi;
            mergeData->refPos.psi = positionData.pos.psi;
            mergeData->refPos.rho = positionData.pos.rho;

            // scan points
            for (k = 0; k < SCAN2D_SENSOR_NUM_MAX; k++)
            {
                if (scan2dInst[k] >= 0)
                {
                    // scan2d timeout 5s
                    if (scan2dTimeout[k] > 100 * ((float)1000.0f / dataBufferPeriodTime))
                    {
                        GDOS_PRINT("dataBufferPeriodTIme %d\n", dataBufferPeriodTime);
                        GDOS_ERROR("Data timeout Scan2d(%i/%i)\n", scan2dSys[k], scan2dInst[k]);

                        dataMbx.peekEnd();
                        return -ETIMEDOUT;
                    }
                    scan2dTimeout[k]++;

                    sinRho = sin(odoData->pos.rho);
                    cosRho = cos(odoData->pos.rho);

                    for (l = 0; l < scan2dSectorNum[k]; l++)
                    {
                        for (i = 0; i < scanBuffer[k][l].data.pointNum; i++)
                        {
                            if (mergeData->pointNum >= SCAN2D_POINT_MAX)
                            {
                                GDOS_ERROR("Merged scan exceeds SCAN2D_POINT_MAX %i\n", SCAN2D_POINT_MAX);

                                for (k = 0; k < SCAN2D_SENSOR_NUM_MAX; k++)
                                {
                                    if (scan2dInst[k] >= 0)
                                    {
                                        GDOS_WARNING("Scan2d(%i/%i) pointNum %i",
                                                     scan2dSys[k], scan2dInst[k], scanBuffer[k][l].data.pointNum);
                                    }
                                }
                                dataMbx.peekEnd();
                                return -EOVERFLOW;
                            }

                            j = mergeData->pointNum;

                            posDiffX = odometryBuffer[k][l].pos.x - odoData->pos.x;
                            posDiffY = odometryBuffer[k][l].pos.y - odoData->pos.y;

                            x = scanBuffer[k][l].point[i].x + posDiffX;
                            y = scanBuffer[k][l].point[i].y + posDiffY;

                            mergeData->point[j].x         =   (int)(x * cosRho)
                                                            + (int)(y * sinRho);
                            mergeData->point[j].y         = - (int)(x * sinRho)
                                                            + (int)(y * cosRho);
                            mergeData->point[j].z         =   (int)sqrt(mergeData->point[j].x * mergeData->point[j].x +
                                                                        mergeData->point[j].y * mergeData->point[j].y);
                            mergeData->point[j].type      = scanBuffer[k][l].point[i].type;
                            mergeData->point[j].segment   = scanBuffer[k][l].point[i].segment;
                            mergeData->point[j].intensity = scanBuffer[k][l].point[i].intensity;
                            mergeData->pointNum++;
                        }
                    }
                }
            }

            GDOS_DBG_DETAIL("recordingtime %i pointNum %i x %i y %i\n",
                            mergeData->recordingTime, mergeData->pointNum,
                            odometryBuffer[0][0].pos.x, odometryBuffer[0][0].pos.y);

            putDataBufferWorkSpace(Scan2dData::getDatalen(mergeData));
        }
    }
    else
    {
        GDOS_ERROR("Received unexpected message from %n to %n type %d on data mailbox\n",
                   dataInfo.getSrc(), dataInfo.getDest(), dataInfo.getType());
        dataMbx.peekEnd();
        return -EINVAL;
    }

    dataMbx.peekEnd();

    return 0;
}

int  Scan2dMerge::moduleCommand(RackMessage *msgInfo)
{
    // not for me -> ask RackDataModule
    return RackDataModule::moduleCommand(msgInfo);
}

/*******************************************************************************
 *   !!! NON REALTIME CONTEXT !!!
 *
 *   own non realtime user functions,
 *   moduleInit,
 *   moduleCleanup,
 *   Constructor,
 *   Destructor,
 *   main
 *
 ******************************************************************************/

// init_flags
#define INIT_BIT_DATA_MODULE        0
#define INIT_BIT_MBX_WORK           1
#define INIT_BIT_MBX_DATA           2
#define INIT_BIT_PROXY_POSITION     4
#define INIT_BIT_PROXY_ODOMETRY     5
#define INIT_BIT_PROXY_SCAN2D       6       // has to be highest bit!

int Scan2dMerge::moduleInit(void)
{
    int     ret, k;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    //
    // create mailboxes
    //

    // work Mbx
    ret = createMbx(&workMbx, 1, 128, MBX_IN_KERNELSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    // data Mbx
    ret = createMbx(&dataMbx, 10, sizeof(scan2d_data_msg),
                    MBX_IN_USERSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_DATA);


    //
    // create Proxies
    //

    // odometry
    odometry = new OdometryProxy(&workMbx, odometrySys, odometryInst);
    if (!odometry)
    {
        ret = -ENOMEM;
        goto init_error;
    }
    initBits.setBit(INIT_BIT_PROXY_ODOMETRY);

    // position
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

    // scan2d
    for (k = 0; k < SCAN2D_SENSOR_NUM_MAX; k++)
    {
        if (scan2dInst[k] >= 0)
        {
            scan2d[k] = new Scan2dProxy(&workMbx, scan2dSys[k], scan2dInst[k]);
            if (!scan2d)
            {
                ret = -ENOMEM;
                goto init_error;
            }
            initBits.setBit(INIT_BIT_PROXY_SCAN2D + k);
        }
    }

    return 0;

init_error:

    // !!! call local cleanup function !!!
    Scan2dMerge::moduleCleanup();
    return ret;
}


// non realtime context
void Scan2dMerge::moduleCleanup(void)
{
    int     k;

    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    // free scan2d proxies
    for (k = SCAN2D_SENSOR_NUM_MAX - 1; k >= 0; k--)
    {
        if (scan2dInst[k] >= 0)
        {
            if (initBits.testAndClearBit(INIT_BIT_PROXY_SCAN2D + k))
            {
                delete scan2d[k];
            }
        }
    }

    // free position proxy
    if (initBits.testAndClearBit(INIT_BIT_PROXY_POSITION))
    {
        delete position;
    }

    // free odometry proxy
    if (initBits.testAndClearBit(INIT_BIT_PROXY_ODOMETRY))
    {
        delete odometry;
    }

    // delete data mailbox
    if (initBits.testAndClearBit(INIT_BIT_MBX_DATA))
    {
        destroyMbx(&dataMbx);
    }

    // delete work mailbox
    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }
}

Scan2dMerge::Scan2dMerge(void)
      : RackDataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s datatask error sleep time
                    16,               // command mailbox slots
                    240,              // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    10,               // max buffer entries
                    10)               // data buffer listener
{
    // get static module parameter
    odometrySys   = getIntArg("odometrySys", argTab);
    odometryInst  = getIntArg("odometryInst", argTab);
    positionSys   = getIntArg("positionSys", argTab);
    positionInst  = getIntArg("positionInst", argTab);
    scan2dSys [0] = getIntArg("scan2dSysA", argTab);
    scan2dInst[0] = getIntArg("scan2dInstA", argTab);
    scan2dSys [1] = getIntArg("scan2dSysB", argTab);
    scan2dInst[1] = getIntArg("scan2dInstB", argTab);
    scan2dSys [2] = getIntArg("scan2dSysC", argTab);
    scan2dInst[2] = getIntArg("scan2dInstC", argTab);
    scan2dSys [3] = getIntArg("scan2dSysD", argTab);
    scan2dInst[3] = getIntArg("scan2dInstD", argTab);

    dataBufferMaxDataSize   =sizeof(scan2d_data_msg);
}

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "Scan2dMerge");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    Scan2dMerge *pInst;

    // create new Scan2dMerge
    pInst = new Scan2dMerge();
    if (!pInst)
    {
        printf("Can't create new Scan2dMerge -> EXIT\n");
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
