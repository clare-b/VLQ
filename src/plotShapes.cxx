#include "drawPlots.h"
#include "plotLists.h"
#include "errors.h"
#include "tools.h"
#include "TH1.h"
#include "TFile.h"
#include "backgroundPredictions.h"

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

int main(){

   std::cout << "********** Starting to make Shape Plots **********" << std::endl;

   std::string lepType=""; // "_eee","_mmm","_eem","_mme",""
   bool plotSignalsSeparate=false;


   // make list of plots
   std::vector<std::string> plotNames;
   /*
   plotNames.push_back("Z_pt_1_afterZMass__iso__1BIncl_2jets");
   plotNames.push_back("Z_pt_1_afterZMass__fwd__1BIncl_2jets");
   */
   plotNames.push_back("num_lep_1_afterZMass_");
   plotNames.push_back("numFwdJets_1_afterZMass__iso__1BIncl_2jets");
   plotNames.push_back("numFwdJets_2_afterZpt__iso__1BIncl_2jets");
   plotNames.push_back("ht_2_afterZpt__iso__1BIncl_2jets");
   plotNames.push_back("ht_2_afterZpt__fwd__1BIncl_2jets");

   // choose which signals to plot
   std::vector<std::string> signal_base;
   signal_base.push_back("MadGraphPythia8_AU2CTEQ6L1_TsZt_M650L2_Pt20");
   signal_base.push_back("ProtosPythia_AUET2B_MSTW2008LO_TTS_M650_1lep");
   signal_base.push_back("ProtosPythia_AUET2B_MSTW2008LO_BBS_M650_1lep");

   TH1::AddDirectory(kFALSE); // global variable to detach histograms from files so files can be closed

   // open signal files
   std::vector<TFile*> signal_file(signal_base.size());
   for(int qq=0; qq<signal_base.size(); qq++){
       std::string signal_name = "data/"+plotLists::DATE+"/"+signal_base[qq]+"_nominal.root";
       signal_file[qq] = TFile::Open(signal_name.c_str());
   }

   // make directories for output to go into
   int status = mkdir("output/shapes", S_IRWXU);
   std::string dir;
   if(lepType!="")dir="output/shapes/"+lepType.substr(1)+"/"; 
   else dir = "output/shapes/all/";
   status = mkdir(dir.c_str(), S_IRWXU);
   
   // get prediction for all backgrounds, nominal only
   std::vector<TH1D*> allBackgrounds;
   backgroundPredictions* backgrounds = new backgroundPredictions("nominal", plotNames);
   allBackgrounds=backgrounds->GetTotalPredictions();
   for(int aa=0; aa<plotNames.size(); aa++){
     if(plotNames[aa].find("ht_2_")!=std::string::npos){
       std::cout << "rebinning plot. old number of bins: " << allBackgrounds[aa]->GetNbinsX() << std::endl;
       allBackgrounds[aa]->Rebin(20);
       std::cout << "rebinned plot. new number of bins: " << allBackgrounds[aa]->GetNbinsX() << std::endl;
     }
   }
   delete backgrounds;
   
   // produce plots
   for (int d=0; d<plotNames.size(); d++){
     std::cout << "   plotting: " << plotNames[d] << std::endl;
     
     std::vector<TH1D*> signal_hist;
     signal_hist.resize(signal_base.size(),0);

     size_t pos=plotNames[d].find("1BIncl");
     if(pos != std::string::npos){ // for 1b tag inclusive combine 1bexcl, 2bincl
       std::string plotname_base=plotNames[d].substr(0,pos);
       std::string plotname_ending=plotNames[d].substr(pos+6);
       for(int qq=0; qq<signal_base.size(); qq++){
	 signal_hist[qq]=tools::combineHists((TH1D*)signal_file[qq]->Get((plotname_base+plotLists::CATEGORIES[1]+plotname_ending+lepType).c_str()), (TH1D*)signal_file[qq]->Get((plotname_base+plotLists::CATEGORIES[2]+plotname_ending+lepType).c_str()), 1., 1.);
	 if(signal_hist[qq]){
	   signal_hist[qq]->Scale(plotLists::LUMI/plotLists::GetNormalization(signal_base[qq], "nominal"));
	   if(plotNames[d].find("ht_2_afterZpt_")!=std::string::npos)
	     signal_hist[qq]->Rebin(20);
	   tools::fixOverflow(signal_hist[qq]);
	 }
       }
     }else{
       for(int qq=0; qq<signal_base.size(); qq++){
	 signal_hist[qq] = (TH1D*)signal_file[qq]->Get((plotNames[d]+lepType).c_str());
	 if(signal_hist[qq]){
	   signal_hist[qq]->Scale(plotLists::LUMI/plotLists::GetNormalization(signal_base[qq], "nominal"));
	   if(plotNames[d].find("ht_2_afterZpt_")!=std::string::npos)
	     signal_hist[qq]->Rebin(20);
	   tools::fixOverflow(signal_hist[qq]);
	 }
       }
     }

     if(plotSignalsSeparate){
       // only plot signal that pertains to each plot (either pair or single production)
       if(plotNames[d].find("fwd")!=std::string::npos || plotNames[d].find("Fwd")!=std::string::npos){
	 signal_hist[1]=0;
	 signal_hist[2]=0;
       }else{
	 signal_hist[0]=0;
       }
     }

     // plot linear scale
     TCanvas* can = drawPlots::plotShape_VLQ(allBackgrounds[d],plotLists::GetXAxisName(plotNames[d]), signal_hist, false, lepType);
     std::string tmp_saveName;
     if(lepType=="") tmp_saveName = "output/shapes/all";
     else tmp_saveName = "output/shapes/"+lepType.substr(1);
     std::string saveName = tmp_saveName+"/"+plotNames[d]+"_lin.pdf";     
     can->SaveAs(saveName.c_str());
     delete can;

     // plot log scale
     can = drawPlots::plotShape_VLQ(allBackgrounds[d],plotLists::GetXAxisName(plotNames[d]), signal_hist, true, lepType);
     saveName = tmp_saveName+"/"+plotNames[d]+"_log.pdf";     
     can->SaveAs(saveName.c_str());
     delete can;

   } // end loop over plots
   
   std::cout << "********** Done! **********" << std::endl;
}
