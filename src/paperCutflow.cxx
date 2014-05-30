#include "drawPlots.h"
#include "plotLists.h"
#include "backgroundPredictions.h"
#include "errors.h"
#include "tools.h"
#include "TH1.h"
#include "TFile.h"
#include "signalErrors.h"

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
#include <map>
#include <algorithm>

void printCutflow(std::vector<std::vector<std::vector<double> > > values,
		 std::vector<std::vector<double> > combinedValues,
		 std::vector<std::vector<double> > errors,
		 std::vector<double> combinedErrors, 
		 std::vector<std::string> background_names, 
		 std::vector<std::vector<TH1D*> > signal_hist, 
		 std::vector<TH1D*> data_hist, 
		  std::string outputFileName,
		  std::vector<std::string> plotNames);

int getBackgroundNum(int inputNum);

double getTotalError(double statError, double systErrorUp, double systErrorDown);

int main(int argc,char** argv){

   std::cout << "********** Starting To Get Paper Cutflows **********" << std::endl;

   if(argc<2) std::cout << "ERROR: please give production type as argument (either pair or single)" << std::endl;
   std::string productionType = argv[1]; // either "pair" or "single"
   if(productionType != "pair" && productionType != "single")
     std::cout << "ERROR: production type should be either pair or single" << std::endl;

   // choose plots and signals to use
   std::vector<std::string> suffixNames;
   std::vector<std::string> signal_base;

   signal_base.push_back("ProtosPythia_AUET2B_MSTW2008LO_BBS_M650_1lep");
   signal_base.push_back("ProtosPythia_AUET2B_MSTW2008LO_TTS_M650_1lep");

   if(productionType=="pair"){
     suffixNames.push_back("_1_afterZMass__iso_");
     suffixNames.push_back("_1_afterZMass__iso__2jets");
     suffixNames.push_back("_2_afterZpt__iso__2jets");
     suffixNames.push_back("_2_afterZpt__iso__1BIncl_2jets");
   }else{
     suffixNames.push_back("_2_afterZpt__fwd__1BIncl_2jets");

     signal_base.push_back("MadGraphPythia8_AU2CTEQ6L1_TsZt_M650L2_Pt20");
   }

   std::string plotToUse="MET";
   std::vector<std::string> plotNames;
   for(int i=0; i<suffixNames.size(); i++){
     plotNames.push_back(plotToUse+suffixNames[i]);
   }

   TH1::AddDirectory(kFALSE); // global variable to detach histograms from files so files can be closed

   // choose background names for table
   std::vector<std::string> background_names;
   background_names.push_back("WZ");
   background_names.push_back("t#bar{t}+X");
   background_names.push_back("Other SM");
   
   // get list of systematics
  std::vector<std::string> systematicList=plotLists::SYSTEMATICS;

  // individual background predictions, nominal and systematic
  std::vector<std::vector<std::vector<double> > > valuesForTable;
   valuesForTable.resize(plotNames.size());
   for(int j=0; j<plotNames.size(); j++){
     valuesForTable[j].resize(systematicList.size());     
     for(int m=0; m<systematicList.size(); m++)
       valuesForTable[j][m].resize(background_names.size(), 0);
   }

   // total background predictions, nominal and systematic
   std::vector<std::vector<double> > combinedValuesForTable;
   combinedValuesForTable.resize(plotNames.size());
   for(int j=0; j<plotNames.size(); j++)
       combinedValuesForTable[j].resize(systematicList.size(), 0);

   // MC stat errors on individual background predictions
  std::vector<std::vector<double> > mcErrors;
   mcErrors.resize(plotNames.size());
   for(int j=0; j<plotNames.size(); j++)
       mcErrors[j].resize(background_names.size(), 0);

   // MC stat errors on total background prediction
   std::vector<double> mcTotalErrors;
   mcTotalErrors.resize(plotNames.size(), 0);
   
   std::string data_name = "data/"+plotLists::DATE+"/data_nominal.root";
   TFile* data_file = TFile::Open(data_name.c_str());

   // open signal files
   std::vector<TFile*> signal_file(signal_base.size());
   for(int qq=0; qq<signal_base.size(); qq++){
     std::string signal_name = "data/"+plotLists::DATE+"/"+signal_base[qq]+"_nominal.root";
     signal_file[qq] = TFile::Open(signal_name.c_str());
   }
   
   // make directories for output to go into
   int status = mkdir("output/tex", S_IRWXU);
   status = mkdir("output/tex/paperCutflow", S_IRWXU);

   // get integrals and errors for each background prediction
   for(int bb=0; bb<systematicList.size(); bb++){
     backgroundPredictions* backgrounds = new backgroundPredictions(systematicList[bb], plotNames, "");
     std::vector<std::vector<TH1D*> > histograms = backgrounds->GetAllPredictions();
     std::vector<TH1D*> totalHistograms = backgrounds->GetTotalPredictions();
     for(int cc=0; cc<plotNames.size(); cc++){
       for(int i=0; i<3; i++){
	 valuesForTable[cc][bb][i]==0.;
	 if(bb==0)
	   mcErrors[cc][i]=0.;
       }
       for(int dd=0; dd<histograms[cc].size(); dd++){
	 int backgroundNum = getBackgroundNum(dd);
	 double error;
	 if(histograms[cc][dd]){
	   int numBins = histograms[cc][dd]->GetNbinsX();
	   if(valuesForTable[cc][bb][backgroundNum]==0)
	     valuesForTable[cc][bb][backgroundNum]=histograms[cc][dd]->IntegralAndError(1, numBins, error);
	   else
	     valuesForTable[cc][bb][backgroundNum]+=histograms[cc][dd]->IntegralAndError(1, numBins, error);
	   if(bb==0)
	     mcErrors[cc][backgroundNum]+=error;
	 }
       } // end of loop over backgrounds
       double totalError;
       if(totalHistograms[cc]){
	 int numBins=totalHistograms[cc]->GetNbinsX();
	 combinedValuesForTable[cc][bb]=totalHistograms[cc]->IntegralAndError(1, numBins, totalError);
	 if(bb==0)
	   mcTotalErrors[cc]=totalError;
       }else{
	 combinedValuesForTable[cc][bb]=0.;
	 if(bb==0)
	   mcTotalErrors[cc]=0.;
       }
     } // end of loop over plots
     delete backgrounds;
     for(int r=0; r<histograms.size(); r++){
       delete totalHistograms[r];
       for(int s=0; s<histograms[0].size(); s++)
	 delete histograms[r][s];
     }
   }
   
   std::vector<std::vector<TH1D*> > signal_hist;
   signal_hist.resize(plotNames.size());
   for(int u=0; u<plotNames.size(); u++){
     signal_hist[u].resize(signal_base.size(),0);
   }
   std::vector<TH1D*> data_hist;
   data_hist.resize(plotNames.size());
   
   for (int d=0; d<plotNames.size(); d++){
     std::string histName = plotNames[d];
     
     size_t pos=histName.find("1BIncl");
     if(pos != std::string::npos){ // for 1b tag inclusive combine 1bexcl, 2bincl
       std::string plotname_base=histName.substr(0,pos);
       std::string plotname_ending=histName.substr(pos+6);
       for(int qq=0; qq<signal_base.size(); qq++){
	 signal_hist[d][qq]=tools::combineHists((TH1D*)signal_file[qq]->Get((plotname_base+plotLists::CATEGORIES[1]+plotname_ending).c_str()), (TH1D*)signal_file[qq]->Get((plotname_base+plotLists::CATEGORIES[2]+plotname_ending).c_str()), 1., 1.);
	   if(signal_hist[d][qq])
	     signal_hist[d][qq]->Scale(plotLists::LUMI/plotLists::GetNormalization(signal_base[qq], "nominal"));      
       }
	 data_hist[d]=tools::combineHists((TH1D*)data_file->Get((plotname_base+plotLists::CATEGORIES[1]+plotname_ending).c_str()), (TH1D*)data_file->Get((plotname_base+plotLists::CATEGORIES[2]+plotname_ending).c_str()), 1., 1.);
     }else{
	 for(int qq=0; qq<signal_base.size(); qq++){
	   signal_hist[d][qq] = (TH1D*)signal_file[qq]->Get((histName).c_str());
	   if(signal_hist[d][qq])
	     signal_hist[d][qq]->Scale(plotLists::LUMI/plotLists::GetNormalization(signal_base[qq], "nominal"));
	 }
	 data_hist[d] = (TH1D*)data_file->Get((histName).c_str());
     }
   } // end loop over plots

   if(productionType=="pair")
     printCutflow(valuesForTable, combinedValuesForTable, mcErrors, mcTotalErrors, background_names, signal_hist, data_hist, "paper_cutflow", plotNames);
   else
     printCutflow(valuesForTable, combinedValuesForTable, mcErrors, mcTotalErrors, background_names, signal_hist, data_hist, "paper_cutflow_singleProd", plotNames);
   
   delete data_file;

   std::cout << "********** Done! **********" << std::endl;
}


void printCutflow(std::vector<std::vector<std::vector<double> > > values, std::vector<std::vector<double> > combinedValues, std::vector<std::vector<double> > errors, std::vector<double> combinedErrors, std::vector<std::string> background_names, std::vector<std::vector<TH1D*> > signal_hist, std::vector<TH1D*> data_hist, std::string outputFileName, std::vector<std::string> plotNames){

  std::string saveName = "output/tex/paperCutflow/"+outputFileName+".tex";
  int numBins=15;
  int precision=2;

  int numCol=values.size();
  std::cout << "writing file: " << saveName << std::endl;

  ofstream myfile;
  myfile.open(saveName.c_str());
  //  myfile << "\\begin{table}[htdp] \n";
  myfile << "\\footnotesize \n";
  myfile << "\\begin{tabular}{";
  for(int a=0; a<=numCol; a++)
    myfile << " c";
  myfile << " } \n";
  myfile << "\\hline \n";
  myfile << "\\hline \n";
  if(numCol > 1)
    myfile << " & Trilepton Ch. & $\\geq 2$ jets & $p_T(Z)>150$ GeV & N_{tag}\\geq 1  \\\\ \n";
  else
    myfile << " & Trilepton Ch. \\\\ \n";
  myfile << "\\hline \n";

  // values[a][b][c], a is cut, b is systematic, c is background
  // combinedValues[a][b], a is cut, b is systematic
  // errors[a][b], a is cut, b is background
  // combinedErrors[a], a is cut
  
  // calculate systematic errors
  std::vector<std::vector<double> > syst_errors_up;
  std::vector<std::vector<double> > syst_errors_down;
  syst_errors_up.resize(errors.size());
  syst_errors_down.resize(errors.size());
  std::vector<double> syst_combinedErrors_up;
  syst_combinedErrors_up.resize(errors.size());
  std::vector<double> syst_combinedErrors_down;
  syst_combinedErrors_down.resize(errors.size());

  for(int i=0; i<errors.size(); i++){ // loop over cuts
    syst_errors_up[i].resize(errors[0].size(), 0);
    syst_errors_down[i].resize(errors[0].size(), 0);
    double comb_up=0;
    double comb_down=0;
    for(int j=0; j<errors[0].size(); j++){ // loop over backgrounds
      syst_errors_up[i][j]=0.;
      syst_errors_down[i][j]=0.;
      double up=0;
      double down=0;
      for(int k=1; k<values[i].size(); k++){ // loop over systematics
	if(k % 2 == 0) continue;
	std::vector<double> errors=tools::getUpAndDownErrors(values[i][k][j], values[i][k+1][j], values[i][0][j], "");
	up+=pow(errors[0],2);
	down+=pow(errors[1],2);
	if(j==0){
	  std::vector<double> comb_errors=tools::getUpAndDownErrors(combinedValues[i][k], combinedValues[i][k+1], combinedValues[i][0], "");
	  comb_up+=pow(comb_errors[0],2);
	  comb_down+=pow(comb_errors[1],2);
	} // doesn't depend on backgrounds, so only run for j==0
      } // end loop over systematics
      if(sqrt(up)<.015) syst_errors_up[i][j]=0.;
      else syst_errors_up[i][j]=sqrt(up);
      if(sqrt(down)<.015) syst_errors_down[i][j]=0.;
      else syst_errors_down[i][j]=sqrt(down);
    } // end loop over backgrounds
    
    if(sqrt(comb_up)<.015) syst_combinedErrors_up[i]=0.;
    else syst_combinedErrors_up[i]=sqrt(comb_up);
    if(sqrt(comb_down)<.015) syst_combinedErrors_down[i]=0.;
    else syst_combinedErrors_down[i]=sqrt(comb_down);

  }
  

  for(int isample=0; isample<background_names.size(); isample++){

    myfile// << std::setprecision(precision) 
	   << background_names[isample] ;
    for(int mcat=0; mcat<numCol; mcat++){
      double totalError = getTotalError(errors[mcat][isample], syst_errors_up[mcat][isample], syst_errors_down[mcat][isample]);
      tools::sigFigs(values[mcat][0][isample], totalError, precision);
      myfile << " & " << values[mcat][0][isample] << "$ \\pm " << totalError <<" $";
    }
    myfile << " \\\\ \n"; 
  }
  
  myfile << "\\hline \n";

  myfile << "Total SM";
  for(int jcat=0; jcat<numCol; jcat++){
    double totalError = getTotalError(combinedErrors[jcat], syst_combinedErrors_up[jcat], syst_combinedErrors_down[jcat]);
    tools::sigFigs(combinedValues[jcat][0], totalError, precision);
    myfile << " & " << combinedValues[jcat][0]<< "$ \\pm " << totalError <<" $";
  }
  myfile << " \\\\ \n";

  myfile << "\\hline \n";
  myfile << "\\hline \n";

  myfile << "Data";
  for(int qcat=0; qcat<numCol; qcat++){
    if(data_hist[qcat])
      myfile << " & " << data_hist[qcat]->Integral(0, numBins+1);
    else
      myfile << " & 0.0";
  }
  myfile << " \\\\ \n";

  myfile << "\\hline \n";
  myfile << "\\hline \n";



  myfile << "$B\\bar{B}$ ($m_B$=650 GeV)";
  for(int kcat=0; kcat<numCol; kcat++){
    if(signal_hist[kcat][0]){
      double stat;
      signalErrors* bb_errors = new signalErrors("ProtosPythia_AUET2B_MSTW2008LO_BBS_M650_1lep", plotNames[kcat], signal_hist[kcat][0]);
      double value = signal_hist[kcat][0]->IntegralAndError(0, numBins+1, stat);
      double totalError = getTotalError(stat, bb_errors->GetSystUp(), bb_errors->GetSystDown());
      tools::sigFigs(value, totalError, precision);
      myfile << " & " << value << "$ \\pm " << totalError << " $";
      delete bb_errors;
    }else
      myfile << " & 0.0";
  }
  myfile << " \\\\ \n";

  myfile << "$T\\bar{T}$ ($m_T$=650 GeV)";
  for(int pcat=0; pcat<numCol; pcat++){
    if(signal_hist[pcat][1]){
      double stat;
      signalErrors* tt_errors = new signalErrors("ProtosPythia_AUET2B_MSTW2008LO_TTS_M650_1lep", plotNames[pcat], signal_hist[pcat][1]);
      double value = signal_hist[pcat][1]->IntegralAndError(0, numBins+1, stat);
      double totalError = getTotalError(stat, tt_errors->GetSystUp(), tt_errors->GetSystDown());
      tools::sigFigs(value, totalError, precision);
      myfile << " & " << value << "$ \\pm " << totalError << " $";
      delete tt_errors;
    }else
      myfile << " & 0.0";
  }
  myfile << " \\\\ \n";

  if(numCol==1){ // only for the single production table

    myfile << "$T\\bar{b}q$ (650 GeV, $\\lambda_T = 2)";
    for(int pcat=0; pcat<numCol; pcat++){
      if(signal_hist[pcat][1]){
	double stat;
	signalErrors* t_errors = new signalErrors("MadGraphPythia8_AU2CTEQ6L1_TsZt_M650L2_Pt20", plotNames[pcat], signal_hist[pcat][2]);
	double value = signal_hist[pcat][2]->IntegralAndError(0, numBins+1, stat);
	double totalError = getTotalError(stat, t_errors->GetSystUp(), t_errors->GetSystDown());
	tools::sigFigs(value, totalError, precision);
	myfile << " & " << value << "$ \\pm " << totalError << " $";
	delete t_errors;
      }else
	myfile << " & 0.0";
    }
    myfile << " \\\\ \n";
    
    
  }
 

  myfile << "\\hline \n";
  myfile << "\\hline \n";
  myfile << "\\end{tabular} \n";
  // myfile << "\\end{table} \n";
  myfile.close();

}

 int getBackgroundNum(int inputNum){

   // return 0 for WZ, 1 for ttbar+X, 2 for other

   if(plotLists::SAMPLES_TO_PLOT[inputNum]=="WZ_Sherpa")
     return 0;
   else if(plotLists::SAMPLES_TO_PLOT[inputNum]=="ttbarX")
     return 1;
   else
     return 2;

 }

double getTotalError(double statError, double systErrorUp, double systErrorDown){

  double systError=0;
  systErrorUp > systErrorDown ? systError=systErrorUp : systError=systErrorDown;
  double totalError = sqrt(pow(statError,2)+pow(systError,2));

  return totalError;

}
