/* ***** BEGIN LICENSE BLOCK *****
*
* $Id$ $Name$
*
* Version: MPL 1.1/GPL 2.0/LGPL 2.1
*
* The contents of this file are subject to the Mozilla Public License
* Version 1.1 (the "License"); you may not use this file except in compliance
* with the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
* the specific language governing rights and limitations under the License.
*
* The Original Code is BBC Research and Development code.
*
* The Initial Developer of the Original Code is the British Broadcasting
* Corporation.
* Portions created by the Initial Developer are Copyright (C) 2004.
* All Rights Reserved.
*
* Contributor(s): Thomas Davies (Original Author)
*
* Alternatively, the contents of this file may be used under the terms of
* the GNU General Public License Version 2 (the "GPL"), or the GNU Lesser
* Public License Version 2.1 (the "LGPL"), in which case the provisions of
* the GPL or the LGPL are applicable instead of those above. If you wish to
* allow use of your version of this file only under the terms of the either
* the GPL or LGPL and not to allow others to use your version of this file
* under the MPL, indicate your decision by deleting the provisions above
* and replace them with the notice and other provisions required by the GPL
* or LGPL. If you do not delete the provisions above, a recipient may use
* your version of this file under the terms of any one of the MPL, the GPL
* or the LGPL.
* ***** END LICENSE BLOCK ***** */


////////////////////////////////////////////////////////////////
//classes and functions for motion estimation and compensation//
////////////////////////////////////////////////////////////////

#include <libdirac_common/motion.h>
#include <cmath>

//motion compensation stuff//
/////////////////////////////

//arithmetic functions
void ArithAddObj::DoArith(ValueType &lhs, const CalcValueType rhs, const CalcValueType &Weight) const 
{
    CalcValueType t = ((rhs*Weight)+512)>>10;
    lhs+=short(t);
}

void ArithSubtractObj::DoArith(ValueType &lhs, const CalcValueType rhs, const CalcValueType &Weight) const 
{
    CalcValueType t = ((rhs*Weight)+512)>>10;
    lhs-=short(t);
}

void ArithHalfAddObj::DoArith(ValueType &lhs, const CalcValueType rhs, const CalcValueType &Weight) const 
{
    CalcValueType t = ((rhs*Weight)+1024)>>11;
    lhs+=short(t);
}

void ArithHalfSubtractObj::DoArith(ValueType &lhs, const CalcValueType rhs, const CalcValueType &Weight) const 
{
    CalcValueType t = ((rhs*Weight)+1024)>>11;
    lhs-=short(t);
}


//Overlapping blocks are acheived by applying a 2D raised cosine shape
//to them. This function facilitates the calculations
float RaisedCosine(float t, float B)
{
    if(std::abs(t)>(B+1.0)/2.0) return 0.0f;
    else if(std::abs(t)<(1.0-B)/2.0) return 1.0f;
    else return(0.5*(1.0+std::cos(3.141592654*(std::abs(t)-(1.0-B)/2.0)/B)));
}

//Calculates a weighting block.
//bparams defines the block parameters so the relevant weighting arrays can be created.
//FullX and FullY refer to whether the weight should be adjusted for the edge of an image.
//eg. 1D Weighting shapes in x direction

//  FullX true        FullX false
//     ***           ********
//   *     *                  *
//  *       *                  *
//*           *                  *
void CreateBlock(const OLBParams &bparams, bool FullX, bool FullY, TwoDArray<CalcValueType>& WeightArray)
{

    //Create temporary array.
    TwoDArray<float> CalcArray( WeightArray.LengthY() , WeightArray.LengthX() );

    //Calculation variables
    float rolloffX = (float(bparams.Xblen()+1)/float(bparams.Xbsep())) - 1;
    float rolloffY = (float(bparams.Yblen()+1)/float(bparams.Ybsep())) - 1;
    float val;

    //Initialise the temporary array to one
    for(int y = 0; y < bparams.Yblen(); ++y)
        {
        for(int x = 0; x < bparams.Xblen(); ++x)
        {
            CalcArray[y][x] = 1;
        }// x
    }// y 

    //Window temporary array in the x direction
    for(int y = 0; y < bparams.Yblen(); ++y)
    {
        for(int x = 0; x < bparams.Xblen(); ++x)
        {
            //Apply the window
            if(!FullX){
                if(x >= (bparams.Xblen())>>1){
                    val = (float(x) - (float(bparams.Xblen()-1)/2.0))/float(bparams.Xbsep());
                    CalcArray[y][x] *= RaisedCosine(val,rolloffX);
                }
            }
            else{
                val = (float(x) - (float(bparams.Xblen()-1)/2.0))/float(bparams.Xbsep());
                CalcArray[y][x] *= RaisedCosine(val,rolloffX);
            }
        }// x
    }// y

    //Window the temporary array in the y direction
    for(int x = 0; x < bparams.Xblen(); ++x)
    {
        for(int y = 0; y < bparams.Yblen(); ++y)
        {
            //Apply the window            
            if(!FullY){
                if(y >= (bparams.Yblen())>>1){
                    val = (float(y) - (float(bparams.Yblen()-1)/2.0))/float(bparams.Ybsep());
                    CalcArray[y][x] *= RaisedCosine(val,rolloffY);
                }
            }
            else{
                val = (float(y) - (float(bparams.Yblen()-1)/2.0))/float(bparams.Ybsep());
                CalcArray[y][x] *= RaisedCosine(val,rolloffY);
            }
        }// y
    }// x

    //Convert the temporary float array into our
    //weight array by multiplying the floating
    //point values by 1024. This can be removed
    //later using a right shift of ten.
    float g;
    for(int y = 0; y < bparams.Yblen(); ++y)
        {
        for(int x = 0; x < bparams.Xblen(); ++x)
            {
            g = floor((CalcArray[y][x]*1024)+0.5);
            WeightArray[y][x] = ValueType(g);
        }// x
    }// y
}

//Flips the values in an array in the x direction.
void FlipX(const TwoDArray<CalcValueType>& Original, const OLBParams &bparams, TwoDArray<CalcValueType>& Flipped)
{
    for(int x = 0; x < bparams.Xblen(); ++x)
    {
        for(int y = 0; y < bparams.Yblen(); ++y)
        {
            Flipped[y][x] = Original[y][(bparams.Xblen()-1) - x];
        }// y
    }// x
}

//Flips the values in an array in the y direction.
void FlipY(const TwoDArray<CalcValueType>& Original, const OLBParams &bparams, TwoDArray<CalcValueType>& Flipped)
{
    for(int x = 0; x < bparams.Xblen(); ++x)
    {
        for(int y = 0; y < bparams.Yblen(); ++y)
        {
            Flipped[y][x] = Original[(bparams.Yblen()-1) - y][x];
        }// y
    }// x
}
