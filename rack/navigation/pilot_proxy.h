/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */
#ifndef __PILOT_PROXY_H__
#define __PILOT_PROXY_H__

#include <main/rack_proxy.h>
#include <main/defines/polar_spline.h>
#include <main/defines/position3d.h>
#include <main/defines/destinationPoint.h>

#define PILOT_DATA_SPLINE_MAX   1000        /**< maximum number of splines */

#define PILOT_HOLD_ENABLED      1
#define PILOT_HOLD_DISABLED     0

#define PILOT_STATE_IDLE        0
#define PILOT_STATE_PLANNING    1
#define PILOT_STATE_RUNNING     2
#define PILOT_STATE_PATH_ERROR  3

//######################################################################
//# Pilot Message Types
//######################################################################

#define MSG_PILOT_SET_DESTINATION           (RACK_PROXY_MSG_POS_OFFSET + 1)
#define MSG_PILOT_HOLD_COMMAND              (RACK_PROXY_MSG_POS_OFFSET + 2)
#define MSG_PILOT_REVERT_COMMAND            (RACK_PROXY_MSG_POS_OFFSET + 3)
#define MSG_PILOT_GET_STATUS                (RACK_PROXY_MSG_POS_OFFSET + 4)
#define MSG_PILOT_STATUS                    (RACK_PROXY_MSG_POS_OFFSET + 5)
#define MSG_PILOT_SET_MULTI_DESTINATION     (RACK_PROXY_MSG_POS_OFFSET + 6)

//######################################################################
//# PilotData (!!! VARIABLE SIZE !!! MESSAGE !!!)
//######################################################################

/* CREATING A MESSAGE :

typedef {
    pilot_data    data;
    polar_spline  spline[ ... ];
} __attribute__((packed)) pilot_data_msg;

pilot_data_msg msg;

ACCESS: msg.data.spline[...] OR msg.spline[...];

*/

/**
 * pilot data structure
 */
typedef struct{
    rack_time_t     recordingTime;          /**< [ms]  global timestamp (has to be first element)*/
    position_3d     pos;                    /**< position on the path */
    position_3d     dest;                   /**< destination of the pilot */
    int32_t         speed;                  /**< [mm/s] current speed set value */
    float           curve;                  /**< [1/mm] current curve set value*/
    int32_t         distanceToDest;         /**< [mm] distance to destination,
                                                      -1 if no destination is set,
                                                       0 if destination is reached */
    uint32_t        pilotState;             /**< current state of the pilot */
    int32_t         splineNum;              /**< number of following splines */
    polar_spline    spline[0];              /**< list of splines */
} __attribute__((packed)) pilot_data;

class PilotData
{
    public:
        static void le_to_cpu(pilot_data *data)
        {
            int i;
            data->recordingTime     = __le32_to_cpu(data->recordingTime);
            Position3D::le_to_cpu(&data->pos);
            Position3D::le_to_cpu(&data->dest);
            data->speed             = __le32_to_cpu(data->speed);
            data->curve             = __le32_float_to_cpu(data->curve);
            data->distanceToDest    = __le32_to_cpu(data->distanceToDest);
            data->splineNum         = __le32_to_cpu(data->splineNum);
            for (i=0; i<data->splineNum; i++)
            {
                PolarSpline::le_to_cpu(&data->spline[i]);
            }
        }

        static void be_to_cpu(pilot_data *data)
        {
            int i;
            data->recordingTime     = __be32_to_cpu(data->recordingTime);
            Position3D::be_to_cpu(&data->pos);
            Position3D::be_to_cpu(&data->dest);
            data->speed             = __be32_to_cpu(data->speed);
            data->curve             = __be32_float_to_cpu(data->curve);
            data->distanceToDest    = __be32_to_cpu(data->distanceToDest);
            data->splineNum         = __be32_to_cpu(data->splineNum);
            for (i=0; i<data->splineNum; i++)
            {
                PolarSpline::be_to_cpu(&data->spline[i]);
            }
        }

        static pilot_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            pilot_data *p_data = (pilot_data *)msgInfo->p_data;

            if (msgInfo->isDataByteorderLe()) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->setDataByteorder();
            return p_data;
        }
};

//######################################################################
//# Pilot Destination Data (static size  - MESSAGE)
//######################################################################

/**
 * pilot destination data structure
 */
typedef struct{
    position_3d    pos;                     /**< position of the destination */
    int            speed;                   /**< [mm/s] maximum allowed velocity for moving
                                                        to the destination */
} __attribute__((packed)) pilot_dest_data;

class PilotDestData
{
    public:
        static void le_to_cpu(pilot_dest_data *data)
        {
            Position3D::le_to_cpu(&data->pos);
            data->speed = __le32_to_cpu(data->speed);
        }

        static void be_to_cpu(pilot_dest_data *data)
        {
            Position3D::be_to_cpu(&data->pos);
            data->speed = __be32_to_cpu(data->speed);
        }

        static pilot_dest_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            pilot_dest_data *p_data = (pilot_dest_data *)msgInfo->p_data;

            if (msgInfo->isDataByteorderLe()) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->setDataByteorder();
            return p_data;
        }
};

//######################################################################
//# Pilot Multi Destination Data (static size  - MESSAGE)
//######################################################################

/**
 * pilot destination data structure
 */
typedef struct{						
    int32_t         	pointNum;              /**< number of following points */
    destination_point   point[0];              /**< list of points */														
} __attribute__((packed)) pilot_multi_dest_data;

class PilotMultiDestData
{
    public:
        static void le_to_cpu(pilot_multi_dest_data *data)
        {
            int i;
            data->pointNum         = __le32_to_cpu(data->pointNum);
            for (i=0; i<data->pointNum; i++)
            {
                DestinationPoint::le_to_cpu(&data->point[i]);
            }
        }

        static void be_to_cpu(pilot_multi_dest_data *data)
        {
            int i;
            data->pointNum         = __be32_to_cpu(data->pointNum);
            for (i=0; i<data->pointNum; i++)
            {
                DestinationPoint::be_to_cpu(&data->point[i]);
            }
        }

        static pilot_multi_dest_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            pilot_multi_dest_data *p_data = (pilot_multi_dest_data *)msgInfo->p_data;

            if (msgInfo->isDataByteorderLe()) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->setDataByteorder();
            return p_data;
        }
};

//######################################################################
//# Pilot Hold Data (static size  - MESSAGE)
//######################################################################

/**
 * pilot hold data structure
 */
typedef struct{
    int32_t         holdState;              /**< hold state */
    rack_time_t     holdTime;               /**< [ms] requested hold time */
    position_3d     pos;                    /**< requested hold position */
} __attribute__((packed)) pilot_hold_data;

class PilotHoldData
{
    public:
        static void le_to_cpu(pilot_hold_data *data)
        {
            data->holdState     = __le32_to_cpu(data->holdState);
            data->holdTime      = __le32_to_cpu(data->holdTime);
            Position3D::le_to_cpu(&data->pos);
        }

        static void be_to_cpu(pilot_hold_data *data)
        {
            data->holdState     = __be32_to_cpu(data->holdState);
            data->holdTime      = __be32_to_cpu(data->holdTime);
            Position3D::be_to_cpu(&data->pos);
        }

        static pilot_hold_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            pilot_hold_data *p_data = (pilot_hold_data *)msgInfo->p_data;

            if (msgInfo->isDataByteorderLe()) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->setDataByteorder();
            return p_data;
        }
};

//######################################################################
//# Pilot Revert Data (static size  - MESSAGE)
//######################################################################

/**
 * pilot revert data structure
 */
typedef struct{
    rack_time_t     revertTime;             /**< [ms] time until revert */
} __attribute__((packed)) pilot_revert_data;

class PilotRevertData
{
    public:
        static void le_to_cpu(pilot_revert_data *data)
        {
            data->revertTime = __le32_to_cpu(data->revertTime);
        }

        static void be_to_cpu(pilot_revert_data *data)
        {
            data->revertTime = __be32_to_cpu(data->revertTime);
        }

        static pilot_revert_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            pilot_revert_data *p_data = (pilot_revert_data*)msgInfo->p_data;

            if (msgInfo->isDataByteorderLe()) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->setDataByteorder();
            return p_data;
        }
};

//######################################################################
//# Pilot Get Status Data (static size  - MESSAGE)
//######################################################################

/**
 * pilot get status data structure
 */
typedef struct{
    rack_time_t     recordingTime;          /**< [ms]  global timestamp (has to be first element)*/
    position_3d     pos;                    /**< position on the path */
    position_3d     dest;                   /**< destination of the pilot */
    int32_t         speed;                  /**< [mm/s] current speed set value */
    float           curve;                  /**< [1/mm] current curve set value*/
    int32_t         distanceToDest;         /**< [mm] distance to destination,
                                                      -1 if no destination is set,
                                                       0 if destination is reached */
    uint32_t        pilotState;             /**< current state of the pilot */
} __attribute__((packed)) pilot_status_data;

class PilotStatusData
{
    public:
        static void le_to_cpu(pilot_status_data *data)
        {
            data->recordingTime     = __le32_to_cpu(data->recordingTime);
            Position3D::le_to_cpu(&data->pos);
            Position3D::le_to_cpu(&data->dest);
            data->speed             = __le32_to_cpu(data->speed);
            data->curve             = __le32_float_to_cpu(data->curve);
            data->distanceToDest    = __le32_to_cpu(data->distanceToDest);
        }

        static void be_to_cpu(pilot_status_data *data)
        {
            data->recordingTime     = __be32_to_cpu(data->recordingTime);
            Position3D::be_to_cpu(&data->pos);
            Position3D::be_to_cpu(&data->dest);
            data->speed             = __be32_to_cpu(data->speed);
            data->curve             = __be32_float_to_cpu(data->curve);
            data->distanceToDest    = __be32_to_cpu(data->distanceToDest);
        }

        static pilot_status_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            pilot_status_data *p_data = (pilot_status_data *)msgInfo->p_data;

            if (msgInfo->isDataByteorderLe()) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->setDataByteorder();
            return p_data;
        }
};


/**
 * Mobile robot motion controller.
 *
 * @ingroup proxies_navigation
 */
class PilotProxy : public RackDataProxy
{

      public:

        PilotProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
               : RackDataProxy(workMbx, sys_id, PILOT, instance)
        {
        };

        ~PilotProxy()
        {
        };


//
// pilot data
//

        int getData(pilot_data *recv_data, ssize_t recv_datalen,
                    rack_time_t timeStamp)
        {
              return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
        }

        int getData(pilot_data *recv_data, ssize_t recv_datalen,
                    rack_time_t timeStamp, uint64_t reply_timeout_ns);

//destination data
    int setDestination(pilot_dest_data *recv_data, ssize_t recv_datalen)
    {
        return setDestination(recv_data, recv_datalen, dataTimeout);
    }

    int setDestination(pilot_dest_data *recv_data, ssize_t recv_datalen,
                       uint64_t reply_timeout_ns);

//destination data
    int setMultiDestination(pilot_multi_dest_data *recv_data, ssize_t recv_datalen)
    {
        return setMultiDestination(recv_data, recv_datalen, dataTimeout);
    }

    int setMultiDestination(pilot_multi_dest_data *recv_data, ssize_t recv_datalen,
                       uint64_t reply_timeout_ns);

//hold command
    int holdCommand(pilot_hold_data *recv_data, ssize_t recv_datalen)
    {
        return holdCommand(recv_data, recv_datalen, dataTimeout);
    }

    int holdCommand(pilot_hold_data *recv_data, ssize_t recv_datalen,
                       uint64_t reply_timeout_ns);

//revert command
    int revert(pilot_revert_data *recv_data, ssize_t recv_datalen)
    {
        return revert(recv_data, recv_datalen, dataTimeout);
    }

    int revert(pilot_revert_data *recv_data, ssize_t recv_datalen,
                       uint64_t reply_timeout_ns);

//get status command
    int getPilotStatus(pilot_status_data *recv_data, ssize_t recv_datalen)
    {
        return getPilotStatus(recv_data, recv_datalen, dataTimeout);
    }

    int getPilotStatus(pilot_status_data *recv_data, ssize_t recv_datalen,
                       uint64_t reply_timeout_ns);
};

#endif // __PILOT_PROXY_H__
