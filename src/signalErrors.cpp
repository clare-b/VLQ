#include "signalErrors.h"
#include "plotLists.h"
#include "tools.h"

#include <TMath.h>
#include <iostream>
#include <fstream>

signalErrors::signalErrors(std::string signal, std::string plotName, TH1D* nominalHist){
   Init();
   CalculateSyst(signal, plotName, nominalHist);
}

signalErrors::~signalErrors(){
}

void signalErrors::Init(){

  TH1::AddDirectory(kFALSE); 

  m_systUp=0.;
  m_systDown=0.;
}

void signalErrors::CalculateSyst(std::string signal, std::string plotName, TH1D* nominalHist){

  if (!nominalHist) return;

  std::string lepType="";

  int numBins = nominalHist->GetNbinsX();
  double tmpUp=0; double tmpDown=0;
  double nominalIntegral=0;
  nominalIntegral = nominalHist->Integral(0, numBins+1);
  std::vector<TH1D*> systHists;
  systHists.resize(plotLists::SYSTEMATICS.size());
  
  // first get list of histograms for all systematics
  for(int i=0; i<plotLists::SYSTEMATICS.size(); i++){
    
    std::string syst = plotLists::SYSTEMATICS[i];

    std::string name = plotLists::getFilenameForSyst(syst, signal);
    // check if file exists
    bool exists = tools::exists(name);
    if(exists){
      TFile* file = TFile::Open(name.c_str());;
      
      // get factor for normalization
      double norm_factor = (plotLists::GetNormalization(signal, syst)==0.) ? plotLists::LUMI/plotLists::GetNormalization(signal, "nominal") : plotLists::LUMI/plotLists::GetNormalization(signal, syst);
      
      if(plotLists::scaleForSyst(syst, signal))
	norm_factor*=plotLists::GetSystScaleFactor(syst);
      
      // plot names include everything in the plot name besides the lepType
      TH1D* hist=0;
      size_t pos=plotName.find("1BIncl");
      if(pos != std::string::npos){ // for 1b tag inclusive
	std::string plotname_base=plotName.substr(0,pos);
	std::string plotname_ending=plotName.substr(pos+6);
	hist = tools::combineHists((TH1D*)file->Get((plotname_base+plotLists::CATEGORIES[1]+plotname_ending+lepType).c_str()), (TH1D*)file->Get((plotname_base+plotLists::CATEGORIES[2]+plotname_ending+lepType).c_str()), 1., 1.);
      }else{
	hist = (TH1D*)file->Get((plotName+lepType).c_str());
      }
      if(hist){
	hist->Scale(norm_factor);
	tools::fixOverflow(hist);
	systHists[i]=hist;
      } // end if hist exists
      delete file;
    } // end check to see if file exists
    else if(plotLists::DEBUG)
      std::cout << "file doesn't exist for: " << signal << std::endl;
  } // end loop over systematics 

  // now that you have the list of histograms, loop through them and integrate to get total errors
  for(int c=0; c<plotLists::SYSTEMATICS.size(); c++){
    if(plotLists::SYSTEMATICS[c]=="nominal") continue;
    // up systematics are odd, down systematics are even
    if(c % 2 == 0) continue;
    std::cout << "systematic: " << plotLists::SYSTEMATICS[c] << std::endl;
    if(systHists[c] || systHists[c+1]){
      if(systHists[c])
	tmpUp=systHists[c]->Integral(0, numBins+1);
      if(systHists[c+1])
	tmpDown=systHists[c+1]->Integral(0, numBins+1);
      std::vector<double> errors=tools::getUpAndDownErrors(tmpUp, tmpDown, nominalIntegral, plotLists::SYSTEMATICS[c]);
      m_systUp+=pow(errors[0],2);
      m_systDown+=pow(errors[1],2);	     
    } // end if one of the systematic histograms exists
  } // end loop over systematics

  m_systUp=sqrt(m_systUp);
  m_systDown=sqrt(m_systDown);

}

double signalErrors::GetSystUp(){
  return m_systUp;
}

double signalErrors::GetSystDown(){
  return m_systDown;
}
