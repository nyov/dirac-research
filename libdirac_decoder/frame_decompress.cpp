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
* Contributor(s): Thomas Davies (Original Author), Scott R Ladd
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


//Decompression of frames
/////////////////////////

#include <libdirac_common/bit_manager.h>
#include <libdirac_decoder/frame_decompress.h>
#include <libdirac_decoder/comp_decompress.h>
#include <libdirac_common/mot_comp.h>
#include <libdirac_common/mv_codec.h>
#include <libdirac_common/golomb.h>

#include <iostream>

using std::vector;

FrameDecompressor::FrameDecompressor(DecoderParams& decp, ChromaFormat cf)
: 
m_decparams(decp),
m_cformat(cf)
{}

bool FrameDecompressor::Decompress(FrameBuffer& my_buffer)
{

	FrameParams fparams( m_cformat , my_buffer.GetFParams().Xl() , my_buffer.GetFParams().Yl() );

 	//Get the frame header (which includes the frame number)
	bool header_failure=ReadFrameHeader(fparams);

	if ( !(m_decparams.BitsIn().End()) && !header_failure )
	{//if we've not finished the data, can proceed

		if ( !m_skipped )
		{//if we're not m_skipped then we can decode the rest of the frame

			if ( m_decparams.Verbose() )
				std::cerr<<std::endl<<"Decoding frame "<<fparams.FrameNum()<<" in display order";		

 			//Add a frame into the buffer ready to receive the data		
			my_buffer.PushFrame(fparams);
			Frame& my_frame = my_buffer.GetFrame(fparams.FrameNum());//Reference to the frame being decoded
			FrameSort fsort = fparams.FSort();
			MvData* mv_data;
			unsigned int num_mv_bits;

			if ( fsort != I_frame )
			{//do all the MV stuff		
				mv_data = new MvData(m_decparams.XNumMB(),m_decparams.YNumMB() , m_decparams.XNumBlocks() , m_decparams.YNumBlocks());

 				//decode mv data
				if (m_decparams.Verbose())
					std::cerr<<std::endl<<"Decoding motion data ...";		
				MvDataCodec my_mv_decoder( &m_decparams.BitsIn(), 50 , m_cformat );
				my_mv_decoder.InitContexts();//may not be necessary
				num_mv_bits = UnsignedGolombDecode( m_decparams.BitsIn() );

 				//Flush to the end of the header for the MV bits			
				m_decparams.BitsIn().FlushInput();

 				//Decompress the MV bits
				my_mv_decoder.Decompress( *mv_data , num_mv_bits );				
			}

 	 	 	//decode components
			CompDecompress( my_buffer,fparams.FrameNum() , Y );
			if ( fparams.CFormat() != Yonly )
			{
				CompDecompress( my_buffer , fparams.FrameNum() , U );		
				CompDecompress( my_buffer , fparams.FrameNum() , V );
			}

			if ( fsort != I_frame )
			{//motion compensate to add the data back in if we don't have an I frame
				MotionCompensator mycomp(m_decparams);
				mycomp.SetCompensationMode(ADD);
				mycomp.CompensateFrame(my_buffer , fparams.FrameNum() , *mv_data);		
				delete mv_data;	
			}
			my_frame.Clip();

			if (m_decparams.Verbose())
				std::cerr<<std::endl;		

		}//?m_skipped,!End()
		else if (m_skipped){
 		//TBD: decide what to return if we're m_skipped. Nearest frame in temporal order??	

		}

 		//exit success
		return EXIT_SUCCESS;
	}
 	//exit failure
	return EXIT_FAILURE;
}

void FrameDecompressor::CompDecompress(FrameBuffer& my_buffer, int fnum,CompSort cs)
{
	if ( m_decparams.Verbose() )
		std::cerr<<std::endl<<"Decoding component data ...";
	CompDecompressor my_compdecoder( m_decparams , my_buffer.GetFrame(fnum).GetFparams() );	
	PicArray& comp_data=my_buffer.GetComponent( fnum , cs );
	my_compdecoder.Decompress( comp_data );
}

bool FrameDecompressor::ReadFrameHeader( FrameParams& fparams )
{

	if ( !m_decparams.BitsIn().End() )
	{
 		//read the frame number
		int temp_int;
		m_decparams.BitsIn().InputBytes((char*) &temp_int,4);
		fparams.SetFrameNum(temp_int);

     	//read whether the frame is m_skipped or not
		m_skipped=m_decparams.BitsIn().InputBit();

		if (!m_skipped)
		{

             //read the expiry time relative to the frame number
			fparams.SetExpiryTime( int( UnsignedGolombDecode( m_decparams.BitsIn() ) ) );

         	//read the frame sort
			fparams.SetFSort( FrameSort( UnsignedGolombDecode( m_decparams.BitsIn() ) ) );

			if ( fparams.FSort() != I_frame ){

 				//if not an I-frame, read how many references there are
				fparams.Refs().clear();
				fparams.Refs().resize( UnsignedGolombDecode( m_decparams.BitsIn() ) );

             	//for each reference, read the reference numbers
				for ( size_t I = 0 ; I < fparams.Refs().size() ; ++I )
				{
					fparams.Refs()[I] = fparams.FrameNum() + GolombDecode( m_decparams.BitsIn() );
				}//I

 				//determine whether or not there is global motion vector data
				m_use_global= m_decparams.BitsIn().InputBit();

                 //determine whether or not there is block motion vector data
				m_use_block_mv= m_decparams.BitsIn().InputBit();

                 //if there is global but no block motion vector data, determine the prediction mode to use
 				//for the whole frame
				if ( m_use_global && !m_use_block_mv )
					m_global_pred_mode= PredMode(UnsignedGolombDecode( m_decparams.BitsIn() ));

			}//?is not an I frame
		}//?m_skipped

 		//flush the header
		m_decparams.BitsIn().FlushInput();

 		//exit success
		return EXIT_SUCCESS;
	}//?m_decparams.BitsIn().End()
	else
	{
 		//exit failure	
		return EXIT_FAILURE;
	}
}
