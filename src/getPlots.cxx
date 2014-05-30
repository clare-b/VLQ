#include "drawPlots.h"
#include "plotLists.h"
#include "errors.h"
#include "tools.h"
#include "backgroundPredictions.h"
#include "TH1.h"
#include "TFile.h"
#include "THStack.h"

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

int main(int argc,char** argv){

   std::cout << "********** Starting Plotting **********" << std::endl;

   //   std::string lepType=""; // "_eee","_mmm","_eem","_mme",""
   if(argv[1]==0) std::cout << "ERROR: must give channel as argument" << std::endl;
   std::string lepType=argv[1];
   bool drawSystematics=true;

   // open output root file
   TFile* outputData = new TFile(("data/rootFilesWithPlots/data"+lepType+".root").c_str(), "RECREATE");
   TFile* outputErrors = new TFile(("data/rootFilesWithPlots/errors"+lepType+".root").c_str(), "RECREATE");
   TFile* outputStacks = new TFile(("data/rootFilesWithPlots/stacks"+lepType+".root").c_str(), "RECREATE");
   TFile* outputTsZt = new TFile(("data/rootFilesWithPlots/TsZt"+lepType+".root").c_str(), "RECREATE");
   TFile* outputTTS = new TFile(("data/rootFilesWithPlots/TTS"+lepType+".root").c_str(), "RECREATE");
   TFile* outputBBS = new TFile(("data/rootFilesWithPlots/BBS"+lepType+".root").c_str(), "RECREATE");

   // get long list of plots
   std::vector<std::string> long_plotNames;
   std::vector<std::string> plotNames;
   if(lepType=="")long_plotNames=plotLists::PLOTNAMES;
   else if(lepType=="_eee" || lepType=="_mme") long_plotNames=plotLists::PLOTNAMES_E;
   else long_plotNames=plotLists::PLOTNAMES_M;
   
   for(int v=0; v<long_plotNames.size(); v++){
     std::string plotName = long_plotNames[v];
     if(plotName.find("3rdLep_charge")!=std::string::npos)
       plotNames.push_back(plotName);
     else if(plotName.find("3rdLep_isTight")!=std::string::npos)
       plotNames.push_back(plotName);
     else if(plotName.find("MET")!=std::string::npos)
       plotNames.push_back(plotName);   
     else if(plotName.find("MWT")!=std::string::npos)
       plotNames.push_back(plotName);   
     else if(plotName.find("Z_mass")!=std::string::npos)
       plotNames.push_back(plotName);  
     else if(plotName.find("Z_pt")!=std::string::npos)
       plotNames.push_back(plotName);    
     else if(plotName.find("ht_")!=std::string::npos)
       plotNames.push_back(plotName);  
     else if(plotName.find("numBjets_")!=std::string::npos)
       plotNames.push_back(plotName);  
     else if(plotName.find("numFwdJets_")!=std::string::npos)
       plotNames.push_back(plotName);  
     else if(plotName.find("numJets_")!=std::string::npos)
       plotNames.push_back(plotName);  
     else if(plotName.find("num_lep_")!=std::string::npos)
       plotNames.push_back(plotName);  
   }
   
   // signals to plot are single production and 650 mass point for TTS
   std::vector<std::string> signal_base;
   signal_base.push_back("MadGraphPythia8_AU2CTEQ6L1_TsZt_M650L2_Pt20");
   signal_base.push_back("ProtosPythia_AUET2B_MSTW2008LO_TTS_M650_1lep");
   signal_base.push_back("ProtosPythia_AUET2B_MSTW2008LO_BBS_M650_1lep");
   
   // global variable to detach histograms from files so files can be closed
   TH1::AddDirectory(kFALSE); 
   
   // get and open data file
   std::string data_name = "data/"+plotLists::DATE+"/data_nominal.root";
   TFile* data_file = TFile::Open(data_name.c_str());

   // open signal files
   std::vector<TFile*> signal_file(signal_base.size());
   for(int a=0; a<signal_base.size(); a++){
       std::string signal_name = "data/"+plotLists::DATE+"/"+signal_base[a]+"_nominal.root";
       signal_file[a] = TFile::Open(signal_name.c_str());
   }
   
   // make directories for output to go into
   int status = mkdir("output/pdf", S_IRWXU);
   std::string dir;
   if(lepType!="")dir="output/pdf/"+lepType.substr(1)+"/"; 
   else dir = "output/pdf/all/";
   status = mkdir(dir.c_str(), S_IRWXU); // first separate into channel directory
   for(int y=0; y<plotLists::JETCATEGORIES.size()+1; y++){
     std::string temp_dir;
     if(y==plotLists::JETCATEGORIES.size())temp_dir = dir+"general/";
     else temp_dir = dir+plotLists::JETCATEGORIES[y]+"/";
     status = mkdir(temp_dir.c_str(), S_IRWXU);
     status = mkdir((temp_dir+"general").c_str(), S_IRWXU);// then make b-tag categories inside those
     for(int w=0; w<plotLists::CATEGORIES.size(); w++){
       status = mkdir((temp_dir+plotLists::CATEGORIES[w]).c_str(), S_IRWXU);
     }
   }

   // this contains the total background prediction for each systematic
   std::vector<std::vector<TH1D*> > systematicHists; // nominal is the first one
   systematicHists.resize( plotLists::SYSTEMATICS.size() );
   for(int y=0; y< plotLists::SYSTEMATICS.size(); y++)
     systematicHists[y].resize( plotNames.size(), 0 );

   std::vector<std::string> backgroundNames;
   std::vector<std::vector<TH1D*> > nominalHistograms;

   // get list of systematics
   std::vector<std::string> systematicList;
   if(drawSystematics) systematicList=plotLists::SYSTEMATICS;
   else systematicList.push_back("nominal");
   
   // get all background predictions for nominal and systematics
   for(int i=0; i<systematicList.size(); i++){
     std::cout << "systematic: " << systematicList[i] << std::endl;
     backgroundPredictions* backgrounds = new backgroundPredictions(systematicList[i], plotNames, lepType);
     systematicHists[i]=backgrounds->GetTotalPredictions();
     if(i==0){ // nominal case
       backgroundNames = backgrounds->GetBackgroundNames();
       nominalHistograms = backgrounds->GetAllPredictions();
     }
     delete backgrounds;
   }

   errors* errList = new errors(systematicHists, drawSystematics, plotNames);
   
   // produce all plots
   for (int d=0; d<plotNames.size(); d++){
     std::cout << "   plotting: " << plotNames[d] << std::endl;
     
     std::vector<TH1D*> signal_hist;
     signal_hist.resize(signal_base.size(), 0);
     TH1D* data_hist=0;
     
     // getting signal and data histograms
     size_t pos=plotNames[d].find("1BIncl");
     if(pos != std::string::npos){ // for 1b tag inclusive combine 1bexcl, 2bincl
       std::string plotname_base=plotNames[d].substr(0,pos);
       std::string plotname_ending=plotNames[d].substr(pos+6);
       int count = 0;
       for(int qq=0; qq<signal_base.size(); qq++){
	 signal_hist[count]=tools::combineHists((TH1D*)signal_file[qq]->Get((plotname_base+plotLists::CATEGORIES[1]+plotname_ending+lepType).c_str()), (TH1D*)signal_file[qq]->Get((plotname_base+plotLists::CATEGORIES[2]+plotname_ending+lepType).c_str()), 1., 1.);
	 if(signal_hist[count]){
	   signal_hist[count]->SetName(plotNames[d].c_str());
	   signal_hist[count]->Scale(plotLists::LUMI/plotLists::GetNormalization(signal_base[qq], "nominal"));
	   tools::fixOverflow(signal_hist[count]);
	 }
	 count++;
       }
       TH1D* data_hist=tools::combineHists((TH1D*)data_file->Get((plotname_base+plotLists::CATEGORIES[1]+plotname_ending+lepType).c_str()), (TH1D*)data_file->Get((plotname_base+plotLists::CATEGORIES[2]+plotname_ending+lepType).c_str()), 1., 1.);
       data_hist->SetName(plotNames[d].c_str());
       std::cout << "data hist name: " << data_hist->GetName() << std::endl;
       tools::fixOverflow(data_hist);
     }else{
       int count=0;
       for(int qq=0; qq<signal_base.size(); qq++){
	 signal_hist[count] = (TH1D*)signal_file[qq]->Get((plotNames[d]+lepType).c_str());
	 if(signal_hist[count]){
	   signal_hist[count]->Scale(plotLists::LUMI/plotLists::GetNormalization(signal_base[qq], "nominal"));
	   tools::fixOverflow(signal_hist[count]);
	 }
	 count++;
       }
       data_hist = (TH1D*)data_file->Get((plotNames[d]+lepType).c_str());
       tools::fixOverflow(data_hist);
     }
     
     // only plot histograms that have something in them
     if(systematicHists[0][d]){
       //TCanvas* can = drawPlots::plotAll_VLQ(nominalHistograms[d], backgroundNames, plotLists::GetXAxisName(plotNames[d]), signal_hist, data_hist, errList->GetErrors()[d], plotLists::isSignalRegion(plotNames[d]), plotLists::useLogScale(plotNames[d]), lepType);
       
       THStack* Stack = new THStack();
       std::string title = std::string(";") + plotLists::GetXAxisName(plotNames[d]) + ";Events";
       Stack->SetTitle(title.c_str());
   
       std::vector<TH1D*> histos = nominalHistograms[d];
       for(int b=0; b<histos.size(); b++){
	 if(histos[b]){
	   histos[b]->SetLineColor(1);
	   histos[b]->SetFillColor(tools::setColor(backgroundNames[b]));
	   histos[b]->SetName(backgroundNames[b].c_str());
	   Stack->Add(histos[b]);
	 }
       }

       // save histograms
       outputData->cd();
       if(data_hist)
	 data_hist->Write();
       outputErrors->cd();
       errList->GetErrors()[d]->SetName(plotNames[d].c_str());
       errList->GetErrors()[d]->Write();
       outputStacks->cd();
       Stack->SetName(plotNames[d].c_str());
       Stack->Write();
       outputTsZt->cd();
       if(signal_hist[0])
	 signal_hist[0]->Write();
       outputTTS->cd();
       if(signal_hist[1])
	 signal_hist[1]->Write();
       outputBBS->cd();
       if(signal_hist[2])
	 signal_hist[2]->Write();
       
     }
   }
   
   delete data_file;
   
   std::cout << "********** Done! **********" << std::endl;
}
