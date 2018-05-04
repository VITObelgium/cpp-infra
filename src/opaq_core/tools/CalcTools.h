/*
 * StringTools.h
 *
 *  Created on: Feb 26, 2014
 *      Author: vlooys
 */

#pragma once

#include <vector>

namespace opaq
{

namespace CalcTools
{

/**
* @brief least square regression
* @param xy
* @param slope
* @param icept
* @param r2
*/
template <typename T>
double lsqreg(const std::vector<T>& xy, double& slope, double& icept)
{
    double SUMx = 0;     //sum of x values
    double SUMy = 0;     //sum of y values
    double SUMxy = 0;    //sum of x * y
    double SUMxx = 0;    //sum of x^2
    double SUMres = 0;   //sum of squared residue
    double res = 0;      //residue squared
    double SUM_Yres = 0; //sum of squared of the discrepancies
    double AVGy = 0;     //mean of y
    double AVGx = 0;     //mean of x
    double Yres = 0;     //squared of the discrepancies

    if (xy.empty()) return 0.0;

    //calculate various sums
    for (auto& point : xy)
    {
        //sum of x
        SUMx = SUMx + point.x();
        //sum of y
        SUMy = SUMy + point.y();
        //sum of squared x*y
        SUMxy = SUMxy + point.x() * point.y();
        //sum of squared x
        SUMxx = SUMxx + point.x() * point.y();
    }

    //calculate the means of x and y
    AVGy = SUMy / xy.size();
    AVGx = SUMx / xy.size();

    //slope or a1
    slope = (xy.size() * SUMxy - SUMx * SUMy) / (xy.size() * SUMxx - SUMx*SUMx);

    //y itercept or a0
    icept = AVGy - slope * AVGx;

    //calculate squared residues, their sum etc.
    for (auto& point : xy)
    {
        //current (y_i - a0 - a1 * x_i)^2
        Yres = pow((point.y() - icept - (slope * point.x())), 2);

        //sum of (y_i - a0 - a1 * x_i)^2
        SUM_Yres += Yres;

        //current residue squared (y_i - AVGy)^2
        res = pow(point.y() - AVGy, 2);

        //sum of squared residues
        SUMres += res;
    }

    //calculate r^2 coefficient of determination
    return (SUMres - SUM_Yres) / SUMres;
}

// om : observation in x, model in y
template <typename T>
double rmse(const std::vector<T>& om)
{
    double r = 0.0;

    if (om.size() == 0) return r;

    for (auto& point : om)
    {
        r += (point.y() - point.x()) * (point.y() - point.x());
    }

    return sqrt(r / om.size());
}

// om : observation in x, model in y
template <typename T>
double bias(const std::vector<T>& om)
{
    double b = 0.0;
    double sx = 0.0;
    double sy = 0.0;

    if (om.size() == 0) return b;

    for (auto& point : om)
    {
        sx = sx + point.x();
        sy = sy + point.y();
    }

    return (sy - sx) / om.size();
}

}
}
