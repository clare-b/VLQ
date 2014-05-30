#include "drawPlots.h"
#include "plotLists.h"
#include "errors.h"
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
#include <iomanip>

void printYields(std::vector<std::vector<TH1D*> > signal_hist, 
		 std::string outputFileName, std::string vlqType);

int main(int argc,char** argv){

  std::string vlqType=argv[1]; // "TTS","BBS" or "TsZt"

   std::cout << "********** Starting To Get "+vlqType+" Yields **********" << std::endl;
   
   std::vector<std::string> lepType;
   lepType.push_back("");
   lepType.push_back("_eee");
   lepType.push_back("_mme");
   lepType.push_back("_eem");
   lepType.push_back("_mmm");
   std::string outputFileName_1b_fwd = vlqType+"_yields_1btag_fwd";
   std::string histName_1b_fwd = "MET_2_afterZpt__fwd__1BIncl_2jets";

   std::string outputFileName_1b = vlqType+"_yields_1btag";
   std::string histName_1b = "MET_2_afterZpt__iso__1BIncl_2jets";

   TH1::AddDirectory(kFALSE); // global variable to detach histograms from files so files can be closed

   std::vector<std::string> signal_base;

   if(vlqType=="TTS" || vlqType=="BBS"){
     for(int i=0; i<=10; i++){
       int massPoint=350+50*i;
       std::stringstream ss;
       ss << massPoint;
       std::string mass = ss.str();
       signal_base.push_back("ProtosPythia_AUET2B_MSTW2008LO_"+vlqType+"_M"+mass+"_1lep");
     }
   }else{
     for(int i=0; i<=9; i++){
       int massPoint=600+50*i;
       std::stringstream ss;
       ss << massPoint;
       std::string mass = ss.str();
       signal_base.push_back("MadGraphPythia8_AU2CTEQ6L1_"+vlqType+"_M"+mass+"L2_Pt20");
     }
   }

   std::vector<TFile*> signal_file(signal_base.size());
   for(int qq=0; qq<signal_base.size(); qq++){
       std::string signal_name = "data/"+plotLists::DATE+"/"+signal_base[qq]+"_nominal.root";
       std::cout << "signal name: " << signal_name << std::endl;
       signal_file[qq] = TFile::Open(signal_name.c_str());
   }
   
   // make directories for output to go into
   int status = mkdir("output/tex", S_IRWXU);
   status = mkdir("output/tex/signalYields", S_IRWXU);

   if(vlqType=="TsZt"){
     std::vector<std::vector<TH1D*> > signal_hist_1b_fwd;
     signal_hist_1b_fwd.resize(lepType.size());
     for(int aa=0; aa<lepType.size(); aa++){
       signal_hist_1b_fwd[aa].resize(signal_base.size(),0);
     }
     
     for(int aa=0; aa<lepType.size(); aa++){  
       for(int qq=0; qq<signal_base.size(); qq++){
	 signal_hist_1b_fwd[aa][qq] = tools::getHistFromFile(signal_file[qq], histName_1b_fwd+lepType[aa]);
	 if(signal_hist_1b_fwd[aa][qq])
	   signal_hist_1b_fwd[aa][qq]->Scale(plotLists::LUMI/plotLists::GetNormalization(signal_base[qq], "nominal"));
       }
     }
     printYields(signal_hist_1b_fwd, outputFileName_1b_fwd, vlqType);
   
   }else{


     std::vector<std::vector<TH1D*> > signal_hist_1b;
     signal_hist_1b.resize(lepType.size());
     for(int aa=0; aa<lepType.size(); aa++){
       signal_hist_1b[aa].resize(signal_base.size(),0);
     }
     
     for(int aa=0; aa<lepType.size(); aa++){  
       for(int qq=0; qq<signal_base.size(); qq++){
	 signal_hist_1b[aa][qq] = tools::getHistFromFile(signal_file[qq], histName_1b+lepType[aa]);
	 if(signal_hist_1b[aa][qq])
	   signal_hist_1b[aa][qq]->Scale(plotLists::LUMI/plotLists::GetNormalization(signal_base[qq], "nominal"));
       }
     }
     printYields(signal_hist_1b, outputFileName_1b, vlqType);
     
   }
   
   std::cout << "********** Done! **********" << std::endl;
}


void printYields(std::vector<std::vector<TH1D*> > signal_hist, std::string outputFileName, std::string vlqType){

  std::string saveName = "output/tex/signalYields/"+outputFileName+".tex";
  int numBins=15;
  int numCol=5;
  int precision=3;

  std::cout << "writing file: " << saveName << std::endl;

  ofstream myfile;
  myfile.open(saveName.c_str());
  myfile << "\\begin{table}[htdp] \n";
  myfile << "\\begin{center} \n";
  myfile << "\\footnotesize \n";
  myfile << "\\begin{tabular}{| c || c | c | c | c | c |} \n";
  myfile << "\\hline \n";
  myfile << "\\hline \n";
  myfile << "$m_{Q}$ [GeV] & all channels & $(ee)e$ & $(\\mu \\mu)e$ & $(ee)\\mu$ & $(\\mu \\mu)\\mu$ \\\\ \n";
  myfile << "\\hline \n";
  myfile << "\\hline \n";

  if(vlqType=="TTS" || vlqType=="BBS"){
    for(int i=0; i<=10; i++){
      myfile << 350+50*i << std::setprecision(precision);
      for(int kcat=0; kcat<numCol; kcat++){
	if(signal_hist[kcat][i]){
	  double error;
	  myfile << " & " << signal_hist[kcat][i]->IntegralAndError(0, numBins+1, error);
	  myfile << " $\\pm$ " << error;
	}else
	  myfile << " & 0.0 $\\pm$ 0.0";
      }
      myfile << " \\\\ \n";
    }
  }else{
    for(int i=0; i<=9; i++){
      myfile << 600+50*i << std::setprecision(precision);
      for(int kcat=0; kcat<numCol; kcat++){
	if(signal_hist[kcat][i]){
	  double error;
	  myfile << " & " << signal_hist[kcat][i]->IntegralAndError(0, numBins+1, error);
	  myfile << " $\\pm$ " << error;
	}else
	  myfile << " & 0.0 $\\pm$ 0.0";
      }
      myfile << " \\\\ \n";
    }
  }
  
  myfile << "\\hline \n";
  myfile << "\\hline \n";
  myfile << "\\end{tabular} \n";
  myfile << "\\end{center} \n";
  myfile << "\\end{table} \n";
  myfile.close();

}

