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

#include <util/instrumentation/libdirac_instrument/pred_mode.h>

// constructor
DrawPredMode::DrawPredMode(Frame & frame, DrawFrameMotionParams & draw_params, const TwoDArray<PredMode> & mode)
:
    DrawOverlay(frame, draw_params),
    m_mode(mode)
{}

// destructor
DrawPredMode::~DrawPredMode()
{}

// colours a motion vector block according to prediction frame
void DrawPredMode::DrawBlock(int j, int i)
{
    int power = 0, U = 0, V = 0;

    // get prediction mode
    if (m_mode[j][i] == INTRA)
        power=400; // red
    else if (m_mode[j][i] == REF1_ONLY)
        power=1000; // blue
    else if (m_mode[j][i] == REF2_ONLY)
        power=200; // yellow
    else if (m_mode[j][i] == REF1AND2)
        power=0; // green

    GetPowerUV(power, U, V);
    DrawMvBlockUV(j, i, U+500, V+500);
}

// displays colours representing prediction references
void DrawPredMode::DrawLegend()
{
    // blank background
    for (int ypx=m_frame.Ydata().LastY()-64; ypx<=m_frame.Ydata().LastY(); ++ypx)
    {
        for (int xpx=7; xpx>=0; --xpx)
            m_frame.Ydata()[ypx][xpx]=500;
    }

    int U=0, V=0;
    
    GetPowerUV(400, U, V); // intra
    DrawBlockUV(m_frame.Udata().LastY()-(64/m_draw_params.ChromaFactorY())+1, 0, U+500, V+500);
    DrawBlockUV(m_frame.Udata().LastY()-(56/m_draw_params.ChromaFactorY())+1, 0, U+500, V+500);

    GetPowerUV(1000, U, V); // ref 1
    DrawBlockUV(m_frame.Udata().LastY()-(48/m_draw_params.ChromaFactorY())+1, 0, U+500, V+500);
    DrawBlockUV(m_frame.Udata().LastY()-(40/m_draw_params.ChromaFactorY())+1, 0, U+500, V+500);    

    GetPowerUV(200, U, V); // ref 2
    DrawBlockUV(m_frame.Udata().LastY()-(32/m_draw_params.ChromaFactorY())+1, 0, U+500, V+500);
    DrawBlockUV(m_frame.Udata().LastY()-(24/m_draw_params.ChromaFactorY())+1, 0, U+500, V+500);

    GetPowerUV(0, U, V); // ref 1 and 2
    DrawBlockUV(m_frame.Udata().LastY()-(16/m_draw_params.ChromaFactorY())+1, 0, U+500, V+500);
    DrawBlockUV(m_frame.Udata().LastY()-(8/m_draw_params.ChromaFactorY())+1, 0, U+500, V+500);

    // black horizontal lines
    for (int xpx=15; xpx>=0; --xpx)
    {
        m_frame.Ydata()[m_frame.Ydata().LastY()-64][xpx]=0;
        m_frame.Ydata()[m_frame.Ydata().LastY()-48][xpx]=0;
        m_frame.Ydata()[m_frame.Ydata().LastY()-32][xpx]=0;
    }

    for (int xpx=31; xpx>=0; --xpx)
    {
        m_frame.Ydata()[m_frame.Ydata().LastY()-16][xpx]=0;
    }

    // draw labels
    DrawCharacter(m_symbols.LetterI(), m_frame.Ydata().LastY()-63, 8);
    DrawCharacter(m_symbols.Number1(), m_frame.Ydata().LastY()-47, 8);
    DrawCharacter(m_symbols.Number2(), m_frame.Ydata().LastY()-31, 8);
    DrawCharacter(m_symbols.Number1(), m_frame.Ydata().LastY()-15, 8);
    DrawCharacter(m_symbols.SymbolPlus(), m_frame.Ydata().LastY()-15, 16);
    DrawCharacter(m_symbols.Number2(), m_frame.Ydata().LastY()-15, 24);

    // blank background
    for (int ypx=m_frame.Udata().LastY()-(16/m_draw_params.ChromaFactorY()); ypx<=m_frame.Udata().LastY(); ++ypx)
    {
        // no chrominance
        for (int xpx=(32/m_draw_params.MvYBlockX())-1; xpx>=(16/m_draw_params.ChromaFactorX()); --xpx)
        {
            m_frame.Udata()[ypx][xpx]=500;
            m_frame.Vdata()[ypx][xpx]=500;
        }
    }
}