/* ***** BEGIN LICENSE BLOCK *****
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
* Contributor(s):
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

/*
*
* $Author$
* $Revision$
* $Log$
* Revision 1.4  2004-06-18 16:00:28  tjdwave
* Removed chroma format parameter cformat from CodecParams and derived
* classes to avoid duplication. Made consequential minor mods to
* seq_{de}compress and frame_{de}compress code.
* Revised motion compensation to use built-in arrays for weighting
* matrices and to enforce their const-ness.
* Removed unnecessary memory (de)allocations from Frame class copy constructor
* and assignment operator.
*
* Revision 1.3  2004/05/12 08:35:34  tjdwave
* Done general code tidy, implementing copy constructors, assignment= and const
* correctness for most classes. Replaced Gop class by FrameBuffer class throughout.
* Added support for frame padding so that arbitrary block sizes and frame
* dimensions can be supported.
*
* Revision 1.2  2004/04/11 22:50:46  chaoticcoyote
* Modifications to allow compilation by Visual C++ 6.0
* Changed local for loop declarations into function-wide definitions
* Replaced variable array declarations with new/delete of dynamic array
* Added second argument to allocator::alloc calls, since MS has no default
* Fixed missing and namespace problems with min, max, cos, and abs
* Added typedef unsigned int uint (MS does not have this)
* Added a few missing std:: qualifiers that GCC didn't require
*
* Revision 1.1.1.1  2004/03/11 17:45:43  timborer
* Initial import (well nearly!)
*
* Revision 0.1.0  2004/02/20 09:36:09  thomasd
* Dirac Open Source Video Codec. Originally devised by Thomas Davies,
* BBC Research and Development
*
*/

////////////////////////////////////////////////////////////////
//classes and functions for motion estimation and compensation//
////////////////////////////////////////////////////////////////

#include "libdirac_common/motion.h"
#include <cmath>

//motion compensation stuff//
/////////////////////////////

//arithmetic functions
void ArithAddObj::DoArith(ValueType &lhs, const CalcValueType rhs, const CalcValueType &Weight) const {
	CalcValueType t = ((rhs*Weight)+512)>>10;
	lhs+=short(t);
}

void ArithSubtractObj::DoArith(ValueType &lhs, const CalcValueType rhs, const CalcValueType &Weight) const {
	CalcValueType t = ((rhs*Weight)+512)>>10;
	lhs-=short(t);
}

void ArithHalfAddObj::DoArith(ValueType &lhs, const CalcValueType rhs, const CalcValueType &Weight) const {
	CalcValueType t = ((rhs*Weight)+1024)>>11;
	lhs+=short(t);
}

void ArithHalfSubtractObj::DoArith(ValueType &lhs, const CalcValueType rhs, const CalcValueType &Weight) const {
	CalcValueType t = ((rhs*Weight)+1024)>>11;
	lhs-=short(t);
}


//Overlapping blocks are acheived by applying a 2D raised cosine shape
//to them. This function facilitates the calculations
float RaisedCosine(float t, float B){
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

void CreateBlock(const OLBParams &bparams, bool FullX, bool FullY, TwoDArray<CalcValueType>& WeightArray){

	//Create temporary array.
	TwoDArray<float> CalcArray(WeightArray.length(0),WeightArray.length(1));

	//Calculation variables
	float rolloffX = (float(bparams.XBLEN+1)/float(bparams.XBSEP)) - 1;
	float rolloffY = (float(bparams.YBLEN+1)/float(bparams.YBSEP)) - 1;
	float val;

	//Initialise the temporary array to one
	for(int y = 0; y < bparams.YBLEN; ++y){
		for(int x = 0; x < bparams.XBLEN; ++x){
			CalcArray[y][x] = 1;
		}
	}

	//Window temporary array in the x direction
	for(int y = 0; y < bparams.YBLEN; ++y){
		for(int x = 0; x < bparams.XBLEN; ++x){
			//Apply the window
			if(!FullX){
				if(x >= (bparams.XBLEN)>>1){
					val = (float(x) - (float(bparams.XBLEN-1)/2.0))/float(bparams.XBSEP);
					CalcArray[y][x] *= RaisedCosine(val,rolloffX);
				}
			}
			else{
				val = (float(x) - (float(bparams.XBLEN-1)/2.0))/float(bparams.XBSEP);
				CalcArray[y][x] *= RaisedCosine(val,rolloffX);
			}
		}
	}

	//Window the temporary array in the y direction
	for(int x = 0; x < bparams.XBLEN; ++x){
		for(int y = 0; y < bparams.YBLEN; ++y){
			//Apply the window			
			if(!FullY){
				if(y >= (bparams.YBLEN)>>1){
					val = (float(y) - (float(bparams.YBLEN-1)/2.0))/float(bparams.YBSEP);
					CalcArray[y][x] *= RaisedCosine(val,rolloffY);
				}
			}
			else{
				val = (float(y) - (float(bparams.YBLEN-1)/2.0))/float(bparams.YBSEP);
				CalcArray[y][x] *= RaisedCosine(val,rolloffY);
			}
		}
	}

	//Convert the temporary float array into our
	//weight array by multiplying the floating
	//point values by 1024. This can be removed
	//later using a right shift of ten.
	float g;
	for(int y = 0; y < bparams.YBLEN; ++y){
		for(int x = 0; x < bparams.XBLEN; ++x){
			g = floor((CalcArray[y][x]*1024)+0.5);
			WeightArray[y][x] = ValueType(g);
		}
	}
}

//Flips the values in an array in the x direction.
void FlipX(const TwoDArray<CalcValueType>& Original, const OLBParams &bparams, TwoDArray<CalcValueType>& Flipped){
	for(int x = 0; x < bparams.XBLEN; ++x){
		for(int y = 0; y < bparams.YBLEN; ++y){
			Flipped[y][x] = Original[y][(bparams.XBLEN-1) - x];
		}
	}
}

//Flips the values in an array in the y direction.
void FlipY(const TwoDArray<CalcValueType>& Original, const OLBParams &bparams, TwoDArray<CalcValueType>& Flipped){
	for(int x = 0; x < bparams.XBLEN; ++x){
		for(int y = 0; y < bparams.YBLEN; ++y){
			Flipped[y][x] = Original[(bparams.YBLEN-1) - y][x];
		}
	}
}
