 /* ***** BEGIN LICENSE BLOCK *****
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
 * Contributor(s):
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

/*
*
* $Author$
* $Revision$
* $Log$
* Revision 1.9  2004-05-24 15:59:30  tjdwave
* Changed CLI names and functions to house style.
*
* Revision 1.8  2004/05/20 12:37:33  tjdwave
* Corrected help text for CLI.
*
* Revision 1.7  2004/05/19 17:39:34  chaoticcoyote
* Restored make_debug.sh to it's proper function
* Modified command line parser to correctly handle boolean options
*
* Revision 1.6  2004/05/18 07:46:13  tjdwave
* Added support for I-frame only coding by setting num_L1 equal 0; num_L1 negative gives a single initial I-frame ('infinitely' many L1 frames). Revised quantiser selection to cope with rounding error noise.
*
* Revision 1.5  2004/05/12 16:03:32  tjdwave
*
* Done general code tidy, implementing copy constructors, assignment= and const
*  correctness for most classes. Replaced Gop class by FrameBuffer class throughout. Added support for frame padding so that arbitrary block sizes and frame
*  dimensions can be supported.
*
* Revision 1.4  2004/05/11 14:17:58  tjdwave
* Removed dependency on XParam CLI library for both encoder and decoder.
*
* Revision 1.3  2004/05/10 04:41:48  chaoticcoyote
* Updated dirac algorithm document
* Modified encoder to use simple, portable command-line parser
*
* Revision 1.2  2004/04/12 01:57:46  chaoticcoyote
* Fixed problem Intel C++ had in finding xparam headers on Linux
* Solved Segmentation Fault bug in pic_io.cpp
*
* Revision 1.1.1.1  2004/03/11 17:45:43  timborer
* Initial import (well nearly!)
*
* Revision 0.1.0  2004/02/20 09:36:08  thomasd
* Dirac Open Source Video Codec. Originally devised by Thomas Davies,
* BBC Research and Development
*
*/

#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
#include <string>
#include "libdirac_encoder/seq_compress.h"
#include "libdirac_common/pic_io.h"
#include "libdirac_common/cmd_line.h"

using namespace std;


static void display_help()
{
	cout << "\nDIRAC wavelet video coder.";
	cout << "\n";
	cout << "\nUsage: progname -<flag1> [<flag_val>] ... <input1> <input2> ...";
	cout << "\nIn case of multiple assignment to the same parameter, the last holds.";
	cout << "\n";
	cout << "\nName    Type   I/O Default Value Description";
	cout << "\n====    ====   === ============= ===========                                       ";
	cout << "\ninput   string  I  [ required ]  Input file name";
	cout << "\noutput  string  I  [ required ]  Output file name";
	cout << "\nstream  bool    I  true          Use streaming compression presets";
	cout << "\nHD720p  bool    I  false         Use HD-720p compression presets";
	cout << "\nHD1080  bool    I  false         Use HD-1080 compression presets";
	cout << "\nSD576   bool    I  false         Use SD-576 compression presets";
	cout << "\nL1_sep  ulong   I  0UL           Separation of L1 frames";
	cout << "\nnum_L1  ulong   I  0UL           Number of L1 frames";
	cout << "\nxblen   ulong   I  0UL           Overlapping block horizontal length";
	cout << "\nyblen   ulong   I  0UL           Overlapping block vertical length";
	cout << "\nxbsep   ulong   I  0UL           Overlapping block horizontal separation";
	cout << "\nybsep   ulong   I  0UL           Overlapping block vertical separation";
	cout << "\ncpd     ulong   I  0UL           Perceptual weighting - vertical cycles per degree";
	cout << "\nqf      float   I  0.0F          Overall quality factor (sets other quality factors)";
	cout << "\nIqf     float   I  20.0F         I-frame quality factor (overrides -qf)";
	cout << "\nL1qf    float   I  22.0F         L1-frame quality factor (overrides -qf)";
	cout << "\nL2qf    float   I  24.0F         L2-frame quality factor (overrides -qf)";
	cout << "\nverbose bool    I  false         Verbose mode";
	cout << endl;
}

int main (int argc, char* argv[]){
	/*********************************************************************************/
			/**********  command line parameter parsing*********/

		 /********** create params object to handle command line parameter parsing*********/
	//To do: put parsing in a different function/constructor.

    // create a list of boolean options
	set<string> bool_opts;
	bool_opts.insert("verbose");
	bool_opts.insert("stream");
	bool_opts.insert("HD720p");
	bool_opts.insert("HD1080");
	bool_opts.insert("SD576");

	CommandLine args(argc,argv,bool_opts);

	//the variables we'll read parameters into
	EncoderParams encparams;
	OLBParams bparams;
	char input_name[84];//input name for pictures
	char output_name[84];//output name for pictures
	char bit_name[84];	//output name for the bitstream
	string input,output;

	float qf(30);		//quality/quatisation factors. The higher the factor, the lower the quality
	float Iqf(30);		//and the lower the bitrate
	float L1qf(32);
	float L2qf(34);

	//factors for setting motion estimation parameters
	float factor1 = 0.0F;
	float factor2 = 0.0F;
	float factor3 = 0.0F;

	//Set default values. To do: these should really be set in the constructor for the encoder parameters
	//These default values assume a streaming preset.
	encparams.L1_SEP =  3;
	encparams.NUM_L1 = 11;
	bparams.XBLEN=12;
	bparams.YBLEN=12;
	bparams.XBSEP=8;
	bparams.YBSEP=8;
	encparams.UFACTOR=3.0f;
	encparams.VFACTOR=1.75f;
	encparams.CPD=20.0f;
	encparams.I_lambda=pow(10.0,(Iqf/12.0)-0.3);
	encparams.L1_lambda=pow(10.0,((L1qf/9.0)-0.81));
	encparams.L2_lambda=pow(10.0,((L2qf/9.0)-0.81));
	factor3 = 250.0;

	encparams.VERBOSE=false;

	if (argc<3)//need at least 3 arguments - the program name, an input and an output
	{
		display_help();
	}
	else//carry on!
	{
		// parse command-line arguments
		//Do required inputs
		if (args.GetInputs().size()==2){
			input=args.GetInputs()[0];
			output=args.GetInputs()[1];
		}

		//check we have real inputs
		if ((input.length() == 0) || (output.length() ==0))
		{
			display_help();
			exit(1);
		}

		for (size_t i=0;i<input.length();i++)
			input_name[i]=input[i];

		input_name[input.length()] = '\0';

		for (size_t i=0;i<output.length();i++)
			output_name[i]=output[i];

		output_name[output.length()] = '\0';

		strncpy(bit_name,output_name,84);
		strcat(bit_name,".drc");

		//Now do the options
		//Start with quantisation factors
		for (vector<CommandLine::option>::const_iterator opt = args.GetOptions().begin();
			opt != args.GetOptions().end(); ++opt){
			if (opt->m_name == "qf")
			{
				qf = atof(opt->m_value.c_str());

				Iqf  = qf;
				L1qf = Iqf + 2.0f;
				L2qf = Iqf + 5.0f;
			}
		}//opt

		//go over and override quantisation factors if they've been specifically defined for
	   	//each frame type
		for (vector<CommandLine::option>::const_iterator opt = args.GetOptions().begin();
			opt != args.GetOptions().end(); ++opt)	{
			if (opt->m_name == "Iqf")
			{
				Iqf = atof(opt->m_value.c_str());
			}
			else if (opt->m_name == "L1qf")
			{
				L1qf = atof(opt->m_value.c_str());
			}
			else if (opt->m_name == "L2qf")
			{
				L2qf = atof(opt->m_value.c_str());
			}
		}//opt

			//Now do checking for presets
		for (vector<CommandLine::option>::const_iterator opt = args.GetOptions().begin();
			opt != args.GetOptions().end(); ++opt)
		{
			if (opt->m_name == "stream")
			{
				encparams.L1_SEP=3;
				encparams.NUM_L1=11;
				bparams.XBLEN=12;
				bparams.YBLEN=12;
				bparams.XBSEP=8;
				bparams.YBSEP=8;
				encparams.UFACTOR=3.0f;
				encparams.VFACTOR=1.75f;
				encparams.CPD=20.0f;
				encparams.I_lambda=pow(10.0,(Iqf/12.0)-0.3);
				encparams.L1_lambda=pow(10.0,((L1qf/9.0)-0.81));
				encparams.L2_lambda=pow(10.0,((L2qf/9.0)-0.81));

				factor3=250.0;
			}
			else if (opt->m_name == "HD720p")
			{
				encparams.L1_SEP=6;
				encparams.NUM_L1=3;
				bparams.XBLEN=16;
				bparams.YBLEN=16;
				bparams.XBSEP=10;
				bparams.YBSEP=12;
				encparams.UFACTOR=3.0f;
				encparams.VFACTOR=1.75f;
				encparams.CPD=20.0f;

				encparams.I_lambda=pow(10.0,((Iqf/13.34)+0.12));
				encparams.L1_lambda=pow(10.0,((L1qf/11.11)+0.14));
				encparams.L2_lambda=pow(10.0,((L2qf/11.11)+0.14));

				factor3 = 2000.0;
			}
			else if (opt->m_name == "HD1080")
			{
				encparams.L1_SEP=3;
				encparams.NUM_L1=3;
				bparams.XBLEN=20;
				bparams.YBLEN=20;
				bparams.XBSEP=16;
				bparams.YBSEP=16;
				encparams.UFACTOR=3.0f;
				encparams.VFACTOR=1.75f;
				encparams.CPD=32.0f;

				//TBC - not yet tuned
				encparams.I_lambda=pow(10.0,((Iqf/8.9)-0.58));
				encparams.L1_lambda=pow(10.0,((L1qf/9.7)+0.05));
				encparams.L2_lambda=pow(10.0,((L2qf/9.7)+0.05));

				factor3 = 100.0;
			}
			else if (opt->m_name == "SD576")
			{
				encparams.L1_SEP=3;
				encparams.NUM_L1=3;
				bparams.XBLEN=12;
				bparams.YBLEN=12;
				bparams.XBSEP=8;
				bparams.YBSEP=8;
				encparams.UFACTOR=3.0f;
				encparams.VFACTOR=1.75f;
				encparams.CPD=32.0f;

				encparams.I_lambda=pow(10.0,((Iqf/8.9)-0.58));
				encparams.L1_lambda=pow(10.0,((L1qf/9.7)+0.05));
				encparams.L2_lambda=pow(10.0,((L2qf/9.7)+0.05));

				factor3 = 100.0;
			}
		}//opt

		//now go over again and override presets with other values
		for (vector<CommandLine::option>::const_iterator opt = args.GetOptions().begin();
			opt != args.GetOptions().end(); ++opt)
		{
			if (opt->m_name == "L1_sep")
			{
				encparams.L1_SEP = strtoul(opt->m_value.c_str(),NULL,10);
			}
			else if (opt->m_name == "num_L1")
			{
				encparams.NUM_L1 = strtoul(opt->m_value.c_str(),NULL,10);
			}
			else if (opt->m_name == "xblen")
			{
				bparams.XBLEN = strtoul(opt->m_value.c_str(),NULL,10);
			}
			else if (opt->m_name == "yblen")
			{
				bparams.YBLEN = strtoul(opt->m_value.c_str(),NULL,10);
			}
			else if (opt->m_name == "xbsep")
			{
				bparams.XBSEP = strtoul(opt->m_value.c_str(),NULL,10);
			}
			else if (opt->m_name == "ybsep")
			{
				bparams.YBSEP = strtoul(opt->m_value.c_str(),NULL,10);
			}
			else if (opt->m_name == "cpd")
			{
				encparams.CPD = strtoul(opt->m_value.c_str(),NULL,10);
			}
			else if (opt->m_name == "verbose")
			{
				encparams.VERBOSE = true;
			}
		}//opt

		//Now rationalise the GOP options
		//this stuff should really be done in a constructor!
		if (encparams.NUM_L1<0){//don't have a proper GOP
			encparams.L1_SEP=std::max(1,encparams.L1_SEP);
		}
		else if (encparams.NUM_L1==0){//have I-frame only coding
			encparams.L1_SEP=0;
		}


  /********************************************************************/
	     //next do picture file stuff

   		/* ------ open input files & get params -------- */

		PicInput myinputpic(input_name);
		myinputpic.ReadPicHeader();
		encparams.cformat=myinputpic.GetSeqParams().cformat;

  		/* ------- open output files and write the header -------- */

		PicOutput myoutputpic(output_name,myinputpic.GetSeqParams());
		myoutputpic.WritePicHeader();

	/********************************************************************/


	//set up all the block parameters so we have a self-consistent set
		encparams.SetBlockSizes(bparams);
	//Finally, do the motion estimation Lagrangian parameters
  	//factor1 normalises the Lagrangian ME factors to take into account different overlaps
		const OLBParams& bparams2=encparams.LumaBParams(2);//in case we've changed them
		factor1=float(bparams2.XBLEN*bparams2.YBLEN)/
			float(bparams2.XBSEP*bparams2.YBSEP);
     //factor2 normalises the Lagrangian ME factors to take into account the number of
     //blocks in the picture. The more blocks there are, the more the MV field must be
     //smoothed and hence the higher the ME lambda must be
		int xnumblocks=myinputpic.GetSeqParams().xl/bparams2.XBSEP;
		int ynumblocks=myinputpic.GetSeqParams().yl/bparams2.YBSEP;
		factor2=sqrt(float(xnumblocks*ynumblocks));
    //factor3 is an heuristic factor taking into account the different CPD values and picture sizes, since residues
    //after motion compensation will have a different impact depending upon the perceptual weighting
    //in the subsequent wavelet transform. This has to be tuned by hand. Probably varies with bit-rate too.
		float ratio=factor1*factor2/factor3;
		encparams.L1_ME_lambda=encparams.L1_lambda*ratio;
		encparams.L2_ME_lambda=encparams.L2_lambda*ratio;
		encparams.L1I_ME_lambda=encparams.L1I_lambda*ratio;

   /********************************************************************/
      //open the bitstream file
		std::ofstream outfile(bit_name,std::ios::out | std::ios::binary);	//bitstream output

   /********************************************************************/
    	//do the work!!
		SequenceCompressor seq_compressor(&myinputpic,&outfile,encparams);
		seq_compressor.CompressNextFrame();
		for (int I=0;I<myinputpic.GetSeqParams().zl;++I){
			if (!seq_compressor.Finished())
				myoutputpic.WriteNextFrame(seq_compressor.CompressNextFrame());
		}//I

   /********************************************************************/
 	//close the bitstream file
		outfile.close();

		if (encparams.VERBOSE)
			std::cerr<<std::endl<<"Finished encoding";
		return EXIT_SUCCESS;


	}//?sufficient args?
}
