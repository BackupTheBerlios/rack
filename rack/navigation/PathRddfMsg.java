/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2010 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Matthias Hentschel  <hentschel@rts.uni-hannover.de>
 *
 */
package rack.navigation;

import java.io.*;

import rack.main.RackProxy;
import rack.main.defines.*;
import rack.main.tims.*;

public class PathRddfMsg extends TimsMsg
{
    public int basePointNum = 0;
    public Waypoint2d[] basePoint;


    public int getDataLen()
    {
        return (4 + basePointNum * Waypoint2d.getDataLen());

    }

    public PathRddfMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public PathRddfMsg(int basePointNum)
    {
        basePoint = new Waypoint2d[basePointNum];

        msglen = HEAD_LEN + getDataLen();
    }

    public PathRddfMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    public boolean checkTimsMsgHead()
    {
        if (type == RackProxy.MSG_DATA)
        {
            return (true);
        }
        else
        {
            return (false);
        }
    }

    public void readTimsMsgBody(InputStream in) throws IOException
    {
        EndianDataInputStream dataIn;

        if (bodyByteorder == BIG_ENDIAN)
        {
            dataIn = new BigEndianDataInputStream(in);
        }
        else
        {
            dataIn = new LittleEndianDataInputStream(in);
        }

        basePointNum = dataIn.readInt();
        basePoint    = new Waypoint2d[basePointNum];

        for (int i = 0; i < basePointNum; i++)
        {
            basePoint[i].readData(dataIn);
        }

        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(basePointNum);

        for (int i = 0; i < basePointNum; i++)
        {
            basePoint[i].writeData(dataOut);
        }
    }
}
