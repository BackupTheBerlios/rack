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
package rack.drivers;

import rack.main.*;
import rack.main.tims.*;

public class GpsProxy extends RackDataProxy
{

    public GpsProxy(int system, int instance, TimsMbx replyMbx)
    {
        super(RackName.create(system, RackName.GPS, instance, 0), replyMbx, 1000);
    }

    public synchronized GpsDataMsg getData(int recordingTime)
    {
        try
        {
            TimsRawMsg raw = getRawData(recordingTime);

            if (raw != null)
            {
                GpsDataMsg data = new GpsDataMsg(raw);
                return data;
            }
            else
            {
                return null;
            }
        }
        catch(TimsException e)
        {
            System.out.println(e.toString());
            return null;
        }
    }
    
    public synchronized GpsDataMsg getData()
    {
        return(getData(0));
    }    
}
