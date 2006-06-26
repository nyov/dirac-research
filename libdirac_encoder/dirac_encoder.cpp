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
* Contributor(s): Anuradha Suraparaju (Original Author),
*                 Andrew Kennedy
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

#include <cstring>
#include <sstream>
#include <fstream>
#include <queue>
#include <string>
#include <libdirac_common/dirac_assertions.h>
#include <libdirac_common/common.h>
#include <libdirac_common/frame.h>
#include <libdirac_common/pic_io.h>
#include <libdirac_encoder/dirac_encoder.h>
#include <libdirac_encoder/seq_compress.h>
#include <libdirac_byteio/dirac_byte_stream.h>
#include <libdirac_common/video_format_defaults.h>
using namespace dirac;
using namespace std;

template <class T >
void copy_2dArray (const TwoDArray<T> & in, T *out)
{
    for (int j=0 ; j<in.LengthY() ; ++j)
    {
        for (int i=0 ; i<in.LengthX() ; ++i)
        {
            // out[j*in.LengthX() + i] =  in[j][i];
            *out++ =  in[j][i];
        }// i
    }// j
}

void copy_2dArray (const TwoDArray<PredMode> & in, int *out)
{
    for (int j=0 ; j<in.LengthY() ; ++j)
    {
        for (int i=0 ; i<in.LengthX() ; ++i)
        {
            // out[j*in.LengthX() + i] =  in[j][i];
            *out++ =  in[j][i];
        }// i
    }// j
}

void copy_2dArray (const TwoDArray<bool> & in, int *out)
{
    for (int j=0 ; j<in.LengthY() ; ++j)
    {
        for (int i=0 ; i<in.LengthX() ; ++i)
        {
            // out[j*in.LengthX() + i] =  in[j][i];
            *out++ =  in[j][i];
        }// i
    }// j
}

void copy_mv ( const MvArray& mv, dirac_mv_t *dmv)
{
    for (int j=0 ; j<mv.LengthY() ; ++j)
    {
        for (int i=0 ; i<mv.LengthX() ; ++i)
        {
            //dmv[j*mv.LengthX() + i].x = mv[j][i].x;
            //dmv[j*mv.LengthX() + i].y = mv[j][i].y;
            (*dmv).x = mv[j][i].x;
            (*dmv).y = mv[j][i].y;
            dmv++;
        }// i
    }// j
}

void copy_mv_cost (const TwoDArray<MvCostData> &pc, dirac_mv_cost_t *dpc)
{
    for (int j=0 ; j<pc.LengthY() ; ++j)
    {
        for (int i=0 ; i<pc.LengthX() ; ++i)
        {
            //dpc[j*pc.LengthX() + i].SAD = pc[j][i].SAD;
            //dpc[j*pc.LengthX() + i].mvcost = pc[j][i].mvcost;
            (*dpc).SAD = pc[j][i].SAD;
            (*dpc).mvcost = pc[j][i].mvcost;
            dpc++;
        }// i
    }// j
}

/*
    Function that allocates the locally managed instrumentation data
*/
void alloc_instr_data(dirac_instr_t *instr)
{
    instr->mb_split_mode = new int [instr->mb_ylen*instr->mb_xlen];
    memset (instr->mb_split_mode, 0, sizeof(int)*instr->mb_ylen*instr->mb_xlen);

    instr->mb_common_mode = new int [instr->mb_ylen*instr->mb_xlen];
    memset (instr->mb_common_mode, 0, 
                    sizeof(int)*instr->mb_ylen*instr->mb_xlen);

    instr->mb_costs = new float [instr->mb_ylen*instr->mb_xlen];
    memset (instr->mb_costs, 0, sizeof(float)*instr->mb_ylen*instr->mb_xlen);

    instr->pred_mode = new int [instr->mv_ylen * instr->mv_xlen];
    memset (instr->pred_mode, 0, sizeof(int)*instr->mv_ylen*instr->mv_xlen);
        
    instr->intra_costs = new float [instr->mv_ylen * instr->mv_xlen];
    memset (instr->intra_costs, 0, sizeof(float)*instr->mv_ylen*instr->mv_xlen);
        
    instr->bipred_costs = new dirac_mv_cost_t [instr->mv_ylen * instr->mv_xlen];
    memset (instr->bipred_costs, 0, sizeof(dirac_mv_cost_t)*instr->mv_ylen*instr->mv_xlen);
        
    instr->dc_ycomp = new short [instr->mv_ylen * instr->mv_xlen];
    memset (instr->dc_ycomp, 0, sizeof(short)*instr->mv_ylen*instr->mv_xlen);
        
    instr->dc_ucomp = new short [instr->mv_ylen * instr->mv_xlen];
    memset (instr->dc_ucomp, 0, sizeof(short)*instr->mv_ylen*instr->mv_xlen);
        
    instr->dc_vcomp = new short [instr->mv_ylen * instr->mv_xlen];
    memset (instr->dc_vcomp, 0, sizeof(short)*instr->mv_ylen*instr->mv_xlen);
        
    for (int i = 0; i < 2; i++)
    {
        instr->mv[i] = new dirac_mv_t[instr->mv_ylen * instr->mv_xlen];
        memset (instr->mv[i], 0, 
            sizeof(dirac_mv_t)*instr->mv_ylen*instr->mv_xlen);
    }
        
    for (int i = 0; i < 2; i++)
    {
        instr->pred_costs[i] = new dirac_mv_cost_t[instr->mv_ylen * instr->mv_xlen];
        memset (instr->pred_costs[i], 0, 
            sizeof(dirac_mv_cost_t)*instr->mv_ylen*instr->mv_xlen);
    }
}

/*
    Function that frees the locally managed instrumentation data
*/
void dealloc_instr_data(dirac_instr_t *instr)
{
    if (instr->mb_split_mode)
        delete [] instr->mb_split_mode;

    if (instr->mb_common_mode)
        delete [] instr->mb_common_mode;

    if (instr->mb_costs)
        delete [] instr->mb_costs;

    if (instr->pred_mode)
        delete [] instr->pred_mode;

    if (instr->intra_costs)
        delete [] instr->intra_costs;

    if (instr->bipred_costs)
        delete [] instr->bipred_costs;

    if (instr->dc_ycomp)
        delete [] instr->dc_ycomp;

    if (instr->dc_ucomp)
        delete [] instr->dc_ucomp;

    if (instr->dc_vcomp)
        delete [] instr->dc_vcomp;

    for (int i = 0; i < 2; i++)
    {
        if (instr->mv[i])
            delete [] instr->mv[i];
    }
    for (int i = 0; i < 2; i++)
    {
        if (instr->pred_costs[i])
            delete [] instr->pred_costs[i];
    }
}

/*
    Wrapper class around the SequenceCompressor Class. This class is used 
    by the "C" encoder interface
*/
class DiracEncoder
{
public:
    // constructor
    DiracEncoder(const dirac_encoder_context_t *enc_ctx, bool verbose);
    // destructor
    ~DiracEncoder();

    // Load the next frame of uncompressed data into the SequenceCompressor
    bool LoadNextFrame(unsigned char *data, int size);
    // Compress the next frame of data
    int CompressNextFrame();

    // Set the encode frame in encoder to the encoded frame data
    int GetEncodedData(dirac_encoder_t *encoder);
    
    // Set the locally decoded frame data in encoder
    int GetDecodedData (dirac_encoder_t *encoder);

    // Set the instrumentation data in encoder
    void GetInstrumentationData (dirac_encoder_t *encoder);

    // Set the end of sequence infomration in encoder
    int GetSequenceEnd(dirac_encoder_t *encoder);

    // Set the buffer to hold the locally decoded frame
    void SetDecodeBuffer (unsigned char *buffer, int buffer_size);

    // Return the sequence parameters
    const SeqParams& GetSeqParams() const { return m_sparams; }

    // Return the encoder parameters
    const EncoderParams& GetEncParams() const { return m_encparams; }

    // Return the source parameters
    const SourceParams& GetSrcParams() const { return m_srcparams; }

private:

    // Set the sequence parameters
    void SetSequenceParams (const dirac_encoder_context_t *enc_ctx);
    
    // Set the encoder parameters
    void SetEncoderParams (const dirac_encoder_context_t *enc_ctx);

    // Get the frame statistics
    void GetFrameStats(dirac_encoder_t *encoder);

    // Get the seuqence statistics
    void GetSequenceStats(dirac_encoder_t *encoder,
                          const DiracByteStats& dirac_seq_stats);

private:
    // sequence compressor
    SequenceCompressor *m_comp;
    // sequence parameters
    SeqParams m_sparams;
    // Source parameters
    SourceParams m_srcparams;
    // encoder parameters
    EncoderParams m_encparams;
    // locally encoded frame in coded order
    const Frame *m_enc_frame;
    // locally encoded frame ME data
    const MEData *m_enc_medata;
    // locally decoded frame number in display order
    int m_decfnum;
    //locally decoded frame type
    FrameSort m_decfsort;
    // locally decoded frame number in display order
    int m_show_fnum;
    // total number of frame loaded so far
    int m_num_loaded_frames;
    // total number of frames encoded so far
    int m_num_coded_frames;
    // verbose flag
    bool m_verbose;
    // input stream for uncompressed input frame
    MemoryStreamInput m_inp_ptr;
    // output stream for locally decoded frame
    MemoryStreamOutput m_out_ptr;
    // buffer to hold locally decoded frame. Set by SetDecodeBuffer
    unsigned char *m_dec_buf;
    // size of buffer to hold locally decoded data. Set by SetDecodeBuffer
    int m_dec_bufsize;
    // Flag that determines if locally decoded frames are to be returned. Set
    // in Constructor
    bool m_return_decoded_frames;
    // Flag that determines if instrumentation data is to be returned. Set
    // in Constructor
    bool m_return_instr_data;

    // Output destination for compressed data in bitstream format
    DiracByteStream m_dirac_byte_stream;
};

/*
    Instrumentation callback. This function is passed as a parameter to the
    SequenceCompressor constructor. It is called by the 
    FrameCompressor::Compress function once the frame is successfully compressed
*/
void DiracEncoder::GetInstrumentationData (dirac_encoder_t *encoder)
{
    ASSERT (encoder != NULL);
    dirac_instr_t *instr = &encoder->instr;
    dirac_instr_t old_instr = *instr;

    if (!m_return_instr_data)
        return;

    const FrameParams& fparams = m_enc_frame->GetFparams();
    const FrameSort fsort = fparams.FSort();

    instr->fnum = fparams.FrameNum();
    instr->ftype = fsort.IsIntra() ? INTRA_FRAME : INTER_FRAME;
    instr->rtype = fsort.IsRef() ? REFERENCE_FRAME : NON_REFERENCE_FRAME;
    instr->num_refs = 0;
    encoder->instr_data_avail = 1;

    if (fsort.IsIntra())
    {
        // no MV data for Intra coded data
        return;
    }

    TESTM (m_enc_medata != NULL, "ME data available");

    // Reference info
    instr->num_refs = fparams.Refs().size();
    ASSERTM (instr->num_refs <= 2, "Max # reference frames is 2");

    for (int i=0; i<instr->num_refs; ++i)
        instr->refs[i] = fparams.Refs()[i];

    // Block separation params
    instr->ybsep = m_encparams.LumaBParams(2).Ybsep();
    instr->xbsep = m_encparams.LumaBParams(2).Xbsep();

    // Num macroblocks
    instr->mb_ylen = m_enc_medata->MBSplit().LengthY();
    instr->mb_xlen = m_enc_medata->MBSplit().LengthX();

    // Motion vector array dimensions
    instr->mv_ylen = m_enc_medata->Vectors(1).LengthY();
    instr->mv_xlen = m_enc_medata->Vectors(1).LengthX();

    if (old_instr.mb_ylen != instr->mb_ylen || 
        old_instr.mb_xlen != instr->mb_xlen ||
        old_instr.mv_ylen != instr->mv_ylen || 
        old_instr.mv_xlen != instr->mv_xlen)
    {
        dealloc_instr_data(instr);
        alloc_instr_data(instr);
    }

    copy_2dArray (m_enc_medata->MBSplit(), instr->mb_split_mode);
    copy_2dArray (m_enc_medata->MBCommonMode(), instr->mb_common_mode);
    copy_2dArray (m_enc_medata->MBCosts(), instr->mb_costs);
    copy_2dArray (m_enc_medata->Mode(), instr->pred_mode);
    copy_2dArray (m_enc_medata->IntraCosts(), instr->intra_costs);

    if (instr->num_refs > 1)
    {
        copy_mv_cost (m_enc_medata->BiPredCosts(), instr->bipred_costs);
    }

    copy_2dArray (m_enc_medata->DC( Y_COMP ), instr->dc_ycomp);
    if (m_enc_medata->DC().Length() == 3) 
    {
        copy_2dArray (m_enc_medata->DC( U_COMP ), instr->dc_ucomp);
        copy_2dArray (m_enc_medata->DC( V_COMP ), instr->dc_vcomp);
    }

    for (int i=1; i<=instr->num_refs; ++i)
    {
        copy_mv (m_enc_medata->Vectors(i), instr->mv[i-1]);
        copy_mv_cost (m_enc_medata->PredCosts(i), instr->pred_costs[i-1]);
    }
}

DiracEncoder::DiracEncoder(const dirac_encoder_context_t *enc_ctx, 
                           bool verbose) :
    m_sparams(static_cast<VideoFormat>(enc_ctx->enc_params.video_format), false),
    m_srcparams(static_cast<VideoFormat>(enc_ctx->enc_params.video_format), true),
    m_encparams(static_cast<VideoFormat>(enc_ctx->enc_params.video_format), INTER_FRAME, 2, true),
    m_show_fnum(-1),
    m_num_loaded_frames(0),
    m_num_coded_frames(0),
    m_verbose(verbose),
    m_dec_buf(0),
    m_dec_bufsize(0),
    m_return_decoded_frames(enc_ctx->decode_flag > 0),
    m_return_instr_data(enc_ctx->instr_flag > 0)

{
    // Setup sequence parameters
    SetSequenceParams (enc_ctx);
    // Setup encoder parameters
    m_encparams.SetVerbose( verbose );
    SetEncoderParams (enc_ctx);

     // NOTE: FIXME - make video-depth a user-parameter
    m_sparams.SetVideoDepth(8);
    // Set up the input data stream (uncompressed data)
    m_inp_ptr.SetSequenceParams(m_sparams);
    // Set up the output data stream (locally decoded frame)
    m_out_ptr.SetSequenceParams(m_sparams);
    
    // initialise the sequence compressor
    m_comp = new SequenceCompressor (&m_inp_ptr, m_srcparams, m_encparams, m_dirac_byte_stream);

   
}

void DiracEncoder::SetDecodeBuffer (unsigned char *buffer, int buffer_size)
{
    m_dec_buf = buffer;
    m_dec_bufsize = buffer_size;
    m_return_decoded_frames = true;
}

DiracEncoder::~DiracEncoder()
{
    delete m_comp;
}

void DiracEncoder::SetSequenceParams (const dirac_encoder_context_t *enc_ctx)
{
    m_sparams.SetCFormat( enc_ctx->seq_params.chroma );
    m_sparams.SetXl( enc_ctx->seq_params.width );
    m_sparams.SetYl( enc_ctx->seq_params.height );
    m_sparams.SetVideoDepth( enc_ctx->seq_params.video_depth );
    m_srcparams.SetInterlace( enc_ctx->src_params.interlace );
    m_srcparams.SetTopFieldFirst( enc_ctx->src_params.topfieldfirst );
    if (m_srcparams.FrameRate().m_num != (unsigned int)enc_ctx->src_params.frame_rate.numerator || m_srcparams.FrameRate().m_denom != (unsigned int)enc_ctx->src_params.frame_rate.denominator)
    {
        m_srcparams.SetFrameRate( enc_ctx->src_params.frame_rate.numerator,
                            enc_ctx->src_params.frame_rate.denominator );
    }
}

void DiracEncoder::SetEncoderParams (const dirac_encoder_context_t *enc_ctx)
{
    TEST (enc_ctx != NULL);
    OLBParams bparams(12, 12, 8, 8);

    m_encparams.SetLocalDecode(enc_ctx->decode_flag);
    m_encparams.SetQf(enc_ctx->enc_params.qf);
    m_encparams.SetLossless(enc_ctx->enc_params.lossless);
    m_encparams.SetL1Sep(enc_ctx->enc_params.L1_sep);
    m_encparams.SetNumL1(enc_ctx->enc_params.num_L1);
    m_encparams.SetCPD(enc_ctx->enc_params.cpd);
    m_encparams.SetUFactor(1.5f);
    m_encparams.SetVFactor(0.75f);
    m_encparams.SetMVPrecision(enc_ctx->enc_params.mv_precision);
    bparams.SetYblen( enc_ctx->enc_params.yblen );
    bparams.SetXblen( enc_ctx->enc_params.xblen );
    bparams.SetYbsep( enc_ctx->enc_params.ybsep );
    bparams.SetXbsep( enc_ctx->enc_params.xbsep );

    // Now rationalise the GOP options
    // this stuff should really be done in a constructor!
    if (m_encparams.NumL1()<0)
    {
        //don't have a proper GOP
        m_encparams.SetL1Sep( std::max(1 , m_encparams.L1Sep()) );
    }
    else if (m_encparams.NumL1() == 0)
    {
        //have I-frame only coding
        m_encparams.SetL1Sep(0);
    }
    m_encparams.SetOrigXl( enc_ctx->seq_params.width );
    m_encparams.SetOrigYl( enc_ctx->seq_params.height );
    m_encparams.SetBlockSizes( bparams , enc_ctx->seq_params.chroma );

    // Set transforms parameters
    m_encparams.SetIntraTransformFilter(enc_ctx->enc_params.intra_wlt_filter);
    m_encparams.SetInterTransformFilter(enc_ctx->enc_params.inter_wlt_filter);
    m_encparams.SetSpatialPartition(enc_ctx->enc_params.spatial_partition);
    // Set default spatial partitoning to false if wavelet depth is not 
    // equal to default value.
    m_encparams.SetDefaultSpatialPartition(enc_ctx->enc_params.def_spatial_partition && m_encparams.TransformDepth() == enc_ctx->enc_params.wlt_depth );
    m_encparams.SetTransformDepth(enc_ctx->enc_params.wlt_depth);
    m_encparams.SetCodeBlockMode(enc_ctx->enc_params.spatial_partition && enc_ctx->enc_params.multi_quants ? QUANT_MULTIPLE : QUANT_SINGLE);
}


bool DiracEncoder::LoadNextFrame (unsigned char *data, int size)
{
    TESTM (m_comp->Finished() != true, "Did not reach end of sequence");
    m_inp_ptr.SetMembufReference(data, size);
    if (m_comp->LoadNextFrame())
    {
        m_num_loaded_frames++;
        return true;
    }
    return false;
}

int DiracEncoder::CompressNextFrame ()
{
    TESTM (m_comp->Finished() != true, "Did not reach end of sequence");

    if (!m_num_loaded_frames)
        return 0;

    Frame &myframe = m_comp->CompressNextFrame();

    m_enc_frame = m_comp->GetFrameEncoded();
    m_enc_medata = m_comp->GetMEData();

    m_decfnum = -1;
    if (m_return_decoded_frames && 
            myframe.GetFparams().FrameNum() != m_show_fnum)
    {
        int ret_val;
        m_show_fnum = myframe.GetFparams().FrameNum();
        TEST (! (m_return_decoded_frames && !m_dec_buf) );
        if (m_return_decoded_frames && m_dec_buf)
        {
            // write locally decoded frame to decode buffer
            m_out_ptr.SetMembufReference(m_dec_buf, m_dec_bufsize);
            ret_val = m_out_ptr.WriteNextFrame(myframe);

            if (ret_val)
            {
                m_decfnum = m_show_fnum;
                m_decfsort = myframe.GetFparams().FSort();
            }
        }
    }

    if(!m_dirac_byte_stream.IsUnitAvailable())
        return 0;

 
    m_num_coded_frames++;
    TESTM (m_enc_frame != 0, "Encoder frame available");
    
    return 1;
}

void DiracEncoder::GetFrameStats(dirac_encoder_t *encoder)
{
    
    dirac_enc_framestats_t *fstats = &encoder->enc_fstats;
    DiracByteStats dirac_byte_stats = m_dirac_byte_stream.GetLastUnitStats();

    fstats->mv_bits = dirac_byte_stats.GetBitCount(STAT_MV_BYTE_COUNT);
  //  fstats->mv_hdr_bits = foutput.MVBytes() * 8;

    fstats->ycomp_bits = dirac_byte_stats.GetBitCount(STAT_YCOMP_BYTE_COUNT);
  //  fstats->ycomp_hdr_bits = foutput.ComponentHeadBytes( Y_COMP ) * 8;

    fstats->ucomp_bits = dirac_byte_stats.GetBitCount(STAT_UCOMP_BYTE_COUNT);
  //  fstats->ucomp_hdr_bits = foutput.ComponentHeadBytes( U_COMP ) * 8;

    fstats->vcomp_bits = dirac_byte_stats.GetBitCount(STAT_VCOMP_BYTE_COUNT);
   // fstats->vcomp_hdr_bits = foutput.ComponentHeadBytes( V_COMP ) * 8;

    fstats->frame_bits = dirac_byte_stats.GetBitCount(STAT_TOTAL_BYTE_COUNT);
   // fstats->frame_hdr_bits = foutput.FrameHeadBytes() * 8;

    DiracEncoder *compressor = (DiracEncoder *)encoder->compressor;
    if (compressor->GetEncParams().Verbose())
    {
        std::cerr<<std::endl<<"Number of MV bits="<<fstats->mv_bits;
        std::cerr<<std::endl<<"Number of bits for Y="<<fstats->ycomp_bits;
        std::cerr<<std::endl<<"Number of bits for U="<<fstats->ucomp_bits;
        std::cerr<<std::endl<<"Number of bits for V="<<fstats->vcomp_bits;
        std::cerr<<std::endl<<"Total frame bits="<<fstats->frame_bits;
    }
}

int DiracEncoder::GetEncodedData (dirac_encoder_t *encoder)
{
    int size = 0;
    dirac_enc_data_t *encdata = &encoder->enc_buf;

    string output = m_dirac_byte_stream.GetBytes();
    size = output.size();
    //std::cerr << std::endl << "ParseUnit size=" << size << std::endl;
    if (size > 0)
    {
        if (encdata->size < size )
        {
            return -1;
        }
        memmove (encdata->buffer, output.c_str(),  output.size());
        encoder->enc_fparams.fnum = m_enc_frame->GetFparams().FrameNum();
        encoder->enc_fparams.ftype = m_enc_frame->GetFparams().FSort().IsIntra() ? INTRA_FRAME : INTER_FRAME;
        encoder->enc_fparams.rtype = m_enc_frame->GetFparams().FSort().IsRef() ? REFERENCE_FRAME : NON_REFERENCE_FRAME;

        // Get frame statistics
        GetFrameStats (encoder);
        encdata->size = size;

        GetInstrumentationData(encoder);
        encoder->encoded_frame_avail = 1;
    }
    else
    {
        encdata->size = 0;
    }
    
    m_dirac_byte_stream.Clear();

    return size;
}

int DiracEncoder::GetDecodedData (dirac_encoder_t *encoder)
{
    dirac_frameparams_t *fp = &encoder->dec_fparams;

    int ret_stat = (m_decfnum != -1);
    if (m_return_decoded_frames && m_decfnum != -1)
    {
        fp->ftype = m_decfsort.IsIntra() ? INTRA_FRAME : INTER_FRAME;
        fp->rtype = m_decfsort.IsRef() ? REFERENCE_FRAME : NON_REFERENCE_FRAME;
        fp->fnum = m_decfnum;
        encoder->decoded_frame_avail = 1;
        m_decfnum = -1;
    }
    return ret_stat;
}

void DiracEncoder::GetSequenceStats(dirac_encoder_t *encoder,
                                    const DiracByteStats& dirac_seq_stats)
{
    dirac_enc_seqstats_t *sstats = &encoder->enc_seqstats;

    sstats->seq_bits = dirac_seq_stats.GetBitCount(STAT_TOTAL_BYTE_COUNT);
    sstats->mv_bits = dirac_seq_stats.GetBitCount(STAT_MV_BYTE_COUNT);
    sstats->ycomp_bits = dirac_seq_stats.GetBitCount(STAT_YCOMP_BYTE_COUNT);
    sstats->ucomp_bits = dirac_seq_stats.GetBitCount(STAT_UCOMP_BYTE_COUNT);
    sstats->vcomp_bits = dirac_seq_stats.GetBitCount(STAT_VCOMP_BYTE_COUNT);

    sstats->bit_rate = int((sstats->seq_bits * 
                        (double)m_srcparams.FrameRate().m_num)/
                        (m_srcparams.FrameRate().m_denom * m_num_coded_frames));
    
    DiracEncoder *compressor = (DiracEncoder *)encoder->compressor;
    if (compressor->GetEncParams().Verbose())
    {
        std::cerr<<std::endl<<std::endl<<"Total bits for sequence="<<sstats->seq_bits;
        std::cerr<<std::endl<<"Of these: "<<std::endl;
        std::cerr<<std::endl<<sstats->ycomp_bits <<" were Y, ";
        std::cerr<<std::endl<<sstats->ucomp_bits <<" were U, ";
        std::cerr<<std::endl<<sstats->vcomp_bits<<" were V, and ";
        std::cerr<<std::endl<<sstats->mv_bits<<" were motion vector data.";
    }
}

int DiracEncoder::GetSequenceEnd (dirac_encoder_t *encoder)
{
    dirac_enc_data_t *encdata = &encoder->enc_buf;
    DiracByteStats dirac_seq_stats=m_comp->EndSequence();
    string output = m_dirac_byte_stream.GetBytes();
    int size = output.size();
    if (size > 0)
    {
        if (encdata->size < size )
        {
            return -1;
        }
        memmove (encdata->buffer, output.c_str(),  size);
        GetSequenceStats(encoder,
                         dirac_seq_stats);
        encdata->size = size;
    }
    else
    {
        encdata->size = 0;
    }
    m_dirac_byte_stream.Clear();
    return size;
}

static bool InitialiseEncoder (const dirac_encoder_context_t *enc_ctx, bool verbose, dirac_encoder_t *encoder)
{
    TEST (enc_ctx != NULL);
    TEST (encoder != NULL);
    
    if (enc_ctx->seq_params.width == 0 || enc_ctx->seq_params.height == 0)
        return false;

    if (enc_ctx->seq_params.chroma < format444 || 
            enc_ctx->seq_params.chroma >= formatNK)
        return false;

    if (!enc_ctx->src_params.frame_rate.numerator || 
            !enc_ctx->src_params.frame_rate.denominator)
        return false;

    memmove (&encoder->enc_ctx, enc_ctx, sizeof(dirac_encoder_context_t));

    encoder->dec_buf.id = 0;

    switch ( enc_ctx->seq_params.chroma )
    {
    case format420:
         encoder->enc_ctx.seq_params.chroma_width = enc_ctx->seq_params.width/2;
         encoder->enc_ctx.seq_params.chroma_height = enc_ctx->seq_params.height/2;
         break;
    case format422:
         encoder->enc_ctx.seq_params.chroma_width = enc_ctx->seq_params.width/2;
         encoder->enc_ctx.seq_params.chroma_height = enc_ctx->seq_params.height;
         break;
    case format444:
    default:
         encoder->enc_ctx.seq_params.chroma_width = enc_ctx->seq_params.width;
         encoder->enc_ctx.seq_params.chroma_height = enc_ctx->seq_params.height;
         break;
    }

    try
    {
        DiracEncoder *comp = new DiracEncoder (&encoder->enc_ctx, verbose);
    
        encoder->compressor = comp;
        if (encoder->enc_ctx.decode_flag)
        {
            int bufsize = (encoder->enc_ctx.seq_params.width * encoder->enc_ctx.seq_params.height)+ 2*(encoder->enc_ctx.seq_params.chroma_width*encoder->enc_ctx.seq_params.chroma_height);

            encoder->dec_buf.buf[0] = new unsigned char [bufsize];
            encoder->dec_buf.buf[1] = encoder->dec_buf.buf[0] + 
                (encoder->enc_ctx.seq_params.width * encoder->enc_ctx.seq_params.height);
            encoder->dec_buf.buf[2] = encoder->dec_buf.buf[1] + 
                (encoder->enc_ctx.seq_params.chroma_width*encoder->enc_ctx.seq_params.chroma_height);
    
            comp->SetDecodeBuffer (encoder->dec_buf.buf[0], bufsize);
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}
    
static void SetSequenceParameters(dirac_encoder_context_t *enc_ctx,
                                  const VideoFormat& video_format)
{
    TEST (enc_ctx != NULL);
    dirac_seqparams_t &sparams = enc_ctx->seq_params;
    dirac_sourceparams_t &src_params = enc_ctx->src_params;
    
    // create object containing sequence params
    SeqParams default_seq_params(video_format);
    SourceParams default_src_params(video_format);

    sparams.height = default_seq_params.Yl();
    sparams.width = default_seq_params.Xl();
    sparams.chroma = default_seq_params.CFormat();
    sparams.video_depth = default_seq_params.GetVideoDepth();
    src_params.frame_rate.numerator = default_src_params.FrameRate().m_num;
    src_params.frame_rate.denominator = default_src_params.FrameRate().m_denom;
    src_params.interlace = default_src_params.Interlace();
    src_params.topfieldfirst = default_src_params.TopFieldFirst();
}

static void SetEncoderParameters(dirac_encoder_context_t *enc_ctx,
                                 const VideoFormat& video_format)
{
    TEST (enc_ctx != NULL);
    dirac_encparams_t &encparams = enc_ctx->enc_params;

    encparams.video_format = static_cast<int>(video_format);
    // set encoder defaults
    EncoderParams default_enc_params(video_format);

    encparams.qf = default_enc_params.Qf();
    encparams.cpd = default_enc_params.CPD();
    encparams.L1_sep = default_enc_params.L1Sep();
    encparams.lossless = default_enc_params.Lossless();
    encparams.num_L1 = default_enc_params.NumL1();

    // set default block params
    OLBParams default_block_params;
    SetDefaultBlockParameters(default_block_params, video_format);
    encparams.xblen = default_block_params.Xblen();
    encparams.yblen = default_block_params.Yblen();
    encparams.xbsep = default_block_params.Xbsep();
    encparams.ybsep = default_block_params.Ybsep();

    // set default MV parameters
    encparams.mv_precision = default_enc_params.MVPrecision();

    // set default transform parameters
    WltFilter wf;
    SetDefaultTransformFilter(INTRA_FRAME, wf);
    encparams.intra_wlt_filter = wf;
    SetDefaultTransformFilter(INTER_FRAME, wf);
    encparams.inter_wlt_filter = wf;
    encparams.wlt_depth = default_enc_params.TransformDepth();
    encparams.spatial_partition = default_enc_params.SpatialPartition();
    encparams.def_spatial_partition = default_enc_params.DefaultSpatialPartition();
    encparams.multi_quants = default_enc_params.GetCodeBlockMode() == QUANT_MULTIPLE;
}

#ifdef __cplusplus
extern "C" {
#endif

extern DllExport void dirac_encoder_context_init ( dirac_encoder_context_t *enc_ctx, dirac_encoder_presets_t preset)
{
    TEST (enc_ctx != NULL);
    memset (enc_ctx, 0, sizeof(dirac_encoder_context_t));

    // preset is the video format
    int ps = static_cast<int>(preset);
    VideoFormat video_format(static_cast<VideoFormat>(ps));
    SetSequenceParameters (enc_ctx, video_format);
    SetEncoderParameters (enc_ctx, video_format);
}

extern DllExport dirac_encoder_t *dirac_encoder_init (const dirac_encoder_context_t *enc_ctx, int verbose)
{
    /* Allocate for encoder */
    dirac_encoder_t *encoder = new dirac_encoder_t;

    memset (encoder, 0, sizeof(dirac_encoder_t));

    /* initialse the encoder context */
    if (!InitialiseEncoder(enc_ctx, verbose>0, encoder))
    {
        delete encoder;
        return NULL;
    }

    encoder->encoded_frame_avail = encoder->decoded_frame_avail = 0;
    encoder->instr_data_avail = 0;


    return encoder;
}

extern DllExport int dirac_encoder_load (dirac_encoder_t *encoder, unsigned char *uncdata, int uncdata_size)
{
    TEST (encoder != NULL);
    TEST (encoder->compressor != NULL);
    DiracEncoder *compressor = (DiracEncoder *)encoder->compressor;
    int ret_stat = 0;
    try
    {
        if ( compressor->LoadNextFrame (uncdata, uncdata_size))
        {
            ret_stat = uncdata_size;
        }
    }
    catch (...)
    {
        if (compressor->GetEncParams().Verbose())
            std::cerr << "dirac_encoder_load failed" << std::endl;
        ret_stat = -1;
    }
    return ret_stat;
}

extern DllExport dirac_encoder_state_t 
      dirac_encoder_output (dirac_encoder_t *encoder)
{
    TEST (encoder != NULL);
    TEST (encoder->compressor != NULL);
    TEST (encoder->enc_buf.size != 0);
    TEST (encoder->enc_buf.buffer != NULL);
    DiracEncoder *compressor = (DiracEncoder *)encoder->compressor;
    dirac_encoder_state_t ret_stat = ENC_STATE_BUFFER;

    encoder->encoded_frame_avail = 0;
    encoder->decoded_frame_avail = 0;
    encoder->instr_data_avail = 0;

    try
    {
        if (compressor->CompressNextFrame() != 0)
        {
            if (compressor->GetEncodedData (encoder) < 0)
                ret_stat = ENC_STATE_INVALID;
            else
            {
                if (encoder->enc_buf.size > 0)
                {
                    ret_stat = ENC_STATE_AVAIL;
                }
    
            }
        }
        if (encoder->enc_ctx.decode_flag)
            compressor->GetDecodedData(encoder);
    }
    catch (...)
    {
        if (compressor->GetEncParams().Verbose())
            std::cerr << "GetEncodedData failed..." << std::endl;

        ret_stat = ENC_STATE_INVALID;
    }
    return ret_stat;
}

extern DllExport int dirac_encoder_end_sequence (dirac_encoder_t *encoder)
{
    TEST (encoder != NULL);
    TEST (encoder->compressor != NULL);
    DiracEncoder *compressor = (DiracEncoder *)encoder->compressor;
    int ret_stat;

    encoder->encoded_frame_avail = 0;
    encoder->decoded_frame_avail = 0;
    encoder->instr_data_avail = 0;

    try
    {
        ret_stat = compressor->GetSequenceEnd (encoder);
        encoder->end_of_sequence = 1;

        if (compressor->GetDecodedData(encoder))
        {
            encoder->decoded_frame_avail = 1;
        }
    }
    catch (...)
    {
        if (compressor->GetEncParams().Verbose())
            std::cerr << "GetSequenceEnd failed..." << std::endl;
        ret_stat = -1;
    }
    return ret_stat;
}

extern DllExport void dirac_encoder_close (dirac_encoder_t *encoder)
{
    TEST (encoder != NULL);
    TEST (encoder->compressor != NULL);
    
    delete (DiracEncoder *)(encoder->compressor);

    if (encoder->enc_ctx.instr_flag)
    {
        dealloc_instr_data(&encoder->instr);
    }

    if (encoder->enc_ctx.decode_flag)
    {
        delete [] encoder->dec_buf.buf[0];
    }
    delete encoder;
}


#ifdef __cplusplus
}
#endif
