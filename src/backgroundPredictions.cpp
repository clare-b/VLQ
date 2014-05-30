#include "backgroundPredictions.h"
#include "plotLists.h"
#include "tools.h"
#include "TH1.h"
#include "TFile.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <iterator>
#include <cassert>
#include <sstream>

backgroundPredictions::backgroundPredictions(std::string syst, std::vector<std::string> plotNames, std::string lepType, bool rebin){

  Init(plotNames, rebin);
  CalculatePredictions(syst, plotNames, lepType);

}

backgroundPredictions::~backgroundPredictions(){
}

void backgroundPredictions::Init(std::vector<std::string> plotNames, bool rebin){

  // get list of file names
  m_fileNames = plotLists::FILENAMES_FOR_PLOTTING;

  // get list of backgrounds
  m_backgrounds.resize(0);
   for(int i=0; i<plotLists::SAMPLES_TO_PLOT.size(); i++){
     if(plotLists::SAMPLES_TO_PLOT[i].find("sliced")==std::string::npos)
       m_backgrounds.push_back(plotLists::SAMPLES_TO_PLOT[i]);
   }
   
   // set size of m_totalPredictions
   m_totalPredictions.resize(plotNames.size(), 0);
   
   // set size of m_allPredictions
   m_allPredictions.resize(plotNames.size());
   for(int j=0; j<plotNames.size(); j++){
     m_allPredictions[j].resize(m_backgrounds.size(), 0);
   }

   m_rebin=rebin;

}

void backgroundPredictions::CalculatePredictions(std::string syst, std::vector<std::string> plotNames, std::string lepType){
  
  for (int a=0; a<m_fileNames.size(); a++){
    
    if(plotLists::DEBUG) std::cout << "file: " << m_fileNames[a] << std::endl;
    std::string name = plotLists::getFilenameForSyst(syst, m_fileNames[a]);
    // check if file exists
    bool exists = tools::exists(name);
    if(exists){
      TFile* file = TFile::Open(name.c_str());;
      
      std::string process = plotLists::GetFileSampleName(m_fileNames[a]);
      int numForSample=-1;
      for (int c=0; c<m_backgrounds.size(); c++){
	if(process.substr(0,5)=="Zjets"){
	  if(process.find(m_backgrounds[c]) != std::string::npos) numForSample = c;
	}
	else if(process==m_backgrounds[c]) numForSample = c;
      }
      // get factor for normalization
      double norm_factor = (plotLists::GetNormalization(m_fileNames[a], syst)==0.) ? plotLists::LUMI/plotLists::GetNormalization(m_fileNames[a], "nominal") : plotLists::LUMI/plotLists::GetNormalization(m_fileNames[a], syst);
      
      if(plotLists::scaleForSyst(syst, m_fileNames[a]))
	norm_factor*=plotLists::GetSystScaleFactor(syst);

      // plot names include everything in the plot name besides the lepType
      for(int b=0; b<plotNames.size(); b++){
	TH1D* hist=0;
	size_t pos=plotNames[b].find("1BIncl");
	if(pos != std::string::npos){ // for 1b tag inclusive
	  std::string plotname_base=plotNames[b].substr(0,pos);
	  std::string plotname_ending=plotNames[b].substr(pos+6);
	  hist = tools::combineHists((TH1D*)file->Get((plotname_base+plotLists::CATEGORIES[1]+plotname_ending+lepType).c_str()), (TH1D*)file->Get((plotname_base+plotLists::CATEGORIES[2]+plotname_ending+lepType).c_str()), 1., 1.);
	}else{
	  hist = (TH1D*)file->Get((plotNames[b]+lepType).c_str());
	}
	if(hist){
	  hist->Scale(norm_factor);
	  tools::fixOverflow(hist);
	  if(m_rebin)
	    tools::checkRebin(plotNames[b],hist);	
	  TH1D* hist_2 = (TH1D*)hist->Clone();
	  TH1D* temp = m_allPredictions[b][numForSample];
	  m_allPredictions[b][numForSample]= tools::combineHists(temp, hist_2, 1., 1.);
	  TH1D* temp2 = m_totalPredictions[b];
	  m_totalPredictions[b] = tools::combineHists(temp2, hist, 1., 1.);
	  //	  if(plotNames[b]=="Z_pt_1_afterZMass__iso__1BExcl_2jets") std::cout << "integral: " << m_totalPredictions[b]->Integral(0, m_totalPredictions[b]->GetNbinsX()+1) << std::endl;
	} // end if hist exists
      } // end loop over plots
      delete file;
    } // end check to see if file exists
    else if(plotLists::DEBUG)
      std::cout << "file doesn't exist for: " << m_fileNames[a] << std::endl;
  } // end loop over files

  
  
}

std::vector<TH1D*> backgroundPredictions::GetTotalPredictions(){
  return m_totalPredictions;
}

std::vector<std::vector<TH1D*> > backgroundPredictions::GetAllPredictions(){
  return m_allPredictions;
}

std::vector<std::string> backgroundPredictions::GetBackgroundNames(){
  return m_backgrounds;
}
