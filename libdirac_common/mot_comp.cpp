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
* Contributor(s): Richard Felton (Original Author), 
*                 Thomas Davies,
*                 Steve Bearcroft
*                 Mike Ferenduros
*                 Anuradha Suraparaju
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


#include <libdirac_common/mot_comp.h>
#if defined(HAVE_MMX)
#include <libdirac_common/mot_comp_mmx.h>
#endif
#include <libdirac_common/motion.h>
#include <libdirac_common/frame_buffer.h>
using namespace dirac;

using std::vector;

//--public member functions--//
///////////////////////////////

// Convenience function to perform motion compensation on a frame
// Static function that motion compensates a frame. It uses the
// MV precision value in the CodecParams to instantiate the 
// appropriate MotionCompensation sub-class.
void MotionCompensator::CompensateFrame(const CodecParams &cp, 
                                        const AddOrSub direction , 
                                        FrameBuffer& buffer , 
                                        const int fnum, 
                                        const MvData& mv_data )
{
    switch (cp.MVPrecision())
    {
    case 3:
    {
        MotionCompensator_EighthPixel my_comp(cp);
        my_comp.CompensateFrame( direction , buffer , fnum , mv_data);
        break;
    }
    case 1:
    {
        MotionCompensator_HalfPixel my_comp(cp);
        my_comp.CompensateFrame( direction , buffer , fnum , mv_data);
        break;
    }
    case 0:
    {
        MotionCompensator_Pixel my_comp(cp);
        my_comp.CompensateFrame( direction , buffer , fnum , mv_data);
        break;
    }
    default:
    {
        MotionCompensator_QuarterPixel my_comp(cp);
        my_comp.CompensateFrame( direction , buffer , fnum , mv_data);
        break;
    }
    }

    return;
}

// Constructor
// Initialises the lookup tables that is needed for motion 
// motion compensation. Creates the necessary arithmetic objects and
// calls ReConfig to create weighting blocks to fit the values within
// m_cparams.
MotionCompensator::MotionCompensator( const CodecParams &cp ): 
    m_cparams(cp),
    luma_or_chroma(true)
{
    //Configure weighting blocks for the first time
    m_block_weights = NULL;
    m_half_block_weights = NULL;
    ReConfig();
}

// Destructor
MotionCompensator::~MotionCompensator(){

//Tidy up the pointers
    delete[] m_block_weights;
    delete[] m_half_block_weights;
}

//Called to perform motion compensated addition/subtraction on an entire frame.
void MotionCompensator::CompensateFrame( const AddOrSub direction , 
                                                    FrameBuffer& my_buffer ,
                                                    int fnum , 
                                                    const MvData& mv_data)
{
     m_add_or_sub = direction;

     int ref1_idx,ref2_idx;    
     Frame& my_frame=my_buffer.GetFrame(fnum);
     const FrameSort& fsort=my_frame.GetFparams().FSort();

     m_cformat = my_frame.GetFparams().CFormat();

     if (fsort!=I_frame)
     {//we can motion compensate

         const std::vector<int>& refs=my_frame.GetFparams().Refs();
         if (refs.size()>0)
         {
             //extract the references
             ref1_idx = refs[0];
             if ( refs.size()>1 )
                 ref2_idx = refs[1];
             else
                 ref2_idx = refs[0];

             const Frame& ref1frame = my_buffer.GetFrame(ref1_idx);
             const Frame& ref2frame = my_buffer.GetFrame(ref2_idx);

             luma_or_chroma = true;                
             //now do all the components
             CompensateComponent( my_frame , ref1frame , ref2frame , mv_data , Y_COMP);

         if ( m_cformat != Yonly )
             {
                 luma_or_chroma = false;                
                 CompensateComponent( my_frame , ref1frame , ref2frame , mv_data , U_COMP);
                 CompensateComponent( my_frame , ref1frame , ref2frame , mv_data , V_COMP);
             }
         }
     }
}

//--private member functions--//
////////////////////////////////

//Needs to be called if the blocksize changes (and
//on startup). This method creates an array of weighting
//blocks that are used to acheive correctly overlapping
//blocks.
void MotionCompensator::ReConfig()
{
    if (luma_or_chroma)
        m_bparams = m_cparams.LumaBParams(2);
    else
        m_bparams = m_cparams.ChromaBParams(2);

    if(m_block_weights != NULL)
        delete[] m_block_weights;

    if(m_half_block_weights != NULL)
        delete[] m_half_block_weights;

    // Create new weights array.
    m_block_weights = new TwoDArray<ValueType>[9];
    m_half_block_weights = new TwoDArray<ValueType>[9];
    for(int i = 0; i < 9; i++)
    {
        m_block_weights[i].Resize(  m_bparams.Yblen() , m_bparams.Xblen() );
        m_half_block_weights[i].Resize(  m_bparams.Yblen() , m_bparams.Xblen() );
    }
    // We can create all nine weighting blocks by calculating values
    // for four blocks and mirroring them to generate the others.
    CreateBlock( m_bparams , false , false , m_half_block_weights[0] );
    CreateBlock( m_bparams , false , true , m_half_block_weights[3] );
    CreateBlock( m_bparams , true , false , m_half_block_weights[1] );
    CreateBlock( m_bparams , true , true , m_half_block_weights[4] );

    // Note order of flipping is important.    
    FlipX( m_half_block_weights[3] , m_bparams , m_half_block_weights[5] );
    FlipX( m_half_block_weights[0] , m_bparams , m_half_block_weights[2] );
    FlipY( m_half_block_weights[0] , m_bparams , m_half_block_weights[6] );
    FlipX( m_half_block_weights[6] , m_bparams , m_half_block_weights[8] );
    FlipY( m_half_block_weights[1] , m_bparams , m_half_block_weights[7] );

    for( int k = 0; k < 9; k++)
    {
        for ( int j =m_half_block_weights[k].FirstY(); j <= m_half_block_weights[k].LastY(); j++)
        {
            for ( int i =m_half_block_weights[k].FirstX(); i <= m_half_block_weights[k].LastX(); i++)
            {
                m_block_weights[k][j][i] = m_half_block_weights[k][j][i] << 1;
            }// i
        }// j
    }// k
}

void MotionCompensator::CompensateComponent( Frame& picframe , 
                                                        const Frame &ref1frame , 
                                                        const Frame& ref2frame ,
                                                        const MvData& mv_data ,
                                                        const CompSort cs)
{
    // Set up references to pictures and references
    PicArray& pic_data_out = picframe.Data( cs );

    // Size of frame component being motion compensated
    ImageCoords orig_pic_size(pic_data_out.LengthX(), pic_data_out.LengthY());
    const PicArray& ref1up = ref1frame.UpData( cs );
    const PicArray& ref2up = ref2frame.UpData( cs );

    // Set up a row of blocks which will contain the MC data, which
    // we'll add or subtract to pic_data_out
    TwoDArray<CalcValueType> pic_data(m_bparams.Yblen(), pic_data_out.LengthX(), 0 );

    // Factors to compensate for subsampling of chroma
    int xscale_factor = 1;
    int yscale_factor = 1;

    if ( cs != Y_COMP )
    {
        if (m_cformat == format420)
        {
            xscale_factor = 2;
            yscale_factor = 2;
        }
        else if (m_cformat == format422)
        {
            xscale_factor = 2;
            yscale_factor = 1;
        }
        else if (m_cformat == format411)
        {
            xscale_factor = 4;
            yscale_factor = 1;
        }

    } 

    // Reference to the relevant DC array
    const TwoDArray<ValueType>& dcarray = mv_data.DC( cs );

    // Set up references to the vectors
    const int num_refs = picframe.GetFparams().Refs().size();
    const MvArray* mv_array1; 
    const MvArray* mv_array2;
    mv_array1 = &mv_data.Vectors(1);
    if (num_refs ==2 )
        mv_array2 = &mv_data.Vectors(2);
    else
        mv_array2 = &mv_data.Vectors(1);

    ReConfig();//set all the weighting blocks up    

    //Blocks are listed left to right, line by line.
    MVector mv1,mv2;
    PredMode block_mode;
    ValueType dc;

    //Coords of the top-left corner of a block
    ImageCoords pos;

    //Loop for each block in the output image.
    //The CompensateBlock function will use the image pointed to by ref1up
    //and add the compensated pixels to the image pointed to by pic_data.
    size_t wgt_idx;

    //Loop over all the block rows
    pos.y = -m_bparams.Yoffset();
    int save_from_row = m_bparams.Ybsep()-m_bparams.Yoffset();

    const int x_end_data = pic_data_out.FirstX() + std::min(pic_data_out.LengthX(), m_cparams.XNumBlocks()*m_bparams.Xbsep() );
    const int y_end_data = pic_data_out.FirstY() + std::min(pic_data_out.LengthY(), m_cparams.YNumBlocks()*m_bparams.Ybsep() );

    for(int yblock = 0; yblock < m_cparams.YNumBlocks(); ++yblock)
    {
        pos.x = -m_bparams.Xoffset();
        //loop over all the blocks in a row
        for(int xblock = 0 ; xblock < m_cparams.XNumBlocks(); ++xblock)
        {

            //Decide which weights to use.
            if((xblock != 0)&&(xblock < m_cparams.XNumBlocks() - 1))
            {
                if((yblock != 0)&&(yblock < m_cparams.YNumBlocks() - 1))    
                    wgt_idx = 4;
                else if(yblock == 0) 
                    wgt_idx = 1;
                else 
                    wgt_idx= 7;
            }
            else if(xblock == 0)
            {
                if((yblock != 0)&&(yblock < m_cparams.YNumBlocks() - 1))    
                    wgt_idx = 3;
                else if(yblock == 0) 
                    wgt_idx = 0;
                else 
                    wgt_idx = 6;
            }
            else
            {
                if((yblock != 0)&&(yblock < m_cparams.YNumBlocks() - 1))    
                    wgt_idx = 5;
                else if(yblock == 0) 
                    wgt_idx = 2;
                else 
                    wgt_idx = 8;
            }

            block_mode = mv_data.Mode()[yblock][xblock];

            if(block_mode == REF1_ONLY)
            {
                mv1 = (*mv_array1)[yblock][xblock];
                mv1.x /= xscale_factor;
                mv1.y /= yscale_factor;

                CompensateBlock(pic_data, orig_pic_size, ref1up, mv1, pos, m_block_weights[wgt_idx]);
            }
            else if (block_mode == REF2_ONLY)
            {                
                mv2 = (*mv_array2)[yblock][xblock];
                mv2.x /= xscale_factor;
                mv2.y /= yscale_factor;

                CompensateBlock(pic_data, orig_pic_size, ref2up, mv2, pos, m_block_weights[wgt_idx]);
            }
            else if(block_mode == REF1AND2)
            {
                mv1 = (*mv_array1)[yblock][xblock];
                mv1.x /= xscale_factor;
                mv1.y /= yscale_factor;

                CompensateBlock(pic_data, orig_pic_size, ref1up, mv1, pos, m_half_block_weights[wgt_idx]);
                mv2 = (*mv_array2)[yblock][xblock];
                mv2.x /= xscale_factor;
                mv2.y /= yscale_factor;

                CompensateBlock(pic_data, orig_pic_size, ref2up, mv2, pos, m_half_block_weights[wgt_idx]);                    
            }
            else
            {//we have a DC block.
                dc = dcarray[yblock][xblock]<<2;// DC is only given 8 bits, 
                                            // so need to shift to get 10-bit data

                DCBlock(pic_data, orig_pic_size, dc,pos, m_block_weights[wgt_idx]);
            }

            //Increment the block horizontal position
            pos.x += m_bparams.Xbsep();

        }//xblock

          // Update the pic data
          // Use only the first Ybsep rows since the remaining rows are
          // needed for the next row of blocks since we are using overlapped
          // blocks motion compensation
          if (m_add_or_sub == SUBTRACT)
          {
              int start_y = std::max(pic_data_out.FirstY() , pos.y) ;
               int end_y = std::min (pic_data_out.FirstY() + pos.y + m_bparams.Ybsep() , y_end_data); 
               if ((pic_data_out.FirstY() + pos.y +  m_bparams.Yblen()) >= y_end_data)
               {
                    end_y = y_end_data;
               }
               //std::cerr << "start_y=" << start_y;
               //std::cerr << "end_y=" << end_y << std::endl;

             for ( int i = start_y, pos_y = 0; i < end_y; i++, pos_y++)
             {
                 CalcValueType *pic_row = pic_data[pos_y];
                 ValueType *out_row = pic_data_out[i];

                 for ( int j =pic_data_out.FirstX(); j < x_end_data; ++j)
                 {
                     out_row[j] -= static_cast<ValueType>( (pic_row[j] + 1024) >> 11 );
                 }

                 // Okay, we've done all the actual blocks. Now if the picture is further padded
                 // we need to set the padded values to zero beyond the last block in the row,
                 // for all the picture lines in the block row. Need only do this when we're
                 // subtracting.

                 for (int j=( m_cparams.XNumBlocks()*m_bparams.Xbsep() ); j<pic_data_out.LengthX() ; ++j )
                 {
                     out_row[pic_data_out.FirstX()+j] = 0;
                 }
             }
          }
          else // (m_add_or_sub == ADD)
          {
              int start_y = std::max(pic_data_out.FirstY() , pos.y) ;
              int end_y = std::min (pic_data_out.FirstY() + pos.y + m_bparams.Ybsep() , pic_data_out.FirstY() + pic_data_out.LengthY()); 
              //if (pos.y +  m_bparams.Yblen() >= pic_data_out.LengthY())
              if (yblock == (m_cparams.YNumBlocks() - 1))
              {
                   end_y += (m_bparams.Yblen()-m_bparams.Ybsep());
                   if (end_y > pic_data_out.LengthY())
                        end_y = pic_data_out.LengthY();
              }
               //std::cerr << "start_y=" << start_y;
               //std::cerr << "end_y=" << end_y << std::endl;
#if defined (HAVE_MMX)
             CompensateComponentAddAndShift_mmx (start_y, end_y,
                                                 pic_data, pic_data_out);
#else
             for ( int i = start_y, pic_y = 0; i < end_y; i++, pic_y++)
             {
                 CalcValueType *pic_row = pic_data[pic_y];
                 ValueType *out_row = pic_data_out[i];

                 for ( int j =pic_data_out.FirstX(); j <= pic_data_out.LastX(); j++)
                 {
                     out_row[j] += static_cast<ValueType>( (pic_row[j] + 1024) >> 11 ); 
                 }
             }
#endif
        }
        //Increment the block vertical position
        pos.y += m_bparams.Ybsep();

        // Copy the rows required to motion compensate the next row of block.
        // This is usually Yblen-Ybsep rows.
        memmove (pic_data[0], pic_data[save_from_row], (m_bparams.Yblen() - save_from_row)*pic_data.LengthX()*sizeof(CalcValueType));
        memset( pic_data[m_bparams.Yblen() - save_from_row], 0, save_from_row*pic_data.LengthX()*sizeof(CalcValueType) );
        save_from_row = m_bparams.Ybsep();

    }//yblock

    if ( m_add_or_sub == SUBTRACT)
    {
        // Finally, now we've done all the blocks, we must set all padded lines below 
        // the last row equal to 0, if we're subtracting
        for ( int y=m_cparams.YNumBlocks()*m_bparams.Ybsep() ; y<pic_data_out.LengthY() ; ++y )
        {
            ValueType *out_row = pic_data_out[y];
            for ( int x=0 ; x<pic_data_out.LengthX() ; ++x )
            {
                out_row[x] = 0;
            }

        }
    }
}

void MotionCompensator::DCBlock( TwoDArray<CalcValueType> &pic_data ,
                                            const ImageCoords& orig_pic_size ,
                                            const ValueType dc , 
                                            const ImageCoords& pos ,
                                            const TwoDArray<ValueType>& wt_array)
{

    //Coordinates in the image being written to.
    const ImageCoords start_pos( std::max(0 , pos.x) , std::max(0 , pos.y) );
    const ImageCoords end_pos( std::min(pos.x + m_bparams.Xblen() , orig_pic_size.x ) , 
                               std::min(pos.y + m_bparams.Yblen() , orig_pic_size.y ) );

    //The difference between the desired start point
    //pos and the actual start point start_pos.
    const ImageCoords diff(start_pos.x - pos.x , start_pos.y - pos.y);

    //Quick process where we can just copy from the double size image.

    //std::cerr << "start_y=" << start_pos.y;
    //std::cerr << "end_y=" << end_pos.y << std::endl;
    CalcValueType *pic_curr = &pic_data[0][start_pos.x]; 
    ValueType *wt_curr = &wt_array[diff.y][diff.x]; 
    const int block_width = end_pos.x - start_pos.x;
    //- go down a row and back up
    const int pic_next (pic_data.LengthX() - block_width );
    //- go down a row and back up
    const int wt_next( wt_array.LengthX() - block_width );

    for(int y = end_pos.y-start_pos.y; y > 0; --y, pic_curr+=pic_next, wt_curr+=wt_next)
    {
        for(int x=block_width; x > 0; --x, ++pic_curr, ++wt_curr)
        {
            *pic_curr += CalcValueType (dc) * *wt_curr;
        }
    }
}

//Overlapping blocks are acheived by applying a 2D raised cosine shape
//to them. This function facilitates the calculations
float MotionCompensator::RaisedCosine(float t, float B)
{
    if(std::abs(t)>(B+1.0)/2.0) 
        return 0.0f;
    else if(std::abs(t)<(1.0-B)/2.0) 
        return 1.0f;
    else 
        return( 0.5 * ( 1.0 + std::cos( 3.141592654 * ( std::abs(t)-(1.0-B)/2.0 )/B ) ) );
}

// Calculates a weighting block.
// bparams defines the block parameters so the relevant weighting arrays can be created.
// FullX and FullY refer to whether the weight should be adjusted for the edge of an image.
// eg. 1D Weighting shapes in x direction

//  FullX true        FullX false
//     ***           ********
//   *     *                  *
//  *       *                  *
//*           *                  *
void MotionCompensator::CreateBlock( const OLBParams &bparams , 
                                                bool FullX , 
                                                bool FullY , 
                                                TwoDArray<ValueType>& WeightArray)
{
    // Create temporary arrays
    OneDArray<CalcValueType> HWts( WeightArray.LengthX() );
    OneDArray<CalcValueType> VWts( WeightArray.LengthY() );

    // Calculation variables
    float rolloffX = (float(bparams.Xblen()+1)/float(bparams.Xbsep())) - 1;
    float rolloffY = (float(bparams.Yblen()+1)/float(bparams.Ybsep())) - 1;
    float val;

    // Window in the x direction
    for(int x = 0; x < bparams.Xblen(); ++x)
    {
        val = (float(x) - (float(bparams.Xblen()-1)/2.0))/float(bparams.Xbsep());
        HWts[x] = static_cast<CalcValueType>( 32.0 * RaisedCosine(val,rolloffX) );
        HWts[x] = std::max( HWts[x] , 1 );
        HWts[x] = std::min( HWts[x] , 32 );
    }// x

    // Window in the y direction
    for(int y = 0; y < bparams.Yblen(); ++y)
    {
        val = (float(y) - (float(bparams.Yblen()-1)/2.0))/float(bparams.Ybsep());
        VWts[y] = static_cast<CalcValueType>( 32.0 * RaisedCosine(val,rolloffY) );
        VWts[y] = std::max( VWts[y] , 1 );
        VWts[y] = std::min( VWts[y] , 32 );
    }// y

    // Rationalise to avoid rounding errors
    for(int x = HWts.Last(); x > HWts.Last()-bparams.Xoffset(); --x)
    {
        if (HWts[x] + HWts[HWts.Last()-(x-bparams.Xbsep())] > 32)
            HWts[HWts.Last()-(x-bparams.Xbsep())] = 32-HWts[x];
        
        else if (HWts[x] + HWts[HWts.Last()-(x-bparams.Xbsep())] < 32)
            HWts[x] = 32-HWts[HWts.Last()-(x-bparams.Xbsep())];
    }// x 

    // Now reflect or pad, as appropriate
    if (!FullX)
    {
        for( int x = 0; x < (bparams.Xblen()>>1) ; ++x)
            HWts[x] = 32;
    }
    else
    {
        for( int x = 0; x < (bparams.Xblen()>>1); ++x)
            HWts[x] = HWts[HWts.Last()-x];
    }

    // Rationalise to avoid rounding errors
    for(int y = VWts.Last(); y > VWts.Last()-bparams.Yoffset(); --y)
    {
        if (VWts[y] + VWts[VWts.Last()-(y-bparams.Ybsep())] > 32)
            VWts[VWts.Last()-(y-bparams.Ybsep())] = 32-VWts[y];
        else if (VWts[y] + VWts[VWts.Last()-(y-bparams.Ybsep())] < 32)
            VWts[y] = 32-VWts[VWts.Last()-(y-bparams.Ybsep())];
    }// x 

    // Reflect or pad, as appropriate
    if (!FullY)
    {
        for( int y = 0 ; y < (bparams.Yblen()>>1); ++y)
            VWts[y] = 32;
    }
    else
    {
        for( int y = 0 ; y < (bparams.Yblen()>>1); ++y)
            VWts[y] = VWts[VWts.Last()-y];
    }

    for(int y = 0; y < bparams.Yblen(); ++y)
    {
        for(int x = 0; x < bparams.Xblen(); ++x)
        {
            WeightArray[y][x] = VWts[y] * HWts[x];
        }// x
    }// y

}

// Flips the values in an array in the x direction.
void MotionCompensator::FlipX( const TwoDArray<ValueType>& Original , 
                                          const OLBParams &bparams , 
                                          TwoDArray<ValueType>& Flipped)
{
    for(int y = 0; y < bparams.Yblen(); ++y)
    {
        for(int x = 0; x < bparams.Xblen(); ++x)
        {
            Flipped[y][x] = Original[y][(bparams.Xblen()-1) - x];
        }// y
    }// x
}

// Flips the values in an array in the y direction.
void MotionCompensator::FlipY( const TwoDArray<ValueType>& Original , 
                                          const OLBParams &bparams , 
                                          TwoDArray<ValueType>& Flipped)
{
    for(int y = 0; y < bparams.Yblen(); ++y)
    {
        for(int x = 0; x < bparams.Xblen(); ++x)
        {
            Flipped[y][x] = Original[(bparams.Yblen()-1) - y][x];
        }// y
    }// x
}

// Concrete Sub-Classes
// Class that implement the CompensateBlock function based on pixel
// precision values

// Motion Compesation class that provides pixel precision compensation

MotionCompensator_Pixel::MotionCompensator_Pixel( const CodecParams &cp ) : 
    MotionCompensator( cp )
{}

void MotionCompensator_Pixel::CompensateBlock( 
                                    TwoDArray<CalcValueType> &pic_data , 
                                    const ImageCoords& orig_pic_size , 
                                    const PicArray &refup_data , 
                                    const MVector &mv , 
                                    const ImageCoords& pos , 
                                    const TwoDArray<ValueType>& wt_array )
{
    //Coordinates in the image being written to.
    const ImageCoords start_pos( std::max(pos.x,0) , std::max(pos.y,0) );
    const ImageCoords end_pos( std::min( pos.x + m_bparams.Xblen() , orig_pic_size.x ) , 
                               std::min( pos.y + m_bparams.Yblen() , orig_pic_size.y ) );

    //The difference between the desired start point
    //pos and the actual start point start_pos.
    const ImageCoords diff( start_pos.x - pos.x , start_pos.y - pos.y );

    //Where to start in the upconverted image - scaled since ref is upconverted
    const ImageCoords ref_start( (start_pos.x + mv.x)<<1 , (start_pos.y + mv.y)<<1 );

    //An additional stage to make sure the block to be copied does not fall outside
    //the reference image.
    const int refXlen = refup_data.LengthX();
    const int refYlen = refup_data.LengthY();
    bool do_bounds_checking = false;

    //Check if there are going to be any problems copying the block from
    //the upvconverted reference image.
    if( ref_start.x < 0 ) 
        do_bounds_checking = true;
    else if( ref_start.x + ((end_pos.x - start_pos.x - 1)<<1 ) >= refXlen )
        do_bounds_checking = true;
    if( ref_start.y < 0 ) 
        do_bounds_checking = true;
    else if( ref_start.y + ((end_pos.y - start_pos.y - 1)<<1 ) >= refYlen)
        do_bounds_checking = true;

    CalcValueType *pic_curr = &pic_data[0][start_pos.x];
    ValueType *wt_curr = &wt_array[diff.y][diff.x];

    const int block_width = end_pos.x - start_pos.x;

    const int pic_next( pic_data.LengthX() - block_width ); // - go down a row and back up
    const int wt_next( wt_array.LengthX() - block_width );  // - go down a row and back up

    if( !do_bounds_checking )
    {
        ValueType *refup_curr = &refup_data[ref_start.y][ref_start.x];
        const int refup_next( 2*(refXlen - block_width) );         // - go down a row and back up
        for( int y=end_pos.y-start_pos.y; y > 0; --y, pic_curr+=pic_next, wt_curr+=wt_next, refup_curr+=refup_next )
        {
            for( int x=block_width; x > 0; --x, ++pic_curr, ++wt_curr, refup_curr+=2 )
            {
                *pic_curr += CalcValueType( refup_curr[0] )* *wt_curr;
            }// x
        }// y
    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.

        for( int y=end_pos.y-start_pos.y, ry=ref_start.y, by=BChk(ry,refYlen); 
             y > 0; 
             --y, pic_curr+=pic_next, wt_curr+=wt_next , ry+=2 , by=BChk(ry,refYlen) )
        {
             for( int x=block_width , rx=ref_start.x , bx=BChk(rx,refXlen); 
                  x >0 ; --x, ++pic_curr, ++wt_curr, rx+=2 , bx=BChk(rx,refXlen) )
             {
                 *pic_curr += CalcValueType( refup_data[by][bx] )* *wt_curr;
             }// x
        }// y

    }
}


// Motion Compesation class that provides half-pixel precision compensation
MotionCompensator_HalfPixel::MotionCompensator_HalfPixel( const CodecParams &cp ) : 
    MotionCompensator( cp )
{}

void MotionCompensator_HalfPixel::CompensateBlock( TwoDArray<CalcValueType> &pic_data , 
                                          const ImageCoords& orig_pic_size , 
                                          const PicArray &refup_data , 
                                          const MVector &mv , 
                                          const ImageCoords& pos , 
                                          const TwoDArray<ValueType>& wt_array )
{
    //Coordinates in the image being written to.
    const ImageCoords start_pos( std::max(pos.x,0) , std::max(pos.y,0) );
    const ImageCoords end_pos( std::min( pos.x + m_bparams.Xblen() , orig_pic_size.x) , 
                               std::min( pos.y + m_bparams.Yblen() , orig_pic_size.y ) );

    //The difference between the desired start point
    //pos and the actual start point start_pos.
    const ImageCoords diff( start_pos.x - pos.x , start_pos.y - pos.y );

    //Where to start in the upconverted image
    const ImageCoords ref_start( ( start_pos.x<<1 ) + mv.x ,( start_pos.y<<1 ) + mv.y );

    //An additional stage to make sure the block to be copied does not fall outside
    //the reference image.
    const int refXlen = refup_data.LengthX();
    const int refYlen = refup_data.LengthY();

    bool do_bounds_checking = false;

    //Check if there are going to be any problems copying the block from
    //the upvconverted reference image.

    if( ref_start.x < 0 ) 
        do_bounds_checking = true;
    else if( ref_start.x + ((end_pos.x - start_pos.x -1 )<<1 ) >= refXlen )
        do_bounds_checking = true;
    if( ref_start.y < 0 ) 
        do_bounds_checking = true;
    else if( ref_start.y + ((end_pos.y - start_pos.y - 1 )<<1 ) >= refYlen)
        do_bounds_checking = true;

    CalcValueType *pic_curr = &pic_data[0][start_pos.x];
    ValueType *wt_curr = &wt_array[diff.y][diff.x];
 
    const int block_width = end_pos.x - start_pos.x;

    const int pic_next( pic_data.LengthX() - block_width );// go down a row and back up
    const int wt_next( wt_array.LengthX() - block_width ); // go down a row and back up

    if( !do_bounds_checking )
    {  
        ValueType *refup_curr = &refup_data[ref_start.y][ref_start.x];
        const int refup_next( (refXlen - block_width)*2 );// go down 2 rows and back up

        for( int y=end_pos.y-start_pos.y; y > 0; --y, pic_curr+=pic_next, wt_curr+=wt_next, refup_curr+=refup_next )
        {
            for( int x=block_width; x > 0; --x, ++pic_curr, ++wt_curr, refup_curr+=2 )
            {
                *pic_curr += CalcValueType( refup_curr[0] )* *wt_curr;
            }
        }
    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.
        for( int y=end_pos.y-start_pos.y, ry=ref_start.y, by=BChk(ry,refYlen); 
             y>0; 
             --y, pic_curr+=pic_next, wt_curr+=wt_next , ry+=2 , by=BChk(ry,refYlen))
        {
             for( int x=block_width , rx=ref_start.x , bx=BChk(rx,refXlen); 
                  x>0 ; 
                  --x, ++pic_curr, ++wt_curr, rx+=2 , bx=BChk(rx,refXlen))
             {
                 *pic_curr += CalcValueType( refup_data[by][bx] )* *wt_curr;
             }// x
        }// y
    }
}

// Motion Compesation class that provides quarter-pixel precision compensation
MotionCompensator_QuarterPixel::MotionCompensator_QuarterPixel( const CodecParams &cp ) : 
    MotionCompensator( cp )
{}

#if !defined (HAVE_MMX)
void MotionCompensator_QuarterPixel::CompensateBlock( TwoDArray<CalcValueType> &pic_data , 
                                       const ImageCoords& orig_pic_size , 
                                       const PicArray &refup_data , 
                                       const MVector &mv , 
                                       const ImageCoords& pos , 
                                       const TwoDArray<ValueType>& wt_array )
{
    //Coordinates in the image being written to.
    const ImageCoords start_pos( std::max(pos.x,0) , std::max(pos.y,0) );
    const ImageCoords end_pos( std::min( pos.x + m_bparams.Xblen() , orig_pic_size.x ) , 
                               std::min( pos.y + m_bparams.Yblen() , orig_pic_size.y ) );

    //The difference between the desired start point
    //pos and the actual start point start_pos.
    const ImageCoords diff( start_pos.x - pos.x , start_pos.y - pos.y );

    // Set up the start point in the reference image by rounding the motion vector
    // to 1/2 pel accuracy.NB: bit shift rounds negative values DOWN, as required
    const MVector roundvec( mv.x>>1 , mv.y>>1 );

    //Get the remainder after rounding. NB rmdr values always 0 or 1
    const MVector rmdr( mv.x & 1 , mv.y & 1 );

    //Where to start in the upconverted image
    const ImageCoords ref_start( ( start_pos.x<<1 ) + roundvec.x ,( start_pos.y<<1 ) + roundvec.y );

    //An additional stage to make sure the block to be copied does not fall outside
    //the reference image.
    const int refXlen = refup_data.LengthX();
    const int refYlen = refup_data.LengthY();
    CalcValueType *pic_curr = &pic_data[0][start_pos.x];
    ValueType *wt_curr = &wt_array[diff.y][diff.x];

    const int block_width = end_pos.x - start_pos.x;

    const int pic_next( pic_data.LengthX() - block_width ); //go down a row and back to beginning of block line
    const int wt_next( wt_array.LengthX() - block_width ); //go down a row and back to beginning of block line

    bool do_bounds_checking = false;

    //Check if there are going to be any problems copying the block from
    //the upvconverted reference image.
    if( ref_start.x < 0 ) 
        do_bounds_checking = true;
    else if( ref_start.x + ((end_pos.x - start_pos.x)<<1 ) >= refXlen )
        do_bounds_checking = true;
    if( ref_start.y < 0 ) 
        do_bounds_checking = true;
    else if( ref_start.y + ((end_pos.y - start_pos.y)<<1 ) >= refYlen)
        do_bounds_checking = true;
     
     //std::cerr << "start_y=" << start_pos.y;
     //std::cerr << "end_y=" << end_pos.y << std::endl;
    if( !do_bounds_checking )
    {
        ValueType *refup_curr = &refup_data[ref_start.y][ref_start.x];
        const int refup_next( ( refXlen - block_width )*2 ); //go down 2 rows and back to beginning of block line
        if( rmdr.x == 0 && rmdr.y == 0 )
        {
            for( int y=end_pos.y-start_pos.y; y > 0; --y, pic_curr+=pic_next, wt_curr+=wt_next, refup_curr+=refup_next )
            {
                for( int x=block_width; x > 0; --x, ++pic_curr, ++wt_curr, refup_curr+=2 )
                {
                    *pic_curr += CalcValueType( refup_curr[0] )* *wt_curr;
                }
            }
        }
        else if( rmdr.y == 0 )
        {
            for( int y=end_pos.y-start_pos.y; y > 0; --y, pic_curr+=pic_next, wt_curr+=wt_next, refup_curr+=refup_next )
            {
                for( int x=block_width; x > 0; --x, ++pic_curr, ++wt_curr, refup_curr+=2 )
                {
                    *pic_curr += ((    CalcValueType( refup_curr[0] ) +
                                       CalcValueType( refup_curr[1] ) +
                                       1
                                  ) >> 1) * *wt_curr;
                }
            }
        }
        else if( rmdr.x == 0 )
        {
            for( int y=end_pos.y-start_pos.y; y > 0; --y, pic_curr+=pic_next, wt_curr+=wt_next, refup_curr+=refup_next )
            {
                for( int x=block_width; x > 0; --x, ++pic_curr, ++wt_curr, refup_curr+=2 )
                {
                    *pic_curr += ((    CalcValueType( refup_curr[0] ) +
                                       CalcValueType( refup_curr[refXlen] ) +
                                       1
                                   ) >> 1) * *wt_curr;
                }
            }
        }
        else
        {
            for( int y=end_pos.y-start_pos.y; y > 0; --y, pic_curr+=pic_next, wt_curr+=wt_next, refup_curr+=refup_next )
            {
                for( int x=block_width; x > 0; --x, ++pic_curr, ++wt_curr, refup_curr+=2 )
                {
                    *pic_curr += ((    CalcValueType( refup_curr[0] ) +
                                       CalcValueType( refup_curr[1] ) +
                                       CalcValueType( refup_curr[refXlen+0] ) +
                                       CalcValueType( refup_curr[refXlen+1] ) +
                                       2
                                   ) >> 2) * *wt_curr;
                }
            }
        }
    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.

        //weights for doing linear interpolation, calculated from the remainder values
        const ValueType linear_wts[4] = {  (2 - rmdr.x) * (2 - rmdr.y),    //tl
                                           rmdr.x * (2 - rmdr.y),          //tr
                                           (2 - rmdr.x) * rmdr.y,          //bl
                                           rmdr.x * rmdr.y };              //br


       for(int c = 0, wY = diff.y, uY = ref_start.y,BuY=BChk(uY,refYlen),BuY1=BChk(uY+1,refYlen);
           c < end_pos.y - start_pos.y; ++c, ++wY, uY += 2,BuY=BChk(uY,refYlen),BuY1=BChk(uY+1,refYlen))
       {
           for(int l = start_pos.x, wX = diff.x, uX = ref_start.x,BuX=BChk(uX,refXlen),BuX1=BChk(uX+1,refXlen);
               l < end_pos.x; ++l, ++wX, uX += 2,BuX=BChk(uX,refXlen),BuX1=BChk(uX+1,refXlen))
           {

               pic_data[c][l] += ((     linear_wts[0] * CalcValueType( refup_data[BuY][BuX] ) +
                                        linear_wts[1] * CalcValueType( refup_data[BuY][BuX1] ) +
                                        linear_wts[2] * CalcValueType( refup_data[BuY1][BuX] )+
                                        linear_wts[3] * CalcValueType( refup_data[BuY1][BuX1] ) +
                                        2
                                  ) >> 2) * wt_array[wY][wX];
           }//l
       }//c

    }
}
#endif

// Motion Compesation class that provides one eighth-pixel precision 
// compensation
MotionCompensator_EighthPixel::MotionCompensator_EighthPixel( const CodecParams &cp ) : 
    MotionCompensator( cp )
{}

void MotionCompensator_EighthPixel::CompensateBlock( TwoDArray<CalcValueType> &pic_data , 
                                      const ImageCoords& orig_pic_size , 
                                      const PicArray &refup_data , 
                                      const MVector &mv , 
                                      const ImageCoords& pos , 
                                      const TwoDArray<ValueType>& wt_array )
{

    //Coordinates in the image being written to.
    const ImageCoords start_pos( std::max(pos.x,0) , std::max(pos.y,0) );
    const ImageCoords end_pos( std::min( pos.x + m_bparams.Xblen() , orig_pic_size.x ) , 
                               std::min( pos.y + m_bparams.Yblen() , orig_pic_size.y ) );

    //The difference between the desired start point
    //pos and the actual start point start_pos.
    const ImageCoords diff( start_pos.x - pos.x , start_pos.y - pos.y );

    //Set up the start point in the reference image by rounding the motion vector
    //NB: bit shift rounds negative values DOWN, as required
    const MVector roundvec( mv.x>>2 , mv.y>>2 );

    //Get the remainder after rounding. NB rmdr values always 0,1,2 or 3
    const MVector rmdr( mv.x & 3 , mv.y & 3 );

    //Where to start in the upconverted image
    const ImageCoords ref_start( ( start_pos.x<<1 ) + roundvec.x ,( start_pos.y<<1 ) + roundvec.y );

    //weights for doing linear interpolation, calculated from the remainder values
    const ValueType linear_wts[4] = {  (4 - rmdr.x) * (4 - rmdr.y),    //tl
                                    rmdr.x * (4 - rmdr.y),          //tr
                                    (4 - rmdr.x) * rmdr.y,          //bl
                                    rmdr.x * rmdr.y };              //br

    //An additional stage to make sure the block to be copied does not fall outside
    //the reference image.
    const int refXlen = refup_data.LengthX();
    const int refYlen = refup_data.LengthY();
    bool do_bounds_checking = false;

    //Check if there are going to be any problems copying the block from
    //the upvconverted reference image.
    if( ref_start.x < 0 ) 
        do_bounds_checking = true;
    else if( ref_start.x + ((end_pos.x - start_pos.x)<<1 ) >= refXlen )
        do_bounds_checking = true;
    if( ref_start.y < 0 ) 
        do_bounds_checking = true;
    else if( ref_start.y + ((end_pos.y - start_pos.y)<<1 ) >= refYlen)
        do_bounds_checking = true;


    if( !do_bounds_checking )
    {
        CalcValueType *pic_curr = &pic_data[0][start_pos.x];
        ValueType *refup_curr = &refup_data[ref_start.y][ref_start.x];
        ValueType *wt_curr = &wt_array[diff.y][diff.x];
 
        const int block_width = end_pos.x - start_pos.x;

        const int pic_next = pic_data.LengthX() - block_width;                //go down a row and back up
        const int refup_next = (refup_data.LengthX() - block_width )*2;        //go down 2 rows and back up
        const int wt_next = wt_array.LengthX() - block_width;                //go down a row and back up

        if( rmdr.x == 0 && rmdr.y == 0 )
        {
            for( int y=end_pos.y-start_pos.y; y > 0; --y, pic_curr+=pic_next, wt_curr+=wt_next, refup_curr+=refup_next )
            {
                for( int x=block_width; x > 0; --x, ++pic_curr, ++wt_curr, refup_curr+=2 )
                {
                    *pic_curr += CalcValueType( refup_curr[0] )* *wt_curr;
                }
            }
        }
        else if( rmdr.y == 0 )
        {
            for( int y=end_pos.y-start_pos.y; y > 0; --y, pic_curr+=pic_next, wt_curr+=wt_next, refup_curr+=refup_next )
            {
                for( int x=block_width; x > 0; --x, ++pic_curr, ++wt_curr, refup_curr+=2 )
                {
                    *pic_curr += ((    linear_wts[0] * CalcValueType( refup_curr[0] ) +
                                       linear_wts[1] * CalcValueType( refup_curr[1] ) +
                                       8
                                   ) >> 4) * *wt_curr;
                }
            }
        }
        else if( rmdr.x == 0 )
        {
            for( int y=end_pos.y-start_pos.y; y > 0; --y, pic_curr+=pic_next, wt_curr+=wt_next, refup_curr+=refup_next )
            {
                for( int x=block_width; x > 0; --x, ++pic_curr, ++wt_curr, refup_curr+=2 )
                {
                    *pic_curr += ((    linear_wts[0] * CalcValueType( refup_curr[0] ) +
                                       linear_wts[2] * CalcValueType( refup_curr[refXlen+0] ) +
                                       8
                                   ) >> 4) * *wt_curr;
                }
            }
        }
        else
        {
            for( int y=end_pos.y-start_pos.y; y > 0; --y, pic_curr+=pic_next, wt_curr+=wt_next, refup_curr+=refup_next )
            {
                for( int x=block_width; x > 0; --x, ++pic_curr, ++wt_curr, refup_curr+=2 )
                {
                    *pic_curr += ((    linear_wts[0] * CalcValueType( refup_curr[0] ) +
                                       linear_wts[1] * CalcValueType( refup_curr[1] ) +
                                       linear_wts[2] * CalcValueType( refup_curr[refXlen+0] ) +
                                       linear_wts[3] * CalcValueType( refup_curr[refXlen+1] ) +
                                       8
                                   ) >> 4) * *wt_curr;
                }
            }
        }
    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.

        for(int c = 0, wY = diff.y, uY = ref_start.y,BuY=BChk(uY,refYlen),BuY1=BChk(uY+1,refYlen);
            c < end_pos.y - start_pos.y; ++c, ++wY, uY += 2,BuY=BChk(uY,refYlen),BuY1=BChk(uY+1,refYlen))
        {
            for(int l = start_pos.x, wX = diff.x, uX = ref_start.x,BuX=BChk(uX,refXlen),BuX1=BChk(uX+1,refXlen);
                l < end_pos.x; ++l, ++wX, uX += 2,BuX=BChk(uX,refXlen),BuX1=BChk(uX+1,refXlen))
            {

                pic_data[c][l] += (( linear_wts[0] * CalcValueType( refup_data[BuY][BuX] ) +
                                     linear_wts[1] * CalcValueType( refup_data[BuY][BuX1] ) +
                                     linear_wts[2] * CalcValueType( refup_data[BuY1][BuX] )+
                                     linear_wts[3] * CalcValueType( refup_data[BuY1][BuX1] ) +
                                     8
                                   ) >> 4) * wt_array[wY][wX];
            }//l
        }//c

    }

}
