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

#include <util/instrumentation/libdirac_instrument/draw_overlay.h>

// constructor
DrawOverlay::DrawOverlay(Frame & frame, DrawFrameMotionParams & draw_params)
:
    m_frame(frame),
    m_draw_params(draw_params)
{}

// destructor
DrawOverlay::~DrawOverlay()
{}

// calculates U and V from a value ranging 0 ~ 1000 (normalising is carried out in calling function)
void DrawOverlay::GetPowerUV(int power, int & U, int & V)
{
    // first convert power into RGB values
    // ranging 0 ~ 500
    float R = 0, G = 0, B = 0;

    // check which region value is located
    if (power < 200)
    {
        B = 0;
        G = 1000;
        R = 5 * power;
    }
    else if (power >= 200 && power < 400)
    {
        R = 1000;
        B = 0;
        G = 1000 - (5 * (power - 200));
    }
    else if (power >= 400 && power < 600)
    {
        R = 1000;
        G = 0;
        B = 5 * (power - 400);
    }
    else if (power >= 600 && power < 800)
    {
        B = 1000;
        G = 0;
        R = 1000 - (5 * (power - 600));
    }
    else if (power >= 800 && power < 1000)
    {
        B = 1000;
        R = 0;
        G = 5 * (power - 800);
    }
    else if (power >= 1000)
    {
        B = 1000;
        R = 0;
        G = 1000;
    }

    R *= 0.25;
    G *= 0.25;
    B *= 0.25;

    // now convert RGB to UV
    float Y=(0.3*R)+(0.59*G)+(0.11*B);
    U=int(B-Y);
    V=int(R-Y);
}

// draws power bar legend
void DrawOverlay::DrawPowerBar(int min, int max)
{
    // loop over rows
    for (int ypx=40; ypx<=m_frame.Ydata().LastY(); ++ypx)
    {
        // black line
        m_frame.Ydata()[ypx][5]=0;

        for (int xpx=0; xpx<5; ++xpx)
            m_frame.Ydata()[ypx][xpx]=512; // grey background
    }

    // draw colour on line by line basis
    for (int ypx=40/m_draw_params.ChromaFactorY(); ypx<=m_frame.Udata().LastY(); ++ypx)
    {
        // find equivalent power value
        double power = (1000 * ((m_frame.Udata().LastY()+1) - (40/m_draw_params.ChromaFactorY()) - ypx) /
                        (m_frame.Udata().LastY()-(40/m_draw_params.ChromaFactorY())+1));

        // get U V values for power
        int U=0, V=0;
        GetPowerUV((int)power, U, V);

        for (int xpx=0; xpx<=4/m_draw_params.ChromaFactorX(); ++xpx)
        {
            m_frame.Udata()[ypx][xpx]=U+512;
            m_frame.Vdata()[ypx][xpx]=V+512;
        }
    }

    // draw min and max labels
    DrawValue(min, m_frame.Ydata().LastY()-15, 0);
    DrawValue(max, 40, 8);
    DrawCharacter(m_symbols.SymbolGreater(), 40, 0);
}

// draws a 8x16 character in luma
void DrawOverlay::DrawCharacter(const PicArray & ch, int y_offset, int x_offset)
{
    // loop over samples in 8x16 block
    for (int y=y_offset, y_ch=0; y<y_offset+16; ++y, ++y_ch)
    {
        for (int x=x_offset, x_ch=0; x<x_offset+8; ++x, ++x_ch)
        {
            m_frame.Ydata()[y][x]=ch[y_ch][x_ch]*1024;
        }// x
    }// y

    // remove chroma from digit
    for (int ypx=y_offset/m_draw_params.ChromaFactorY(); ypx<(y_offset+16)/m_draw_params.ChromaFactorY(); ++ypx)
    {
        for (int xpx=x_offset/m_draw_params.ChromaFactorX(); xpx<(x_offset+8)/m_draw_params.ChromaFactorX(); ++xpx)
        {
            m_frame.Udata()[ypx][xpx]=512;
            m_frame.Vdata()[ypx][xpx]=512;
        }// xpx
    }// ypx
}

// draws value in luma
void DrawOverlay::DrawValue(int number, int y_offset, int x_offset)
{
    int digits;
    // number of digits in frame number
    if (number < 10)
        digits = 1;
    else if (number >= 10 && number < 100)
        digits=2;
    else if (number >= 100 && number < 1000)
        digits=3;
    else if (number >= 1000 && number < 10000)
        digits=4;
    else if (number >= 10000 && number < 100000)
        digits=5;

    // loop over digits
    for (int digit=digits; digit>0; --digit)
    {
        int value;

        // get digit, largest first
        if (digit == 5)
            value = (int)number/10000;
        else if (digit == 4)
            value = (int)number/1000;
        else if (digit == 3)
            value = (int)number/100;
        else if (digit == 2)
            value = (int)number/10;
        else if (digit == 1)
            value = number;

        // set arrow to correct number PicArray
        if (value == 0)
            DrawCharacter(m_symbols.Number0(), y_offset, x_offset);
        else if (value == 1)
            DrawCharacter(m_symbols.Number1(), y_offset, x_offset);
        else if (value == 2)
            DrawCharacter(m_symbols.Number2(), y_offset, x_offset);
        else if (value == 3)
            DrawCharacter(m_symbols.Number3(), y_offset, x_offset);
        else if (value == 4)
            DrawCharacter(m_symbols.Number4(), y_offset, x_offset);
        else if (value == 5)
            DrawCharacter(m_symbols.Number5(), y_offset, x_offset);
        else if (value == 6)
            DrawCharacter(m_symbols.Number6(), y_offset, x_offset);
        else if (value == 7)
            DrawCharacter(m_symbols.Number7(), y_offset, x_offset);
        else if (value == 8)
            DrawCharacter(m_symbols.Number8(), y_offset, x_offset);
        else if (value == 9)
            DrawCharacter(m_symbols.Number9(), y_offset, x_offset);

        // remove most significant digit
        if (digit == 5)
            number -= value * 10000;
        else if (digit == 4)
            number -= value * 1000;
        else if (digit == 3)
            number -= value * 100;
        else if (digit == 2)
            number -= value * 10;

        x_offset+=8;
    }
}

// draws both reference frame numbers
void DrawOverlay::DrawReferenceNumbers(int ref1, int ref2)
{
    // draw letters: 'R1:' and 'R2:' on consecutive lines
    DrawCharacter(m_symbols.LetterR(), 16, 0);
    DrawCharacter(m_symbols.LetterR(), 32, 0);
    DrawCharacter(m_symbols.Number1(), 16, 8);
    DrawCharacter(m_symbols.Number2(), 32, 8);
    DrawCharacter(m_symbols.SymbolColon(), 16, 16);
    DrawCharacter(m_symbols.SymbolColon(), 32, 16);

    if (ref1==-1)
        DrawCharacter(m_symbols.SymbolMinus(), 16, 24);
    else
        DrawValue(ref1, 16, 24);
    if (ref2==-1)
        DrawCharacter(m_symbols.SymbolMinus(), 32, 24);
    else
        DrawValue(ref2, 32, 24);
}

// draws frame number
void DrawOverlay::DrawFrameNumber(int fnum)
{
    DrawCharacter(m_symbols.LetterF(), 0, 0);
    DrawValue(fnum, 0, 8);
}

// draws used reference frame number
void DrawOverlay::DrawReferenceNumber(int ref, int ref_frame)
{
    DrawCharacter(m_symbols.LetterR(), 16, 0);
    DrawCharacter(m_symbols.SymbolColon(), 16, 16);
    
    if (ref==1)
        DrawCharacter(m_symbols.Number1(), 16, 8);
    else if (ref==2)
        DrawCharacter(m_symbols.Number2(), 16, 8);
    
    if (ref_frame==-1)
        DrawCharacter(m_symbols.SymbolMinus(), 16, 24);
    else
        DrawValue(ref_frame, 16, 24);
}

// colours a single block, referenced by motion vector
void DrawOverlay::DrawMvBlockUV(int ymv, int xmv, int U, int V)
{
    // loop over chroma samples in block
    for (int y=0; y<m_draw_params.MvUVBlockY(); ++y)
    {
        for (int x=0; x<m_draw_params.MvUVBlockX(); ++x)
        {
            m_frame.Udata()[(ymv*m_draw_params.MvUVBlockY())+y][(xmv*m_draw_params.MvUVBlockX())+x]=U;
            m_frame.Vdata()[(ymv*m_draw_params.MvUVBlockY())+y][(xmv*m_draw_params.MvUVBlockX())+x]=V;
        }// xpx
    }// ypx
}

// colours a single block, referenced by TL chroma pixel
void DrawOverlay::DrawBlockUV(int ypx, int xpx, int U, int V)
{
    // loop over chroma samples in block
    for (int y=ypx; y<ypx+m_draw_params.MvUVBlockY(); ++y)
    {
        for (int x=xpx; x<xpx+m_draw_params.MvUVBlockX(); ++x)
        {
            m_frame.Udata()[y][x]=U;
            m_frame.Vdata()[y][x]=V;
        }// xpx
    }// ypx
}