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
 *      Marko Reimer  <reimer@rts.uni-hannover.de>
 *
 */
#ifndef __OBJ_RECOG_PROXY_H__
#define __OBJ_RECOG_PROXY_H__

#include <main/rack_proxy.h>
#include <main/defines/obj_recog_object.h>
#include <main/defines/position3d.h>
#include <main/rack_name.h>

//######################################################################
//# ObjRecog Message Types
//######################################################################

#define MSG_SET_ESTIMATE              (RACK_PROXY_MSG_POS_OFFSET + 1)
#define MSG_STOP_RECOG                (RACK_PROXY_MSG_POS_OFFSET + 2)

//######################################################################
//# ObjRecog data defines
//######################################################################

#define OBJ_RECOG_OBJECT_MAX       100      /**< maximum number of objects */

//######################################################################
//# ObjRecogData (!!! VARIABLE SIZE !!!)
//######################################################################

/* CREATING A MESSAGE :

typedef {
  obj_recog_data    data;
  obj_recog_object  object[ ... ];
} obj_recog_data_msg;

obj_recog_data_msg msg;

ACCESS: msg.data.object[...] OR msg.object[...]; !!!

*/

/**
 * object recognition data structure
 */
typedef struct {
    rack_time_t         recordingTime;      /**< [ms]  global timestamp (has to be first element)*/
    position_3d         refPos;             /**< global position of the reference coordinate
                                                 system */
    position_3d         varRefPos;          /**< standard deviation of the reference position */
    int32_t             objectNum;          /**< number of following objects */
    obj_recog_object    object[0];          /**< list of objects */
} __attribute__((packed)) obj_recog_data;

class ObjRecogData
{
    public:
        static void le_to_cpu(obj_recog_data *data)
        {
            int i;
            data->recordingTime = __le32_to_cpu(data->recordingTime);
            Position3D::le_to_cpu(&data->refPos);
            Position3D::le_to_cpu(&data->varRefPos);
            data->objectNum     = __le32_to_cpu(data->objectNum);
            for (i = 0; i < data->objectNum; i++)
            {
                ObjRecogObject::le_to_cpu(&data->object[i]);
            }
        }

        static void be_to_cpu(obj_recog_data *data)
        {
            int i;
            data->recordingTime = __be32_to_cpu(data->recordingTime);
            Position3D::be_to_cpu(&data->refPos);
            Position3D::be_to_cpu(&data->varRefPos);
            data->objectNum     = __be32_to_cpu(data->objectNum);
            for (i = 0; i < data->objectNum; i++)
            {
                ObjRecogObject::be_to_cpu(&data->object[i]);
            }
        }

        static obj_recog_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            obj_recog_data *p_data = (obj_recog_data *)msgInfo->p_data;

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

        static size_t getDatalen(obj_recog_data *data)
        {
            return (sizeof(obj_recog_data) + sizeof(obj_recog_object) * data->objectNum);
        }
};

/**
 * Perception components that extract objects and object positions from sensor input.
 *
 * @ingroup proxies_perception
 */
class ObjRecogProxy : public RackDataProxy {

    public:

        ObjRecogProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
              : RackDataProxy(workMbx, sys_id, OBJ_RECOG, instance)
        {
        };

        ~ObjRecogProxy()
        {
        };

//
// get data
//

        int getData(obj_recog_data *recv_data, ssize_t recv_datalen,
                    rack_time_t timeStamp)
        {
            return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
        }

        int getData(obj_recog_data *recv_data, ssize_t recv_datalen,
                    rack_time_t timeStamp, uint64_t reply_timeout_ns);

        int getNextData(obj_recog_data *recv_data, ssize_t recv_datalen,
                    uint64_t reply_timeout_ns);

        int setEstimate(obj_recog_data *recv_data, ssize_t recv_datalen)
        {
            return setEstimate(recv_data, recv_datalen, dataTimeout);
        }

        int setEstimate(obj_recog_data *recv_data, ssize_t recv_datalen,
                    uint64_t reply_timeout_ns);

        int stopRecognition(obj_recog_data *recv_data, ssize_t recv_datalen)
        {
            return stopRecognition(recv_data, recv_datalen, dataTimeout);
        }

        int stopRecognition(obj_recog_data *recv_data, ssize_t recv_datalen,
                    uint64_t reply_timeout_ns);
};

#endif
