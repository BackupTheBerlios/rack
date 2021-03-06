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
#ifndef __WAYPOINT2D_H__
#define __WAYPOINT2D_H__

/**
 * waypoint 2d structure
 * @ingroup main_defines
 */
typedef struct {
    int32_t x;                              /**< [mm] x-coordinate */
    int32_t y;                              /**< [mm] y-coordinate */
    int32_t speed;                          /**< [mm/s] velocity */
    int32_t maxRadius;                      /**< [mm] absolute value of the maximum turning
                                                      radius */
    int32_t type;                           /**< type flag */
    int32_t request;                        /**< request flags */
    int32_t lbo;                            /**< [mm] lateral boundary offset */
    int32_t id;                             /**< waypoint id */
    int32_t wayId;                          /**< way id */
    int32_t layer;                          /**< layer */
    int32_t actionStart;                    /**< start action */
    int32_t actionEnd;                      /**< end action */
} __attribute__((packed))waypoint_2d;

class Waypoint2d
{
      public:

        static void le_to_cpu(waypoint_2d *data)
        {
            data->x           = __le32_to_cpu(data->x);
            data->y           = __le32_to_cpu(data->y);
            data->speed       = __le32_to_cpu(data->speed);
            data->maxRadius   = __le32_to_cpu(data->maxRadius);
            data->type        = __le32_to_cpu(data->type);
            data->request     = __le32_to_cpu(data->request);
            data->lbo         = __le32_to_cpu(data->lbo);
            data->id          = __le32_to_cpu(data->id);
            data->wayId       = __le32_to_cpu(data->wayId);
            data->layer       = __le32_to_cpu(data->layer);
            data->actionStart = __le32_to_cpu(data->actionStart);
           data->actionEnd   = __le32_to_cpu(data->actionEnd);
        }

        static void be_to_cpu(waypoint_2d *data)
        {
            data->x           = __be32_to_cpu(data->x);
            data->y           = __be32_to_cpu(data->y);
            data->speed       = __be32_to_cpu(data->speed);
            data->maxRadius   = __be32_to_cpu(data->maxRadius);
            data->type        = __be32_to_cpu(data->type);
            data->request     = __be32_to_cpu(data->request);
            data->lbo         = __be32_to_cpu(data->lbo);
            data->id          = __be32_to_cpu(data->id);
            data->wayId       = __be32_to_cpu(data->wayId);
            data->layer       = __be32_to_cpu(data->layer);
            data->actionStart = __be32_to_cpu(data->actionStart);
            data->actionEnd   = __be32_to_cpu(data->actionEnd);
        }
};

#endif // __WAYPOINT2D_H__
