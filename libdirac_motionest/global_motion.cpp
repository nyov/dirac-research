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
* Contributor(s):	Chris Bowley (Original Author) 
*					Marc Servais
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

#include <libdirac_motionest/global_motion.h>
using namespace dirac;

GlobalMotion::GlobalMotion(const FrameBuffer & frame_buffer,
						   MEData & me_data,
						   unsigned int frame_number,
						   const EncoderParams & encparams)
						   :
m_frame_buffer(frame_buffer),
m_me_data(me_data),
m_frame_number(frame_number),
m_gmv(me_data.Vectors(1).LengthY(), me_data.Vectors(1).LengthX()),
m_inliers(me_data.Vectors(1).LengthY(), me_data.Vectors(1).LengthX()),
m_encparams(encparams)
{
	m_debug = true;
}

GlobalMotion::~GlobalMotion()
{}

/*
Manages global motion approximation
*/
void GlobalMotion::ModelGlobalMotion(int ref_number)
{	

	// initialise inliers array
	for (int j=0;j<m_inliers.LengthY();j++)	for (int i=0;i<m_inliers.LengthX();i++) m_inliers[j][i] = 1;

	// make initial approximation using full motion vector field
	OneDArray<float> initial_params(8);
	for (int i=0; i<8; i++) initial_params[i] = 0.0; 

	// create model to approximate global motion
	ModelAffine * model = new ModelAffine(m_me_data, m_encparams);
	model->CalculateModelParameters(m_me_data.Vectors(ref_number), m_inliers, initial_params);

	if (m_debug)
	{
		std::cerr << std::endl << "  Initial affine parameters  : ";
		std::cerr << initial_params[0] << " " << initial_params[1] << " ";
		std::cerr << initial_params[2] << " " << initial_params[3] << " ";
		std::cerr << initial_params[4] << " " << initial_params[5] << " ";
	}

	TestGlobalMotionModel * test = new TestMvSAD(m_frame_buffer.GetFrame(m_frame_number),
		m_frame_buffer.GetFrame(ref_number),
		m_me_data,
		m_frame_number,
		ref_number,
		m_encparams);

	// Only using affine global motion for now: change to projective later.
	//ModelProjective * model_projective = new ModelProjective(m_me_data, m_encparams);
	//model_projective->CalculateModelParameters(m_me_data.Vectors(ref_number), m_inliers, initial_params);


/*
	// display initial parameters
	if (m_debug)
	{
	std::cerr << std::endl << "Initial projective parameters:  ";
	std::cerr << initial_params[0] << " " << initial_params[1] << " ";
	std::cerr << initial_params[2] << " " << initial_params[3] << " ";
	std::cerr << initial_params[4] << " " << initial_params[5] << " ";
	std::cerr << initial_params[6] << " " << initial_params[7] << " ";
	}
*/	

	// test initial approximation
	model->GenerateGlobalMotionVectors(m_gmv, initial_params);
	int initial_score = test->Test(m_gmv, m_inliers);
	if (m_debug) std::cerr << std::endl << "  Initial score (Low=Good)   = " << initial_score;

	// ********************************************************************************
	// reject motion vectors outliying from global motion
	// ********************************************************************************

	int score = initial_score;
	int number_of_inliers = 0;

	score = OutlierReject_Edge(ref_number, *model, *test, score);
	score = OutlierReject_SAD(ref_number, *model, *test, score);
	score = OutlierReject_LocalMean(ref_number, *model, *test, score);
	score = OutlierReject_Intensity(ref_number, *model, *test, score);
	//score = OutlierReject_Value(ref_number, *model, *test, score);
	score = OutlierReject_Outlier(ref_number, *model, *test, score);

	// ********************************************************************************
	// refine motion vector field
	// ********************************************************************************

	// weighted local filter
	RefineMotionVectorField * filter = new FilterWeightedLocal(m_frame_buffer, m_me_data,
		m_frame_number, ref_number,	*model, *test, m_inliers);
	filter->Refine(m_gmv, score);
	delete filter;
	if (m_debug) std::cerr << std::endl << "  Score after filter         = " << score;

	// ********************************************************************************
	// done refinement process
	// ********************************************************************************
 
	// make approximation using optimised motion vector field
	OneDArray<float> optimised_params(8);
	model->CalculateModelParameters(m_gmv, m_inliers, optimised_params);
	
	// display optimised parameters
	if (m_debug)
	{
		std::cerr << std::endl << "  Optimised parameters       : ";
		std::cerr << optimised_params[0] << " " << optimised_params[1] << " ";
		std::cerr << optimised_params[2] << " " << optimised_params[3] << " ";
		std::cerr << optimised_params[4] << " " << optimised_params[5] << " ";
		std::cerr << optimised_params[6] << " " << optimised_params[7] << " ";
	}    

	// create global motion vector fields from parameters and test  
	model->GenerateGlobalMotionVectors(m_gmv, optimised_params);
	int optimised_score = test->Test(m_gmv, m_inliers);

	if (m_debug) std::cerr << std::endl << "  Optimised score            = " << optimised_score;

	// update model parameters (for instrumentation)
	if (initial_score < optimised_score)  
	{
		m_me_data.GlobalMotionParameters(ref_number) = initial_params;
		std::cerr << std::endl << "  Initial Score " << initial_score << " < " << optimised_score << " Optimised Score";
	}
	else
	{
		m_me_data.GlobalMotionParameters(ref_number) = optimised_params;
		std::cerr << std::endl << "  Initial Score " << initial_score << " >= " << optimised_score << " Optimised Score";
	}

	if (m_debug)
	{
		std::cerr << std::endl << "  Using parameters           : ";
		std::cerr << m_me_data.GlobalMotionParameters(ref_number)[0] << " " << m_me_data.GlobalMotionParameters(ref_number)[1] << " ";
		std::cerr << m_me_data.GlobalMotionParameters(ref_number)[2] << " " << m_me_data.GlobalMotionParameters(ref_number)[3] << " ";
		std::cerr << m_me_data.GlobalMotionParameters(ref_number)[4] << " " << m_me_data.GlobalMotionParameters(ref_number)[5] << " ";
		std::cerr << m_me_data.GlobalMotionParameters(ref_number)[6] << " " << m_me_data.GlobalMotionParameters(ref_number)[7] << " ";
	}    

	// update global motion vector field
	model->GenerateGlobalMotionVectors(m_me_data.GlobalMotionVectors(ref_number),
		m_me_data.GlobalMotionParameters(ref_number));

	// update inliers (for instrumentation)
	m_me_data.GlobalMotionInliers(ref_number) = m_inliers;

	delete test;
}

//-----------------------------------------------------------------------------------------------------------

int GlobalMotion::OutlierReject_Edge(int ref_number, ModelAffine& model, TestGlobalMotionModel& test, int score)
{
	RejectMotionVectorOutliers * refine;
	refine = new RejectEdge(m_frame_buffer, m_me_data, m_frame_number, ref_number, model, test, m_inliers, m_encparams);
	refine->Reject(score);
	delete refine;
	if (m_debug) {
		std::cerr << std::endl << "  Score after edge rejection = " << score;
		int number_of_inliers = 0;
		for (int j=0; j<m_inliers.LengthY(); ++j) {
			for (int i=0; i<m_inliers.LengthX(); ++i) {
				if(m_inliers[j][i] == 1) number_of_inliers++;
			}
		}
		std::cerr << " (with " << 100*((float)number_of_inliers)/((float)(m_inliers.LengthX()*m_inliers.LengthY())) << "% inliers)";
	}
	return score;
}


int GlobalMotion::OutlierReject_SAD(int ref_number, ModelAffine& model, TestGlobalMotionModel& test, int score)
{
	RejectMotionVectorOutliers * refine;
	refine = new RejectSAD(m_frame_buffer, m_me_data, m_frame_number, ref_number, model, test, m_inliers, m_encparams);
	refine->Reject(score);
	delete refine;
	if (m_debug) {
		std::cerr << std::endl << "  Score after SAD rejection  = " << score;
		int number_of_inliers = 0;
		for (int j=0; j<m_inliers.LengthY(); ++j) {
			for (int i=0; i<m_inliers.LengthX(); ++i) {
				if(m_inliers[j][i] == 1) number_of_inliers++;
			}
		}
		std::cerr << " (with " << 100*((float)number_of_inliers)/((float)(m_inliers.LengthX()*m_inliers.LengthY())) << "% inliers)";
	}
	return score;
}


int GlobalMotion::OutlierReject_LocalMean(int ref_number, ModelAffine& model, TestGlobalMotionModel& test, int score)
{
	RejectMotionVectorOutliers * refine;
	refine = new RejectLocal(m_frame_buffer, m_me_data, m_frame_number, ref_number, model, test, m_inliers, m_encparams);
	refine->Reject(score);
	delete refine;
	if (m_debug) {
		std::cerr << std::endl << "  Score after Local Mean rej.= " << score;
		int number_of_inliers = 0;
		for (int j=0; j<m_inliers.LengthY(); ++j) {
			for (int i=0; i<m_inliers.LengthX(); ++i) {
				if(m_inliers[j][i] == 1) number_of_inliers++;
			}
		}
		std::cerr << " (with " << 100*((float)number_of_inliers)/((float)(m_inliers.LengthX()*m_inliers.LengthY())) << "% inliers)";
	}
	return score;
}


int GlobalMotion::OutlierReject_Intensity(int ref_number, ModelAffine& model, TestGlobalMotionModel& test, int score)
{
	RejectMotionVectorOutliers * refine;
	refine = new RejectIntensity(m_frame_buffer, m_me_data, m_frame_number, ref_number, model, test, m_inliers, m_encparams);
	refine->Reject(score);
	delete refine;
	if (m_debug) {
		std::cerr << std::endl << "  Score after Intensity rej. = " << score;
		int number_of_inliers = 0;
		for (int j=0; j<m_inliers.LengthY(); ++j) {
			for (int i=0; i<m_inliers.LengthX(); ++i) {
				if(m_inliers[j][i] == 1) number_of_inliers++;
			}
		}
		std::cerr << " (with " << 100*((float)number_of_inliers)/((float)(m_inliers.LengthX()*m_inliers.LengthY())) << "% inliers)";
	}
	return score;
}


int GlobalMotion::OutlierReject_Value(int ref_number, ModelAffine& model, TestGlobalMotionModel& test, int score)
{
	//
	// MARC: At the moment this gives an error. Also, not sure if just having a large motion
	// vector is a sufficiently good reason to not use it for estimating global motion! What
	// about the case when global motion corresponds to a large translation?
	//
	RejectMotionVectorOutliers * refine;
	refine = new RejectValue(m_frame_buffer, m_me_data, m_frame_number, ref_number, model, test, m_inliers, m_encparams);
	refine->Reject(score);
	delete refine;
	if (m_debug) {
		std::cerr << std::endl << "  Score after Value reject.  = " << score;
		int number_of_inliers = 0;
		for (int j=0; j<m_inliers.LengthY(); ++j) {
			for (int i=0; i<m_inliers.LengthX(); ++i) {
				if(m_inliers[j][i] == 1) number_of_inliers++;
			}
		}
		std::cerr << " (with " << 100*((float)number_of_inliers)/((float)(m_inliers.LengthX()*m_inliers.LengthY())) << "% inliers)";
	}
	return score;
}

int GlobalMotion::OutlierReject_Outlier(int ref_number, ModelAffine& model, TestGlobalMotionModel& test, int score)
{
	RejectMotionVectorOutliers * refine;
	refine = new RejectOutlier(m_frame_buffer, m_me_data, m_frame_number, ref_number, model, test, m_inliers, m_encparams);
	refine->Reject(score);
	delete refine;
	if (m_debug) {
		std::cerr << std::endl << "  Score after Outlier rej.   = " << score;
		int number_of_inliers = 0;
		for (int j=0; j<m_inliers.LengthY(); ++j) {
			for (int i=0; i<m_inliers.LengthX(); ++i) {
				if(m_inliers[j][i] == 1) number_of_inliers++;
			}
		}
		std::cerr << " (with " << 100*((float)number_of_inliers)/((float)(m_inliers.LengthX()*m_inliers.LengthY())) << "% inliers)";
	}
	return score;
}


