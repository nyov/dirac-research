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
* Contributor(s): Chris Bowley (Original Author)
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

#include <util/instrumentation/libdirac_instrument/overlay.h>

// constructor
Overlay::Overlay (const OverlayParams & overlayparams, Frame & frame)
:
    m_oparams(overlayparams),
    m_frame(frame)
{
    CalculateFactors(frame.GetFparams().CFormat());
}

// destructor - doesn't do anything!
Overlay::~Overlay ()
{}

// process intra frame
void Overlay::ProcessFrame()
{
    // if a mid-grey background is selected instead of the original luma, do that
    if (m_oparams.Background()==0)
    {
        for (int j=0; j<=m_frame.Ydata().LastY(); ++j)
        {
            for (int i=0; i<=m_frame.Ydata().LastX(); ++i)
                m_frame.Ydata()[j][i]=512;
        }
    }

    // set chroma arrays to zero
    for (int j=0; j<m_frame.Udata().LengthY(); ++j)
    {
        for (int i=0; i<m_frame.Udata().LengthX(); ++i)
        {
            m_frame.Udata()[j][i]=512;
            m_frame.Vdata()[j][i]=512;
        }
    }

    // in order to draw frame number and 'I' label, create a dummy DrawPredMode object
    // and call appropriate functions. Not the most elegant!
    MEData me_data(1,1,1,1);
    DrawPredMode dummy(m_frame, m_draw_params, me_data.Mode());
    dummy.DrawFrameNumber(m_frame.GetFparams().FrameNum());
    dummy.DrawCharacter(dummy.Symbols().LetterI(), 16, 0);
}

// process motion-compensated frame
void Overlay::ProcessFrame(const MEData & me_data, const OLBParams & block_params)
{
    m_draw_params.SetMvYBlockY(block_params.Ybsep());
    m_draw_params.SetMvYBlockX(block_params.Xbsep());
    m_draw_params.SetMvUVBlockY(block_params.Ybsep()/m_draw_params.ChromaFactorY());
    m_draw_params.SetMvUVBlockX(block_params.Xbsep()/m_draw_params.ChromaFactorX());
    m_draw_params.SetPicY(m_frame.Ydata().LengthY());
    m_draw_params.SetPicX(m_frame.Ydata().LengthX());

    //std::cerr<<std::endl<<"Pic: "<<m_draw_params.PicY()<<" "<<m_draw_params.PicX();

    PadFrame(me_data);

    // if a mid-grey background is selected instead of the original luma, do that
    if (m_oparams.Background()==0)
    {
        for (int j=0; j<=m_frame.Ydata().LastY(); ++j)
        {
            for (int i=0; i<=m_frame.Ydata().LastX(); ++i)
                m_frame.Ydata()[j][i]=512;
        }
    }

    // set up references
    if (m_oparams.Reference() == 2 && (m_frame.GetFparams().Refs().size() < 2 || m_frame.GetFparams().Refs()[0] == m_frame.GetFparams().Refs()[1]))
    {
        m_ref = -1;
        m_mv_scale = 1;
    }
    else
    {
        m_ref = m_frame.GetFparams().Refs()[m_oparams.Reference()-1];
        m_mv_scale = std::abs(m_frame.GetFparams().FrameNum()-m_frame.GetFparams().Refs()[m_oparams.Reference()-1]); // scale motion vectors for temporal difference
    }

    // now do the overlaying!
    DoOverlay(me_data);
}

// manages the overlaying process dependent on the command-line options
void Overlay::DoOverlay(const MEData & me_data)
{
    // Overlay Class Structure
    // =======================
    //                                      +-------------+
    //                                      | DrawOverlay |
    //                                      +-------------+
    //                                         | | | | |
    //                                         | | | | +------------------------------+
    //           +-----------------------------+ | | +---------------+                |
    //           |                   +-----------+ +--+              |                |
    //           |                   |                |              |                |
    // +------------------+ +------------------+ +---------+ +---------------+ +--------------+
    // | DrawMotionColour | | DrawMotionArrows | | DrawSad | | DrawSplitMode | | DrawPredMode |
    // +------------------+ +------------------+ +---------+ +---------------+ +--------------+
    //                               |
    //                               |
    //                  +------------------------+
    //                  | DrawMotionColourArrows |
    //                  +------------------------+
    //
    // In order to create a new overlay, sub-class DrawOverlay and override DrawBlock() and DrawLegend() functions

    // create poitner to DrawOverlay object
    DrawOverlay * draw_overlay_ptr;

    // choose appropriate object dependent on command line option
    switch (m_oparams.Option())
    {
        case motion_arrows :
            draw_overlay_ptr = new DrawMotionArrows(m_frame, m_draw_params,
                                                    me_data.Vectors(m_oparams.Reference()), m_mv_scale);
            break;

        case motion_colour_arrows :
            draw_overlay_ptr = new DrawMotionColourArrows(m_frame, m_draw_params,
                                                          me_data.Vectors(m_oparams.Reference()), m_mv_scale,
                                                          m_oparams.MvClip());
            break;

        case motion_colour :
            draw_overlay_ptr = new DrawMotionColour(m_frame, m_draw_params,
                                                    me_data.Vectors(m_oparams.Reference()),
                                                    m_mv_scale, m_oparams.MvClip());
            break;

        case SAD :
            draw_overlay_ptr = new DrawSad(m_frame, m_draw_params, me_data.PredCosts(m_oparams.Reference()),
                                           me_data.Mode(), m_oparams.SADClip());
            break;

        case split_mode :
            draw_overlay_ptr = new DrawSplitMode(m_frame, m_draw_params, me_data.MBSplit());
            break;

        case pred_mode :
            draw_overlay_ptr = new DrawPredMode(m_frame, m_draw_params, me_data.Mode());
            break;
    }
    
    // if we are trying to overlay information which does not exist because frame only
    // has a single reference, remove chroma and display frame number and legend
    if (m_ref==-1 && m_oparams.Option() != pred_mode && m_oparams.Option() != split_mode)
    {
        for (int y=0; y<m_frame.Udata().LengthY(); ++y)
        {
            for (int x=0; x<m_frame.Udata().LengthX(); ++x)
            {
                m_frame.Udata()[y][x] = 512;
                m_frame.Vdata()[y][x] = 512;
            }
        }
        
        if (m_oparams.Legend())
            draw_overlay_ptr->DrawLegend();
            
        draw_overlay_ptr->DrawFrameNumber(m_frame.GetFparams().FrameNum());        
        draw_overlay_ptr->DrawReferenceNumber(m_oparams.Reference(), m_ref);
    }
    // otherwise, loop over motion vector blocks and draw as appropriate to overlay
    else
    {
        // carry out overlay on block by block basis
        for (int j=0; j<me_data.Vectors(1).LengthY(); ++j)
        {
            for (int i=0; i<me_data.Vectors(1).LengthX(); ++i)
            {
                draw_overlay_ptr->DrawBlock(j, i); 
            }
            
        }

        if (m_oparams.Legend())
            draw_overlay_ptr->DrawLegend();
            
        draw_overlay_ptr->DrawFrameNumber(m_frame.GetFparams().FrameNum());

        if (m_oparams.Option() == pred_mode || m_oparams.Option() == split_mode)
            draw_overlay_ptr->DrawReferenceNumbers(m_frame.GetFparams().Refs()[0], m_frame.GetFparams().Refs()[1]);
        else
            draw_overlay_ptr->DrawReferenceNumber(m_oparams.Reference(), m_ref);
    }
}

// calculates the resolution factor between chroma and luma samples
void Overlay::CalculateFactors(const ChromaFormat & cformat)
{
    if (cformat == Yonly)
    {
        m_draw_params.SetChromaFactorY(1);
        m_draw_params.SetChromaFactorX(1);
    }
    else if (cformat == format422)
    {
        m_draw_params.SetChromaFactorY(1);
        m_draw_params.SetChromaFactorX(2);
    }
    else if (cformat == format420)
    {
        m_draw_params.SetChromaFactorY(2);
        m_draw_params.SetChromaFactorX(2);
    }
    else if (cformat == format444)
    {
        m_draw_params.SetChromaFactorY(1);
        m_draw_params.SetChromaFactorX(1);
    }
    else if (cformat == format411)
    {
        m_draw_params.SetChromaFactorY(4);
        m_draw_params.SetChromaFactorX(4);
    }
    else
    {
        m_draw_params.SetChromaFactorY(1);
        m_draw_params.SetChromaFactorX(1);
    }
}

// calculate if frame requires padding due to requirement of integer number of macroblocks
void Overlay::PadFrame(const MEData & me_data)
{
    int frame_x = m_frame.Ydata().LengthX();
    int frame_y = m_frame.Ydata().LengthY();

    // copy frame components
    PicArray Ydata(m_frame.Ydata());
    PicArray Udata(m_frame.Udata());
    PicArray Vdata(m_frame.Vdata());

    // if there is not an integer number of macroblocks horizontally, pad until there is
    if (m_frame.Ydata().LengthX() % me_data.MBSplit().LengthX() != 0)
    {
        do
        {
            ++frame_x;
        }
        while (frame_x % me_data.MBSplit().LengthX() != 0);       
    }

    // if there is not an integer number of macroblocks vertically, pad until there is
    if (m_frame.Ydata().LengthX() % me_data.MBSplit().LengthY() != 0)
    {
        do
        {
            ++frame_y;
        }
        while (frame_y % me_data.MBSplit().LengthY() != 0);
    }

    // if padding was required in either horizontal or vertical, adjust frame size and reload component data
    if (m_frame.Ydata().LengthX() % me_data.MBSplit().LengthX() != 0 || m_frame.Ydata().LengthY() % me_data.MBSplit().LengthY() != 0)
    {
        m_frame.Ydata().Resize(frame_y, frame_x);
        m_frame.Udata().Resize(frame_y / m_draw_params.ChromaFactorY(), frame_x / m_draw_params.ChromaFactorX());
        m_frame.Vdata().Resize(frame_y / m_draw_params.ChromaFactorY(), frame_x / m_draw_params.ChromaFactorX());
       
        for (int j=0; j<Ydata.LengthY(); ++j)
        {
            for (int i=0; i<Ydata.LengthX(); ++i)
            {
                m_frame.Ydata()[j][i]=Ydata[j][i];
            }
        }
        
        for (int j=0; j<Udata.LengthY(); ++j)
        {
            for (int i=0; i<Udata.LengthX(); ++i)
            {
                m_frame.Udata()[j][i]=Udata[j][i];
                m_frame.Vdata()[j][i]=Vdata[j][i];
            }
        }

    }

}
