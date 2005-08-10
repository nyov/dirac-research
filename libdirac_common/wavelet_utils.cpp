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


#include <libdirac_common/wavelet_utils.h>
#include <libdirac_common/common.h>
#include <cstdlib>
using namespace dirac;

// Default constructor
CodeBlock::CodeBlock()
:
    m_skipped( false )
{
    Init( 0 , 0 , 0 , 0 );
}

// Constructor
CodeBlock::CodeBlock( const int xstart , 
                      const int ystart , 
                      const int xend , 
                      const int yend)
:
    m_skipped( false )
{
    Init( xstart , ystart , xend , yend );
}

// Initialises the code block
void CodeBlock::Init( const int xstart , 
                      const int ystart , 
                      const int xend , 
                      const int yend )
{
    m_xstart = xstart;
    m_xend = xend;
    m_ystart = ystart;
    m_yend= yend;

    m_xl = xend - xstart;
    m_yl = yend - ystart;
}


// Default constructor
Subband::Subband()
{
    // this space intentionally left blank
}

// Constructor
Subband::Subband(int xpos,int ypos, int xlen, int ylen):
    m_xp( xpos ),
    m_yp( ypos ),
    m_xl( xlen ),
    m_yl( ylen ),
    m_wt( 1.0 ),
    m_code_block_array(),
    m_skipped( false )
{
    SetNumBlocks( 1 , 1 );
}

// Constructor
Subband::Subband(int xpos,int ypos, int xlen, int ylen, int d)
  :
    m_xp( xpos ),
    m_yp( ypos ),
    m_xl( xlen ), 
    m_yl( ylen ),
    m_wt( 1.0 ),
    m_depth( d ),
    m_code_block_array(),
    m_skipped( false )
{
    SetNumBlocks( 1 , 1 );
}

void Subband::SetWt( const float w )
{
    m_wt = w;

    for (int j=0; j<m_code_block_array.LengthY() ; ++j)
        for (int i=0; i<m_code_block_array.LengthX() ; ++i)
            m_code_block_array[j][i].SetWt( m_wt );
}

void Subband::SetNumBlocks( const int ynum , const int xnum)
{
    m_code_block_array.Resize( ynum , xnum );

    OneDArray<int> xbounds( xnum + 1 );
    OneDArray<int> ybounds( ynum + 1 );

    for (int i=0; i<=xnum ; ++i)
    {
            xbounds[i] = ( i * m_xl )/xnum + m_xp;
    }// i

    for (int j=0; j<=ynum ; ++j)
    {
            ybounds[j] = ( j * m_yl )/ynum + m_yp;
    }// j

    for (int j=0; j<m_code_block_array.LengthY() ; ++j)
        for (int i=0; i<m_code_block_array.LengthX() ; ++i)
            m_code_block_array[j][i].Init( xbounds[i] , ybounds[j] , 
                                           xbounds[i+1] , ybounds[j+1] ); 

}

//! Destructor
Subband::~Subband()
{}

//subband list methods

void SubbandList::Init(const int depth,const int xlen,const int ylen)
{
    int xl=xlen; 
    int yl=ylen; 
    
    Clear(); 
    Subband* tmp;
     
    for (int level = 1; level <= depth; ++level)
    {
        xl/=2; 
        yl/=2; 
        
        tmp=new Subband(xl , 0 , xl , yl , level); 
        AddBand( *tmp ); 
        delete tmp; 
        
        tmp=new Subband( 0 , yl , xl , yl , level); 
        AddBand( *tmp ); 
        delete tmp; 
        
        tmp=new Subband( xl , yl , xl , yl , level); 
        AddBand( *tmp ); 
        delete tmp; 
        
        if (level == depth)
        {
            tmp=new Subband( 0 , 0 , xl , yl , level); 
            AddBand( *tmp ); 
            delete tmp; 
        }        
    }
    //now set the parent-child relationships
    int len = bands.size(); 
    (*this)(len).SetParent(0);         
    (*this)(len).AddChild(len-3); 
    (*this)(len-3).SetParent(len); 
    (*this)(len).AddChild(len-2); 
    (*this)(len-2).SetParent(len); 
    (*this)(len).AddChild(len-1); 
    (*this)(len-1).SetParent(len); 

    for (int level = 1; level < depth; ++level)
    {
         //do parent-child relationship for other bands
        (*this)(3*level + 1).AddChild( 3*(level-1) + 1); 
        (*this)(3*(level-1) + 1).SetParent(3*level + 1); 

        (*this)(3*level + 2).AddChild(3*(level-1) + 2); 
        (*this)(3*(level-1) + 2).SetParent(3*level + 2); 

        (*this)(3*level + 3).AddChild(3*(level-1) + 3); 
        (*this)(3*(level-1) + 3).SetParent(3*level + 3); 
    }// level
}

//wavelet transform methods
///////////////////////////

//public methods

WaveletTransform::WaveletTransform(int d, WltFilter f)
  : m_depth(d),
    m_filt_sort(f)
{
    switch( m_filt_sort )
    {

    case APPROX97 :
        m_vhfilter = new VHFilterApprox9_7;
        break;

    case FIVETHREE : 
        m_vhfilter = new VHFilter5_3;
        break;

    case THIRTEENFIVE :
        m_vhfilter = new VHFilter13_5;
        break;

    default :
        m_vhfilter = new VHFilterDaub9_7;
    }
}

//! Destructor
WaveletTransform::~WaveletTransform()
{
    delete m_vhfilter;
}

void WaveletTransform::Transform(const Direction d, PicArray& pic_data)
{
    int xl,yl; 

    if (d == FORWARD)
    {
        xl=pic_data.LengthX(); 
        yl=pic_data.LengthY(); 
        
        for (int l = 1; l <= m_depth; ++l , xl>>=1 , yl>>=1)
        {
            m_vhfilter->Split(0,0,xl,yl,pic_data); 
        }

        m_band_list.Init( m_depth , pic_data.LengthX() , pic_data.LengthY() );
    }
    else
    {
        xl = pic_data.LengthX()/(1<<(m_depth-1)); 
        yl = pic_data.LengthY()/(1<<(m_depth-1)); 
        
        for (int l = 1; l <= m_depth; ++l, xl<<=1 , yl<<=1 )
        {
            m_vhfilter->Synth(0,0,xl,yl,pic_data); 
        }
        //band list now inaccurate, so clear        
        m_band_list.Clear();     
    }
}

void WaveletTransform::SetBandWeights (const float cpd, 
                                       const FrameSort& fsort,
                                       const ChromaFormat& cformat,
                                       const CompSort csort)
{
    //NB - only designed for progressive to date    

    int xlen, ylen, xl, yl, xp, yp;
    float xfreq, yfreq;
    float temp;

    // Compensate for chroma subsampling

    float chroma_xfac(1.0);
    float chroma_yfac(1.0);

    if( csort != Y_COMP)
    {
        if( cformat == format422)
        {
            chroma_xfac = 2.0;
            chroma_yfac = 1.0;
        }
        else if( cformat == format411 )
        {
            chroma_xfac = 4.0;
            chroma_yfac = 1.0;
        }
        else if( cformat == format420 )
        {
            chroma_xfac = 2.0;
            chroma_yfac = 2.0;
        }

    }

    xlen = 2 * m_band_list(1).Xl();
    ylen = 2 * m_band_list(1).Yl();

    if (cpd != 0.0)
    {
        for( int i = 1; i<=m_band_list.Length() ; i++ )
        {
            xp = m_band_list(i).Xp();
            yp = m_band_list(i).Yp();
            xl = m_band_list(i).Xl();
            yl = m_band_list(i).Yl();


            xfreq = cpd * ( float(xp) + (float(xl)/2.0) ) / float(xlen);
            yfreq = cpd * ( float(yp) + (float(yl)/2.0) ) / float(ylen);

            if ( fsort != I_frame )
            {
                xfreq /= 8.0;
                yfreq /= 8.0;
            }


            temp = PerceptualWeight( xfreq/chroma_xfac , yfreq/chroma_yfac , csort );

            m_band_list(i).SetWt(temp);
        }// i

        // Give more welly to DC in a completely unscientific manner ...
        // (should really relate this to the frame rate)
        m_band_list( m_band_list.Length() ).SetWt(m_band_list(13).Wt()/6.0);

        // Make sure dc is always the lowest weight
        float min_weight=m_band_list(m_band_list.Length()).Wt();

        for( int b=1 ; b<=m_band_list.Length()-1 ; b++ )
            min_weight = ((min_weight>m_band_list(b).Wt()) ? m_band_list(b).Wt() : min_weight);

        m_band_list( m_band_list.Length() ).SetWt( min_weight );

        // Now normalize weights so that white noise is always weighted the same

        // Overall factor to ensure that white noise ends up with the same RMS, whatever the weight
        double overall_factor=0.0;
        //fraction of the total number of samples belonging to each subband
        double subband_fraction;    

        for( int i=1 ; i<=m_band_list.Length() ; i++ )
        {
            subband_fraction = 1.0/((double) m_band_list(i).Scale() * m_band_list(i).Scale());
            overall_factor += subband_fraction/( m_band_list(i).Wt() * m_band_list(i).Wt() );
        }
        overall_factor = std::sqrt( overall_factor );

        //go through and normalise

        for( int i=m_band_list.Length() ; i>0 ; i-- )
            m_band_list(i).SetWt( m_band_list(i).Wt() * overall_factor );
    }
    else
    {//cpd=0 so set all weights to 1

        for( int i=1 ; i<=m_band_list.Length() ; i++ )
           m_band_list(i).SetWt( 1.0 );        

    }

    //Finally, adjust to compensate for the absence of scaling in the transform
    //Factor used to compensate:
    double lfac = m_vhfilter->GetLowFactor();  
    double hfac = m_vhfilter->GetHighFactor(); 

    for ( int i=1 ; i<=m_band_list.Length() ; ++i )
    {

        m_band_list(i).SetWt( m_band_list(i).Wt() / std::pow(lfac,m_depth-1) );

        if ( m_band_list(i).Xp() == 0 && m_band_list(i).Yp() == 0)
            temp = lfac * lfac;
        else if ( m_band_list(i).Xp() != 0 && m_band_list(i).Yp() != 0)
            temp = hfac * hfac;
        else
            temp = lfac * hfac;

        m_band_list(i).SetWt( m_band_list(i).Wt() / temp );

    }// i        

} 

ValueType WaveletTransform::GetMeanDCVal() const
{
    /* The DC band is created by applying the low-pass filter vertically
       and horizontally to create a low-pass subband, and repeating this
       m_depth times. So if m_depth=4, we have 8 applications of the filter.
       The gain applied to values in the DC band is approximated by the gain
       of the low-pass filter, to the power 8. It's only approximate because
       integer lifting implementations involve rounding.
    */

    switch (m_filt_sort)
    {
    case APPROX97 :
        return 512;    
    case FIVETHREE :
        return 512;
    case THIRTEENFIVE :
        return 512;
    default :
        // Assume Daubechies (9,7)
        // Gain of low-pass filter is approx 1.2302 so values are
        // 1.23^(depth*2) * 512

        switch( m_depth )
        {
        case 1 :
            return (ValueType) 775;
        case 2 :
            return (ValueType) 1174;
        case 3 :
            return (ValueType) 1778;
        case 4 :
            return (ValueType) 2692;
        case 5 :
            return (ValueType) 4074;
        case 6 :
            return (ValueType) 6165;
        default :
            return (ValueType) 512;
        }
    }

}


// Private functions //
///////////////////////
// NOTEL MMX version is defined in wavelet_utils_mmx.cpp
// the corresponding changes are made in wavelet_utils_mmx.cpp as well
void WaveletTransform::VHFilter::Interleave( const int xp , 
                                             const int yp , 
                                             const int xl , 
                                             const int yl , 
                                             PicArray& pic_data)
{
    TwoDArray<ValueType> temp_data( yl , xl );
    const int xl2( xl>>1);
    const int yl2( yl>>1);
    const int yend( yp + yl );

    // Make a temporary copy of the subband
    for (int j = yp; j<yend ; j++ )
        memcpy( temp_data[j-yp] , pic_data[j]+xp , xl * sizeof( ValueType ) );

    // Re-order to interleave
    for (int j = 0, s=yp; j<yl2 ; j++, s+=2)
    {
        for (int i = 0 , r=xp ; i<xl2 ; i++ , r += 2)
            pic_data[s][r] = temp_data[j][i];
        for (int i = xl2, r=xp+1; i<xl ; i++ , r += 2)
            pic_data[s][r] = temp_data[j][i];
    }// j 

    for (int j = yl2, s=yp+1 ; j<yl ; j++ , s += 2)
    {
        for (int i = 0 , r=xp ; i<xl2 ; i++ , r += 2)
            pic_data[s][r] = temp_data[j][i];
        for (int i = xl2, r=xp+1; i<xl ; i++ , r += 2)
            pic_data[s][r] = temp_data[j][i];
    }// j 

}

void WaveletTransform::VHFilter::DeInterleave( const int xp , 
                                               const int yp , 
                                               const int xl , 
                                               const int yl , 
                                               PicArray& pic_data)
{
    TwoDArray<ValueType> temp_data( yl , xl );
    const int xl2( xl>>1);
    const int yl2( yl>>1);
    const int xend( xp + xl );
    const int yend( yp + yl );

    // Make a temporary copy of the subband
    for (int  j = yp; j<yend ; j++ )
        memcpy( temp_data[j-yp] , pic_data[j]+xp , xl * sizeof( ValueType ) );

    // Re-order to de-interleave
    for (int  j = yp, s=0; j<yp+yl2 ; j++, s+=2)
    {
        for (int i = xp , r=0 ; i<xp+xl2 ; i++ , r += 2)
            pic_data[j][i] = temp_data[s][r];
        for (int i = xp+xl2, r=1; i<xend ; i++ , r += 2)
            pic_data[j][i] = temp_data[s][r];
    }// j 

    for (int j = yp+yl2, s=1 ; j<yend ; j++ , s += 2)
    {
        for (int i = xp , r=0 ; i<xp+xl2 ; i++ , r += 2)
            pic_data[j][i] = temp_data[s][r];
        for (int i = xp+xl2, r=1; i<xend ; i++ , r += 2)
            pic_data[j][i] = temp_data[s][r];
    }// j 

}

void WaveletTransform::VHFilterDaub9_7::Split (const int xp , 
                                               const int yp , 
                                               const int xl , 
                                               const int yl , 
                                               PicArray& pic_data)
{

    //version based on integer-like types
    //using edge-extension rather than reflection

    const int xend=xp+xl;
    const int yend=yp+yl;

    ValueType* line_data; 

    // Positional variables
    int i,j,k; 
  
    // Objects to do lifting stages 
    // (in revese order and type from synthesis)
    const PredictStep97< 6497 > predictA;
    const PredictStep97< 217 > predictB;
    const UpdateStep97< 3616 > updateA;
    const UpdateStep97< 1817 > updateB;

     //first do horizontal 

    for (j = yp;  j < yend; ++j)
    {
        // First lifting stage
        line_data = pic_data[j];                 

        predictA.Filter( line_data[xp+1] , line_data[xp+2] , line_data[xp] );
        predictB.Filter( line_data[xp] , line_data[xp+1] , line_data[xp+1] );

        for ( k = xp+3; k < xend-1; k+=2)
        {
            predictA.Filter( line_data[k] , line_data[k+1] , line_data[k-1] );
            predictB.Filter( line_data[k-1] , line_data[k-2] , line_data[k] );
        }// i
        
        predictA.Filter( line_data[xend-1] , line_data[xend-2] , line_data[xend-2] );
        predictB.Filter( line_data[xend-2] , line_data[xend-3] , line_data[xend-1] );


         //second lifting stage 
        
        updateA.Filter( line_data[xp+1] , line_data[xp+2] , line_data[xp] );
        updateB.Filter( line_data[xp] , line_data[xp+1] , line_data[xp+1] );

        for ( k = xp+3;  k < xend-1; k+=2)
        { 
            updateA.Filter( line_data[k] , line_data[k+1] , line_data[k-1] );
            updateB.Filter( line_data[k-1] , line_data[k-2] , line_data[k] );
        }// i

        updateA.Filter( line_data[xend-1] , line_data[xend-2] , line_data[xend-2] );
        updateB.Filter( line_data[xend-2] , line_data[xend-3] , line_data[xend-1] );

    }// j

    // next do vertical

    // First lifting stage

    // top edge - j=xp
    for ( i = xp ; i<xend ; ++ i)
    {
        predictA.Filter( pic_data[yp+1][i] , pic_data[yp+2][i] , pic_data[yp][i] );
        predictB.Filter( pic_data[yp][i] , pic_data[yp+1][i] , pic_data[yp+1][i] );
    }// i

    // middle bit
    for ( k = yp+3 ; k<yend-1 ; k+=2)
    {
        for ( i = xp ; i<xend ; ++ i)
        {
            predictA.Filter( pic_data[k][i] , pic_data[k+1][i] , pic_data[k-1][i] );
            predictB.Filter( pic_data[k-1][i] , pic_data[k-2][i] , pic_data[k][i] );
        }// i
    }// j
    // bottom edge
    for ( i = xp ; i<xend ; ++ i)
    {
        predictA.Filter( pic_data[yend-1][i] , pic_data[yend-2][i] , pic_data[yend-2][i] );
        predictB.Filter( pic_data[yend-2][i] , pic_data[yend-3][i] , pic_data[yend-1][i] );
    }// i

    // Second lifting stage

    // top edge - j=xp
    for ( i = xp ; i<xend ; ++ i)
    {
        updateA.Filter( pic_data[yp+1][i] , pic_data[yp+2][i] , pic_data[yp][i] );
        updateB.Filter( pic_data[yp][i] , pic_data[yp+1][i] , pic_data[yp+1][i] );
    }// i

    // middle bit
    for ( k = yp+3 ; k<yend-1 ; k+=2)
    {
        for ( i = xp ; i<xend ; ++ i)
        {
            updateA.Filter( pic_data[k][i] , pic_data[k+1][i] , pic_data[k-1][i] );
            updateB.Filter( pic_data[k-1][i] , pic_data[k-2][i] , pic_data[k][i] );
        }// i
    }// j
    // bottom edge
    for ( i = xp ; i<xend ; ++ i)
    {
        updateA.Filter( pic_data[yend-1][i] , pic_data[yend-2][i] , pic_data[yend-2][i] );
        updateB.Filter( pic_data[yend-2][i] , pic_data[yend-3][i] , pic_data[yend-1][i] );
    }// i

    // Lastly, have to reorder so that subbands are no longer interleaved
    DeInterleave( xp ,yp ,xl ,yl , pic_data );

}

void WaveletTransform::VHFilterDaub9_7::Synth (const int xp ,
                                               const int yp , 
                                               const int xl , 
                                               const int yl , 
                                               PicArray& pic_data)
{

    int i,j,k;

    const int xend( xp+xl );
    const int yend( yp+yl );

    const PredictStep97< 1817 > predictB;
    const PredictStep97< 3616 > predictA;
    const UpdateStep97< 217 > updateB;
    const UpdateStep97< 6497 > updateA;

    ValueType* line_data;

    // Firstly reorder to interleave subbands, so that subsequent calculations can be in-place
    Interleave( xp , yp , xl , yl , pic_data );

    // Next, do the vertical synthesis
    // First lifting stage

    // Begin with the bottom edge
    for ( i = xend-1 ; i>=xp ; --i)
    {
        predictB.Filter( pic_data[yend-2][i] , pic_data[yend-3][i] , pic_data[yend-1][i] );
        predictA.Filter( pic_data[yend-1][i] , pic_data[yend-2][i] , pic_data[yend-2][i] );
    }// i
    // Next, do the middle bit
    for ( k = yend-3 ; k>yp+1 ; k-=2)
    {
        for ( i = xend-1 ; i>=xp ; --i)
        {
            predictB.Filter( pic_data[k-1][i] , pic_data[k-2][i] , pic_data[k][i] );
            predictA.Filter( pic_data[k][i] , pic_data[k+1][i] , pic_data[k-1][i] );
        }// i
    }// j
    // Then do the top edge
    for ( i = xend-1 ; i>=xp ; --i)
    {
        predictB.Filter( pic_data[yp][i] , pic_data[yp+1][i] , pic_data[yp+1][i] );
        predictA.Filter( pic_data[yp+1][i] , pic_data[yp+2][i] , pic_data[yp][i] );
    }// i

    // Second lifting stage

    // Begin with the bottom edge
    for ( i = xend-1 ; i>=xp ; --i)
    {
        updateB.Filter( pic_data[yend-2][i] , pic_data[yend-3][i] , pic_data[yend-1][i] );
        updateA.Filter( pic_data[yend-1][i] , pic_data[yend-2][i] , pic_data[yend-2][i] );
    }// i
    // Next, do the middle bit
    for ( k = yend-3 ; k>yp+1 ; k-=2)
    {
        for ( i = xend-1 ; i>=xp ; --i)
        {
            updateB.Filter( pic_data[k-1][i] , pic_data[k-2][i] , pic_data[k][i] );
            updateA.Filter( pic_data[k][i] , pic_data[k+1][i] , pic_data[k-1][i] );
        }// i
    }// j
    // Then do the top edge
    for ( i = xend-1 ; i>=xp ; --i)
    {
        updateB.Filter( pic_data[yp][i] , pic_data[yp+1][i] , pic_data[yp+1][i] );
        updateA.Filter( pic_data[yp+1][i] , pic_data[yp+2][i] , pic_data[yp][i] );
    }// i


    // Next do the horizontal synthesis
    for (j = yend-1;  j >= yp ; --j)
    {
        // First lifting stage 
        line_data = pic_data[j];

        predictB.Filter( line_data[xend-2] , line_data[xend-3] , line_data[xend-1] ); 
        predictA.Filter( line_data[xend-1] , line_data[xend-2] , line_data[xend-2] );

        for ( k = xend-3;  k > xp+1; k-=2)
        { 
            predictB.Filter( line_data[k-1] , line_data[k-2] , line_data[k] );
            predictA.Filter( line_data[k] , line_data[k+1] , line_data[k-1] );
        }// i

        predictB.Filter( line_data[xp] , line_data[xp+1] , line_data[xp+1] );
        predictA.Filter( line_data[xp+1] , line_data[xp+2] , line_data[xp] );

        // Second lifting stage

        updateB.Filter( line_data[xend-2] , line_data[xend-3] , line_data[xend-1] );
        updateA.Filter( line_data[xend-1] , line_data[xend-2] , line_data[xend-2] );

        for ( k = xend-3;  k > xp+1; k-=2)
        {
            updateB.Filter( line_data[k-1] , line_data[k-2] , line_data[k] );
            updateA.Filter( line_data[k] , line_data[k+1] , line_data[k-1] );
        }// i

        updateB.Filter( line_data[xp] , line_data[xp+1] , line_data[xp+1] );
        updateA.Filter( line_data[xp+1] , line_data[xp+2] , line_data[xp] );

    }

} 

void WaveletTransform::VHFilter5_3::Split(const int xp , 
                                          const int yp , 
                                          const int xl , 
                                          const int yl , 
                                          PicArray& pic_data)
{

    //version based on integer-like types
    //using edge-extension rather than reflection

    const int xend=xp+xl;
    const int yend=yp+yl;

    ValueType* line_data; 

    // Positional variables
    int i,j,k; 
  
    // Objects to do lifting stages 
    // (in revese order and type from synthesis)
    const PredictStepShift< 1 > predict;
    const UpdateStepShift< 2 > update;

     //first do horizontal 

    for (j = yp;  j < yend; ++j)
    {
        // First lifting stage
        line_data = &pic_data[j][xp];                 

        predict.Filter( line_data[1] , line_data[2] , line_data[0] );
        update.Filter( line_data[0] , line_data[1] , line_data[1] );

        for (k = 3; k < xl-1; k+=2)
        {
            predict.Filter( line_data[k] , line_data[k+1] , line_data[k-1] );
            update.Filter(  line_data[k-1] , line_data[k-2] , line_data[k] );
        }// i
        
        predict.Filter( line_data[xl-1] , line_data[xl-2] , line_data[xl-2] );
        update.Filter( line_data[xl-2] , line_data[xl-3] , line_data[xl-1] );

    }// j

    // next do vertical

    // First lifting stage

    // top edge - j=xp
    for ( i = xp ; i<xend ; ++ i)
    {
        predict.Filter( pic_data[yp+1][i] , pic_data[yp+2][i] , pic_data[yp][i] );
        update.Filter( pic_data[yp][i] , pic_data[yp+1][i] , pic_data[yp+1][i] );
    }// i

    // middle bit
    for (k = yp+3 ; k<yend-1 ; k+=2)
    {
        for ( i = xp ; i<xend ; ++ i)
        {
            predict.Filter( pic_data[k][i] , pic_data[k+1][i] , pic_data[k-1][i] );
            update.Filter( pic_data[k-1][i] , pic_data[k-2][i] , pic_data[k][i] );
        }// i
    }// j
    // bottom edge
    for ( i = xp ; i<xend ; ++ i)
    {
        predict.Filter( pic_data[yend-1][i] , pic_data[yend-2][i] , pic_data[yend-2][i] );
        update.Filter( pic_data[yend-2][i] , pic_data[yend-3][i] , pic_data[yend-1][i] );
    }// i

    // Lastly, have to reorder so that subbands are no longer interleaved
    DeInterleave( xp , yp , xl , yl , pic_data );
}

#if !defined HAVE_MMX
// NOTE: MMX version is defined in wavelet_utils_mmx.cpp
// the corresponding changes are made in wavelet_utils_mmx.cpp as well
void WaveletTransform::VHFilter5_3::Synth(const int xp ,
                                          const int yp , 
                                          const int xl , 
                                          const int yl , 
                                          PicArray& pic_data)
{
    int i,j,k;

    const int xend( xp+xl );
    const int yend( yp+yl );

    const PredictStepShift< 2 > predict;
    const UpdateStepShift< 1 > update;

    ValueType* line_data;

    // Firstly reorder to interleave subbands, so that subsequent calculations can be in-place
    Interleave( xp , yp , xl , yl , pic_data );

    // Next, do the vertical synthesis
    // First lifting stage

    // Begin with the bottom edge
    for ( i = xend-1 ; i>=xp ; --i)
    {
        predict.Filter( pic_data[yend-2][i] , pic_data[yend-3][i] , pic_data[yend-1][i] );
        update.Filter( pic_data[yend-1][i] , pic_data[yend-2][i] , pic_data[yend-2][i] );
    }// i
    // Next, do the middle bit
    for ( k = yend-3 ; k>yp+1 ; k-=2)
    {
        for ( i = xend-1 ; i>=xp ; --i)
        {
            predict.Filter( pic_data[k-1][i] , pic_data[k-2][i] , pic_data[k][i] );
            update.Filter( pic_data[k][i] , pic_data[k+1][i] , pic_data[k-1][i] );
        }// i
    }// j
    // Then do the top edge
    for ( i = xend-1 ; i>=xp ; --i)
    {
        predict.Filter( pic_data[yp][i] , pic_data[yp+1][i] , pic_data[yp+1][i] );
        update.Filter( pic_data[yp+1][i] , pic_data[yp+2][i] , pic_data[yp][i] );
    }// i

    // Next do the horizontal synthesis
    for (j = yend-1;  j >= yp ; --j)
    {
        // First lifting stage 
        line_data = &pic_data[j][xp];

        predict.Filter( line_data[xl-2] , line_data[xl-3] , line_data[xl-1] ); 
        update.Filter( line_data[xl-1] , line_data[xl-2] , line_data[xl-2] );

        for ( k = xl-3;  k > 1; k-=2)
        { 
            predict.Filter( line_data[k-1] , line_data[k-2] , line_data[k] );
            update.Filter( line_data[k] , line_data[k+1] , line_data[k-1] );
        }// i

        predict.Filter( line_data[0] , line_data[1] , line_data[1] );
        update.Filter( line_data[1] , line_data[2] , line_data[0] );

    }

}
#endif

void WaveletTransform::VHFilterApprox9_7::Split(const int xp , 
                                                const int yp , 
                                                const int xl , 
                                                const int yl ,
                                                PicArray& pic_data)
{

    //version based on integer-like types
    //using edge-extension rather than reflection

    const int xend=xp+xl;
    const int yend=yp+yl;

    ValueType* line_data; 

    // Positional variables
    int i,j,k; 

    PredictStepFourTap< 4 , 9 , -1 > predict;
    UpdateStepShift< 2 > update;

    //first do horizontal 

    for (j = yp;  j < yend; ++j)
    {
        line_data = &pic_data[j][xp];                 

        // First lifting stage
        predict.Filter( line_data[1] , line_data[0]  , line_data[2] , line_data[0] , line_data[4] );
        for (k=3 ; k<xl-3 ; k+=2)
        {
            predict.Filter( line_data[k] , line_data[k-1] , line_data[k+1] , line_data[k-3] , line_data[k+3] );
        }// i 
        predict.Filter( line_data[xl-3] , line_data[xl-4] , line_data[xl-2] , line_data[xl-6] , line_data[xl-2] );
        predict.Filter( line_data[xl-1] , line_data[xl-2] , line_data[xl-2] , line_data[xl-4] , line_data[xl-2] );

        //Second lifting stage 

        update.Filter( line_data[0] , line_data[1] , line_data[1] );
        for (i=2 ; i<xl-1 ; i+=2 )
        {
            update.Filter( line_data[i] , line_data[i-1] , line_data[i+1] );
        }// i 

   }// j

    // next do vertical

    // First lifting stage
    // top line
    for ( i = xp ; i<xend ; ++ i)
    {
        predict.Filter( pic_data[yp+1][i] , pic_data[yp][i] , pic_data[yp+2][i] , pic_data[yp][i] , pic_data[yp+4][i] );
    }// i

    // middle bit
    for ( k = yp+3 ; k<yend-3 ; k+=2)
    {
        for ( i = xp ; i<xend ; ++i)
        {
            predict.Filter( pic_data[k][i] , pic_data[k-1][i] , pic_data[k+1][i] , pic_data[k-3][i] , pic_data[k+3][i] );
        }// i
    }// j

    // bottom lines
    for ( i = xp ; i<xend ; ++ i)
    {
        predict.Filter( pic_data[yend-3][i] , pic_data[yend-4][i] , pic_data[yend-2][i] , pic_data[yend-6][i] , pic_data[yend-2][i] );
        predict.Filter( pic_data[yend-1][i] , pic_data[yend-2][i] , pic_data[yend-2][i] , pic_data[yend-4][i] , pic_data[yend-2][i] );
    }// i

    //Second lifting stage
    for ( i = xp ; i<xend ; ++ i)
    {
        update.Filter( pic_data[yp][i] , pic_data[yp+1][i] , pic_data[yp+1][i] );

    }// i
    // middle bit
    for ( j = yp+2 ; j<yend-1 ; j+=2 , k+=2)
    {
        for ( i = xp ; i<xend ; ++i)
        {
            update.Filter( pic_data[j][i] , pic_data[j-1][i] , pic_data[j+1][i] );
        }// i
    }// j

    // Lastly, have to reorder so that subbands are no longer interleaved
    DeInterleave( xp , yp , xl , yl , pic_data );
}

#if !defined(HAVE_MMX)
// NOTE: MMX version is defined in wavelet_utils_mmx.cpp
// the corresponding changes are made in wavelet_utils_mmx.cpp as well
void WaveletTransform::VHFilterApprox9_7::Synth(const int xp , 
                                                const int yp , 
                                                const int xl , 
                                                const int yl , 
                                                PicArray& pic_data)
{
    int i,j;

    const int xend( xp+xl );
    const int yend( yp+yl );

    PredictStepShift<2> predict;
    UpdateStepFourTap< 4 , 9 , -1> update;

    ValueType* line_data;

    // Firstly reorder to interleave subbands, so that subsequent calculations can be in-place
    Interleave( xp , yp , xl ,yl , pic_data );

    // First, do the vertical synthesis

    // First lifting stage
    // Middle bit
    for ( j=yend-2 ; j>=yp+2 ; j-=2 )
    {
        for ( i = xend-1 ; i>=xp ; --i)
        {
            predict.Filter( pic_data[j][i] , pic_data[j-1][i] , pic_data[j+1][i] );
        }// i
    }// j

    // top line
    for ( i = xend-1 ; i>=xp ; --i)
    {
        predict.Filter( pic_data[yp][i] , pic_data[yp+1][i] , pic_data[yp+1][i] );
    }// i


    // Second lifting stage
    for ( i = xend-1 ; i>=xp ; --i)
    {

        update.Filter( pic_data[yend-1][i] , pic_data[yend-2][i] , pic_data[yend-2][i] , pic_data[yend-4][i] , pic_data[yend-2][i] );
        update.Filter( pic_data[yend-3][i] , pic_data[yend-4][i] , pic_data[yend-2][i] , pic_data[yend-6][i] , pic_data[yend-2][i] );
    }// i

    // middle bit
    for ( j=yend-5 ; j>=yp+3 ; j-=2)
    {
        for ( i = xend-1 ; i>=xp ; --i)
        {
            update.Filter( pic_data[j][i] , pic_data[j-1][i] , pic_data[j+1][i] , pic_data[j-3][i] , pic_data[j+3][i] );
        }// i
    }// k

    for ( i = xend-1 ; i>=xp ; --i)
    {
        update.Filter( pic_data[yp+1][i] , pic_data[yp][i] , pic_data[yp+2][i] , pic_data[yp][i] , pic_data[yp+4][i] );
    }// i

    // Next do the horizontal synthesis
    for (j = yend-1;  j >= yp; --j)
    {
        line_data = &pic_data[j][xp];                 

        // First lifting stage
        for (i=xl-2 ; i>=2 ; i-=2)
        {
            predict.Filter( line_data[i] , line_data[i-1] , line_data[i+1] );
        }// i 
        predict.Filter( line_data[0] , line_data[1] , line_data[1] );

        // Second lifting stage
        update.Filter( line_data[xl-1] , line_data[xl-2] , line_data[xl-2] , line_data[xl-4] , line_data[xl-2] );
        update.Filter( line_data[xl-3] , line_data[xl-4] , line_data[xl-2] , line_data[xl-6] , line_data[xl-2] );
        for (i=xl-5 ; i>=3 ; i-=2)
        {
            update.Filter( line_data[i] , line_data[i-1] , line_data[i+1] , line_data[i-3] , line_data[i+3] );
        }// i 
        update.Filter( line_data[1] , line_data[0] , line_data[2] , line_data[0] , line_data[4] );


    }// j


}
#endif

void WaveletTransform::VHFilter13_5::Split(const int xp , 
                                           const int yp , 
                                           const int xl , 
                                           const int yl , 
                                           PicArray& pic_data)
{

    //version based on integer-like types
    //using edge-extension rather than reflection

    const int xend=xp+xl;
    const int yend=yp+yl;

    PredictStepFourTap< 4 , 9 , -1 > predict;
    UpdateStepFourTap< 5 , 9 , -1> update;

    ValueType* line_data; 

    // Positional variables
    int i,j,k; 
  
     //first do horizontal 

    for (j = yp;  j < yend; ++j)
    {
        line_data = &pic_data[j][xp];                 

        // First lifting stage
        predict.Filter( line_data[1] , line_data[0] ,line_data[2] , line_data[0] , line_data[4] ); 
        for (k=3 ; k<xl-3 ; k+=2)
        {
            predict.Filter( line_data[k] , line_data[k-1] , line_data[k+1] , line_data[k-3] , line_data[k+3] );
        }// i 

        predict.Filter( line_data[xl-3] , line_data[xl-4] , line_data[xl-2] , line_data[xl-6] , line_data[xl-2] );
        predict.Filter( line_data[xl-1] , line_data[xl-2] , line_data[xl-2] , line_data[xl-4] , line_data[xl-2] );

         //second lifting stage 
        update.Filter( line_data[0] , line_data[1] , line_data[1] , line_data[3] , line_data[1] );
        update.Filter( line_data[2] , line_data[1] , line_data[3] , line_data[5] , line_data[1] );
        for (k=4 ; k<xl-3 ; k+=2)
        {
            update.Filter( line_data[k] , line_data[k-1] , line_data[k+1] , line_data[k-3] , line_data[k+3] );
        }// i 

        update.Filter( line_data[xl-2] , line_data[xl-3] , line_data[xl-1] , line_data[xl-5] , line_data[xl-1] );
    }// j

    // next do vertical

    // First lifting stage

    // top edge - j=xp
    for ( i = xp ; i<xend ; ++ i)
    {
        predict.Filter( pic_data[yp+1][i] , pic_data[yp][i] , pic_data[yp+2][i] , pic_data[yp][i] , pic_data[yp+4][i] ); 
    }// i

    // middle bit
    for ( k = yp+3 ; k<yend-3 ; k+=2)
    {
        for ( i = xp ; i<xend ; ++ i)
        {
            predict.Filter( pic_data[k][i] , pic_data[k-1][i] , pic_data[k+1][i] , pic_data[k-3][i] , pic_data[k+3][i] );
        }// i
    }// j
    // bottom edge
    for ( i = xp ; i<xend ; ++ i)
    {
        predict.Filter( pic_data[yend-3][i] , pic_data[yend-4][i] , pic_data[yend-2][i] , pic_data[yend-6][i] , pic_data[yend-2][i] );
        predict.Filter( pic_data[yend-1][i] , pic_data[yend-2][i] , pic_data[yend-2][i] , pic_data[yend-4][i] , pic_data[yend-2][i] );
    }// i

    // Second lifting stage

    // top edge - j=xp
    for ( i = xp ; i<xend ; ++ i)
    {
        update.Filter( pic_data[yp][i] , pic_data[yp+1][i] , pic_data[yp+1][i] , pic_data[yp+3][i] , pic_data[yp+1][i] );
        update.Filter( pic_data[yp+2][i] , pic_data[yp+1][i] , pic_data[yp+3][i] , pic_data[yp+5][i] , pic_data[yp+1][i] );
    }// i

    // middle bit
    for ( k = yp+4 ; k<yend-3 ; k+=2)
    {
        for ( i = xp ; i<xend ; ++ i)
        {
           update.Filter( pic_data[k][i] , pic_data[k-1][i] , pic_data[k+1][i] , pic_data[k-3][i] , pic_data[k+3][i] );
        }// i
    }// j
    // bottom edge
    for ( i = xp ; i<xend ; ++ i)
    {
        update.Filter( pic_data[yend-2][i] , pic_data[yend-3][i] , pic_data[yend-1][i] , pic_data[yend-5][i] , pic_data[yend-1][i] );
    }// i

    // Lastly, have to reorder so that subbands are no longer interleaved
    DeInterleave( xp , yp , xl , yl , pic_data );
}

#if !defined(HAVE_MMX)
// NOTE: MMX version is defined in wavelet_utils_mmx.cpp
// the corresponding changes are made in wavelet_utils_mmx.cpp as well
void WaveletTransform::VHFilter13_5::Synth(const int xp ,
                                           const int yp , 
                                           const int xl ,
                                           const int yl , 
                                           PicArray& pic_data)
{
    int i,j,k;

    const int xend( xp+xl );
    const int yend( yp+yl );

    PredictStepFourTap< 5 , 9 , -1 > predict;
    UpdateStepFourTap< 4 , 9 , -1> update;

    // Firstly reorder to interleave subbands, so that subsequent calculations can be in-place
    Interleave( xp , yp , xl , yl , pic_data );  

    // Next, do the vertical synthesis

    // First lifting stage
    // bottom edge
    for ( i = xp ; i<xend ; ++ i)
    {
        predict.Filter( pic_data[yend-2][i] , pic_data[yend-3][i] , pic_data[yend-1][i] , pic_data[yend-5][i] , pic_data[yend-1][i] );
    }// i

    // middle bit
    for ( k = yend-4 ; k>=yp+4 ; k-=2)
    {
        for ( i = xp ; i<xend ; ++ i)
        {
           predict.Filter( pic_data[k][i] , pic_data[k-1][i] , pic_data[k+1][i] , pic_data[k-3][i] , pic_data[k+3][i] );
        }// i
    }// j

    // top edge - j=xp
    for ( i = xp ; i<xend ; ++ i)
    {
        predict.Filter( pic_data[yp+2][i] , pic_data[yp+1][i] , pic_data[yp+3][i] , pic_data[yp+5][i] , pic_data[yp+1][i] );
        predict.Filter( pic_data[yp][i] , pic_data[yp+1][i] , pic_data[yp+1][i] , pic_data[yp+3][i] , pic_data[yp+1][i] );

    }// i

    // Second lifting stage
    // bottom edge
    for ( i = xp ; i<xend ; ++ i)
    {
        update.Filter( pic_data[yend-1][i] , pic_data[yend-2][i] , pic_data[yend-2][i] , pic_data[yend-4][i] , pic_data[yend-2][i] );
        update.Filter( pic_data[yend-3][i] , pic_data[yend-4][i] , pic_data[yend-2][i] , pic_data[yend-6][i] , pic_data[yend-2][i] );

    }// i

    // middle bit
    for ( k = yend-5 ; k>=yp+3 ; k-=2)
    {
        for ( i = xp ; i<xend ; ++ i)
        {
            update.Filter( pic_data[k][i] , pic_data[k-1][i] , pic_data[k+1][i] , pic_data[k-3][i] , pic_data[k+3][i] );
        }// i
    }// j

    // top edge - j=xp
    for ( i = xp ; i<xend ; ++ i)
    {
        update.Filter( pic_data[yp+1][i] , pic_data[yp][i] , pic_data[yp+2][i] , pic_data[yp][i] , pic_data[yp+4][i] ); 
    }// i

    // Next do the horizontal synthesis

    ValueType* line_data;

    for (j = yend-1;  j >= yp ; --j)
    {
        line_data = &pic_data[j][xp];                 

        // First lifting stage

        predict.Filter( line_data[xl-2] , line_data[xl-3] , line_data[xl-1] , line_data[xl-5] , line_data[xl-1] );

        for (k=xl-4 ; k>=4 ; k-=2)
        {
            predict.Filter( line_data[k] , line_data[k-1] , line_data[k+1] , line_data[k-3] , line_data[k+3] );

        }// i 
        predict.Filter( line_data[2] , line_data[1] , line_data[3] , line_data[5] , line_data[1] );
        predict.Filter( line_data[0] , line_data[1] , line_data[1] , line_data[3] , line_data[1] );

         //second lifting stage 
        update.Filter( line_data[xl-1] , line_data[xl-2] , line_data[xl-2] , line_data[xl-4] , line_data[xl-2] );
        update.Filter( line_data[xl-3] , line_data[xl-4] , line_data[xl-2] , line_data[xl-6] , line_data[xl-2] );

        for (k=xl-5 ; k>=3 ; k-=2)
        {
            update.Filter( line_data[k] , line_data[k-1] , line_data[k+1] , line_data[k-3] , line_data[k+3] );
        }// i 

        update.Filter( line_data[1] , line_data[0] , line_data[2] , line_data[0] , line_data[4] ); 

    }// j
} 
#endif

// Returns a perceptual noise weighting based on extending CCIR 959 values
// assuming a two-d isotropic response. Also has a fudge factor of 20% for chroma
float WaveletTransform::PerceptualWeight( const float xf , 
                                     const float yf ,  
                                     const CompSort cs )
{
    double freq_sqd( xf*xf + yf*yf );

    if ( cs != Y_COMP )
        freq_sqd *= 1.2;

    return 0.255 * std::pow( 1.0 + 0.2561*freq_sqd , 0.75) ;

}
