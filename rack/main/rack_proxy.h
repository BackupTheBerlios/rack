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
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
#ifndef _RACK_PROXY_H_
#define _RACK_PROXY_H_

#include <stdlib.h>

#include <main/rack_mailbox.h>
#include <main/rack_time.h>
#include <main/rack_gdos.h>

//######################################################################
//# RACK message types
//######################################################################

//
// global message commands (positive)
//

#define MSG_ON                         1
#define MSG_OFF                        2
#define MSG_GET_STATUS                 3
#define MSG_GET_DATA                   4
#define MSG_GET_CONT_DATA              6
#define MSG_STOP_CONT_DATA             7
#define MSG_GET_NEXT_DATA              8
#define MSG_GET_PARAM                  9
#define MSG_SET_PARAM                  10

// global message returns (negative)
#define MSG_OK                         TIMS_MSG_OK
#define MSG_ERROR                      TIMS_MSG_ERROR
#define MSG_TIMEOUT                    TIMS_MSG_TIMEOUT
#define MSG_NOT_AVAILABLE              TIMS_MSG_NOT_AVAILABLE
#define MSG_ENABLED                   -4
#define MSG_DISABLED                  -5
#define MSG_DATA                      -6
#define MSG_CONT_DATA                 -7
#define MSG_PARAM                     -9

#define RACK_PROXY_MSG_POS_OFFSET      20
#define RACK_PROXY_MSG_NEG_OFFSET     -20

//######################################################################
//# Rack get data (static size)
//######################################################################

typedef struct rack_get_data_s
{
    rack_time_t   recordingTime;  // has to be first element
} __attribute__((packed)) rack_get_data;

class RackGetData
{
    public:
        static void le_to_cpu(rack_get_data *data)
        {
            data->recordingTime  = __le32_to_cpu(data->recordingTime);
        }

        static void be_to_cpu(rack_get_data *data)
        {
            data->recordingTime = __be32_to_cpu(data->recordingTime);
        }

        static rack_get_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            rack_get_data *p_data = (rack_get_data *)msgInfo->p_data;

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
//# Rack get continuous data (static size)
//######################################################################

typedef struct rack_get_cont_data_s
{
    rack_time_t periodTime;
    uint32_t    dataMbxAdr;
} __attribute__((packed)) rack_get_cont_data;

class RackGetContData
{
    public:
        static void le_to_cpu(rack_get_cont_data *data)
        {
            data->periodTime = __le32_to_cpu(data->periodTime);
            data->dataMbxAdr = __le32_to_cpu(data->dataMbxAdr);
        }

        static void be_to_cpu(rack_get_cont_data *data)
        {
            data->periodTime = __be32_to_cpu(data->periodTime);
            data->dataMbxAdr = __be32_to_cpu(data->dataMbxAdr);
        }

        static rack_get_cont_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            rack_get_cont_data *p_data = (rack_get_cont_data *)msgInfo->p_data;

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
//# Rack continuous data (static size)
//######################################################################

typedef struct rack_cont_data_s
{
    rack_time_t periodTime;
} __attribute__((packed)) rack_cont_data;

class RackContData
{
    public:
        static void le_to_cpu(rack_cont_data *data)
        {
            data->periodTime = __le32_to_cpu(data->periodTime);
        }

        static void be_to_cpu(rack_cont_data *data)
        {
            data->periodTime = __be32_to_cpu(data->periodTime);
        }

        static rack_cont_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            rack_cont_data *p_data = (rack_cont_data *)msgInfo->p_data;

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
//# Rack get continuous data (static size)
//######################################################################

typedef struct rack_stop_cont_data_s
{
    uint32_t    dataMbxAdr;
} __attribute__((packed)) rack_stop_cont_data;

class RackStopContData
{
    public:
        static void le_to_cpu(rack_stop_cont_data *data)
        {
            data->dataMbxAdr = __le32_to_cpu(data->dataMbxAdr);
        }

        static void be_to_cpu(rack_stop_cont_data *data)
        {
            data->dataMbxAdr = __be32_to_cpu(data->dataMbxAdr);
        }

        static rack_stop_cont_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            rack_stop_cont_data *p_data = (rack_stop_cont_data *)msgInfo->p_data;

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

#define RACK_PARAM_MAX_STRING_LEN 80

#define RACK_PARAM_INT32    0
#define RACK_PARAM_STRING   1
#define RACK_PARAM_FLOAT    2

typedef struct rack_param_s
{
    char        name[RACK_PARAM_MAX_STRING_LEN];
    int32_t     type;
    int32_t     valueInt32;
    float       valueFloat;
    char        valueString[RACK_PARAM_MAX_STRING_LEN];
} __attribute__((packed)) rack_param;

class RackParam
{
    public:
        static void le_to_cpu(rack_param *data)
        {
            data->type = __le32_to_cpu(data->type);
            data->valueInt32 = __le32_to_cpu(data->valueInt32);
            data->valueFloat = __le32_float_to_cpu(data->valueFloat);
        }

        static void be_to_cpu(rack_param *data)
        {
            data->type = __be32_to_cpu(data->type);
            data->valueInt32 = __be32_to_cpu(data->valueInt32);
            data->valueFloat = __be32_float_to_cpu(data->valueFloat);
        }
};

#define RACK_PARAM_MAX_PARAMETER_NUM 50

typedef struct rack_param_msg_s
{
    int32_t     parameterNum;
    rack_param  parameter[0];
} __attribute__((packed)) rack_param_msg;

class RackParamMsg
{
    public:
        static void le_to_cpu(rack_param_msg *data)
        {
            int i;

            data->parameterNum = __le32_to_cpu(data->parameterNum);

            for(i = 0; i < data->parameterNum; i++)
            {
                RackParam::le_to_cpu(&data->parameter[i]);
            }
        }

        static void be_to_cpu(rack_param_msg *data)
        {
            int i;

            data->parameterNum = __be32_to_cpu(data->parameterNum);

            for(i = 0; i < data->parameterNum; i++)
            {
                RackParam::be_to_cpu(&data->parameter[i]);
            }
        }

        static rack_param_msg* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            rack_param_msg *p_data = (rack_param_msg *)msgInfo->p_data;

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
 *
 * @ingroup main_common
 */
class RackProxy {

  protected:
    RackMailbox     *workMbx;
    uint32_t        sysId;
    uint32_t        classId;
    uint32_t        instance;
    uint64_t        onTimeout;
    uint64_t        offTimeout;
    uint64_t        dataTimeout;

    uint32_t        destMbxAdr;

    // only for debugging:
    RackGdos        *gdos;

//
// constructor and destructor
//

    RackProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t class_id,
              uint32_t instance);
    ~RackProxy();

//
// internal proxy functions
//

    int proxySendCmd(int8_t msgtype, uint64_t timeout);
    int proxySendDataCmd(int8_t msgtype, void *msgData, size_t datalen,
                        uint64_t timeout);
    int proxyRecvDataCmd(int8_t send_msgtype, const int8_t recv_msgtype,
                         void *recv_data, size_t recv_datalen, uint64_t timeout,
                         RackMessage *p_msginfo);
    int proxySendRecvDataCmd(int8_t send_msgtype, void *send_data,
                             size_t send_datalen, const int8_t recv_msgtype,
                             void *recv_data, size_t recv_datalen,
                             uint64_t timeout, RackMessage *msgInfo);

  public:

//
// on
//

    int on(void)    // use default timeout
    {
        return on(onTimeout);
    }

    int on(uint64_t reply_timeout_ns)   // use special timeout
    {
        return proxySendCmd(MSG_ON, reply_timeout_ns);
    }

//
// off
//

    int off(void)   // use default timeout
    {
        return off(offTimeout);
    }

    int off(uint64_t reply_timeout_ns)  // use special timeout
    {
        return proxySendCmd(MSG_OFF, reply_timeout_ns);
    }


//
// get module status
//

    int getStatus(void)  // use default timeout
    {
        return getStatus(dataTimeout);
    }

    int getStatus(uint64_t reply_timeout_ns); // use special timeout

//
// get module parameter
//

    int getParameter(rack_param_msg *parameter, int maxParameterNum)  // use default timeout
    {
        return getParameter(parameter, maxParameterNum, dataTimeout);
    }

    int getParameter(rack_param_msg *parameter, int maxParameterNum, uint64_t reply_timeout_ns); // use special timeout

//
// set module parameter
//

    int setParameter(rack_param_msg *parameter, int parameterNum)  // use default timeout
    {
        return setParameter(parameter, parameterNum, dataTimeout);
    }

    int setParameter(rack_param_msg *parameter, int parameterNum, uint64_t reply_timeout_ns); // use special timeout

//
// additional inline functions
//
    unsigned int getDestAdr(void)
    {
        return destMbxAdr;
    }

//
// timeouts
//

    void setOnTimeout(uint64_t onTimeout_ns)
    {
        onTimeout = onTimeout_ns;
    }

    void setOffTimeout(uint64_t offTimeout_ns)
    {
        offTimeout = offTimeout_ns;
    }

    void setDataTimeout(uint64_t dataTimeout_ns)
    {
        dataTimeout = dataTimeout_ns;
    }

};

//######################################################################
//# class RackDataProxy
//######################################################################

class RackDataProxy : public RackProxy {

    protected:

//
// constructor and destructor
//

    RackDataProxy(RackMailbox *sendMbx, uint32_t sys_id, uint32_t class_id,
                  uint32_t instance);
    ~RackDataProxy();

//
// get data
//

    int getData(void *recv_data, ssize_t recv_max_len, rack_time_t timeStamp,
                uint64_t reply_timeout_ns, RackMessage *msgInfo);

//
// get next data
//

    int getNextData(void *recv_data, ssize_t recv_max_len,
                    uint64_t reply_timeout_ns, RackMessage *msgInfo);

    public:

//
// get continuous data
//

    int getContData(rack_time_t requestPeriodTime, RackMailbox *dataMbx,
                    rack_time_t *realPeriodTime)
    {
        return getContData(requestPeriodTime, dataMbx, realPeriodTime, dataTimeout);
    }

    int getContData(rack_time_t requestPeriodTime, RackMailbox *dataMbx,
                    rack_time_t *realPeriodTime, uint64_t reply_timeout_ns);


//
// stop continuous data
//

    int stopContData(RackMailbox *dataMbx)
    {
        return stopContData(dataMbx, dataTimeout);
    }

    int stopContData(RackMailbox *dataMbx, uint64_t reply_timeout_ns);

};

#endif //_RACK_PROXY_H_
