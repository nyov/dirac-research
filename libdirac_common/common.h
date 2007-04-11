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
*                 Tim Borer,
*                 Anuradha Suraparaju,
*                 Andrew Kennedy
*                 Myo Tun (Brunel University, myo.tun@brunel.ac.uk)
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

#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef  _MSC_VER
#define  _CRT_SECURE_NO_DEPRECATE
#endif // _MSC_VER

#include <libdirac_common/arrays.h>
#include <libdirac_common/common_types.h>
#include <libdirac_common/dirac_assertions.h>
#include <vector>
#include <cmath>
namespace dirac
{
    /*! \file
        This file contains common classes used throughout the encoder and 
        decoder.  The main classes are the encoder and decoder parameters for 
        controlling the encode and decode processes. These are passed 
        throughout the codec.  There are also parameter classes for sequences 
        and frames.
    */


    //Some basic enumeration types used throughout the codec ...//
    //////////////////////////////////////////////////////////////

    //! Prediction modes for blocks
    enum PredMode{ INTRA , REF1_ONLY , REF2_ONLY , REF1AND2, UNDEFINED };

    //! Types of picture component
    enum CompSort{ Y_COMP , U_COMP , V_COMP };

    //! Addition or subtraction
    enum AddOrSub{ ADD , SUBTRACT };

    //! Forward or backward
    enum Direction { FORWARD , BACKWARD };

    //! Contexts used for coefficient coding
    enum CtxAliases
    {//used for residual coding
        SIGN0_CTX,          // -sign, previous symbol is 0
        SIGN_POS_CTX,       // -sign, previous symbol is +ve
        SIGN_NEG_CTX,       // -sign, previous symbol is -ve

        // Follow bit contexts
        Z_FBIN1z_CTX,       // -bin 1, parent is zero, neighbours zero
        Z_FBIN1nz_CTX,      // -bin 1, parent is zero, neighbours non-zero
        Z_FBIN2_CTX,        // -bin 2, parent is zero
        Z_FBIN3_CTX,        // -bin 3, parent is zero
        Z_FBIN4_CTX,        // -bin 4, parent is zero
        Z_FBIN5_CTX,        // -bin 5, parent is zero
        Z_FBIN6plus_CTX,    // -bins 6 plus, parent is zero

        NZ_FBIN1z_CTX,      // -bin 1, parent is non-zero, neighbours zero
        NZ_FBIN1nz_CTX,     // -bin 1, parent is non-zero, neighbours non-zero
        NZ_FBIN2_CTX,       // -bin 2, parent is non-zero
        NZ_FBIN3_CTX,       // -bin 3, parent is non-zero
        NZ_FBIN4_CTX,       // -bin 4, parent is non-zero
        NZ_FBIN5_CTX,       // -bin 5, parent is non-zero
        NZ_FBIN6plus_CTX,   // -bins 6 plus, parent is non-zero

        // Information bit contexts
        INFO_CTX,

        BLOCK_SKIP_CTX,     // - blocks are skipped
        Q_OFFSET_FOLLOW_CTX,   // - code block quantiser offset magnitude
        Q_OFFSET_INFO_CTX,  // - code block quantiser offset info context
        Q_OFFSET_SIGN_CTX,   // - code block quantiser offset sign
        TOTAL_COEFF_CTXS   // The total number of coefficient contexts
    };

    //! Contexts used for MV data coding
    enum MvCtxAliases
    {
        // DC value contexts //
        ///////////////////////

        DC_FBIN1_CTX,       
        DC_FBIN2plus_CTX,   
        DC_INFO_CTX,
        DC_SIGN_CTX,

        // Motion vector contexts //
        ////////////////////////////

    
        MV_FBIN1_CTX,     
        MV_FBIN2_CTX,     
        MV_FBIN3_CTX,     
        MV_FBIN4_CTX,     
        MV_FBIN5plus_CTX, 

        MV_INFO_CTX,

        MV_SIGN_CTX, 


        // Prediction mode contexts

        PMODE_BIT0_CTX,     // -bit 0, prediction mode value
        PMODE_BIT1_CTX,     // -bin 1, prediction mode value
        
        
        // Macroblock contexts
       
        MB_SPLIT_BIN1_CTX,  // bin 1, MB split mode vals
        MB_SPLIT_BIN2_CTX,  // bin 2, MB split mode vals. Bin 3 not required
        
        MB_SPLIT_INFO_CTX,  // info context for MB split mode
        
        TOTAL_MV_CTXS       // The total number of motion vector contexts
    };


    /**
    * Function to convert an integer to a valid VideoFormat
    *@param video_format Integer corresponding to a format
    *@return Valid video-format (returns VIDEO_FORMAT_UNDEFINED if no valid format found)
    */
    VideoFormat IntToVideoFormat(int video_format);

    /**
    * Function to convert an integer to a valid VideoFormat
    *@param chroma_format Integer corresponding to a format
    *@return Valid chroma-format (returns formatNK if no valid format found)
    */
    ChromaFormat IntToChromaFormat(int chroma_format);

     /**
    * Function to convert an integer to a valid FrameRate type
    *@param frame_rate_idx Integer corresponding to a frame-rate in the spec table
    *@return Valid FrameRateType (returns FRAMERATE_UNDEFINED if no valid frame-rate found)
    */
    FrameRateType IntToFrameRateType(int frame_rate_idx);

    /**
    * Function to convert an integer to a valid AspectRatio type
    *@param aspect_ratio_idx Integer corresponding to a aspect-ratio in the spec table
    *@return Valid AspectRatioType (returns ASPECT_RATIO_UNDEFINED if no valid aspect-ratio found)
    */
    AspectRatioType IntToAspectRatioType(int aspect_ratio_idx);

    /**
    * Function to convert an integer to a valid SignalRange type
    *@param signal_range_idx Integer corresponding to a signal-range in the spec table
    *@return Valid SignalRangeType (returns SIGNAL_RANGE_UNDEFINED if no valid signal-ratio found)
    */
    SignalRangeType IntToSignalRangeType(int signal_range_idx);

     /**
    * Function to convert an integer to a valid motion-vector precision type
    *@param mv_prec Integer corresponding to a valid motion-vector precision
    *@return Valid MVPrecisionType (returns SIGNAL_RANGE_UNDEFINED if no valid precision found)
    */
    MVPrecisionType IntToMVPrecisionType(int mv_prec);

    //Classes used throughout the codec//
    /////////////////////////////////////
    //! Frame type Class
    class FrameSort
    {
    public:
        FrameSort() { fs = 0x00; } // default intra non-ref
    
        void SetIntra() { fs &= 0xfe; }
        void SetInter() { fs |= 0x01; }
        void SetNonRef() { fs &= 0xfd; }
        void SetRef() { fs |= 0x02; }
    
        bool IsInter () const { return fs & 0x01; }
        bool IsIntra () const { return !IsInter(); }
        bool IsRef() const { return fs & 0x02; };
        bool IsNonRef() const { return !IsRef(); }
        
        void SetIntraNonRef() { SetIntra(); SetNonRef(); }
        void SetIntraRef() { SetIntra(); SetRef(); }
        void SetInterNonRef() { SetInter(); SetNonRef(); }
        void SetInterRef() { SetInter(); SetRef(); }
    
        bool IsIntraNonRef() const { return (fs & 0x03) == 0x00; }
        bool IsIntraRef() const { return (fs & 0x03) == 0x02; }
        bool IsInterNonRef() const { return (fs & 0x03) == 0x01; }
        bool IsInterRef() const { return (fs & 0x03) == 0x03; }
    
        void Clear() { fs=0x00; }

        static FrameSort IntraRefFrameSort()
        {
            FrameSort fs;
            fs.SetIntraRef();
            return fs;
        }

        static FrameSort InterRefFrameSort()
        {
            FrameSort fs;
            fs.SetInterRef();
            return fs;
        }
        
        static FrameSort IntraNonRefFrameSort()
        {
            FrameSort fs;
            fs.SetIntraNonRef();
            return fs;
        }

        static FrameSort InterNonRefFrameSort()
        {
            FrameSort fs;
            fs.SetInterNonRef();
            return fs;
        }

    private:
        unsigned char fs;
    };

    //! A class for picture component data.
    /*!
        A class for encapsulating picture data, derived from TwoDArray. NB: 
        in the future there will be separate classes for input/output picture 
        data, difference picture data, and wavelet coefficient data. Currently 
        PicArray is used for all of these. TJD 13 April 2004.
     */
    class PicArray: public TwoDArray<ValueType>
    {
    public:
        //! Default constructor
        /*!
            Default constructor creates an empty array.
        */
        PicArray(): TwoDArray<ValueType>(){}
        
        //! Constructor.
        /*!
            Contructor creates a two-D array, with specified size and colour 
            format.
        */
        PicArray(int height, int width, CompSort cs=Y_COMP);
        
        //copy constructor and assignment= derived by inheritance
        
        //! Destructor
        ~PicArray(){}
        
        //! Return which component is stored
        const CompSort& CSort() const;
        
        //! Set the type of component being stored
        void SetCSort(const CompSort cs);
        
    private:
        
        CompSort m_csort;
    };


    //! A structure for recording costs, particularly in quantisation.
    class CostType
    {
    public:
        //! The error (MSE or 4th power)
        double Error;
        
        //! The entropy in bits per symbol.
        double ENTROPY;
        
        //! The Lagrangian combination of MSE+lambda*entropy
        double TOTAL;
    };


    //! A class used for correcting estimates of entropy.
    /*!
        A class used by the encoder for correcting estimates of entropy. Used 
        for selecting quantisers in subband coefficient coding. Factors can be 
        adjusted in the light of previous experience.
     */
    class EntropyCorrector
    {
    public:
        //! Constructor.
        /*!
        Constructs arrays of correction factors of size.
        \param    depth    the depth of the wavelet transform.
        */
        EntropyCorrector(int depth);
        
        ////////////////////////////////////////////////////////////////////
        //NB: Assume default copy constructor, assignment = and destructor//
        ////////////////////////////////////////////////////////////////////    

        //! Returns the correction factor.
        /*!
        Returns the correction factor for the band given also the type of 
        frame and component.
        */
        float Factor(const int bandnum, const FrameSort fsort,const CompSort c) const;

        //! Update the correction factors.
        /*!
        Update the factors for a given subband, component and frame type.
        \param    bandnum    the number of the subband to update
        \param    fsort      frame type
        \param    c          component type
        \param    est_bits    the number of bits it was estimated would be used
        \param    actual_bits    the number of bits that actually were used
         */    
        void Update(int bandnum, FrameSort fsort, CompSort c,int est_bits,int actual_bits);
        
    private:
        //! Initialises the correction factors
        void Init();

        TwoDArray<float> m_Yfctrs;
        TwoDArray<float> m_Ufctrs;
        TwoDArray<float> m_Vfctrs;
    };

    //! Parameters for overlapped block motion compensation
    class OLBParams
    {
        
    public:
        
        //! Default constructor does nothing
        OLBParams(){}
        
        //! Constructor
        /*
            Constructor rationalises proposed parameters to allow suitable 
            overlap and fit in with chroma format
            \param    xblen    the horizontal block length    
            \param    yblen    the vertical block length
            \param    xblen    the horizontal block separation
            \param    yblen    the vertical block separation

        */
        OLBParams(const int xblen, const int yblen, 
                  const int xbsep, const int ybsep);
        
        // Gets ...
        
        //! Returns the horizontal block length
        int Xblen() const {return m_xblen;}
        
        //! Returns the vertical block length
        int Yblen() const {return m_yblen;}
        
        //! Returns the horizontal block separation
        int Xbsep() const {return m_xbsep;}
        
        //! Returns the vertical block separation
        int Ybsep() const {return m_ybsep;}
        
        //! The offset in the horizontal start of the block caused by overlap,=(XBLEN-XBSEP)/2
        int Xoffset() const {return m_xoffset;}
        
        //! The offset in the vertical start of the block caused by overlap,=(YBLEN-YBSEP)/2
        int Yoffset() const {return m_yoffset;}
        
        // ... and sets
        
        //! Sets the block width
        void SetXblen( int xblen ){ m_xblen = xblen; m_xoffset = (m_xblen-m_xbsep)/2;}
        
        //! Sets the block height
        void SetYblen( int yblen ){ m_yblen = yblen; m_yoffset = (m_yblen-m_ybsep)/2;}
        
        //! Sets the block horizontal separation
        void SetXbsep( int xbsep ){ m_xbsep = xbsep; m_xoffset = (m_xblen-m_xbsep)/2;}
        
        //! Sets the block vertical separation
        void SetYbsep( int ybsep ){ m_ybsep = ybsep; m_yoffset = (m_yblen-m_ybsep)/2;}
       
        bool operator == (const OLBParams bparams) const;

        // overloaded stream operators
        friend std::ostream & operator<< (std::ostream &, OLBParams &);
        friend std::istream & operator>> (std::istream &, OLBParams &);
        
        
    private:
        
        int m_xblen;
        int m_yblen;
        int m_xbsep;
        int m_ybsep;
        int m_xoffset;
        int m_yoffset;
    };

    //! Parameters relating to the video sequence being encoded/decoded
    class SeqParams
    {
    public:        
        //! Default Constructor 
        SeqParams(const VideoFormat& video_format=VIDEO_FORMAT_CUSTOM,
                  bool set_defaults=true);
        
        ////////////////////////////////////////////////////////////////////
        //NB: Assume default copy constructor, assignment = and destructor//
        ////////////////////////////////////////////////////////////////////    
        
        //gets ...
        //! Returns the picture width
        int Xl() const {return m_xl;}
        
        //! Returns the picture height
        int Yl() const {return m_yl;}
        
        //! Returns the chroma format of the sequence (Y only, 420, 422 etc)
        ChromaFormat CFormat() const {return m_cformat;}
        
        //! Returns the chroma width
        int ChromaWidth() const;
        
        //! Returns the chroma height
        int ChromaHeight() const;
        
        //! Returns the bitstream version
        int BitstreamVersion() const {return m_bs_ver;}
        
        //! Returns video-format
        VideoFormat GetVideoFormat() const { return m_video_format;}

        //! Returns video depth
        int GetVideoDepth() const { return m_video_depth; }

        // ... Sets
        
        //! Sets the picture width
        void SetXl(int xlen) {m_xl = xlen;}
        
        //! Sets the picture height
        void SetYl(int ylen) {m_yl = ylen;}
        
        //! Sets the chroma format (Y only, 420, 422 etc)
        void SetCFormat(ChromaFormat cf) {m_cformat=cf;}
        
        //! Sets number of bits used in coding
        void SetVideoDepth(int vd){ m_video_depth=vd;}

        //! Sets the bitstream version
        void SetBitstreamVersion(int bs_ver){m_bs_ver=bs_ver;}
       

    private:
        //! Width of video
        int m_xl;
        
        //! Height of video
        int m_yl;
        
        //! Presence of chroma and/or chroma sampling structure 
        ChromaFormat m_cformat;
        
        //! Bitsream version.
        unsigned char  m_bs_ver;

        //!Video-format
        VideoFormat m_video_format;

        //! Number of bits used to compress input signal
        int m_video_depth;
    };

    //! Class defining a rational number
    class Rational
    {
    public:
        //! Numerator
        unsigned int m_num;
        //! Denominator
        unsigned int m_denom;
    };

    //! Parameters relating to the complexity of encoder/decoder
    class ParseParams
    {
    public:
        //! Default constructor
        ParseParams(unsigned int au_pnum = 0);

        // Gets

        //! Get the access unit picture number
        unsigned int AccessUnitPictureNumber() const { return m_au_pnum; }

        //! Get the major version
        unsigned int MajorVersion() const { return m_major_ver; }

        //! Get the minor version
        unsigned int MinorVersion() const { return m_minor_ver; }

        //! Get the Profile
        unsigned int Profile() const { return m_profile; }

        //! Get the Level
        unsigned int Level() const { return m_level; }

        // Sets
        
        //! Set the access unit picture number
        void SetAccessUnitPictureNumber(unsigned int au_pnum) { m_au_pnum = au_pnum; }

        //! Set the major version
        void SetMajorVersion(unsigned int major_ver) { m_major_ver = major_ver; }

        //! Set the minor version
        void SetMinorVersion(unsigned int minor_ver) { m_minor_ver = minor_ver; }

        //! Set the Profile
        void SetProfile(unsigned int level) { m_level = level; }

        //! Set the Level
        void SetLevel(unsigned int profile) { m_profile = profile; }

    private:
        //! Frame number of first Intra Ref Picture after Access unit header
        unsigned int m_au_pnum;
        //! Major Version
        unsigned int m_major_ver;
        //! Minor Version
        unsigned int m_minor_ver;
        //! Profile
        unsigned int m_profile;
        //! Level
        unsigned int m_level;
    };

    //! Parameters relating to the source material being encoded/decoded
    class SourceParams
    {
    public:
        //! default constructor
        SourceParams (const VideoFormat &vf = VIDEO_FORMAT_CUSTOM, 
                      bool set_defaults=true);
        
        ////////////////////////////////////////////////////////////////////
        //NB: Assume default copy constructor, assignment = and destructor//
        ////////////////////////////////////////////////////////////////////    

        // Gets
        //! Returns true if the source material is interlaced
        bool Interlace() const { return m_interlace; }
        
        //! Returns true if top field comes first in time
        bool TopFieldFirst() const { return m_topfieldfirst; }
        
        //! Returns true if fields are sequential i.e. not interleaved
        bool SequentialFields() const { return m_seq_fields; }
       
           //! Return the number for frames per second
        Rational FrameRate() const { return m_framerate; }
       
        //! Return the type from the frame rate table
        FrameRateType FrameRateIndex() const { return m_fr_idx; }
       
           //! Return the pixel aspect ratio
        Rational AspectRatio() const { return m_aspect_ratio; }

         //! Return the type from the aspect ratio table
        AspectRatioType AspectRatioIndex() const { return m_asr_idx; }

        // Clean area parameters
        //! Return the Clean area width
        unsigned int CleanWidth() const { return m_clean_width; }
        //! Return the Clean area height
        unsigned int CleanHeight() const { return m_clean_height; }
        //! Return the Clean area left offset
        unsigned int LeftOffset() const { return m_left_offset; }
        //! Return the Clean area top offset
        unsigned int TopOffset() const { return m_top_offset; }

        // Signal Range parameters

        //! Return the type from the signal range table
        SignalRangeType SignalRangeIndex() const { return m_sr_idx; }

        //! Return the luma offset
        unsigned int LumaOffset() const { return m_luma_offset; }
        //! Return the luma excursion
        unsigned int LumaExcursion() const { return m_luma_excursion; }
        //! Return the chroma offset
        unsigned int ChromaOffset() const { return m_chroma_offset; }
        //! Return the chroma excursion
        unsigned int ChromaExcursion() const { return m_chroma_excursion; }

        //! Return the index into the colour specification table
        unsigned int ColourSpecificationIndex() const { return m_cs_idx; }

        //! Return the colour primaries index
        ColourPrimaries ColourPrimariesIndex() const { return m_col_primary; }
        //! Return the colour matrix index
        ColourMatrix ColourMatrixIndex() const { return m_col_matrix; }
        //! Return the transfer function index
        TransferFunction TransferFunctionIndex() const { return m_transfer_func; }
    
        // Sets
        //! Set if the source material is interlaced
        void SetInterlace(bool interlace) { m_interlace = interlace; }
        
        //! Set Topfield first. True if top field comes first in time
        void SetTopFieldFirst(bool tff) { m_topfieldfirst = tff; }
        
        //! Set 'sequential fields flag. true if fields are sequential i.e. not interleaved
        void SetSequentialFields(bool seq_flds) { m_seq_fields = seq_flds; }
       
           //! Set the frame rate
        void SetFrameRate(const Rational &frate ) 
        {
            m_fr_idx = FRAMERATE_CUSTOM; m_framerate = frate;
        }
           
        //! Set the frame rate
        void SetFrameRate(unsigned int fr_num, unsigned int fr_denom )
        {
            m_fr_idx = FRAMERATE_CUSTOM;
            m_framerate.m_num = fr_num;
            m_framerate.m_denom = fr_denom;
        }
       
        //! Set the frame rate
        void SetFrameRate(FrameRateType fr); 

           //! Set the pixel aspect ratio
        void SetAspectRatio(const Rational &asr) 
        {
            m_asr_idx = ASPECT_RATIO_CUSTOM;
            m_aspect_ratio = asr;
        }
        
        //! Set the pixel aspect ratio
        void SetAspectRatio(unsigned int as_num, unsigned int as_denom )
        { 
            m_asr_idx = ASPECT_RATIO_CUSTOM;
            m_aspect_ratio.m_num = as_num;
            m_aspect_ratio.m_denom = as_denom;
        }
       
        //! Set the Pixel Aspect Ratio
        void SetAspectRatio(AspectRatioType aspect_ratio); 

        // Clean area parameters
        //! Set the Clean area width
        void SetCleanWidth(unsigned int clean_width) { m_clean_width = clean_width; }
        //! Set the Clean area height
        void SetCleanHeight(unsigned int clean_height) { m_clean_height = clean_height; }
        //! Set the Clean area left offset
        void SetLeftOffset(unsigned int left_offset) { m_left_offset = left_offset; }
        //! Set the Clean area top offset
        void SetTopOffset(unsigned int top_offset) { m_top_offset = top_offset; }

        // Signal Range parameters
        //! Set the Signal Range parameters
        void SetSignalRange(SignalRangeType sr);

        //! Set the luma offset
        void SetLumaOffset(unsigned int luma_offset) { m_sr_idx = SIGNAL_RANGE_CUSTOM; m_luma_offset = luma_offset; }
        //! Set the luma excursion
        void SetLumaExcursion(unsigned int luma_exc) { m_sr_idx = SIGNAL_RANGE_CUSTOM; m_luma_excursion = luma_exc; }
        //! Set the chroma offset
        void SetChromaOffset(unsigned int chroma_off) { m_sr_idx = SIGNAL_RANGE_CUSTOM; m_chroma_offset = chroma_off; }
        //! Set the chroma excursion
        void SetChromaExcursion(unsigned int chroma_exc) { m_sr_idx = SIGNAL_RANGE_CUSTOM; m_chroma_excursion = chroma_exc; }

        //! Set the Colour specification
        void SetColourSpecification(unsigned int cs_idx);
        //! Set the colour primaries index
        void SetColourPrimariesIndex(unsigned int cp);
        //! Set the colour matrix index
        void SetColourMatrixIndex(unsigned int cm);
        //! Set the transfer function index
        void SetTransferFunctionIndex(unsigned int tf);

    private:
        //! True if interlaced
        bool m_interlace;
        
        //! If interlaced, true if the top field is first in temporal order
        bool m_topfieldfirst;
        
        //! If interlaced, true if the fields are sequential and not interleaved
        bool m_seq_fields;

        //! Index into frame rate table
        FrameRateType m_fr_idx;

        //! Frame Rate i.e number of frames per second
        Rational m_framerate;

        //! Index into pixel aspect ratio table
        AspectRatioType m_asr_idx;

        //! Pixel Aspect Ratio
        Rational m_aspect_ratio;

        // Clean area parameters

        //! Clean area width
        unsigned int m_clean_width;
        
        //! Clean area height
        unsigned int m_clean_height;
        
        //! Clean area left offset
        unsigned int m_left_offset;
        
        //! Clean area top offset
        unsigned int m_top_offset;

        // signal range parameters

        //! Index into signal range table
        SignalRangeType m_sr_idx;

        //! Luma offset
        unsigned int m_luma_offset;
        //! Luma excursion
        unsigned int m_luma_excursion;
        //! Chroma offset
        unsigned int m_chroma_offset;
        //! Chroma excursion
        unsigned int m_chroma_excursion;

        //! Index into colour spec table
        unsigned int m_cs_idx;

        //! Colour Primaries Index
        ColourPrimaries m_col_primary;

        // Colour Matrix index
        ColourMatrix m_col_matrix;

        // Transfer function index
        TransferFunction m_transfer_func;
    };

    //! Parameters for initialising frame class objects
    class FrameParams 
    {
        
    public:
        //! Default constructor
        FrameParams();
        
        //! Constructor 
        /*!
           Frame chroma format is set Frame sort defaults to I frame.
        */    
        FrameParams(const ChromaFormat& cf, int xlen, int ylen, int c_xlen, int c_ylen, unsigned int video_depth);
        
        //! Constructor
        /*!
           Frame chroma format and frame sort are set.
        */    
        FrameParams(const ChromaFormat& cf, const FrameSort& fs);
        
        //! Constructor
        /*
            All data is derived from the sequence parameters
        */
        FrameParams(const SeqParams& sparams);
        
        //! Constructor
        /*
           All data is derived from the sequence parameters
        */
        FrameParams(const SeqParams& sparams, const FrameSort& fs);
        
        ////////////////////////////////////////////////////////////////////
        //NB: Assume default copy constructor, assignment = and destructor//
        ////////////////////////////////////////////////////////////////////    
        
        // Gets ...
        
        //! Returns the chroma format of the frame
        const ChromaFormat& CFormat() const{return m_cformat;}
        
        //! Returns the luma width of the frame
        int Xl() const{return m_xl;}
        
        //! Returns the luma height of the frame
        int Yl() const{return m_yl;}
        
        //! Returns the chroma width of the frame
        int ChromaXl() const{return m_chroma_xl;}
        
        //! Returns the chroma height of the frame
        int ChromaYl() const{return m_chroma_yl;}
        
        //! Returns the type of the frame
        const FrameSort& FSort() const {return m_fsort;}
        
        //! Returns the number of the frame (in time order)
        int FrameNum() const {return m_fnum;}
        
        //! Returns whether the frame is bi-directionally predicted by checking references
        bool IsBFrame() const;
        
        //! Returns the number of frames after the current frame number after which the frame can be discarded
        int ExpiryTime() const {return m_expiry_time;}
        
        //! Returns an indication of whether the frame has been output yet
        bool Output() const {return m_output;}
        
        //! Returns a const C++ reference to the set of reference frame numbers (will be empty if the frame is an I frame)
        const std::vector<int>& Refs() const {return m_refs;}
        
        //! Returns non-const C++ referece to the vector of reference frames, to allow them to be set
        std::vector<int>& Refs(){return m_refs;}
        
        //! Return the number of reference frames
        unsigned int NumRefs()const {return m_refs.size();}
        
        //! Returns type of frame (see enum)
        FrameType GetFrameType () const { return m_frame_type; }

        //! Returns reference frame type (see enum)
        ReferenceType GetReferenceType() const { return m_reference_type;}
        
        //! Returns the video depth of the frame
        unsigned int GetVideoDepth() const{return m_video_depth;}

        // ... Sets
        
        //! Sets the type of frame
        void SetFSort( const FrameSort& fs );
        
        //! Sets the frame to be Intra/Inter
        void SetFrameType(const FrameType ftype);
        
        //! Sets the frame to be a reference or not
        void SetReferenceType(const ReferenceType rtype);
        
        //! Sets the frame number
        void SetFrameNum( const int fn ){ m_fnum=fn; }
        
        //! Sets how long the frame will stay in the buffer (encoder only)
        void SetExpiryTime( const int expt ){ m_expiry_time=expt; }
        
        //! Sets a flag to indicate that the frame has been output
        void SetAsOutput(){m_output=true;}
        
        //! Sets the chroma format
        void SetCFormat(ChromaFormat cf){ m_cformat = cf; }
        
        //! Sets the frame luma length
        void SetXl(int xl){m_xl = xl; }
        
        //! Sets the frame luma height
        void SetYl(int yl){m_yl = yl; }
        
        //! Sets the chroma length
        void SetChromaXl(int xl){m_chroma_xl = xl; }
        
        //! Sets the chroma height
        void SetChromaYl(int yl){m_chroma_yl = yl; }
        
        //! Sets the video depth of the frame
        void SetVideoDepth(int vd) { m_video_depth = vd; }
        
        //! Returns a const C++ reference to the set of frame numbers to be retired
        std::vector<int>& RetiredFrames() const {return m_retd_list;}
        
        //! Returns a non-const C++ reference to the set of frame numbers to be retired
        std::vector<int>& RetiredFrames() {return m_retd_list;}
        
    private:
        
        //! The chroma format
        ChromaFormat m_cformat;
        
        //! Frame luma width
        int m_xl;
        
        //! Frame luma height
        int m_yl;
        
        //! The frame sort
        FrameSort m_fsort;
        
        //! The set of frame numbers of reference frames
        std::vector<int> m_refs;
        
        //! The number of frames, after the current frame number, after the (de)coding of which the frame can be deleted
        int m_expiry_time;
        
        //! The frame number, in temporal order
        int m_fnum;        

        //! Frame type
        FrameType m_frame_type;

        //! Reference type
        ReferenceType m_reference_type;
        
        //! True if the frame has been output, false if not
        bool m_output;

        //! Chroma length
        int m_chroma_xl;        
        
        //! Chroma height
        int m_chroma_yl;        
        
        //! The set of frame numbers in the retired frame list
        mutable std::vector<int> m_retd_list;

        //! Video depth
        unsigned int m_video_depth;
        
    };

    //! Structure to hold code block sizes when spatial partitioning is used
    class CodeBlocks
    {
    public:
        //! Default Constructor
        CodeBlocks () : m_hblocks(0), m_vblocks(0)
        {}

        //! Constructor
        CodeBlocks (unsigned int  hblocks, unsigned int vblocks) : 
            m_hblocks(hblocks),
            m_vblocks(vblocks)
            {}

        // Gets
        //! Return the number of horizontal code blocks
        unsigned int HorizontalCodeBlocks() const { return m_hblocks; }
        //! Return the number of vertical code blocks
        unsigned int VerticalCodeBlocks() const { return m_vblocks; }
        // Sets 
        //! Set the number of horizontal code blocks
        void SetHorizontalCodeBlocks(unsigned int hblocks) { m_hblocks = hblocks; }
        //! Set the number of vertical code blocks
        void SetVerticalCodeBlocks(unsigned int vblocks) { m_vblocks = vblocks; }
    private:
        //! Number of Horizontal code blocks
        unsigned int m_hblocks;
        //! Number of Vertical code blocks
        unsigned int m_vblocks;
    };
    //! Parameters common to coder and decoder operation
    /*!
        Parameters used throughout both the encoder and the decoder
    */
    class CodecParams
    {
    public:
        
        //! Default constructor 
        CodecParams(const VideoFormat& vd, FrameType ftype, unsigned int num_refs, bool set_defaults);
        
            ////////////////////////////////////////////////////////////////////
            //NB: Assume default copy constructor, assignment = and destructor//
            ////////////////////////////////////////////////////////////////////
        
        // Gets ...    
        
        //! Return the number of macroblocks horizontally
        int XNumMB() const {return m_x_num_mb;}
        
        //! Return the number of macroblocks vertically
        int YNumMB() const {return m_y_num_mb;}
        
        //! Return the number of blocks horizontally
        int XNumBlocks() const {return m_x_num_blocks;}
        
        //! Returns the number of blocks vertically
        int YNumBlocks() const {return m_y_num_blocks;}
        
        //! Returns true if we're operating verbosely, false otherwise
        bool Verbose() const {return m_verbose;}
        
        //! Returns true if we're operatung using interlace tools [not currently defined]
        bool Interlace() const {return m_interlace;}
        
        //! Returns true if the topmost field comes first in time [NB: TBD since this duplicates metadata in the sequence header]
        bool TopFieldFirst() const {return m_topfieldfirst;}    
        
        //! Return the Luma block parameters for each macroblock splitting level
        const OLBParams& LumaBParams(int n) const {return m_lbparams[n];}
        
        //! Return the Chroma block parameters for each macroblock splitting level
        const OLBParams& ChromaBParams(int n) const {return m_cbparams[n];}    

        //! Return the original frame width
        int OrigXl() const {return m_orig_xl;}

        //! Return the original frame height
        int OrigYl() const {return m_orig_yl;}

        //! Return the number of accuracy bits used for motion vectors
        MVPrecisionType MVPrecision() const { return m_mv_precision; }

        //! Return zero transform flag being used for frame (de)coding
        bool ZeroTransform() const { return m_zero_transform; } 

        //! Return the wavelet filter currently being used for frame (de)coding
        WltFilter TransformFilter() const { return m_wlt_filter; } 

        //! Return the transform depth being used for frame (de)coding
        unsigned int TransformDepth() const { return m_wlt_depth; } 

        //! Return multiple quantisers flag being used for frame (de)coding
        CodeBlockMode GetCodeBlockMode() const { return m_cb_mode; } 

        //! Return the spatial partitioning flag being used for frame (de)coding
        bool SpatialPartition() const { return m_spatial_partition; } 
        
        //! Return the default spatial partitioning flag being used for frame (de)coding
        bool DefaultSpatialPartition() const { return m_def_spatial_partition; } 
        //! Return the code blocks for a particular level
        const CodeBlocks &GetCodeBlocks(unsigned int level) const;

        //! Return the video format currently being used for frame (de)coding
        VideoFormat GetVideoFormat() const { return m_video_format; } 
             
        //! Return the global motion flag used for encoding/decoding
        bool UsingGlobalMotion() const { return m_use_global_motion; } 

        //! Return the number of frame weight precision bits
        unsigned int FrameWeightsBits() const { return m_frame_weights_bits; } 

        //! Return the Ref1 weight
        unsigned int Ref1Weight() const { return m_ref1_weight; } 

        //! Return the Ref2 weight
        unsigned int Ref2Weight() const { return m_ref2_weight; } 
             
        // ... and Sets
        //! Set how many MBs there are horizontally
        void SetXNumMB(const int xn){m_x_num_mb=xn;}    
        
        //! Set how many MBs there are vertically
        void SetYNumMB(const int yn){m_y_num_mb=yn;}
        
        //! Set how many blocks there are horizontally
        void SetXNumBlocks(const int xn){m_x_num_blocks=xn;}
        
        //! Set how many blocks there are vertically
        void SetYNumBlocks(const int yn){m_y_num_blocks=yn;}
        
        //! Sets verbosity on or off
        void SetVerbose(bool v){m_verbose=v;}
        
        //! Sets whether interlace tools are to be used
        void SetInterlace(bool intlc){m_interlace=intlc;}
        
        //! Sets whether the topmost field comes first in time [NB: TBD since this duplicates metadata in the sequence header]
        void SetTopFieldFirst(bool topf){m_topfieldfirst=topf;}
        
        //! Set the block sizes for all MB splitting levels given these prototype block sizes for level=2
        void SetBlockSizes(const OLBParams& olbparams , const ChromaFormat cformat);
        //! Set block level luma params
        void SetLumaBlockParams(const OLBParams& olbparams) {m_lbparams[2] = olbparams;}

        //! Set the original frame width
        void SetOrigXl(const int x){m_orig_xl=x;}

        //! Set the original frame height
        void SetOrigYl(const int y){m_orig_yl=y;}

        //! Set the number of accuracy bits for motion vectors
        void SetMVPrecision(const MVPrecisionType p)
        {
            // Assert in debug mode. Maybe we should throw an exception???
            TESTM((p >=0 && p <=3), "Motion precision value in range 0..3");
            m_mv_precision = p;
        }

        void SetMVPrecision(const MVPrecisionType p) const
        {
            // Assert in debug mode. Maybe we should throw an exception???
            TESTM((p >=0 && p <=3), "Motion precision value in range 0..3");
            m_mv_precision = p;
        }

        //! Set the zero transform flag being used for frame (de)coding
        void SetZeroTransform(bool zero_transform)  { m_zero_transform = zero_transform; } 

        //! Set the wavelet filter used for frame (de)coding
        void SetTransformFilter(const WltFilter wf) { m_wlt_filter=wf; } 

        //! Set the wavelet filter used for frame (de)coding
        void SetTransformFilter(unsigned int wf_idx);

        //! Set the transform depth used for frame (de)coding and allocate for the code blocks array
        void SetTransformDepth(unsigned int wd);

        //! Set the multiple quantisers flag usedto frame (de)coding
        void SetCodeBlockMode(unsigned int cb_mode);

        //! Set the spatial partition flag usedto frame (de)coding
        void SetSpatialPartition(bool spatial_partition) { m_spatial_partition=spatial_partition; } 

        //! Set the spatial partition flag usedto frame (de)coding
        void SetDefaultSpatialPartition(bool def_spatial_partition) { m_def_spatial_partition=def_spatial_partition; } 
        
        //! Set the number of code blocks for a particular level
        void  SetCodeBlocks(unsigned int level, unsigned int hblocks, unsigned int vblocks);
        
        //! Set the default number of code blocks for all levels
        void  SetDefaultCodeBlocks(const FrameType& ftype);

        //! Set the video format used for frame (de)coding
        void SetVideoFormat(const VideoFormat vd) { m_video_format=vd; } 

        //! Set the wavelet filter used for frame (de)coding
        void SetUsingGlobalMotion(bool gm) { m_use_global_motion=gm; } 

        //! Set the frame weight precision bits used for (de)coding
        void SetFrameWeightsPrecision(unsigned int wt_prec) { m_frame_weights_bits=wt_prec; } 

        //! Set the ref 1 frame weight
        void SetRef1Weight(unsigned int wt) { m_ref1_weight=wt; } 

        //! Set the ref 2 frame weight
        void SetRef2Weight(unsigned int wt) { m_ref2_weight=wt; } 

    protected:
        //! Return the Wavelet filter associated with the wavelet index
        WltFilter TransformFilter (unsigned int wf_idx);
    private:
        
        //! The number of macroblocks horizontally
        int m_x_num_mb;
        
        //! The number of macroblocks verticaly
        int m_y_num_mb;
        
        //! The number of blocks horizontally
        int m_x_num_blocks;    
        
        //! The number of blocks vertically
        int m_y_num_blocks;
        
        //! Code/decode with commentary if true    
        bool m_verbose;
        
        //! True if input is interlaced, false otherwise
        bool m_interlace;
        
        //! True if interlaced and top field is first in temporal order 
        bool m_topfieldfirst;
        
        OneDArray<OLBParams> m_lbparams;
        OneDArray<OLBParams> m_cbparams;

        //! The original frame width
        int m_orig_xl;

        //! The original frame height
        int m_orig_yl;

        //! The precision of motion vectors (number of accuracy bits eg 1=half-pel accuracy) 
        mutable MVPrecisionType m_mv_precision;

        //! The video format being used
        VideoFormat m_video_format;

        //! Global motion fields
        bool m_use_global_motion;

        //! frame predicion parameters - precision
        unsigned int m_frame_weights_bits;

        //! frame predicion parameters - reference frame 1 weight
        unsigned int m_ref1_weight;
        
        //! frame predicion parameters - reference frame 1 weight
        unsigned int m_ref2_weight;

        //! Zero transform flag
        bool m_zero_transform;

        //! The wavelet filter being used
        WltFilter m_wlt_filter;

        //! Wavelet depth
        unsigned int m_wlt_depth;

        //! Code block mode
        CodeBlockMode m_cb_mode;

        //! Spatial partitioning flag
        bool m_spatial_partition;

        //! Default Spatial partitioning flag
        bool m_def_spatial_partition;

        //! Code block array. Number of entries is m_wlt_depth+1
        OneDArray<CodeBlocks> m_cb;
    };

    //! Parameters for the encoding process
    /*!
        Parameters for the encoding process, derived from CodecParams.
     */
    class EncoderParams: public CodecParams
    {
        //codec params plus parameters relating solely to the operation of the encoder
        
    public:
        //! Default constructor   
        EncoderParams(const VideoFormat& video_format,
                      FrameType ftype = INTER_FRAME,
                      unsigned int num_refs = 2, 
                      bool set_defaults=true);
        
            ////////////////////////////////////////////////////////////////////
            //NB: Assume default copy constructor, assignment = and destructor//
            //This means pointers are copied, not the objects they point to.////       
            ////////////////////////////////////////////////////////////////////
        
         // Gets ...

        //! Returns a flag indicating that we're doing local decoding
        bool LocalDecode() const {return m_loc_decode;}

        //! Get whether we're doing lossless coding
        bool Lossless() const {return m_lossless;}

        //! Get the quality factor
        float Qf() const {return m_qf;}

        //! Return the nominal number of L1 frames before the next I frame
        /*! 
            Return the nominal number of L1 frames before the next I frame. Can be
            overridden by I-frame insertion

        */
        int NumL1() const {return m_num_L1;}

        //! Return the separation between L1 frames (and between L1 and I frames)
        int L1Sep() const {return m_L1_sep;}

        //! Return the amount we're weighting noise in the U component
        float UFactor() const {return m_ufactor;}

        //! Return the amount we're weighting noise in the V component
        float VFactor() const {return m_vfactor;}

        //! Return the number of cycles per degree at the nominal viewing distance for the raster
        float CPD() const {return m_cpd;}

        //! Return whether input denoising is on or off
        bool Denoise() const {return m_denoise;}

        //! Return the Lagrangian parameter to be used for I frames
        float ILambda() const {return m_I_lambda;}

        //! Return the Lagrangian parameter to be used for L1 frames
        float L1Lambda() const {return m_L1_lambda;}

        //! Return the Lagrangian parameter to be used for L2 frames
        float L2Lambda() const {return m_L2_lambda;}

        //! Return the Lagrangian ME parameter to be used for L1 frames
        float L1MELambda() const {return m_L1_me_lambda;}

        //! Return the Lagrangian ME parameter to be used for L2 frames
        float L2MELambda() const {return m_L2_me_lambda;}

        //! Return the size of the GOP
        int GOPLength() const { if (m_num_L1>0) return (m_num_L1+1)*m_L1_sep;
                                return (m_num_L1==0) ? 10 : 0; }

        //! Return the output path to be used for storing diagnositic data
        char * OutputPath() const {return ( char* ) m_output_path.c_str();}
        
        //! Return a reference to the entropy factors
        const EntropyCorrector& EntropyFactors() const {return *m_ent_correct;}
        
        //! Return a reference to the entropy factors - we need to be able to change the values of the entropy factors in situ
        EntropyCorrector& EntropyFactors() {return *m_ent_correct;}
        
        //! Return the Wavelet filter to be used for intra frames
        WltFilter IntraTransformFilter() { return m_intra_wltfilter; }
        
        //! Return the Wavelet filter to be used for Inter frames
        WltFilter InterTransformFilter() { return m_inter_wltfilter; }
        
        //! Return the Target Bit Rate in kbps   
        int TargetRate() {return m_target_rate;}

        // ... and Sets

        //! Sets a flag indicating that we're producing a locally decoded o/p
        void SetLocalDecode( const bool decode ){m_loc_decode=decode;}

        //! Set whether we're doing lossless coding
        void SetLossless(const bool l){m_lossless = l;}

        //! Set the quality factor
        void SetQf(const float qfac){ m_qf=qfac; CalcLambdas(m_qf); }

        //! Set the nominal number of L1 frames between I frames
        void SetNumL1(const int nl){m_num_L1=nl;}

        //! Set the separation between L1 frames
        void SetL1Sep(const int lsep){m_L1_sep=lsep;}

        //! Set the amount to weight noise in the U component
        void SetUFactor(const float uf){m_ufactor=uf;}

        //! Set the amount to weight noise in the V component
        void SetVFactor(const float vf){m_vfactor=vf;}

        //! Set the number of cycles per degree at the nominal viewing distance
        void SetCPD(const float cpd){m_cpd=cpd;}


        //! Set denoising value - true or false
        void SetDenoise(const bool denoise){m_denoise=denoise;}

        //! Set the output path to be used for diagnostic data
        void SetOutputPath(const char * op){ m_output_path = op; }
        
        //! Sets the entropy factors - TBD: set this up in a constructor and pass encoder params around entirely by reference
        void SetEntropyFactors(EntropyCorrector* entcorrect){m_ent_correct=entcorrect;}
        //! Set the Wavelet filter to be used for intra frames
        void SetIntraTransformFilter(unsigned int wf_idx);
       
        //! Set the Wavelet filter to be used for inter frames
        void SetInterTransformFilter(unsigned int wf_idx);
        
        //! Set the Wavelet filter to be used for intra frames
        void SetIntraTransformFilter(WltFilter wf) { m_intra_wltfilter = wf; }
       
        //! Set the Wavelet filter to be used for inter frames
        void SetInterTransformFilter(WltFilter wf) { m_inter_wltfilter = wf; }
        
        //! Set the target bit rate
        void SetTargetRate(const int rate){m_target_rate = rate;}
    private:
            
        //! Calculate the Lagrangian parameters from the quality factor
        void CalcLambdas(const float qf);

    private:

        //! Flag indicating we're doing local decoding
        bool m_loc_decode;
      
        //! A flag indicating we're doing lossless coding
        bool m_lossless;

        //! Quality factor
        float m_qf; 
        
        //! Number of L1 frames before next I frame
        int m_num_L1;
        
        //! Separation between L1 frames
        int m_L1_sep;   
        
        //! factor for weighting U component quantisation errors
        float m_ufactor;
        
        //! factor for weighting V component quantisation errors
        float m_vfactor;
        
        //! Cycles per degree assumed for viewing the video
        float m_cpd;
        
        //! Flag indicating input denoising
        bool m_denoise;
        
        //! Lagrangian parameter for Intra frame coding
        float m_I_lambda;

        //! Lagrangian parameter for L1 frame coding
        float m_L1_lambda;

        //! Lagrangian parameter for L2 frame coding
        float m_L2_lambda;
        
        //! Lagrangian param for L1 motion estimation
        float m_L1_me_lambda;

        //! Lagrangian param for L2 motion estimation
        float m_L2_me_lambda; 
        
        //! Correction factors for quantiser selection 
        EntropyCorrector* m_ent_correct;
        
        //! Output file path
        std::string m_output_path;
        
        //! Wavelet filter for Intra frames
        WltFilter m_intra_wltfilter;

        //! Wavelet filter for Inter frames
        WltFilter m_inter_wltfilter;
        
        //! Target bit rate
        int m_target_rate;
     
    };

    //! Parameters for the decoding process
    /*!
        Parameters for the decoding process. Derived from CodecParams.
     */
    class DecoderParams: public CodecParams
    {
    public:
            //! Default constructor
        DecoderParams(const VideoFormat& video_format = VIDEO_FORMAT_CIF, FrameType ftype=INTRA_FRAME, unsigned int num_refs = 0, bool set_defaults = false);
        
            ////////////////////////////////////////////////////////////////////
            //NB: Assume default copy constructor, assignment = and destructor//
            //This means pointers are copied, not the objects they point to.////       
            ////////////////////////////////////////////////////////////////////
        
       
    private:        
       
    };

    //! A simple bounds checking function, very useful in a number of places
    inline ValueType BChk(const ValueType &num, const ValueType &max)
    {
        if(num < 0) return 0;
        else if(num >= max) return max-1;
        else return num;
    }

    //! Class for encapsulating quantiser data
    class QuantiserLists
    {
    public:
        //! Default constructor
        QuantiserLists();

        //! Returns 4 times the quantisation factor
        inline int QuantFactor4( const int index ) const {return m_qflist4[index]; }

        //! Returns the intra frame quantisation offset for non-zero values
        inline int IntraQuantOffset4( const int index ) const {return m_intra_offset4[index]; }
        //! Returns the inter frame quantisation offset for non-zero values
        inline int InterQuantOffset4( const int index ) const {return m_inter_offset4[index]; }

        //! Returns the maximum quantiser index supported
        inline int MaxQIndex() const {return m_max_qindex; }
    

    private:
        unsigned int m_max_qindex;
        OneDArray<int> m_qflist4;
        OneDArray<int> m_intra_offset4;
        OneDArray<int> m_inter_offset4;

    };

    //! A constant list of the quantisers being used in Dirac
    static const QuantiserLists dirac_quantiser_lists;

} // namespace dirac

#endif
