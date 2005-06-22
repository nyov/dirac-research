/* ***** BEGIN LICENSE BLOCK *****
*
* $Id$ $Name$
*
* Version: MPL 1.1/GPL 2.0/LGPL 2.1
*
* The contents of this file are subject to the Mozilla Public License
* Version 1.1 (the "License");  you may not use this file except in compliance
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
* Contributor(s): Thomas Davies (Original Author), 
*				  Scott R Ladd
*				  Marc Servais
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

#include <libdirac_common/mv_codec.h>

using namespace dirac;

//public functions//
////////////////////
// Constructor for encoding
MvDataCodec::MvDataCodec(BasicOutputManager* bits_out,
						 size_t number_of_contexts,
						 const ChromaFormat& cf)
						 : ArithCodec <MvData> (bits_out,number_of_contexts),
						 m_cformat(cf)
{
	// nada
}        

// Constructor for decoding
MvDataCodec::MvDataCodec(BitInputManager* bits_in,
						 size_t number_of_contexts,
						 const ChromaFormat& cf)
						 : ArithCodec <MvData> (bits_in,number_of_contexts),
						 m_cformat(cf)
{
	// nada
}    

void MvDataCodec::InitContexts() 
{
	for (size_t i = 0;  i < m_context_list.size();  ++i)
		m_context_list[i].SetCounts(1,1);     
}



//protected functions//
///////////////////////

inline void MvDataCodec::Resize(const int context_num)
{
	m_context_list[context_num].HalveCounts();     
}


inline void MvDataCodec::Update( const bool symbol , const int context_num )
{    
	m_context_list[context_num].IncrCount( symbol , 1 ); 

	if ( m_context_list[context_num].Weight()  >= 1024 )
		Resize( context_num ); 
}
inline void MvDataCodec::ResetAll() {}

//coding functions//
////////////////////

//prediction functions

//basic context functions

inline int MvDataCodec::ChooseContext(const MvData& data, const int BinNumber) const
{
	return 0;
}

inline int MvDataCodec::ChooseContext(const MvData& data) const
{
	return 0;
}

inline int MvDataCodec::ChooseSignContext(const MvData& data) const
{
	return 0;
}

//proper context functions

inline int MvDataCodec::ChooseMBSContext(const MvData& data, const int BinNumber) const
{
	if (BinNumber == 1)
		return MB_SPLIT_BIN1_CTX; 
	else
		return MB_SPLIT_BIN2_CTX; 
}

inline int MvDataCodec::ChooseMBCContext(const MvData& data) const
{
	return MB_CMODE_CTX; 
}

inline int MvDataCodec::ChoosePredContext(const MvData& data, const int BinNumber) const
{
	if (BinNumber == 1)
		return PMODE_BIN1_CTX; 
	else if (BinNumber == 2)
		return PMODE_BIN2_CTX; 
	else
		return PMODE_BIN3_CTX; 
}

inline int MvDataCodec::ChooseBlockMotionTypePredContext() const
{
	return BLOCK_USE_GM_CTX; 
}


inline int MvDataCodec::ChooseREF1xContext(const MvData& data, const int BinNumber) const
{
	if (BinNumber == 1)
		return REF1x_BIN1_CTX; 
	else if (BinNumber == 2)
		return REF1x_BIN2_CTX; 
	else if (BinNumber == 3)
		return REF1x_BIN3_CTX; 
	else if (BinNumber == 4)
		return REF1x_BIN4_CTX; 
	else
		return REF1x_BIN5plus_CTX; 
}

inline int MvDataCodec::ChooseREF1xSignContext(const MvData& data) const
{
	return REF1x_SIGN0_CTX;
}

inline int MvDataCodec::ChooseREF1yContext(const MvData& data, const int BinNumber) const
{
	if (BinNumber == 1)
		return REF1y_BIN1_CTX; 
	else if (BinNumber == 2)
		return REF1y_BIN2_CTX; 
	else if (BinNumber == 3)
		return REF1y_BIN3_CTX; 
	else if (BinNumber == 4)
		return REF1y_BIN4_CTX; 
	else
		return REF1y_BIN5plus_CTX; 
}

inline int MvDataCodec::ChooseREF1ySignContext(const MvData& data) const
{
	return REF1y_SIGN0_CTX;
}

inline int MvDataCodec::ChooseREF2xContext(const MvData& data, const int BinNumber) const
{
	if (BinNumber == 1)
		return REF2x_BIN1_CTX; 
	else if (BinNumber == 2)
		return REF2x_BIN2_CTX; 
	else if (BinNumber == 3)
		return REF2x_BIN3_CTX; 
	else if (BinNumber == 4)
		return REF2x_BIN4_CTX; 
	else
		return REF2x_BIN5plus_CTX; 
}

inline int MvDataCodec::ChooseREF2xSignContext(const MvData& data) const
{
	return REF2x_SIGN0_CTX; 
}

inline int MvDataCodec::ChooseREF2yContext(const MvData& data, const int BinNumber) const
{
	if (BinNumber == 1)
		return REF2y_BIN1_CTX; 
	else if (BinNumber == 2)
		return REF2y_BIN2_CTX; 
	else if (BinNumber == 3)
		return REF2y_BIN3_CTX; 
	else if (BinNumber == 4)
		return REF2y_BIN4_CTX; 
	else
		return REF2y_BIN5plus_CTX; 
}

inline int MvDataCodec::ChooseREF2ySignContext(const MvData& data) const
{
	return REF2y_SIGN0_CTX; 
}

inline int MvDataCodec::ChooseYDCContext(const MvData& data, const int BinNumber) const
{
	if (BinNumber == 1)
		return YDC_BIN1_CTX; 
	else
		return YDC_BIN2plus_CTX; 
}

inline int MvDataCodec::ChooseUDCContext(const MvData& data, const int BinNumber) const
{
	if (BinNumber == 1)
		return UDC_BIN1_CTX; 
	else
		return UDC_BIN2plus_CTX; 
}

inline int MvDataCodec::ChooseVDCContext(const MvData& data, const int BinNumber) const
{
	if (BinNumber == 1)
		return VDC_BIN1_CTX; 
	else
		return VDC_BIN2plus_CTX; 
}

inline int MvDataCodec::ChooseYDCSignContext(const MvData& data) const
{
	return YDC_SIGN0_CTX; 
}

inline int MvDataCodec::ChooseUDCSignContext(const MvData& data) const
{
	return UDC_SIGN0_CTX; 
}

inline int MvDataCodec::ChooseVDCSignContext(const MvData& data) const
{
	return VDC_SIGN0_CTX; 
}

inline int MvDataCodec::ChooseGlobalMotionSignContext(const MvData& data, const int ref_num) const
{
	if (ref_num==1)
		return GLOBAL_SIGN_CTX;
	else
		return GLOBAL_SIGN_CTX;
}

inline int MvDataCodec::ChooseGlobalMotionMagContext(const MvData& data, const int param) const
{
	if ((param==4) || (param==5))
		return GLOBAL_MAG_b_CTX;
	else
		return GLOBAL_MAG_Ac_CTX;
}


inline unsigned int MvDataCodec::MBSplitPrediction(const TwoDArray<int> & split_data ) const
{    
	int result = 0;

	std::vector < unsigned int >  nbrs;

	if (mb_xp > 0 && mb_yp > 0)
	{
		nbrs.push_back( split_data[mb_yp-1][mb_xp] ); 
		nbrs.push_back( split_data[mb_yp-1][mb_xp-1] ); 
		nbrs.push_back( split_data[mb_yp][mb_xp-1] ); 

		result = GetMean(nbrs);     
	}
	else if (mb_xp > 0 && mb_yp == 0)
		result = split_data[mb_yp][mb_xp-1]; 
	else if (mb_xp == 0 && mb_yp > 0)
		result =  split_data[mb_yp-1][mb_xp]; 

	return result; 
}

inline bool MvDataCodec::MBCBModePrediction(const TwoDArray <bool> & cbm_data) const
{
	bool result = true;
	std::vector < unsigned int >  nbrs; 

	if (mb_xp > 0 && mb_yp > 0)
	{
		nbrs.push_back( (unsigned int)( cbm_data[mb_yp-1][mb_xp] ) ); 
		nbrs.push_back( (unsigned int)( cbm_data[mb_yp-1][mb_xp-1] ) ); 
		nbrs.push_back( (unsigned int)( cbm_data[mb_yp][mb_xp-1] ) ); 

		result = bool(GetMean(nbrs));     
	}
	else if (mb_xp > 0 && mb_yp == 0)
		result = cbm_data[mb_yp][mb_xp-1]; 
	else if (mb_xp == 0 && mb_yp > 0)
		result = cbm_data[mb_yp-1][mb_xp]; 

	return result; 
}

inline unsigned int MvDataCodec::BlockModePrediction(const TwoDArray < PredMode > & preddata) const
{
	unsigned int result = (unsigned int)(REF1_ONLY);
	std::vector <unsigned int> nbrs; 

	if (b_xp > 0 && b_yp > 0)
	{
		nbrs.push_back( (unsigned int)( preddata[b_yp-1][b_xp] ) ); 
		nbrs.push_back( (unsigned int)( preddata[b_yp-1][b_xp-1] ) ); 
		nbrs.push_back( (unsigned int)( preddata[b_yp][b_xp-1] ) ); 

		result = GetMean(nbrs);
	}
	else if (b_xp > 0 && b_yp == 0)
		result = preddata[b_yp][b_xp-1]; 
	else if (b_xp == 0 && b_yp > 0)
		result = preddata[b_yp-1][b_xp]; 

	return result; 
}

inline bool MvDataCodec::BlockMotionTypePrediction(const TwoDArray < bool > & preddata) const
{
	bool result = true;
	std::vector <unsigned int> nbrs; 

	if (b_xp > 0 && b_yp > 0)
	{
		nbrs.push_back( (unsigned int)( preddata[b_yp-1][b_xp] ) ); 
		nbrs.push_back( (unsigned int)( preddata[b_yp-1][b_xp-1] ) ); 
		nbrs.push_back( (unsigned int)( preddata[b_yp][b_xp-1] ) ); 

		result = (bool)GetMean(nbrs);
	}
	else if (b_xp > 0 && b_yp == 0)
		result = preddata[b_yp][b_xp-1]; 
	else if (b_xp == 0 && b_yp > 0)
		result = preddata[b_yp-1][b_xp]; 

	return result; 
}


inline bool MvDataCodec::MacroBlockMotionTypePrediction(const TwoDArray < bool > & preddata) const
{
	bool result = true;
	std::vector <unsigned int> nbrs; 

	if (mb_xp > 0 && mb_yp > 0)
	{
		nbrs.push_back( (unsigned int)( preddata[mb_yp-1][mb_xp] ) ); 
		nbrs.push_back( (unsigned int)( preddata[mb_yp-1][mb_xp-1] ) ); 
		nbrs.push_back( (unsigned int)( preddata[mb_yp][mb_xp-1] ) ); 

		result = (bool)GetMean(nbrs);
	}
	else if (mb_xp > 0 && mb_yp == 0)
		result = preddata[mb_yp][mb_xp-1]; 
	else if (mb_xp == 0 && mb_yp > 0)
		result = preddata[mb_yp-1][mb_xp]; 

	return result; 
}



inline MVector MvDataCodec::Mv1Prediction(const MvArray& mvarray,
										  const TwoDArray < PredMode > & preddata) const
{
	std::vector <MVector>  nbrs; 
	PredMode pmode;     
	MVector result; 

	if (b_xp > 0 && b_yp > 0)
	{
		pmode = preddata[b_yp-1][b_xp]; 
		if (pmode == REF1_ONLY || pmode == REF1AND2) 
			nbrs.push_back(mvarray[b_yp-1][b_xp]); 

		pmode = preddata[b_yp-1][b_xp-1]; 
		if (pmode == REF1_ONLY || pmode == REF1AND2)
			nbrs.push_back(mvarray[b_yp-1][b_xp-1]); 

		pmode = preddata[b_yp][b_xp-1]; 
		if (pmode == REF1_ONLY || pmode == REF1AND2)        
			nbrs.push_back(mvarray[b_yp][b_xp-1]); 

		if (nbrs.size() > 0)
			result = MvMedian(nbrs); 
	}
	else if (b_xp > 0 && b_yp == 0)
	{
		pmode = preddata[0][b_xp-1]; 
		if (pmode == REF1_ONLY || pmode == REF1AND2)
			result = mvarray[0][b_xp-1]; 
	}
	else if (b_xp == 0 && b_yp > 0)
	{
		pmode = preddata[b_yp-1][0]; 
		if (pmode == REF1_ONLY || pmode == REF1AND2)
			result = mvarray[b_yp-1][0]; 
	}

	return result; 
}

inline MVector MvDataCodec::Mv2Prediction(const MvArray & mvarray,
										  const TwoDArray < PredMode >  & preddata) const
{
	std::vector <MVector>  nbrs; 
	PredMode pmode; 
	MVector result; 

	if (b_xp > 0 && b_yp > 0)
	{
		pmode = preddata[b_yp-1][b_xp]; 
		if (pmode == REF2_ONLY || pmode == REF1AND2)
			nbrs.push_back(mvarray[b_yp-1][b_xp]); 

		pmode = preddata[b_yp-1][b_xp-1]; 
		if (pmode == REF2_ONLY || pmode == REF1AND2)
			nbrs.push_back(mvarray[b_yp-1][b_xp-1]); 

		pmode = preddata[b_yp][b_xp-1]; 
		if (pmode == REF2_ONLY || pmode == REF1AND2)
			nbrs.push_back(mvarray[b_yp][b_xp-1]); 

		if (nbrs.size() > 0)
			result = MvMedian(nbrs); 
	}
	else if (b_xp > 0 && b_yp == 0)
	{
		pmode = preddata[0][b_xp-1]; 
		if(pmode == REF2_ONLY || pmode == REF1AND2)
			result = mvarray[0][b_xp-1]; 
	}
	else if (b_xp == 0 && b_yp > 0)
	{
		pmode = preddata[b_yp-1][0]; 
		if(pmode == REF2_ONLY || pmode == REF1AND2)
			result = mvarray[b_yp-1][0]; 
	}

	return result; 
}

inline ValueType MvDataCodec::DCPrediction(const TwoDArray < ValueType > & dcdata,
										   const TwoDArray < PredMode > & preddata) const
{
	std::vector < int >  nbrs; 
	PredMode pmode;
	ValueType result = 128; 

	if (b_xp > 0 && b_yp > 0)
	{
		pmode = preddata[b_yp-1][b_xp]; 
		if (pmode == INTRA) 
			nbrs.push_back(int(dcdata[b_yp-1][b_xp])); 

		pmode = preddata[b_yp-1][b_xp-1]; 
		if (pmode == INTRA)
			nbrs.push_back(int(dcdata[b_yp-1][b_xp-1])); 

		pmode = preddata[b_yp][b_xp-1]; 
		if (pmode == INTRA)        
			nbrs.push_back(int(dcdata[b_yp][b_xp-1])); 

		if (nbrs.size() > 0)
			result = ValueType(GetMean(nbrs));     
	}
	else if (b_xp > 0 && b_yp == 0)
	{
		pmode = preddata[0][b_xp-1]; 
		if (pmode == INTRA)
			result = dcdata[0][b_xp-1]; 
	}
	else if (b_xp == 0 && b_yp > 0)
	{
		pmode = preddata[b_yp-1][0]; 
		if (pmode == INTRA)
			result = dcdata[b_yp-1][0]; 
	}

	return result;
}


bool MvDataCodec::IsPartlyInterMacroBlock( MvData& in_data )
{
	// returns true if there is at least one non-intra Prediction Unit in the Macro-Block
	int split_depth = in_data.MBSplit()[mb_yp][mb_xp]; 
	int step = 4  >>  (split_depth); 
	bool IsPartlyInter = false;

	for (int y = mb_tlb_y; y < mb_tlb_y+4; y += step)
		for (int x = mb_tlb_x; x < mb_tlb_x+4; x += step)
			IsPartlyInter = IsPartlyInter || (in_data.Mode()[y][x] != INTRA);

	return IsPartlyInter;
}		

void MvDataCodec::DoWorkCode( MvData& in_data )
{
	int step,max; 
	int pstep,pmax; 
	int split_depth; 
	bool common_ref = false; 
	bool MB_has_inter_PU;
	PredMode common_mode;

	std::cerr<<std::endl<<"==> Encoder: Global Motion Flag = " << in_data.m_use_global;
	std::cerr<<";  Global Motion Only Flag = " << in_data.m_use_global_only;
	if (in_data.m_use_global)
	{
		in_data.QuantiseGlobalMotionParameters();
		CodeGlobalMotionParameters(in_data);
		in_data.DequantiseGlobalMotionParameters();
		in_data.GenerateGlobalMotionVectors();
	}

	MB_count = 0; 

	for (mb_yp = 0, mb_tlb_y = 0; mb_yp <  in_data.MBSplit().LengthY(); ++mb_yp, mb_tlb_y += 4)
	{
		for (mb_xp = 0,mb_tlb_x = 0; mb_xp <  in_data.MBSplit().LengthX(); ++mb_xp,mb_tlb_x += 4)
		{

			if (FLAG_GLOBAL_MOTION_BY_MACRO_BLOCK)
				MB_has_inter_PU = false;

			//start with split mode
			CodeMBSplit(in_data); 
			split_depth = in_data.MBSplit()[mb_yp][mb_xp]; 
			step = 4  >>  (split_depth); 
			max = (1 << split_depth); 

			//next do common_ref
			if(split_depth != 0)
			{
				if (USE_COMMON_MODE_FLAG)
					CodeMBCom(in_data); 
				pstep = step; 
				pmax = max; 
			}
			else // Pred. Unit (PU) is the whole MB
			{
				if (USE_COMMON_MODE_FLAG)
					in_data.MBCommonMode()[mb_yp][mb_xp] = true; 
				pstep = 4; 
				pmax = 1; 
			}

			//std::cerr<<std::endl<<"***** Prediction Modes: ";

			if (USE_COMMON_MODE_FLAG)
			{
				common_ref = in_data.MBCommonMode()[mb_yp][mb_xp]; 
				if (common_ref)
				{
					b_yp = mb_tlb_y;
					b_xp = mb_tlb_x;
					common_mode = in_data.Mode()[b_yp][b_xp];
					CodePredmode(in_data);
					//std::cerr<<"(Common Mode = "<<common_mode<<") ";
				}
			}


			//do prediction modes            
			for (b_yp = mb_tlb_y; b_yp < mb_tlb_y+4; b_yp += pstep)
			{
				for (b_xp = mb_tlb_x; b_xp < mb_tlb_x+4; b_xp += pstep)
				{
					if (USE_COMMON_MODE_FLAG && common_ref)
						in_data.Mode()[b_yp][b_xp] = common_mode; 
					else
						CodePredmode(in_data);
					
					//std::cerr<<in_data.Mode()[b_yp][b_xp];
					
					if (FLAG_GLOBAL_MOTION_BY_MACRO_BLOCK)
						MB_has_inter_PU = MB_has_inter_PU || (in_data.Mode()[b_yp][b_xp] != INTRA);
				}
			}

			step = 4 >> (split_depth);   // 1, 2 or 4          

			if (FLAG_GLOBAL_MOTION_BY_MACRO_BLOCK && !MB_has_inter_PU)
				std::cerr<<std::endl<<"Macroblock at ("<<mb_yp<<", "<<mb_xp<<") has ONLY intra prediction unit(s)";

			if (pstep!=step)
				std::cerr<<std::endl<<"********** step = "<<step<<",  pstep = "<<pstep<< "********** ";

			if (FLAG_GLOBAL_MOTION_BY_MACRO_BLOCK) 
			{
				if(in_data.m_use_global) // if using Global Motion
				{
					if(MB_has_inter_PU/*IsPartlyInterMacroBlock(in_data)*/) // if MB is not totally Intra
					{
						if(!in_data.m_use_global_only) // if not using Global Motion exclusively: 
							CodeMacroBlockMotionType(in_data); // encode whether or not it gets used for current Macro-Block		
						
						// Flag constituent PUs as using Global/Block Motion:
						for (b_yp = mb_tlb_y; b_yp < mb_tlb_y+4; b_yp += step)
							for (b_xp = mb_tlb_x; b_xp < mb_tlb_x+4; b_xp += step)
								in_data.BlockUseGlobal()[b_yp][b_xp] = in_data.MacroBlockUseGlobal()[mb_yp][mb_xp]; 
					}
				}
			}


			//now do all the block mvs in the mb            
			for (b_yp = mb_tlb_y; b_yp < mb_tlb_y+4; b_yp += step)
			{
				for (b_xp = mb_tlb_x; b_xp < mb_tlb_x+4; b_xp += step)
				{

					// If Intra Block, then code the DC
					if(in_data.Mode()[b_yp][b_xp] == INTRA)
						CodeDC(in_data);  

					else // If an Inter Block:
					{

						if(in_data.m_use_global_only) // if ONLY using Global Motion: 
							in_data.BlockUseGlobal()[b_yp][b_xp] = true;

						else // if using block (and possibly global) Motion:
						{
							if (!FLAG_GLOBAL_MOTION_BY_MACRO_BLOCK)
								if(in_data.m_use_global) // if using Global Motion
									CodeBlockMotionType(in_data); // encode whether or not it gets used for current Pred. Unit				

							if(!in_data.BlockUseGlobal()[b_yp][b_xp]) // if not using Global Motion for the current Pred. Unit
							{
								if (in_data.Mode()[b_yp][b_xp] == REF1_ONLY || in_data.Mode()[b_yp][b_xp] == REF1AND2 )
									CodeMv1(in_data); 

								if (in_data.Mode()[b_yp][b_xp] == REF2_ONLY || in_data.Mode()[b_yp][b_xp] == REF1AND2 )
									CodeMv2(in_data);
							}
						}


						// propagate throughout Prediction Unit:

						if (in_data.BlockUseGlobal()[b_yp][b_xp]) // if using Global Motion for the current Pred. Unit
						{
							for (int y = b_yp;  y < b_yp + pstep;  y++)
							{
								for (int x = b_xp;  x < b_xp + pstep;  x++) 
								{
									in_data.Vectors(1)[y][x].x = in_data.GlobalMotionVectors(1)[y][x].x; 
									in_data.Vectors(1)[y][x].y = in_data.GlobalMotionVectors(1)[y][x].y; 
									in_data.Vectors(2)[y][x].x = in_data.GlobalMotionVectors(2)[y][x].x; 
									in_data.Vectors(2)[y][x].y = in_data.GlobalMotionVectors(2)[y][x].y;
								}
							}
						}
						else // if not using Global Motion for the current Pred. Unit
						{
							for (int y = b_yp;  y < b_yp + pstep;  y++) 
							{
								for (int x = b_xp;  x < b_xp + pstep;  x++)
								{
									in_data.Vectors(1)[y][x].x = in_data.Vectors(1)[b_yp][b_xp].x; 
									in_data.Vectors(1)[y][x].y = in_data.Vectors(1)[b_yp][b_xp].y; 
									in_data.Vectors(2)[y][x].x = in_data.Vectors(2)[b_yp][b_xp].x; 
									in_data.Vectors(2)[y][x].y = in_data.Vectors(2)[b_yp][b_xp].y;
								}
							}
						}
					}
				}//b_xp
			}//b_yp   

		}//mb_xp
	}//mb_yp
}


void MvDataCodec::CodeGlobalMotionParameters(const MvData& in_data)	
{
	//code the global motion parameters: still a crude implementation!
	//std::cerr << std::endl << "  Coding global motion parameters...";

	int val;
	int abs_val;

	for (int ref_num=1; ref_num<=2; ref_num++) // assumes there are 2 reference frames. Sometimes there is only 1, so we still need to handle this scenario in a better way.
	{
		for (int param=0; param<8; param++) // loop through each global motion parameter
		{
			// Encode Magnitude:
			val = (int)(in_data.GlobalMotionParameters(ref_num)[param]); 
			abs_val = std::abs(val); 
			for (int bin = 1;  bin  <= abs_val;  ++bin) 
			{
				EncodeSymbol( 0 , ChooseGlobalMotionMagContext( in_data , param ) ); 
			}
			EncodeSymbol( 1 , ChooseGlobalMotionMagContext( in_data , param ) ); 

			// Encode Sign (if magnitude is non-zero):
			if (val != 0) 
			{
				EncodeSymbol( ( (val > 0)? 1 : 0) , ChooseGlobalMotionSignContext( in_data , ref_num ) );
			}
		}
	}
}

void MvDataCodec::CodeMBSplit(const MvData& in_data)
{
	int val = in_data.MBSplit()[mb_yp][mb_xp] - MBSplitPrediction( in_data.MBSplit() ); 

	if (val < 0)
		val += 3; //produce prediction mod 3    

	int ctx; 

	for (int bin = 1; bin <= val; ++bin)
	{
		ctx = ChooseMBSContext(in_data,bin); 
		EncodeSymbol(0,ctx); 
	}

	if (val != 2)//if we've had two zeroes, know we must have value 2
		EncodeSymbol(1,ChooseMBSContext(in_data,val+1)); 
}

void MvDataCodec::CodeMBCom(const MvData& in_data)
{
	bool val = in_data.MBCommonMode()[mb_yp][mb_xp]; 

	if (val != MBCBModePrediction( in_data.MBCommonMode() ))
		EncodeSymbol( 1 , ChooseMBCContext( in_data ) ); 
	else
		EncodeSymbol( 0 , ChooseMBCContext( in_data ) ); 
}

void MvDataCodec::CodePredmode(const MvData& in_data)
{
	int val = in_data.Mode()[b_yp][b_xp] - BlockModePrediction( in_data.Mode() ); 

	if (val < 0)
		val += 4;  //produce value mod 4    

	for (int bin = 1;  bin<= val;  ++bin)
		EncodeSymbol( 0 , ChoosePredContext( in_data , bin ) ); 

	if (val  !=  3) //if we've had three zeroes, know we must have value 3
		EncodeSymbol( 1 , ChoosePredContext( in_data , val + 1 ) ); 
}

void MvDataCodec::CodeBlockMotionType(const MvData& in_data)
{
	bool val = in_data.BlockUseGlobal()[b_yp][b_xp] == BlockMotionTypePrediction( in_data.BlockUseGlobal() ); 

	if (val == true)
		EncodeSymbol( 1 , ChooseBlockMotionTypePredContext() );
	else 
		EncodeSymbol( 0 , ChooseBlockMotionTypePredContext() );
}


void MvDataCodec::CodeMacroBlockMotionType(const MvData& in_data)
{
	bool val = in_data.MacroBlockUseGlobal()[mb_yp][mb_xp] == MacroBlockMotionTypePrediction( in_data.MacroBlockUseGlobal() ); 

	if (val == true)
		EncodeSymbol( 1 , ChooseBlockMotionTypePredContext() );
	else 
		EncodeSymbol( 0 , ChooseBlockMotionTypePredContext() );
}


void MvDataCodec::CodeMv1(const MvData& in_data )
{
	const MvArray& mv_array = in_data.Vectors(1);
	const MVector pred = Mv1Prediction( mv_array , in_data.Mode() ); 

	const int valx = mv_array[b_yp][b_xp].x - pred.x; 
	const int abs_valx = std::abs(valx); 

	for (int bin = 1;  bin  <= abs_valx;  ++bin)
		EncodeSymbol( 0 , ChooseREF1xContext( in_data , bin ) ); 

	EncodeSymbol( 1 , ChooseREF1xContext( in_data , abs_valx + 1 ) ); 

	if (valx != 0)
		EncodeSymbol( ( (valx > 0)? 1 : 0) , ChooseREF1xSignContext( in_data ) ); 

	const int valy = mv_array[b_yp][b_xp].y - pred.y;      
	const int abs_valy = std::abs( valy );     

	for (int bin = 1; bin<=abs_valy ; ++bin )
		EncodeSymbol( 0 , ChooseREF1yContext( in_data , bin ) );         

	EncodeSymbol( 1 , ChooseREF1yContext( in_data , abs_valy + 1 ) ); 

	if (valy != 0)
		EncodeSymbol( ( (valy > 0)? 1 : 0) , ChooseREF1ySignContext( in_data ) );  
}

void MvDataCodec::CodeMv2(const MvData& in_data)
{
	const MvArray& mv_array = in_data.Vectors(2);
	const MVector pred = Mv2Prediction( mv_array , in_data.Mode() );     

	const int valx = mv_array[b_yp][b_xp].x - pred.x;  
	const int abs_valx = abs(valx); 

	for (int bin = 1; bin <= abs_valx; ++bin)
		EncodeSymbol( 0 , ChooseREF2xContext( in_data , bin ) ); 

	EncodeSymbol( 1 , ChooseREF2xContext( in_data , abs_valx + 1 ) ); 

	if (valx != 0)
		EncodeSymbol( ( (valx > 0)? 1 : 0) , ChooseREF2xSignContext( in_data ) ); 

	const int valy = mv_array[b_yp][b_xp].y-pred.y;  
	const int abs_valy = std::abs(valy);     

	for (int bin = 1; bin<=abs_valy; ++bin )
		EncodeSymbol( 0 , ChooseREF2yContext( in_data , bin ) ); 

	EncodeSymbol( 1 , ChooseREF2yContext( in_data , abs_valy + 1 ) ); 

	if (valy != 0)
		EncodeSymbol( ( (valy > 0)? 1 : 0) , ChooseREF2ySignContext( in_data ) ); 
}

void MvDataCodec::CodeDC(const MvData& in_data)
{    
	//begin with Y DC value    
	const ValueType valY = in_data.DC( Y_COMP )[b_yp][b_xp]
	- DCPrediction( in_data.DC(Y_COMP) , in_data.Mode() ); 
	const ValueType abs_valY = std::abs( valY ); 

	for (ValueType bin = 1; bin <= abs_valY; ++bin)
		EncodeSymbol( 0 , ChooseYDCContext( in_data , bin ) ); 

	EncodeSymbol( 1 , ChooseYDCContext (in_data , abs_valY + 1 ) ); 

	if (valY != 0)
		EncodeSymbol( ( (valY > 0)? 1 : 0) , ChooseYDCSignContext( in_data ) ); 

	//now do U and V if necessary
	if (m_cformat != Yonly)
	{
		//continue with U and V DC values
		const int valU =  in_data.DC(U_COMP)[b_yp][b_xp]
		- DCPrediction(in_data.DC( U_COMP ) , in_data.Mode()); 
		const int abs_valU = std::abs( valU ); 

		for (ValueType bin = 1;  bin<=abs_valU ; ++bin)
			EncodeSymbol( 0 , ChooseUDCContext( in_data , bin ) ); 

		EncodeSymbol( 1 , ChooseUDCContext( in_data , abs_valU + 1 ) ); 

		if (valU != 0)
			EncodeSymbol( ( (valU > 0) ? 1 : 0) , ChooseUDCSignContext( in_data ) ); 

		const int valV = in_data.DC( V_COMP )[b_yp][b_xp] 
		- DCPrediction( in_data.DC( V_COMP ) , in_data.Mode() ); 
		const int abs_valV = std::abs( valV ); 

		for (ValueType bin = 1; bin<=abs_valV ; ++bin)
			EncodeSymbol( 0 , ChooseVDCContext( in_data , bin ) ); 

		EncodeSymbol( 1 , ChooseVDCContext( in_data , abs_valV + 1 ) ); 

		if (valV != 0)
			EncodeSymbol( ( (valV > 0)? 1 : 0) , ChooseVDCSignContext( in_data ) ); 
	}
}

//decoding functions//
//////////////////////

void MvDataCodec::DoWorkDecode( MvData& out_data, int num_bits)
{
	int step,max; 
	int pstep,pmax;     
	int split_depth; 
	bool common_ref = false; 
	int xstart,ystart;  
	bool MB_has_inter_PU;
	PredMode common_mode;

	std::cerr<<std::endl<<"==> Decoder: Global Motion Flag = " << out_data.m_use_global;
	std::cerr<<";  Global Motion Only Flag = " << out_data.m_use_global_only;
	if (out_data.m_use_global)
	{
		DecodeGlobalMotionParameters(out_data);
		out_data.DequantiseGlobalMotionParameters();
		out_data.GenerateGlobalMotionVectors();
	}


	for (mb_yp = 0, mb_tlb_y = 0; mb_yp < out_data.MBSplit().LengthY(); ++mb_yp, mb_tlb_y += 4)
	{
		for (mb_xp = 0,mb_tlb_x = 0; mb_xp < out_data.MBSplit().LengthX(); ++mb_xp,mb_tlb_x += 4)
		{

			if (FLAG_GLOBAL_MOTION_BY_MACRO_BLOCK)
				MB_has_inter_PU = false;

			//start with split mode
			DecodeMBSplit( out_data ); 
			split_depth = out_data.MBSplit()[mb_yp][mb_xp]; 
			step =  4  >>  (split_depth); 
			max  = (1 << split_depth); 

			//next do common_ref
			if(split_depth  !=  0)
			{
				if (USE_COMMON_MODE_FLAG)
					DecodeMBCom( out_data ); 
				pstep = step; 
				pmax = max; 
			}
			else
			{
				if (USE_COMMON_MODE_FLAG)
					out_data.MBCommonMode()[mb_yp][mb_xp] = true; 
				pstep = 4; 
				pmax = 1; 
			}

			//std::cerr<<std::endl<<"***** Prediction Modes: ";

			if (USE_COMMON_MODE_FLAG)
			{	common_ref = out_data.MBCommonMode()[mb_yp][mb_xp]; 
				if (common_ref)
				{	
					b_yp = mb_tlb_y;
					b_xp = mb_tlb_x;
					DecodePredmode(out_data);
					common_mode = out_data.Mode()[b_yp][b_xp];
					//std::cerr<<"(Common Mode = "<<common_mode<<") ";
				}
			}


			// do prediction modes
			for (b_yp = mb_tlb_y;  b_yp < mb_tlb_y + 4;  b_yp += pstep)
			{                
				for (b_xp = mb_tlb_x; b_xp < mb_tlb_x + 4;  b_xp += pstep)
				{
					if (USE_COMMON_MODE_FLAG && common_ref)
						out_data.Mode()[b_yp][b_xp] = common_mode; 
					else
						DecodePredmode(out_data); 
					
					//std::cerr<<out_data.Mode()[b_yp][b_xp];

					// propagate throughout Prediction Unit                
					for (int y = b_yp;  y < b_yp + pstep;  y++)
						for (int x = b_xp;  x < b_xp + pstep;  x++)
							out_data.Mode()[y][x] = out_data.Mode()[b_yp][b_xp];    

					if (FLAG_GLOBAL_MOTION_BY_MACRO_BLOCK)
						MB_has_inter_PU = MB_has_inter_PU || (out_data.Mode()[b_yp][b_xp] != INTRA);

				}
			}

			if (FLAG_GLOBAL_MOTION_BY_MACRO_BLOCK && !MB_has_inter_PU)
				std::cerr<<std::endl<<"Macroblock at ("<<mb_yp<<", "<<mb_xp<<") has ONLY intra prediction unit(s)";

			if (pstep!=step)
				std::cerr<<std::endl<<"********** step = "<<step<<",  pstep = "<<pstep<< "********** ";

			if (FLAG_GLOBAL_MOTION_BY_MACRO_BLOCK) 
			{
				if(out_data.m_use_global) // if using Global Motion
				{
					if(MB_has_inter_PU/*IsPartlyInterMacroBlock(out_data)*/) // if MB is not totally Intra
					{
						if(!out_data.m_use_global_only) // if not using Global Motion exclusively: 
							DecodeMacroBlockMotionType(out_data); // encode whether or not it gets used for current Macro-Block		

						// Flag constituent PUs as using Global/Block Motion:
						for (b_yp = mb_tlb_y; b_yp < mb_tlb_y+4; b_yp += step)
							for (b_xp = mb_tlb_x; b_xp < mb_tlb_x+4; b_xp += step)
								out_data.BlockUseGlobal()[b_yp][b_xp] = out_data.MacroBlockUseGlobal()[mb_yp][mb_xp]; 
					}
				}
			}



			//now do all the block mvs in the mb
			for (int j = 0; j < max; ++j)
			{                
				for (int i = 0; i < max; ++i)
				{
					xstart = b_xp = mb_tlb_x + i * step; 
					ystart = b_yp = mb_tlb_y + j * step;

					// If Intra Block, then decode the DC
					if(out_data.Mode()[b_yp][b_xp] == INTRA)
						DecodeDC( out_data ); 

					else // If an Inter Block: 
					{
						if(out_data.m_use_global_only) // if ONLY using Global Motion: 
							out_data.BlockUseGlobal()[b_yp][b_xp] = true;

						else // if using block (and possibly Global) Motion:	
						{
							if (!FLAG_GLOBAL_MOTION_BY_MACRO_BLOCK)
								if(out_data.m_use_global) // if using Global Motion
									DecodeBlockMotionType(out_data); // decode whether or not it gets used for current Pred. Unit

							if(!out_data.BlockUseGlobal()[b_yp][b_xp]) // if not using Global Motion for the current Pred. Unit
							{
								if (out_data.Mode()[b_yp][b_xp] == REF1_ONLY || out_data.Mode()[b_yp][b_xp] == REF1AND2 )
									DecodeMv1(out_data); 

								if (out_data.Mode()[b_yp][b_xp] == REF2_ONLY || out_data.Mode()[b_yp][b_xp] == REF1AND2 )
									DecodeMv2(out_data);
							}
						}
					}

					//propagate throughout PU    
					for (int b_yp2 = ystart; b_yp2 < ystart+step; b_yp2++)
					{
						for (int b_xp2 = xstart; b_xp2 < xstart+step; b_xp2++)
						{        
							if(out_data.Mode()[b_yp][b_xp] == INTRA) {
								out_data.DC( Y_COMP )[b_yp2][b_xp2] = out_data.DC( Y_COMP )[ystart][xstart]; 
								out_data.DC( U_COMP )[b_yp2][b_xp2] = out_data.DC( U_COMP )[ystart][xstart]; 
								out_data.DC( V_COMP )[b_yp2][b_xp2] = out_data.DC( V_COMP )[ystart][xstart];
							}
							else {
								if (out_data.BlockUseGlobal()[b_yp][b_xp])
								{
									out_data.Vectors(1)[b_yp2][b_xp2].x = out_data.GlobalMotionVectors(1)[b_yp2][b_xp2].x; 
									out_data.Vectors(1)[b_yp2][b_xp2].y = out_data.GlobalMotionVectors(1)[b_yp2][b_xp2].y; 
									out_data.Vectors(2)[b_yp2][b_xp2].x = out_data.GlobalMotionVectors(2)[b_yp2][b_xp2].x; 
									out_data.Vectors(2)[b_yp2][b_xp2].y = out_data.GlobalMotionVectors(2)[b_yp2][b_xp2].y; 
								}
								else 
								{
									out_data.Vectors(1)[b_yp2][b_xp2].x = out_data.Vectors(1)[ystart][xstart].x; 
									out_data.Vectors(1)[b_yp2][b_xp2].y = out_data.Vectors(1)[ystart][xstart].y; 
									out_data.Vectors(2)[b_yp2][b_xp2].x = out_data.Vectors(2)[ystart][xstart].x; 
									out_data.Vectors(2)[b_yp2][b_xp2].y = out_data.Vectors(2)[ystart][xstart].y; 
								}
							}
						}
					}
				}//i                    
			}//j

		}//mb_xp
	}//mb_yp


	//std::cerr << std::endl << out_data.Vectors(1);
}


void MvDataCodec::DecodeGlobalMotionParameters(MvData& out_data)	
{
	//decode the global motion parameters: still a crude implementation!
	bool bit;
	int val;

	for (int ref_num=1; ref_num<=2; ref_num++) // assumes there are 2 reference frames. Sometimes there is only 1, so we still need to handle this scenario in a better way 
	{
		for (int param=0; param<8; param++) // loop through each global motion parameter
		{
			val = 0;

			// Decode Magnitude:
			do
			{
				DecodeSymbol( bit , ChooseGlobalMotionMagContext( out_data , param ) ); 
				if ( !bit )
					val++; 
				//bin++; 
			}
			while ( !bit ); 

			// Decode Sign (if magnitude is non-zero):
			if (val != 0)
			{
				DecodeSymbol( bit , ChooseGlobalMotionSignContext( out_data , ref_num ) );        
				if (!bit)
					val = -val; 
			}
			out_data.GlobalMotionParameters(ref_num)[param] = val;
		}
	}
}


void MvDataCodec::DecodeMBSplit(MvData& out_data)
{
	int val = 0; 
	int bin = 1; 
	bool bit; 

	do
	{
		DecodeSymbol( bit , ChooseMBSContext( out_data , bin ) ); 

		if (!bit)
			val++; 

		bin++; 
	}
	while (!bit && val != 2);  

	out_data.MBSplit()[mb_yp][mb_xp] = ( val + MBSplitPrediction( out_data.MBSplit() ) ) % 3;     
}

void MvDataCodec::DecodeMBCom( MvData& out_data )
{
	bool bit; 
	DecodeSymbol( bit , ChooseMBCContext( out_data ) ); 

	if ( bit )
		out_data.MBCommonMode()[mb_yp][mb_xp] = !MBCBModePrediction( out_data.MBCommonMode() ); 
	else
		out_data.MBCommonMode()[mb_yp][mb_xp] = MBCBModePrediction( out_data.MBCommonMode() ); 
}

void MvDataCodec::DecodePredmode( MvData& out_data )
{
	int val = 0; 
	int bin = 1; 
	bool bit; 

	do
	{
		DecodeSymbol( bit , ChoosePredContext( out_data , bin ) ); 

		if (!bit)
			val++; 

		bin++; 
	}
	while (!bit && val != 3);  

	out_data.Mode()[b_yp][b_xp] = PredMode( ( val + BlockModePrediction (out_data.Mode() ) ) %4); 
}


void MvDataCodec::DecodeBlockMotionType( MvData& out_data )
{
	bool bit;

	DecodeSymbol( bit , ChooseBlockMotionTypePredContext() ); 

	if (bit)
		out_data.BlockUseGlobal()[b_yp][b_xp] = BlockMotionTypePrediction( out_data.BlockUseGlobal() ); 
	else
		out_data.BlockUseGlobal()[b_yp][b_xp] = !BlockMotionTypePrediction( out_data.BlockUseGlobal() ); 
}


void MvDataCodec::DecodeMacroBlockMotionType( MvData& out_data )
{
	bool bit;

	DecodeSymbol( bit , ChooseBlockMotionTypePredContext() ); 

	if (bit)
		out_data.MacroBlockUseGlobal()[mb_yp][mb_xp] = MacroBlockMotionTypePrediction( out_data.MacroBlockUseGlobal() ); 
	else
		out_data.MacroBlockUseGlobal()[mb_yp][mb_xp] = !MacroBlockMotionTypePrediction( out_data.MacroBlockUseGlobal() ); 
}

void MvDataCodec::DecodeMv1( MvData& out_data )
{
	MVector pred = Mv1Prediction( out_data.Vectors(1) , out_data.Mode() );     
	int val = 0;
	int bin = 1; 
	bool bit; 

	do
	{
		DecodeSymbol( bit , ChooseREF1xContext( out_data , bin ) ); 

		if ( !bit )
			val++; 

		bin++; 
	}
	while ( !bit ); 

	if (val != 0)
	{
		DecodeSymbol( bit , ChooseREF1xSignContext( out_data ) ); 

		if (!bit)
			val = -val; 
	}

	out_data.Vectors(1)[b_yp][b_xp].x = val + pred.x; 

	val = 0; 
	bin = 1; 

	do
	{
		DecodeSymbol( bit , ChooseREF1yContext( out_data , bin ) ); 

		if ( !bit )
			val++; 

		bin++; 
	}
	while ( !bit ); 

	if (val != 0)
	{
		DecodeSymbol( bit , ChooseREF1ySignContext( out_data ) ); 

		if (!bit)
			val = -val; 
	}

	out_data.Vectors(1)[b_yp][b_xp].y = val + pred.y; 
}

void MvDataCodec::DecodeMv2( MvData& out_data )
{
	MVector pred = Mv2Prediction( out_data.Vectors(2) , out_data.Mode() ); 
	int val = 0; 
	int bin = 1; 
	bool bit; 

	do
	{
		DecodeSymbol( bit , ChooseREF2xContext( out_data , bin ) ); 

		if ( !bit )
			val++; 

		bin++; 
	}
	while ( !bit ); 

	if (val != 0)
	{
		DecodeSymbol( bit , ChooseREF2xSignContext( out_data ) ); 

		if (!bit)
			val = -val; 
	}

	out_data.Vectors(2)[b_yp][b_xp].x = val + pred.x; 

	val = 0; 
	bin = 1; 

	do
	{
		DecodeSymbol( bit , ChooseREF2yContext( out_data , bin ) ); 

		if ( !bit )
			val++; 

		bin++; 
	}
	while ( !bit ); 

	if (val != 0)
	{
		DecodeSymbol( bit , ChooseREF2ySignContext( out_data ) ); 

		if ( !bit )
			val = -val; 
	}

	out_data.Vectors(2)[b_yp][b_xp].y = val + pred.y; 
}

/*
void MvDataCodec::UseGlobalMotionForBlock( MvData& mv_data )
{
	if ((mv_data.Mode()[b_yp][b_xp] == REF1_ONLY) || (mv_data.Mode()[b_yp][b_xp] == REF1AND2))
	{
		mv_data.Vectors(1)[b_yp][b_xp].x = mv_data.GlobalMotionVectors(1)[b_yp][b_xp].x;
		mv_data.Vectors(1)[b_yp][b_xp].y = mv_data.GlobalMotionVectors(1)[b_yp][b_xp].y;
	}
	if ((mv_data.Mode()[b_yp][b_xp] == REF2_ONLY) || (mv_data.Mode()[b_yp][b_xp] == REF1AND2))
	{
		mv_data.Vectors(2)[b_yp][b_xp].x = mv_data.GlobalMotionVectors(2)[b_yp][b_xp].x;
		mv_data.Vectors(2)[b_yp][b_xp].y = mv_data.GlobalMotionVectors(2)[b_yp][b_xp].y;
	}
}
*/

void MvDataCodec::DecodeDC( MvData& out_data )
{
	//begin with Y DC value    
	ValueType val = 0; 
	int bin = 1; 
	bool bit; 

	do
	{
		DecodeSymbol( bit , ChooseYDCContext( out_data , bin ) ); 

		if ( !bit )
			val++; 

		bin++; 
	}
	while ( !bit ); 

	if (val != 0)
	{
		DecodeSymbol( bit , ChooseYDCSignContext( out_data ) ); 

		if (!bit)
			val = -val; 
	}

	out_data.DC( Y_COMP )[b_yp][b_xp] = val + DCPrediction( out_data.DC( Y_COMP ) , out_data.Mode()); 

	if (m_cformat != Yonly)
	{
		//move onto U and V DC values
		val = 0; 
		bin = 1; 

		do
		{
			DecodeSymbol( bit , ChooseUDCContext( out_data , bin ) ); 

			if (!bit)
				val++; 

			bin++; 
		}
		while (!bit); 

		if (val != 0)
		{
			DecodeSymbol( bit , ChooseUDCSignContext ( out_data ) ); 

			if (!bit)
				val = -val; 
		}

		out_data.DC( U_COMP )[b_yp][b_xp] = val + DCPrediction( out_data.DC( U_COMP ) , out_data.Mode()); 

		val = 0; 
		bin = 1; 

		do
		{
			DecodeSymbol( bit , ChooseVDCContext( out_data , bin ) ); 

			if ( !bit )
				val++; 

			bin++; 
		}
		while ( !bit ); 

		if (val != 0)
		{
			DecodeSymbol( bit , ChooseVDCSignContext( out_data ) ); 

			if ( !bit )
				val = -val; 
		}

		out_data.DC( V_COMP )[b_yp][b_xp] = val + DCPrediction( out_data.DC( V_COMP ) , out_data.Mode() ); 
	}
}
