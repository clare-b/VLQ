#include "errors.h"
#include "plotLists.h"
#include "tools.h"

#include <TMath.h>
#include <iostream>
#include <fstream>

errors::errors(std::vector<std::vector<TH1D*> > allInputHists, bool includeSyst, std::vector<std::string> my_plotNames){

   Init();
   m_allInputHists = allInputHists;
   CalculateHists(includeSyst, my_plotNames);

}

errors::~errors(){
}

void errors::Init(){
  m_err.resize(plotLists::PLOTNAMES.size(),0);
}

void errors::CalculateHists(bool includeSyst, std::vector<std::string> my_plotNames){
   std::vector<double> listErrorsDown, listErrorsUp; // list over bins with only systematics
   float eDown,eUp; // includes statistical and systematic errors
   int numBins;
   double nominalBinContent=0;
   double syst_upBinContent=0;
   double syst_downBinContent=0;
   for(int a=0; a<my_plotNames.size(); a++){
     if (!m_allInputHists[0][a]) continue;
     std::cout << "plot: " << my_plotNames[a] << std::endl;
     numBins = m_allInputHists[0][a]->GetNbinsX();
     listErrorsDown.resize(numBins+1);listErrorsUp.resize(numBins+1);
     for(int i=0; i<=numBins; i++){
       listErrorsDown[i]=0.;
       listErrorsUp[i]=0.;
     }
     m_err[a] = new TGraphAsymmErrors(numBins);
     
     // start to loop over the bins
     for (int b=1; b<=numBins; b++){
       syst_upBinContent=0;
       syst_downBinContent=0;
       nominalBinContent=m_allInputHists[0][a]->GetBinContent(b);
       m_err[a]->SetPoint(b-1, m_allInputHists[0][a]->GetBinCenter(b), m_allInputHists[0][a]->GetBinContent(b));
       m_err[a]->SetPointEXlow(b-1, m_allInputHists[0][a]->GetBinWidth(b)/2.);
       m_err[a]->SetPointEXhigh(b-1, m_allInputHists[0][a]->GetBinWidth(b)/2.);

       if(includeSyst){
	 for(int c=0; c<plotLists::SYSTEMATICS.size(); c++){
	   if(plotLists::SYSTEMATICS[c]=="nominal") continue;
	   // up systematics are odd, down systematics are even
	   if(c % 2 == 0) continue;
	   std::cout << "systematic: " << plotLists::SYSTEMATICS[c] << std::endl;
	   if(m_allInputHists[c][a] || m_allInputHists[c+1][a]){
	     if(m_allInputHists[c][a])
	       syst_upBinContent=m_allInputHists[c][a]->GetBinContent(b);
	     if(m_allInputHists[c+1][a])
	       syst_downBinContent=m_allInputHists[c+1][a]->GetBinContent(b);
	     std::vector<double> errors=tools::getUpAndDownErrors(syst_upBinContent, syst_downBinContent, nominalBinContent, plotLists::SYSTEMATICS[c]);
	     listErrorsUp[b]+=pow(errors[0],2);
	     listErrorsDown[b]+=pow(errors[1],2);	     
	   } // end if one of the systematic histograms exists
	 } // end loop over systematics
         
	 eDown = sqrt( pow(m_allInputHists[0][a]->GetBinError(b), 2)+ listErrorsDown[b]);
	 eUp = sqrt( pow(m_allInputHists[0][a]->GetBinError(b), 2)+ listErrorsUp[b]);
       } // if we are including systematic uncertainties
       else{
	 eDown = m_allInputHists[0][a]->GetBinError(b);
	 eUp = m_allInputHists[0][a]->GetBinError(b);
       }// if we are only including statistics
       
       // to symmetrize take larger of eDown and eUp
       if(eDown>eUp){
	 m_err[a]->SetPointEYlow(b-1, eDown);
	 m_err[a]->SetPointEYhigh(b-1, eDown);
       }else{
	 m_err[a]->SetPointEYlow(b-1, eUp);
	 m_err[a]->SetPointEYhigh(b-1, eUp);
       }
       
     } // end loop over bins
     
     
     m_err[a]->SetFillStyle(3354);
     m_err[a]->SetFillColor(kBlack);
     m_err[a]->SetLineWidth(0);
     m_err[a]->SetMarkerSize(0);

   } // end loop over plots
}

std::vector<TGraphAsymmErrors*> errors::GetErrors(){
  return m_err;
}
