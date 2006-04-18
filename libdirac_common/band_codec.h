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
*                 Steve Bearcroft
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

#ifndef _BAND_CODEC_H_
#define _BAND_CODEC_H_

#include <libdirac_common/arith_codec.h>
#include <libdirac_common/wavelet_utils.h>

namespace dirac
{

    //Subclasses the arithmetic codec to produce a coding/decoding tool for subbands


    //! A general class for coding and decoding wavelet subband data.
    /*!
        A general class for coding and decoding wavelet subband data, deriving from the abstract ArithCodec class.
     */
    class BandCodec: public ArithCodec<PicArray >
    {
    public:

        //! Constructor for encoding.
        /*!
            Creates a BandCodec object to encode subband data
            \param    bits_out    the output for the encoded bits
            \param    number_of_contexts the number of contexts used in the encoding process
            \param    band_list    the set of all the subbands
            \param     band_num    the number of the subband being coded 
         */
        BandCodec(BasicOutputManager* bits_out,
                  size_t number_of_contexts,
                  const SubbandList& band_list,
                  int band_num);

        //! Constructor for decoding.
        /*!
            Creates a BandCodec object to decode subband data.
            \param    bits_in        the input for the encoded bits
            \param    number_of_contexts the number of contexts used in the decoding process
            \param    band_list    the set of all the subbands
            \param     band_num    the number of the subband being decoded 
         */
        BandCodec(BitInputManager* bits_in,
                  size_t number_of_contexts,
                  const SubbandList& band_list,
                  int band_num);

        //! Initialise the contexts according to predefined counts.
        void InitContexts();

    protected:
        //! Code an individual quantised value and perform inverse-quantisation
        inline void CodeVal( PicArray& in_data , const int xpos , const int ypos , const ValueType val);

        //! Decode an individual quantised value and perform inverse-quantisation
        inline void DecodeVal(PicArray& out_data , const int xpos , const int ypos );

        //! Set a code block area to a given value
        inline void SetToVal( const CodeBlock& code_block , PicArray& pic_data , const ValueType val);

        //! Set all block values to 0
        inline void ClearBlock( const CodeBlock& code_block , PicArray& pic_data);

    private:
        //functions
        // Overridden from the base class
        virtual void DoWorkCode(PicArray& in_data);
        // Ditto
        virtual void DoWorkDecode(PicArray& out_data);

        virtual void CodeCoeffBlock(const CodeBlock& code_block , PicArray& in_data);
        virtual void DecodeCoeffBlock(const CodeBlock& code_block , PicArray& out_data);

        void ResetAll();

        //! A function for choosing the context for "follow bits"
        inline int ChooseFollowContext( const int bin_number ) const;

        //! A function for choosing the context for "information bits"
        inline int ChooseInfoContext() const;

        //! A function for choosing the context for sign bits
        inline int ChooseSignContext(const PicArray& data , const int xpos , const int ypos ) const;

        //! Private, bodyless copy constructor: class should not be copied
        BandCodec(const BandCodec& cpy);
        //! Private, bodyless copy operator=: class should not be assigned
        BandCodec& operator=(const BandCodec& rhs);

    protected:
        //! variables    
        int m_bnum;

        //! the subband being coded
        const Subband m_node;
    
        //! size of the subband
        int m_vol;
    
        //! the number of coefficients after which contexts are reset
        int m_reset_coeff_num;
    
        //! count of the coefficients since the last context reset
        int m_coeff_count;
    
        //! quantisation and inverse quantisation values
        int m_qf, m_qfinv;
    
        //! reconstruction point
        ValueType m_offset;
    
        //! sum of a neighbourhood of previously (de)coded values
        ValueType m_nhood_sum;
    
        //! the parent subband
        Subband m_pnode;
    
        //! coords of the parent subband
        int m_pxp, m_pyp, m_pxl, m_pyl;
    
        //! position of the parent coefficient
        int m_pxpos, m_pypos;
    
        //! True if the parent of a coeff is not zero
        bool m_parent_notzero;
    
        //! used in selecting a context
        ValueType m_cut_off_point;
    };

    //! A class specially for coding the LF subbands 
    /*!
        A class specially for coding the LF subbands, where we don't want to/can't refer to the 
        parent subband.
    */
    class LFBandCodec: public BandCodec
    {
    public:
        //! Constructor for encoding
        /*!
            Creates a LFBandCodec object to encode subband data.
            \param    bits_out    the output for the encoded bits
            \param    number_of_contexts the number of contexts used in the encoding process
            \param    band_list    the set of all the subbands
            \param     band_num    the number of the subband being coded 
         */        
        LFBandCodec(BasicOutputManager* bits_out,
                    size_t number_of_contexts,
                    const SubbandList& band_list,
                    int band_num)
              : BandCodec(bits_out,number_of_contexts,band_list,band_num){}

        //! Constructor for decoding
        /*!
            Creates a LFBandCodec object to decode subband data.
            \param    bits_in        the input for the encoded bits
            \param    number_of_contexts the number contexts used in the decoding process
            \param    band_list    the set of all the subbands
            \param     band_num    the number of the subband being decoded 
         */
        LFBandCodec(BitInputManager* bits_in,
                    size_t number_of_contexts,
                    const SubbandList& band_list,
                    int band_num)
          : BandCodec(bits_in,number_of_contexts,band_list,band_num){}

    private:
        // Overridden from the base class
        void DoWorkCode(PicArray& in_data);
        // Ditto
        void DoWorkDecode(PicArray& out_data);

        void CodeCoeffBlock(const CodeBlock& code_block , PicArray& in_data);
        void DecodeCoeffBlock(const CodeBlock& code_block , PicArray& out_data);

        //! Private, bodyless copy constructor: class should not be copied
        LFBandCodec(const LFBandCodec& cpy);
        //! Private, bodyless copy operator=: class should not be assigned
        LFBandCodec& operator=(const LFBandCodec& rhs);

    };


    //////////////////////////////////////////////////////////////////////////////////
    //Finally,special class incorporating prediction for the DC band of intra frames//
    //////////////////////////////////////////////////////////////////////////////////

    //! A class specially for coding the DC subband of Intra frames 
    /*!
        A class specially for coding the DC subband of Intra frames, using intra-band prediction 
        of coefficients.
    */
    class IntraDCBandCodec: public BandCodec
    {
    public:
        //! Constructor for encoding
        /*!
            Creates a IntraDCBandCodec object to encode subband data, based on parameters
            \param    bits_out    the output for the encoded bits
            \param    number_of_contexts the number of contexts used in the encoding process
            \param    band_list    the set of all the subbands
         */
        IntraDCBandCodec(BasicOutputManager* bits_out,
                         size_t number_of_contexts,
                         const SubbandList& band_list)
          : BandCodec(bits_out,number_of_contexts,band_list,band_list.Length()){}

        //! Constructor for decoding
        /*!
            Creates a LFBandCodec object to decode subband data, based on parameters
            \param    bits_in        the input for the encoded bits
            \param    number_of_contexts the number of contexts used in the decoding process
            \param    band_list    the set of all the subbands
         */    
        IntraDCBandCodec(BitInputManager* bits_in,
                         size_t number_of_contexts,
                         const SubbandList& band_list)
          : BandCodec(bits_in,number_of_contexts,band_list,band_list.Length()){}

    private:
        void DoWorkCode(PicArray& in_data);                    //overridden from the base class
        void DoWorkDecode(PicArray& out_data); //ditto

        void CodeCoeffBlock(const CodeBlock& code_block , PicArray& in_data);
        void DecodeCoeffBlock(const CodeBlock& code_block , PicArray& out_data);

        //! Private, bodyless copy constructor: class should not be copied
        IntraDCBandCodec(const IntraDCBandCodec& cpy); 

        //! Private, bodyless copy operator=: class should not be assigned
        IntraDCBandCodec& operator=(const IntraDCBandCodec& rhs);

        //! Prediction of a DC value from its previously coded neighbours
        ValueType GetPrediction(const PicArray& data , const int xpos , const int ypos ) const;

    private:
        PicArray m_dc_pred_res;
    };


}// end namespace dirac
#endif
