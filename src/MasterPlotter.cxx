#include "drawPlots.h"
#include "plotLists.h"
#include "errors.h"
#include "tools.h"
#include "backgroundPredictions.h"
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

int main(int argc,char** argv){

   std::cout << "********** Starting Plotting **********" << std::endl;

   //   std::string lepType=""; // "_eee","_mmm","_eem","_mme",""
   std::string lepType=argv[1];
   bool drawSystematics=true;

   // get list of plots
   std::vector<std::string> plotNames;
   if(lepType=="")plotNames=plotLists::PLOTNAMES;
   else if(lepType=="_eee" || lepType=="_mme") plotNames=plotLists::PLOTNAMES_E;
   else plotNames=plotLists::PLOTNAMES_M;

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
     backgroundPredictions* backgrounds = new backgroundPredictions(systematicList[i], plotNames, lepType, true);
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
	   signal_hist[count]->Scale(plotLists::LUMI/plotLists::GetNormalization(signal_base[qq], "nominal"));
	   tools::fixOverflow(signal_hist[count]);
	   tools::checkRebin(plotNames[d],signal_hist[count]);
	 }
	 count++;
       }
       data_hist=tools::combineHists((TH1D*)data_file->Get((plotname_base+plotLists::CATEGORIES[1]+plotname_ending+lepType).c_str()), (TH1D*)data_file->Get((plotname_base+plotLists::CATEGORIES[2]+plotname_ending+lepType).c_str()), 1., 1.);
       tools::fixOverflow(data_hist);
       tools::checkRebin(plotNames[d], data_hist);
     }else{
       int count=0;
       for(int qq=0; qq<signal_base.size(); qq++){
	 signal_hist[count] = (TH1D*)signal_file[qq]->Get((plotNames[d]+lepType).c_str());
	 if(signal_hist[count]){
	   signal_hist[count]->Scale(plotLists::LUMI/plotLists::GetNormalization(signal_base[qq], "nominal"));
	   tools::fixOverflow(signal_hist[count]);
	   tools::checkRebin(plotNames[d], signal_hist[count]);
	 }
	 count++;
       }
       data_hist = (TH1D*)data_file->Get((plotNames[d]+lepType).c_str());
       tools::fixOverflow(data_hist);
       tools::checkRebin(plotNames[d], data_hist);
     }
     
     // only plot histograms that have something in them
     if(systematicHists[0][d]){

       std::string extraTag="";
       if(plotNames[d].find("1BIncl")!=std::string::npos){
	 extraTag = "#geq 1 b-tag";
       }else if(plotNames[d].find("0BExcl")!=std::string::npos){
	 extraTag = "0 b-tags";
       }else if(plotNames[d].find("1BExcl")!=std::string::npos){
	 extraTag = "1 b-tag";
       }else if(plotNames[d].find("2BIncl")!=std::string::npos){
	 extraTag = "#geq 2 b-tags";
       }
       if (extraTag == "" && plotNames[d].find("fwd")!=std::string::npos)
	 extraTag = "#geq 1 fwd jet";
       else if(plotNames[d].find("fwd")!=std::string::npos)
	 extraTag = extraTag+", #geq 1 fwd jet";

       // for log scale, only take off single production if it shouldn't be there
       int sig_int = plotLists::includeWhichSignals(plotNames[d]);
       if(sig_int==0 || sig_int==2)
	 signal_hist[0]=0;

       // first plot with log scale
       // for int note or for thesis, you can use the first one?
       //       TCanvas* can = drawPlots::plotAll_VLQ(nominalHistograms[d], backgroundNames, plotLists::GetXAxisName(plotNames[d]), signal_hist, data_hist, errList->GetErrors()[d], false, true, lepType);
       TCanvas* can = drawPlots::plotAll_VLQ_paperStyle(nominalHistograms[d], backgroundNames, plotLists::GetXAxisName(plotNames[d]), signal_hist, data_hist, errList->GetErrors()[d], true, extraTag);
       
       // save histograms
       std::string tmp_saveName;
       if(lepType=="") tmp_saveName = "output/pdf/all";
       else tmp_saveName = "output/pdf/"+lepType.substr(1);
       std::string tmp_saveName_2 = tmp_saveName+"/general/";
       for(int y=0; y<plotLists::JETCATEGORIES.size(); y++){
	 if(plotNames[d].find(plotLists::JETCATEGORIES[y]) != std::string::npos)
	   tmp_saveName_2 = tmp_saveName+"/"+plotLists::JETCATEGORIES[y]+"/";
       } // end loop over jet categories
       std::string saveName=tmp_saveName_2+"general/"+plotNames[d]+"_log.pdf";
       for(int w=0; w<plotLists::CATEGORIES.size(); w++){
	 if(plotNames[d].find(plotLists::CATEGORIES[w]) != std::string::npos)
	   saveName = tmp_saveName_2+plotLists::CATEGORIES[w]+"/"+plotNames[d]+"_log.pdf";
       } // end loop over b-tag categories
       
       can->SaveAs(saveName.c_str());
       delete can;

       // now also take off pair production signal for inclusive plots
       if(sig_int==0){
	 signal_hist[1]=0;
	 signal_hist[2]=0;
       }

       // now plot with linear scale. For int note or thesis can use the first one
       //       can = drawPlots::plotAll_VLQ_paperStyle(nominalHistograms, backgroundNames, plotLists::GetXAxisName(plotNames[d]), signal_hist, data_hist, errList->GetErrors()[d], false, false, lepType);
       can = drawPlots::plotAll_VLQ_paperStyle(nominalHistograms[d], backgroundNames, plotLists::GetXAxisName(plotNames[d]), signal_hist, data_hist, errList->GetErrors()[d], false, extraTag);

       saveName=tmp_saveName_2+"general/"+plotNames[d]+"_lin.pdf";
       for(int w=0; w<plotLists::CATEGORIES.size(); w++){
	 if(plotNames[d].find(plotLists::CATEGORIES[w]) != std::string::npos)
	   saveName = tmp_saveName_2+plotLists::CATEGORIES[w]+"/"+plotNames[d]+"_lin.pdf";
       } // end loop over b-tag categories
       
       can->SaveAs(saveName.c_str());
       delete can;

     }
   }
   
   delete data_file;
   
   std::cout << "********** Done! **********" << std::endl;
}

