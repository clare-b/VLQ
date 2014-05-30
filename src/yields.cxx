#include "drawPlots.h"
#include "plotLists.h"
#include "backgroundPredictions.h"
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
#include <map>
#include <algorithm>

void printYields(std::vector<std::vector<std::vector<double> > > values,
		 std::vector<std::vector<double> > combinedValues,
		 std::vector<std::vector<double> > errors,
		 std::vector<double> combinedErrors, 
		 std::vector<std::string> background_names, 
		 std::vector<std::vector<TH1D*> > signal_hist, 
		 std::vector<TH1D*> data_hist, 
		 bool blind,
		 std::string outputFileName);

void printSystematicTables(std::vector<std::vector<double> > values,
		 std::vector<double> combinedValues,
		 std::vector<std::string> background_names, 
		 std::string outputFileName);

std::vector<int> correspondingSystIndex(std::string backgroundName);

int main(){

   std::cout << "********** Starting To Get Yields **********" << std::endl;

   bool printSystematics=true;

   std::vector<std::string> lepType;
   lepType.push_back("");
   lepType.push_back("_eee");
   lepType.push_back("_mme");
   lepType.push_back("_eem");
   lepType.push_back("_mmm");

   // choose plot to integrate for the yield (shouldn't matter)
   std::string plotToUse = "MET";
   std::vector<std::string> plotNames;
   for(int i=0; i<plotLists::CUTSFORYIELDS.size(); i++){
     plotNames.push_back(plotToUse+plotLists::CUTSFORYIELDS[i]);
   }

   // choose signal files to use
   std::vector<std::string> signal_base;
   signal_base.push_back("MadGraphPythia8_AU2CTEQ6L1_TsZt_M650L2_Pt20");
   signal_base.push_back("ProtosPythia_AUET2B_MSTW2008LO_TTS_M650_1lep");
   signal_base.push_back("ProtosPythia_AUET2B_MSTW2008LO_BBS_M650_1lep");

   TH1::AddDirectory(kFALSE); // global variable to detach histograms from files so files can be closed

   std::vector<std::string> background_names;
   for(int z=0; z<plotLists::SAMPLES_TO_PLOT.size(); z++){
     if(plotLists::SAMPLES_TO_PLOT[z].find("sliced")==std::string::npos){
       std::string background_name = plotLists::GetLegendName(plotLists::SAMPLES_TO_PLOT[z]);
       if(background_name=="t#bar{t}+X")
	 background_name="$t\\bar{t}+X$";
       if(background_name=="t#bar{t}")
	 background_name="$t\\bar{t}$";
       background_names.push_back(background_name);
     }
   }
   
   // get list of systematics
  std::vector<std::string> systematicList;
  if(printSystematics) systematicList=plotLists::SYSTEMATICS;
  else systematicList.push_back("nominal");

  // individual background predictions, nominal and systematic
  std::vector<std::vector<std::vector<std::vector<double> > > > valuesForTable;
   valuesForTable.resize(plotNames.size());
   for(int j=0; j<plotNames.size(); j++){
     valuesForTable[j].resize(lepType.size());
     for(int k=0; k<lepType.size(); k++){
       valuesForTable[j][k].resize(systematicList.size());
       for(int m=0; m<systematicList.size(); m++)
	 valuesForTable[j][k][m].resize(background_names.size(), 0);
     }
   }

   // total background predictions, nominal and systematic
   std::vector<std::vector<std::vector<double> > > combinedValuesForTable;
   combinedValuesForTable.resize(plotNames.size());
   for(int j=0; j<plotNames.size(); j++){
     combinedValuesForTable[j].resize(lepType.size());
     for(int m=0; m<lepType.size(); m++)
       combinedValuesForTable[j][m].resize(systematicList.size(), 0);
   }

   // MC stat errors on individual background predictions
  std::vector<std::vector<std::vector<double> > > mcErrors;
   mcErrors.resize(plotNames.size());
   for(int j=0; j<plotNames.size(); j++){
     mcErrors[j].resize(lepType.size());
     for(int m=0; m<lepType.size(); m++)
       mcErrors[j][m].resize(background_names.size(), 0);
   }

   // MC stat errors on total background prediction
   std::vector<std::vector<double> > mcTotalErrors;
   mcTotalErrors.resize(plotNames.size());
   for(int j=0; j<plotNames.size(); j++)
     mcTotalErrors[j].resize(lepType.size());

   
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
   status = mkdir("output/tex/yields", S_IRWXU);
   status = mkdir("output/tex/systematicTables", S_IRWXU);

   // get integrals and errors for each background prediction
   for(int aa=0; aa<lepType.size(); aa++){
     std::cout << "channel: " << lepType[aa] << std::endl;
     for(int bb=0; bb<systematicList.size(); bb++){
       std::cout << "systematic: " << systematicList[bb] << std::endl;
       backgroundPredictions* backgrounds = new backgroundPredictions(systematicList[bb], plotNames, lepType[aa]);
       std::vector<std::vector<TH1D*> > histograms = backgrounds->GetAllPredictions();
       std::vector<TH1D*> totalHistograms = backgrounds->GetTotalPredictions();
       for(int cc=0; cc<plotNames.size(); cc++){
	 for(int dd=0; dd<background_names.size(); dd++){
	   double error;
	   if(histograms[cc][dd]){
	     int numBins = histograms[cc][dd]->GetNbinsX();
	     valuesForTable[cc][aa][bb][dd]=histograms[cc][dd]->IntegralAndError(1, numBins, error);
	     //std::cout << "values: " << valuesForTable[cc][aa][bb][dd] << std::endl;
	     if(bb==0)
	       mcErrors[cc][aa][dd]=error;
	   }else{ 
	     valuesForTable[cc][aa][bb][dd]=0.;
	     if(bb==0)
	       mcErrors[cc][aa][dd]=0.;
	   }
	 } // end of loop over backgrounds
	 double totalError;
	 if(totalHistograms[cc]){
	   int numBins=totalHistograms[cc]->GetNbinsX();
	   combinedValuesForTable[cc][aa][bb]=totalHistograms[cc]->IntegralAndError(1, numBins, totalError);
	   if(bb==0)
	     mcTotalErrors[cc][aa]=totalError;
	 }else{
	   combinedValuesForTable[cc][aa][bb]=0.;
	   if(bb==0)
	     mcTotalErrors[cc][aa]=0.;
	 }
       } // end of loop over plots
       delete backgrounds;
       for(int r=0; r<histograms.size(); r++){
	 delete totalHistograms[r];
	 for(int s=0; s<histograms[0].size(); s++)
	   delete histograms[r][s];
       }
     }
   }
   
   std::vector<std::vector<TH1D*> > signal_hist;
   signal_hist.resize(lepType.size());
   for(int aa=0; aa<lepType.size(); aa++){
     signal_hist[aa].resize(signal_base.size(),0);
   }
   std::vector<TH1D*> data_hist;
   data_hist.resize(lepType.size(), 0);
   
   for (int d=0; d<plotLists::CUTSFORYIELDS.size(); d++){
     std::string outputFileName = "yields"+plotLists::CUTSFORYIELDS[d];
     std::string histName = plotToUse+plotLists::CUTSFORYIELDS[d];
     
     std::cout << "printing tables for: " << histName << std::endl;

     for(int aa=0; aa<lepType.size(); aa++){
       
       size_t pos=histName.find("1BIncl");
       if(pos != std::string::npos){ // for 1b tag inclusive combine 1bexcl, 2bincl
	 std::string plotname_base=histName.substr(0,pos);
	 std::string plotname_ending=histName.substr(pos+6);
	 for(int qq=0; qq<signal_base.size(); qq++){
	   signal_hist[aa][qq]=tools::combineHists((TH1D*)signal_file[qq]->Get((plotname_base+plotLists::CATEGORIES[1]+plotname_ending+lepType[aa]).c_str()), (TH1D*)signal_file[qq]->Get((plotname_base+plotLists::CATEGORIES[2]+plotname_ending+lepType[aa]).c_str()), 1., 1.);
	   if(signal_hist[aa][qq])
	     signal_hist[aa][qq]->Scale(plotLists::LUMI/plotLists::GetNormalization(signal_base[qq], "nominal"));      
	 }
	 data_hist[aa]=tools::combineHists((TH1D*)data_file->Get((plotname_base+plotLists::CATEGORIES[1]+plotname_ending+lepType[aa]).c_str()), (TH1D*)data_file->Get((plotname_base+plotLists::CATEGORIES[2]+plotname_ending+lepType[aa]).c_str()), 1., 1.);
       }else{
	 for(int qq=0; qq<signal_base.size(); qq++){
	   signal_hist[aa][qq] = (TH1D*)signal_file[qq]->Get((histName+lepType[aa]).c_str());
	   if(signal_hist[aa][qq])
	     signal_hist[aa][qq]->Scale(plotLists::LUMI/plotLists::GetNormalization(signal_base[qq], "nominal"));
	 }
	 data_hist[aa] = (TH1D*)data_file->Get((histName+lepType[aa]).c_str());
       }
     }
     printYields(valuesForTable[d], combinedValuesForTable[d], mcErrors[d], mcTotalErrors[d], background_names, signal_hist, data_hist, plotLists::isSignalRegion(histName), outputFileName);
     outputFileName="systematics"+plotLists::CUTSFORYIELDS[d];
     printSystematicTables(valuesForTable[d][0], combinedValuesForTable[d][0], background_names, outputFileName);
     
   } // end loop over plots
   
   delete data_file;

   std::cout << "********** Done! **********" << std::endl;
}


void printYields(std::vector<std::vector<std::vector<double> > > values, std::vector<std::vector<double> > combinedValues, std::vector<std::vector<double> > errors, std::vector<double> combinedErrors, std::vector<std::string> background_names, std::vector<std::vector<TH1D*> > signal_hist, std::vector<TH1D*> data_hist, bool blind, std::string outputFileName){

  std::string saveName = "output/tex/yields/"+outputFileName+".tex";
  int numBins=15;
  int numCol=5;
  int precision=2;

  std::cout << "writing file: " << saveName << std::endl;

  ofstream myfile;
  myfile.open(saveName.c_str());
  //  myfile << "\\begin{table}[htdp] \n";
  myfile << "\\footnotesize \n";
  myfile << "\\begin{tabular}{c c c c c c} \n";
  myfile << "\\hline \n";
  myfile << "\\hline \n";
  myfile << " & all channels & $(ee)e$ & $(\\mu \\mu)e$ & $(ee)\\mu$ & $(\\mu \\mu)\\mu$ \\\\ \n";
  myfile << "\\hline \n";

  // values[a][b][c], a is channel, b is systematic, c is background
  // combinedValues[a][b], a is channel, b is systematic
  // errors[a][b], a is channel, b is background
  // combinedErrors[a], a is channel
  
  // calculate systematic errors
  std::vector<std::vector<double> > syst_errors_up;
  std::vector<std::vector<double> > syst_errors_down;
  syst_errors_up.resize(errors.size());
  syst_errors_down.resize(errors.size());
  std::vector<double> syst_combinedErrors_up;
  syst_combinedErrors_up.resize(errors.size());
  std::vector<double> syst_combinedErrors_down;
  syst_combinedErrors_down.resize(errors.size());

  for(int i=0; i<errors.size(); i++){ // loop over channels
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

    myfile << std::setprecision(precision) 
	   << background_names[isample] 
	   << " & " << values[0][0][isample] << "$\\pm" << errors[0][isample] << "^{+" << syst_errors_up[0][isample] << "}" << "_{-"<< syst_errors_down[0][isample] << "}$";
    for(int mcat=1; mcat<numCol; mcat++){
      myfile << " & " << values[mcat][0][isample] << "$\\pm" << errors[mcat][isample] << "^{+"<< syst_errors_up[mcat][isample] << "}" << "_{-"<< syst_errors_down[mcat][isample] <<"}$";
    }
    myfile << " \\\\ \n"; 
  }
  
  myfile << "\\hline \n";

  myfile << "Background Prediction";
  for(int jcat=0; jcat<numCol; jcat++){
    myfile << " & " << combinedValues[jcat][0]<< "$\\pm" << combinedErrors[jcat] << "^{+"<< syst_combinedErrors_up[jcat] <<"}" << "_{-"<< syst_combinedErrors_down[jcat] <<"}$";
  }
  myfile << " \\\\ \n";
  myfile << "Single T";
  for(int kcat=0; kcat<numCol; kcat++){
    if(signal_hist[kcat][0])
      myfile << " & " << signal_hist[kcat][0]->Integral(0, numBins+1);
    else
      myfile << " & 0.0";
  }
  myfile << " \\\\ \n";
  myfile << "TTS";
  for(int pcat=0; pcat<numCol; pcat++){
    if(signal_hist[pcat][1])
      myfile << " & " << signal_hist[pcat][1]->Integral(0, numBins+1);
    else
      myfile << " & 0.0";
  }
  myfile << " \\\\ \n";
  if(!blind){
    myfile << "Data";
    for(int qcat=0; qcat<numCol; qcat++){
      if(data_hist[qcat])
	myfile << " & " << data_hist[qcat]->Integral(0, numBins+1);
      else
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

void printSystematicTables(std::vector<std::vector<double> > values, std::vector<double> combinedValues, std::vector<std::string> background_names, std::string outputFileName){

  std::string saveName = "output/tex/systematicTables/"+outputFileName+".tex";
  int numBins=15;
  int precision=2;

  std::cout << "writing systematic file: " << saveName << std::endl;

  ofstream myfile;
  myfile.open(saveName.c_str());
  //  myfile << "\\begin{table}[htdp] \n";
  myfile << "\\tiny \n";
  myfile << "\\noindent\\makebox[\\textwidth]{";
  myfile << "\\begin{tabular}{";
  for(int i=0; i<=background_names.size()+1; i++){
    if(i>0 && i<=background_names.size()){
      if(values[0][i-1]==0) continue;
    }
    myfile << "c ";
  }
  myfile << "} \n";
  myfile << "\\hline \n";
  myfile << "\\hline \n";
  myfile << " & ";
  for(int i=0; i<background_names.size(); i++){
    if(values[0][i]!=0)
      myfile << background_names[i] << " & ";
  }
  myfile << " Total bkg. ";
  myfile << " \\\\ \n";
  myfile << "\\hline \n";

  std::vector<std::string> systList;
  systList.push_back("Luminosity");
  systList.push_back("Cross-section");
  systList.push_back("B-tag SF");
  systList.push_back("C-tag SF");
  systList.push_back("Miss-tag SF");
  systList.push_back("EER");
  systList.push_back("EES");
  systList.push_back("Electron ID SF");
  systList.push_back("Electron rec.");
  systList.push_back("Electron trig SF");
  systList.push_back("JES");
  systList.push_back("JVF SF");
  systList.push_back("Muon ID");
  systList.push_back("Muon rec SF");
  systList.push_back("Muon trig SF");
  systList.push_back("MuID");
  systList.push_back("MuMS");
  systList.push_back("JER");
  systList.push_back("JEFF");

  // values[a][b], a is systematic, b is background
  // combinedValues[a], a is systematic  
  double allBack_XS_up=0;
  double allBack_XS_down=0;
  for(int j=0; j<systList.size(); j++){
    myfile << systList[j] << " & ";
    std::vector<std::string> systToUse;
    //std::cout << "syst list j: " << systList[j] << std::endl;
    for(int i=0; i<background_names.size(); i++){

      if(values[0][i]==0) continue;
      double up=0;
      double down=0;

      if(systList[j]=="Cross-section"){
	std::cout << "cross-section" << std::endl;
	std::vector<int> indices=correspondingSystIndex(background_names[i]);
	for(int n=0; n<indices.size(); n++){
	  int index = indices[n];
	    up+=values[index][i]-values[0][i];
	    allBack_XS_up+=values[index][i]-values[0][i];
	    down+=values[0][i]-values[index+1][i];
	    allBack_XS_down+=values[0][i]-values[index+1][i];
	}
	if(indices.size()>0){
	  up=100.*up/(values[0][i]);
	  down=100.*down/(values[0][i]);
	}else{
	  up=0.;
	  down=0.;
	}
      }else{
	for(int k=0; k<plotLists::SYSTEMATICS.size(); k++){
	  if(plotLists::SYSTEMATICS[k]=="nominal") continue;
	  if(k % 2 == 0) continue;
	  std::string tmpString=plotLists::SYST_NAME_MAP.at(plotLists::SYSTEMATICS[k]);
	  if(tmpString==systList[j]){
	    std::vector<double> errors=tools::getUpAndDownErrors(values[k][i], values[k+1][i], values[0][i], "");
	    std::cout << "errors 0: " << errors[0] << " and errors 1: " << errors[1] << std::endl;
	    up=100.*errors[0]/values[0][i];
	    down=100.*errors[1]/values[0][i];
	  }
	}
      }
      if(up < .1) up=0.;
      if(down < .1) down=0.;
      std::cout << "up: " << up << "down: " << down << std::endl;
      myfile << std::setprecision(precision) << "+" << up << "/-" << down << " & ";
    }
    double totalUp=0;
    double totalDown=0;
    if(systList[j]=="Cross-section"){
      totalUp=100*allBack_XS_up/combinedValues[0];
      totalDown=100*allBack_XS_down/combinedValues[0];
    }else{
      for(int m=0; m<plotLists::SYSTEMATICS.size(); m++){
	if(plotLists::SYSTEMATICS[m]=="nominal") continue;
	if(m % 2 == 0) continue;
	std::string tmpString=plotLists::SYST_NAME_MAP.at(plotLists::SYSTEMATICS[m]);
	if(tmpString==systList[j]){
	  std::vector<double> comb_err=tools::getUpAndDownErrors(combinedValues[m], combinedValues[m+1], combinedValues[0], "");
	  totalUp=100.*comb_err[0]/combinedValues[0];
	  totalDown=100.*comb_err[1]/combinedValues[0];
	}
      }
    }
    if(totalUp<.01) totalUp=0;
    if(totalDown<.01) totalDown=0.;
    myfile << std::setprecision(precision) << "+" << totalUp << "/-" << totalDown;
    myfile << " \\\\ \n";
  }

  myfile << "\\hline \n";
  myfile << "\\hline \n";
  myfile << "\\end{tabular}} \n";
  // myfile << "\\end{table} \n";
  myfile.close();

}

// returns up indices
std::vector<int> correspondingSystIndex(std::string backgroundName){

  std::cout << "background name: " << backgroundName << std::endl;

  std::vector<int> tmp;
  tmp.resize(0);
  int numSyst=plotLists::SYSTEMATICS.size();
  
  if(backgroundName=="$t\\bar{t}+X$"){
    int pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "ttW_XS_up")-plotLists::SYSTEMATICS.begin();
    if(pos<numSyst)
      tmp.push_back(pos);
    pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "ttZ_XS_up")-plotLists::SYSTEMATICS.begin();
    if(pos<numSyst)
      tmp.push_back(pos);
    pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "ttWW_XS_up")-plotLists::SYSTEMATICS.begin();
    if(pos<numSyst)
      tmp.push_back(pos);
  }else if(backgroundName=="Single Top"){
    int pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "singleTopS_XS_up")-plotLists::SYSTEMATICS.begin();
    if(pos<numSyst)
      tmp.push_back(pos);
    pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "singleTopT_XS_up")-plotLists::SYSTEMATICS.begin();
    if(pos<numSyst)
      tmp.push_back(pos);
    pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "singleTopWt_XS_up")-plotLists::SYSTEMATICS.begin();
    if(pos<numSyst)
      tmp.push_back(pos);
  }else if(backgroundName=="WW" || backgroundName=="WZ" || backgroundName=="ZZ"){
    std::cout << "background is diboson" << std::endl;
    int pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "diboson_scale_up")-plotLists::SYSTEMATICS.begin();
    if(pos<numSyst)
      tmp.push_back(pos);
    std::cout << "pos is: " << pos << std::endl;
  }else if(backgroundName=="$t\\bar{t}$"){
    int pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "ttbar_XS_up")-plotLists::SYSTEMATICS.begin();
    if(pos<numSyst)
      tmp.push_back(pos);
  }

  return tmp;

}
