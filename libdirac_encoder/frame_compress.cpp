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
* Contributor(s): Thomas Davies (Original Author), 
*                 Scott R Ladd,
*                 Chris Bowley,
*                 Anuradha Suraparaju,
*                 Tim Borer
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

//Compression of frames//
/////////////////////////

#include <libdirac_encoder/frame_compress.h>
#include <libdirac_encoder/comp_compress.h>
#include <libdirac_common/mot_comp.h>
#include <libdirac_motionest/motion_estimate.h>
#include <libdirac_common/mv_codec.h>
#include <libdirac_common/golomb.h>
#include <libdirac_common/bit_manager.h>
#include <libdirac_common/dirac_assertions.h>
using namespace dirac;

#include <iostream>
#include <sstream>

FrameCompressor::FrameCompressor( EncoderParams& encp ) :
    m_encparams(encp),
    m_me_data(0),
    m_skipped(false),
    m_use_global(false),
    m_use_block_mv(true),
    m_global_pred_mode(REF1_ONLY),
    m_medata_avail(false)
{}

FrameCompressor::~FrameCompressor()
{
    if (m_me_data)
        delete m_me_data;
}

void FrameCompressor::Compress( FrameBuffer& my_buffer ,
                                const FrameBuffer& orig_buffer ,
                                int fnum )
{
    FrameOutputManager& foutput = m_encparams.BitsOut().FrameOutput();

    Frame& my_frame = my_buffer.GetFrame( fnum );
    const FrameParams& fparams = my_frame.GetFparams();
    const FrameSort& fsort = fparams.FSort();
    const ChromaFormat cformat = fparams.CFormat();

    // number of bits written
    unsigned int num_mv_bits;
    m_medata_avail = false;

    CompCompressor my_compcoder(m_encparams , fparams );

    if (m_me_data)
    {
        delete m_me_data;
        m_me_data = 0;
    }

    if ( fsort != I_frame )
    {
        // Set the precision to quarter pixel as standard
        m_encparams.SetMVPrecision( 2 );

        m_me_data = new MEData( m_encparams.XNumMB() , m_encparams.YNumMB(), fparams.NumRefs());

        // Motion estimate first
        MotionEstimator my_motEst( m_encparams );
        my_motEst.DoME( orig_buffer , fnum , *m_me_data );

        // If we have a cut, and an L1 frame, then turn into an I-frame
        AnalyseMEData( *m_me_data );
        if ( m_is_a_cut )
        {
            my_frame.SetFrameSort( I_frame );
            if ( m_encparams.Verbose() )
                std::cerr<<std::endl<<"Cut detected and I-frame inserted!";
        }

    }

   // Set the wavelet filter
   if ( fsort==I_frame )
       m_encparams.SetTransformFilter( APPROX97 );
   else
       m_encparams.SetTransformFilter( FIVETHREE );

    // Write the frame header. We wait until after motion estimation, since
    // this allows us to do cut-detection and (possibly) to decide whether
    // or not to skip a frame before actually encoding anything. However we
    // can do this at any point prior to actually writing any frame data.
    WriteFrameHeader( my_frame.GetFparams() );


    if ( !m_skipped )
    {    // If not skipped we continue with the coding ...

        if ( fsort != I_frame)
        {
             // Code the MV data

            // If we're using global motion parameters, code them
            if (m_use_global)
            {
                /*
                    Code the global motion parameters
                    TBC ....
                */
            }

            // If we're using block motion vectors, code them
            if ( m_use_block_mv )
            {
                MvDataCodec my_mv_coder( &( foutput.MVOutput().Data() ) , 45 , cformat);

                my_mv_coder.InitContexts();//may not be necessary
                num_mv_bits = my_mv_coder.Compress( *m_me_data );            

                UnsignedGolombCode( foutput.MVOutput().Header() , num_mv_bits);
            }

             // Then motion compensate
            MotionCompensator::CompensateFrame( m_encparams , SUBTRACT , 
                                                my_buffer , fnum , 
                                                *m_me_data );
 
        }//?fsort

        //code component data
        my_compcoder.Compress( my_buffer.GetComponent( fnum , Y_COMP) );
        if (cformat != Yonly)
        {
            my_compcoder.Compress( my_buffer.GetComponent( fnum , U_COMP) );
            my_compcoder.Compress( my_buffer.GetComponent( fnum , V_COMP) );
        }

        //motion compensate again if necessary
        if ( fsort != I_frame )
        {
            if ( fsort!= L2_frame || m_encparams.LocalDecode() )
            {
                MotionCompensator::CompensateFrame( m_encparams , ADD , 
                                                    my_buffer , fnum , 
                                                    *m_me_data );
            }
            // Set me data available flag
            m_medata_avail = true;
        }//?fsort

         //finally clip the data to keep it in range
        my_buffer.GetFrame(fnum).Clip();

    }//?m_skipped
}

void FrameCompressor::WriteFrameHeader( const FrameParams& fparams )
{
    BasicOutputManager& frame_header_op = m_encparams.BitsOut().FrameOutput().HeaderOutput();

    // Write the frame start code
    unsigned char frame_start[5] = { START_CODE_PREFIX_BYTE0, 
                                     START_CODE_PREFIX_BYTE1, 
                                     START_CODE_PREFIX_BYTE2, 
                                     START_CODE_PREFIX_BYTE3, 
                                     IFRAME_START_CODE };
    switch(fparams.FSort())
    {
    case I_frame:
        frame_start[4] = IFRAME_START_CODE;
        break;

    case L1_frame:
        frame_start[4] = L1FRAME_START_CODE;
        break;

    case L2_frame:
        frame_start[4] = L2FRAME_START_CODE;
         break;

    default:
//         ASSERTM (false, "Frame type is I_frame or L1_frame or L2_frame");
         break;
    }
    frame_header_op.OutputBytes((char *)frame_start, 5);

    // Write the frame number
    UnsignedGolombCode(frame_header_op , fparams.FrameNum());

    //write whether the frame is m_skipped or not
    frame_header_op.OutputBit( m_skipped );

    if (!m_skipped)
    {// If we're not m_skipped, then we write the rest of the metadata

        // Write the expiry time relative to the frame number 
        UnsignedGolombCode( frame_header_op , fparams.ExpiryTime() );

        // Write the frame sort
        UnsignedGolombCode( frame_header_op , (unsigned int) fparams.FSort() );

        // Write the wavelet filter being used
        UnsignedGolombCode( frame_header_op , (unsigned int) m_encparams.TransformFilter() );

        if (fparams.FSort() != I_frame)
        {        
            // If not an I-frame, write how many references there are        
            UnsignedGolombCode( frame_header_op , (unsigned int) fparams.Refs().size() );

            // For each reference, write the reference number relative to the frame number
            for ( size_t i=0 ; i<fparams.Refs().size() ; ++i )
                GolombCode( frame_header_op , fparams.Refs()[i]-fparams.FrameNum() );

            // Indicate whether or not there is global motion vector data
            frame_header_op.OutputBit( m_use_global );

            // Indicate whether or not there is block motion vector data
            frame_header_op.OutputBit( m_use_block_mv );

            // If there is global but no block motion vector data, indicate the 
            // prediction mode to use for the whole frame
            if ( m_use_global && !m_use_block_mv )
            {
                UnsignedGolombCode( frame_header_op , (unsigned int) m_global_pred_mode );
            }
            // Write the motion vector precision being used for the frame
            UnsignedGolombCode( frame_header_op , (unsigned int) m_encparams.MVPrecision() );
        }

    }// ?m_skipped
}

const MEData* FrameCompressor::GetMEData() const
{
    TESTM (m_me_data != NULL, "m_medata allocated");
    TESTM (m_medata_avail == true, "ME Data available");

    return m_me_data;
}

void FrameCompressor::AnalyseMEData( const MEData& me_data )
{
    // Count the number of intra blocks
    const TwoDArray<PredMode>& modes = me_data.Mode();

    int count_intra = 0;
    for ( int j=0 ; j<modes.LengthY() ; ++j )
    {
        for ( int i=0 ; i<modes.LengthX() ; ++i )
        {
            if ( modes[j][i] == INTRA )
                count_intra++;
        }
    }// j
    
    m_intra_ratio = 100.0*static_cast<double>( count_intra ) / 
                          static_cast<double>( modes.LengthX() * modes.LengthY() );

    if ( m_encparams.Verbose() )
        std::cerr<<std::endl<<m_intra_ratio<<"% of blocks are intra   ";

    // Check the size of SAD errors across reference 1    
    const TwoDArray<MvCostData>& pcosts = me_data.PredCosts( 1 );

    // averege SAD across all relevant blocks
    long double sad_average = 0.0;
    // average SAD in a given block
    long double block_average; 
    // the block parameters
    const OLBParams& bparams = m_encparams.LumaBParams( 2 ); 
    //the count of the relevant blocks
    int block_count = 0;

    for ( int j=0 ; j<pcosts.LengthY() ; ++j )
    {
        for ( int i=0 ; i<pcosts.LengthX() ; ++i )
        {

            if ( modes[j][i] == REF1_ONLY || modes[j][i] == REF1AND2 )
            {
                block_average = pcosts[j][i].SAD /
                                static_cast<long double>( bparams.Xblen() * bparams.Yblen() * 4 );
                sad_average += block_average;
                block_count++;
            }

        }// i
    }// j

    if ( block_count != 0)
        sad_average /= static_cast<long double>( block_count );
   
    if ( (sad_average > 30.0) || (m_intra_ratio > 50.0) )
        m_is_a_cut = true;
    else
        m_is_a_cut = false;
  
}
