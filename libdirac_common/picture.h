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
* Contributor(s): Thomas Davies (Original Author)
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

#ifndef _PICTURE_H_
#define _PICTURE_H_

#include <libdirac_common/common.h>

namespace dirac
{
    //! A class for encapsulating all the data relating to a picture.
    /*!
        A class for encapsulating all the data relating to a picture - all the 
        component data, including upconverted data.
     */
    class Picture
    {

    public:

        //! Constructor
        /*!
            Constructor initialises the picture parameters and the data
         */    
        Picture( const PictureParams& fp );

        //! Copy constructor. Private as not currently used [may want to implement reference counting later.]
        Picture(const Picture& cpy);

        //! Destructor
        virtual ~Picture();

        //! Assignment =. Private as not currently used [may want to implement reference counting later.]
        Picture& operator=( const Picture& rhs );

        //! Picture Copy
        /*!
            Copy contents of picture into the output picture passed to it 
            retaining the picture dimensions of the output picture.
        */
        void CopyContents(Picture& out ) const;

        //! Picture Fill
        /*!
            Initialise contents of picture with value provided
        */
        void Fill(ValueType val );

        //gets and sets
        //! Gets the picture parameters
        PictureParams& GetPparams() const  {return m_fparams;}

        //! Sets the picture sort
        void SetPictureSort( const PictureSort fs ){m_fparams.SetPicSort( fs ); }

        //! Sets the picture type
        void SetPictureType( const PictureType ftype ){m_fparams.SetPictureType( ftype ); }

        //! Sets the picture type
        void SetReferenceType( const ReferenceType rtype ){m_fparams.SetReferenceType( rtype ); }

        //! Reconfigures the the framend to the new parameters. 
        void ReconfigFrame( const PictureParams &fp );

        //! Returns the luma data array
        PicArray& Ydata() {return *m_Y_data;}

        //! Returns the U component
        PicArray& Udata() {return *m_U_data;}

        //! Returns the V component 
        PicArray& Vdata() {return *m_V_data;}

        //! Returns the luma data array
        const PicArray& Ydata() const {return *m_Y_data;}

        //! Returns the U component
        const PicArray& Udata() const {return *m_U_data;}

        //! Returns the V component 
        const PicArray& Vdata() const {return *m_V_data;}

        //! Returns a given component 
        PicArray& Data(CompSort cs);

        //! Returns a given component
        const PicArray& Data(CompSort cs) const;    

        //! Returns upconverted Y data
        PicArray& UpYdata();

        //! Returns upconverted U data
        PicArray& UpUdata();

        //! Returns upconverted V data
        PicArray& UpVdata();

        //! Returns a given upconverted component
        PicArray& UpData(CompSort cs);

        //! Returns upconverted Y data
        const PicArray& UpYdata() const;

        //! Returns upconverted U data
        const PicArray& UpUdata() const;

        //! Returns upconverted V data    
        const PicArray& UpVdata() const;

        //! Returns a given upconverted component
        const PicArray& UpData(CompSort cs) const;

        //! Clip the data to prevent overshoot
        /*!
            Clips the data to lie between 0 and (1<<video_depth)-1 
         */
        void Clip();

        //! Clip the upconverted data to prevent overshoot
        /*!
            Clips the upconverted data to lie between 0 and (1<<video_depth)-1 
         */
        void ClipUpData();

    private:
        mutable PictureParams m_fparams;
        PicArray* m_Y_data;//the 
        PicArray* m_U_data;//component
        PicArray* m_V_data;//data
        mutable PicArray* m_upY_data;//upconverted data. Mutable because we
        mutable PicArray* m_upU_data;//create them on the fly even in const
        mutable PicArray* m_upV_data;//functions.

        //! Initialises the picture once the picture parameters have been set
        void Init();

        //! Delete all the data
        void ClearData();

        //! Clip an individual component
        void ClipComponent(PicArray& pic_data, CompSort cs) const;

        //! Flag that upconversion needs to be re-done
        mutable bool m_redo_upYdata;
        mutable bool m_redo_upUdata;
        mutable bool m_redo_upVdata;
    };

} // namespace dirac


#endif