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

#include <libdirac_motionest/me_subpel.h>
#include <libdirac_common/frame_buffer.h>

#include <iostream>

using std::vector;

SubpelRefine::SubpelRefine(EncoderParams& cp): 
    encparams(cp),
    nshift(4),
    lambda(3)
{
	//define the relative coordinates of the four neighbours	
	nshift[0].x = -1; nshift[0].y = 0;
	nshift[1].x = -1; nshift[1].y = -1;
	nshift[2].x = 0; nshift[2].y = -1;
	nshift[3].x = 1; nshift[3].y = -1;
}

void SubpelRefine::DoSubpel(const FrameBuffer& my_buffer,int frame_num, MvData& mvd)
{
	//main loop for the subpel refinement
	int ref1,ref2;

 	//these factors normalise costs for sub-MBs and MBs to those of blocks, so that the overlap is
 	//take into account (e.g. a sub-MB has length XBLEN+XBSEP and YBLEN+YBSEP):
	factor1x1 = float( 16 * encparams.LumaBParams(2).Xblen() * encparams.LumaBParams(2).Yblen() )/
		float( encparams.LumaBParams(0).Xblen() * encparams.LumaBParams(0).Yblen() );

	factor2x2 = float( 4 * encparams.LumaBParams(2).Xblen() * encparams.LumaBParams(2).Yblen() )/
		float( encparams.LumaBParams(1).Xblen() * encparams.LumaBParams(1).Yblen() );

	const FrameSort fsort = my_buffer.GetFrame(frame_num).GetFparams().FSort();

	if (fsort != I_frame)
	{
		if (fsort==L1_frame)
			lambda = encparams.L1MELambda();
		else
			lambda = encparams.L2MELambda();

		mv_data = &mvd;
		matchparams.pic_data = &( my_buffer.GetComponent(frame_num , Y_COMP));

		const vector<int>& refs = my_buffer.GetFrame(frame_num).GetFparams().Refs();
		num_refs = refs.size();
		ref1 = refs[0];
		if (num_refs>1)
			ref2 = refs[1];
		else	
			ref2 = ref1;
		up1_data = &(my_buffer.GetUpComponent( ref1 , Y_COMP));
		up2_data = &(my_buffer.GetUpComponent( ref2 , Y_COMP));

		for (int yblock=0 ; yblock<encparams.YNumBlocks() ; ++yblock)
		{
			for (int xblock=0 ; xblock<encparams.XNumBlocks() ; ++xblock)
			{				
				DoBlock(xblock,yblock);
			}//xblock		
		}//yblock		
	}
}

void SubpelRefine::DoBlock(int xblock,int yblock)
{
	vector<vector<MVector> > vect_list;
	matchparams.vect_list = &vect_list;
	matchparams.me_lambda = lambda;
	const OLBParams& lbparams = encparams.LumaBParams(2);
	matchparams.Init(lbparams,xblock,yblock);

	//do the first reference
	const MvArray* mvarray = &(mv_data->mv1);
	matchparams.ref_data = up1_data;
	matchparams.mv_pred = GetPred(xblock,yblock,*mvarray);
	FindBestMatchSubp(matchparams,(*mvarray)[yblock][xblock],(mv_data->block_costs1)[yblock][xblock]);
	(*mvarray)[yblock][xblock] = matchparams.best_mv;

	if (num_refs>1)
    {
		//do the second reference
		mvarray = &(mv_data->mv2);
		matchparams.ref_data = up2_data;
		matchparams.mv_pred = GetPred(xblock,yblock,*mvarray);
		FindBestMatchSubp(matchparams,(*mvarray)[yblock][xblock],(mv_data->block_costs2)[yblock][xblock]);
		(*mvarray)[yblock][xblock] = matchparams.best_mv;		
	}
}

MVector SubpelRefine::GetPred(int xblock,int yblock,const MvArray& mvarray){
	MVector mv_pred;
	ImageCoords n_coords;
	vector<MVector> neighbours;

	if (xblock>0 && yblock>0 && xblock<mvarray.LastX())
    {

		for (int i=0 ; i<nshift.Length() ; ++i)
        {
			n_coords.x = xblock+nshift[i].x;
			n_coords.y = yblock+nshift[i].y;
			neighbours.push_back(mvarray[n_coords.y][n_coords.x]);

		}// i
	}
	else 
    {
		for (int i=0 ; i<nshift.Length(); ++i )
        {
			n_coords.x = xblock+nshift[i].x;
			n_coords.y = yblock+nshift[i].y;
			if (n_coords.x>=0 && n_coords.y>=0 && n_coords.x<mvarray.LengthX() && n_coords.y<mvarray.LengthY())
				neighbours.push_back(mvarray[n_coords.y][n_coords.x]);
		}// i
	}

	mv_pred = MvMedian(neighbours);

	return mv_pred;
}
