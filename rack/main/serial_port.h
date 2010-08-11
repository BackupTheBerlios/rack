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
 *      Oliver Wulf <oliver.wulf@web.de> - add linux implementation
 *
 */
#ifndef __SERIAL_PORT_H__
#define __SERIAL_PORT_H__

#include <main/rack_module.h>

#if defined (__XENO__) || defined (__KERNEL__)

#include <rtdm/rtserial.h>

#else

#include "linux/rack_rtserial.h"

#endif

#define SERPORT_MCR_RTS   RTSER_MCR_RTS

/**
 * This is the Serial Port interface of RACK provided to application programs
 * in userspace.
 *
 * @ingroup main_device_driver
 */
class SerialPort
{
    private:

    protected:

        int fd;
        RackModule *module;

    public:

        SerialPort();
        ~SerialPort();

        int open(int dev, const rtser_config *config, RackModule *module);
        int close(void);

        int setConfig(const rtser_config *config);
        int setBaudrate(int baudrate);
        int setRecvTimeout(int64_t timeout);
        int getControl(int32_t *bitmask);
        int setControl(int32_t bitmask);
        int getStatus(struct rtser_status *status);

        int send(const void* data, int dataLen);

        /** receive data with no timestamp and the default timeout */
        int recv(void *data, int dataLen);

        /** receive data with timestamp and the default timeout */
        int recv(void *data, int dataLen, rack_time_t *timestamp);

        /** receive data with timestamp and a specific timeout */
        int recv(void *data, int dataLen, rack_time_t *timestamp,
                 int64_t timeout_ns);

        int waitEvent(struct rtser_event *event);

        int clean(void);
};

#endif // __SERIAL_PORT_H__
