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
#ifndef __POLAR_SPLINE_H__
#define __POLAR_SPLINE_H__

#include <main/defines/point2d.h>
#include <main/defines/position2d.h>
#include <main/defines/waypoint2d.h>

/**
 * polar spline structure
 * @ingroup main_defines
 */
typedef struct
{
    waypoint_2d basepoint;                  /**< next basepoint the spline is heading to */
    position_2d startPos;                   /**< start position of the spline */
    position_2d endPos;                     /**< end position of the spline */
    position_2d centerPos;                  /**< center position of the spline */
    int32_t     length;                     /**< [mm] length of the spline */
    int32_t     radius;                     /**< [mm] radius of the spline */
    int32_t     vMax;                       /**< [mm/s] absolute value of the maximum velocity of
                                                        the spline */
    int32_t     vStart;                     /**< [mm/s] absolute value of the velocity at the start
                                                        of the spline */
    int32_t     vEnd;                       /**< [mm/s] absolute value of the velocity at the end
                                                        of the spline */
    int32_t     accMax;                     /**< [mm/s^2] maximum acceleration on the spline */
    int32_t     decMax;                     /**< [mm/s^2] maximum deceleration on the spline */
    int32_t     type;                       /**< type flag */
    int32_t     request;                    /**< request flag */
    int32_t     lbo;                        /**< [mm] lateral boundary offset */
} __attribute__((packed)) polar_spline;

/**
 *
 * @ingroup main_defines
 */
class PolarSpline
{
   public:
        static void le_to_cpu(polar_spline *data)
        {
            Waypoint2d::le_to_cpu(&data->basepoint);
            Position2D::le_to_cpu(&data->startPos);
            Position2D::le_to_cpu(&data->endPos);
            Position2D::le_to_cpu(&data->centerPos);
            data->length = __le32_to_cpu(data->length);
            data->radius = __le32_to_cpu(data->radius);
            data->vMax   = __le32_to_cpu(data->vMax);
            data->vStart = __le32_to_cpu(data->vStart);
            data->vEnd   = __le32_to_cpu(data->vEnd);
            data->accMax = __le32_to_cpu(data->accMax);
            data->decMax = __le32_to_cpu(data->decMax);
            data->type   = __le32_to_cpu(data->type);
            data->request= __le32_to_cpu(data->request);
            data->lbo    = __le32_to_cpu(data->lbo);
        }

        static void be_to_cpu(polar_spline *data)
        {
            Waypoint2d::be_to_cpu(&data->basepoint);
            Position2D::be_to_cpu(&data->startPos);
            Position2D::be_to_cpu(&data->endPos);
            Position2D::be_to_cpu(&data->centerPos);
            data->length = __be32_to_cpu(data->length);
            data->radius = __be32_to_cpu(data->radius);
            data->vMax   = __be32_to_cpu(data->vMax);
            data->vStart = __be32_to_cpu(data->vStart);
            data->vEnd   = __be32_to_cpu(data->vEnd);
            data->accMax = __be32_to_cpu(data->accMax);
            data->decMax = __be32_to_cpu(data->decMax);
            data->type   = __be32_to_cpu(data->type);
            data->request= __be32_to_cpu(data->request);
            data->lbo    = __be32_to_cpu(data->lbo);
        }

        /**
         * Converts the global "position" into relative "spline" coordinates.
         */
        static void position2spline(position_2d *position, polar_spline *spline,
                                    position_2d *result)
        {
            point_2d        point;
            double          x, y, cosRho, sinRho;
            double          a, a2, a3, a4, b, b2;
            double          r, r1;
            int             lengthSign, radiusSign;

            // transform position into spline coordinate system
            sinRho = sin(spline->centerPos.rho);
            cosRho = cos(spline->centerPos.rho);

            x = (double)(position->x - spline->centerPos.x);
            y = (double)(position->y - spline->centerPos.y);

            point.x = (int)(  x * cosRho + y * sinRho);
            point.y = (int)(- x * sinRho + y * cosRho);

            // set length sign
            if (spline->length >= 0)
            {
                lengthSign = 1;
            }
            else
            {
                lengthSign = -1;
            }

            // set radius sign
            if (spline->radius > 0)
            {
                radiusSign = 1;
            }
            else
            {
                radiusSign = -1;
            }

            // process curved spline
            if (spline->radius != 0)
            {
                result->x = (int)rint(fabs((double)spline->radius *
                                      normaliseAngleSym0((double)lengthSign * (double)radiusSign *
                                      point_2d_polar_angle(point))) * (double)lengthSign);

                // calculate transversal deviation
                a = (double)result->x / (double)spline->radius;
                b = (double)spline->length / (double)spline->radius;
                a2 = a  * a;
                a3 = a2 * a;
                a4 = a3 * a;
                b2 = b  * b;
                r  = (double)rint(spline->radius *
                                 (1.0 + a2/2.0 - a3/b + a4/(2.0*b2)));
                r1 = (double)spline->radius *
                                 (a - (3.0 * a2 / b) + (2.0 * a3 / b2));
                result->y = (int)r - point_2d_polar_distance(point) * radiusSign;

                // calculate angle error
                result->rho = normaliseAngleSym0(position->rho -
                                                (spline->startPos.rho + (a - atan(r1/r))));
            }

            // process direct line
            else
            {
                // calculate longitudinal, transversal and angle values
                result->x    = point.x;
                result->y    = point.y;
                result->rho  = normaliseAngleSym0(position->rho -
                                                  spline->startPos.rho);
            }
        }


        /**
         * Converts the relative spline coordinates "splinePos" into a global
         * "position"
         */
        static void spline2position(position_2d *splinePos,
                                    polar_spline *spline, position_2d *result)
        {
            point_2d        point;
            double          cosRho, sinRho;
            double          a, a2, a3, a4, b, b2;
            int             r, radius;
            int             radiusSign;


            // set radius sign
            if (spline->radius > 0)
            {
                radiusSign = 1;
            }
            else
            {
                radiusSign = -1;
            }

            // process curved spline
            if (spline->radius != 0)
            {
                // calculate splinePos-radius in spline coordinate system
                a = (double)splinePos->x / (double)spline->radius;
                b = (double)spline->length / (double)spline->radius;
                a2 = a  * a;
                a3 = a2 * a;
                a4 = a3 * a;
                b2 = b  * b;
                r = (int)rint(spline->radius *
                             (1.0 + a2/2.0 - a3/b + a4/(2.0*b2)));
                radius = r - splinePos->y;

                // transform spline position into global coordinate system
                sinRho  = sin(-spline->centerPos.rho - a);
                cosRho  = cos(-spline->centerPos.rho - a);
                point.x = (int)( radius * radiusSign * cosRho);
                point.y = (int)(-radius * radiusSign * sinRho);

                // calculate global position and angle values
                result->x    = spline->centerPos.x + point.x;
                result->y    = spline->centerPos.y + point.y;
                result->rho  = normaliseAngle(spline->startPos.rho +
                                              splinePos->rho + (float)a);
            }

            // process direct line
            else
            {
                // transform spline position into global coordinate system
                sinRho  = sin(-spline->centerPos.rho);
                cosRho  = cos(-spline->centerPos.rho);
                point.x = (int)( splinePos->x * cosRho + splinePos->y * sinRho);
                point.y = (int)(-splinePos->x * sinRho + splinePos->y * cosRho);

                // calculate global position and angle values
                result->x    = spline->centerPos.x + point.x;
                result->y    = spline->centerPos.y + point.y;
                result->rho  = normaliseAngle(spline->startPos.rho +
                                              splinePos->rho);
            }
        }
};



#endif /*__POLAR_SPLINE_H__*/
