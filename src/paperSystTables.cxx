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

void printSystematicTables(std::vector<std::vector<double> > values,
			   std::vector<double> combinedValues,
			   std::vector<std::vector<double> > signalValues,
			   std::vector<std::string> background_names, 
			   std::string outputFileName);

std::vector<int> correspondingSystIndex(std::string backgroundName, std::string systToPrint);

int getBackgroundNum(int inputNum);

double getLarger(double up, double down);

int main(){
  
  std::cout << "********** Starting To Get Grouped Systematic Tables **********" << std::endl;
  
  // choose plot to integrate for the yield (shouldn't matter)
  std::string plotToUse = "MET";
  std::vector<std::string> plotNames;
  plotNames.push_back(plotToUse+"_2_afterZpt__iso__1BIncl_2jets");
  plotNames.push_back(plotToUse+"_2_afterZpt__fwd__1BIncl_2jets");
  
  // choose signal files to use
  std::vector<std::string> signal_base;
  signal_base.push_back("ProtosPythia_AUET2B_MSTW2008LO_BBS_M650_1lep");
  signal_base.push_back("ProtosPythia_AUET2B_MSTW2008LO_TTS_M650_1lep");
  signal_base.push_back("MadGraphPythia8_AU2CTEQ6L1_TsZt_M650L2_Pt20");
  
  TH1::AddDirectory(kFALSE); // global variable to detach histograms from files so files can be closed
  
  std::vector<std::string> background_names;
  background_names.push_back("WZ");
  background_names.push_back("$t\\bar{t}+X$");
  background_names.push_back("Other bkg.");
  
  // get list of systematics
  std::vector<std::string> systematicList = plotLists::SYSTEMATICS;
  
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
  for(int j=0; j<plotNames.size(); j++){
    mcErrors[j].resize(background_names.size(), 0);
  }
  
  // MC stat errors on total background prediction
  std::vector<double> mcTotalErrors;
  mcTotalErrors.resize(plotNames.size(), 0);
  
  // make directories for output to go into
  int status = mkdir("output/tex", S_IRWXU);
  status = mkdir("output/tex/yields", S_IRWXU);
  status = mkdir("output/tex/systematicTables", S_IRWXU);
  
  // get integrals and errors for each background prediction
  for(int bb=0; bb<systematicList.size(); bb++){
    std::cout << "systematic: " << systematicList[bb] << std::endl;
    backgroundPredictions* backgrounds = new backgroundPredictions(systematicList[bb], plotNames, "");
    std::vector<std::vector<TH1D*> > histograms = backgrounds->GetAllPredictions();
    std::vector<TH1D*> totalHistograms = backgrounds->GetTotalPredictions();
    for(int cc=0; cc<plotNames.size(); cc++){
      for(int z=0; z<background_names.size(); z++){
	valuesForTable[cc][bb][z]=0.;
	if(bb==0) mcErrors[cc][z]=0.;
      }
      for(int dd=0; dd<histograms[cc].size(); dd++){
	int backgroundNum = getBackgroundNum(dd);
	double error;
	if(histograms[cc][dd]){
	  int numBins = histograms[cc][dd]->GetNbinsX();
	  valuesForTable[cc][bb][backgroundNum]+=histograms[cc][dd]->IntegralAndError(1, numBins, error);
	  //std::cout << "values: " << valuesForTable[cc][aa][bb][dd] << std::endl;
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
  
  // now getting histograms for each signal
  std::vector<std::vector<std::vector<double> > > signalValues;
  signalValues.resize(plotNames.size());
  for(int j=0; j<plotNames.size(); j++){
    signalValues[j].resize(systematicList.size());
    for(int m=0; m<systematicList.size(); m++)
      signalValues[j][m].resize(signal_base.size(), 0.);
  }  
  
  std::cout << "signal base size: " << signal_base.size() << std::endl;
  for(int qq=0; qq<signal_base.size(); qq++){
    std::cout << "signal: " << qq << std::endl;
    for(int aa=0; aa<systematicList.size(); aa++){
      if(systematicList[aa].find("_XS_")!=std::string::npos || systematicList[aa].find("_scale_")!=std::string::npos || systematicList[aa].find("lumi_")!=std::string::npos)
	continue;
      std::string signal_name;
      if(systematicList[aa]=="jeff_up" || systematicList[aa]=="jeff_down")
	signal_name="data/"+plotLists::DATE+"/"+signal_base[qq]+"_jeff.root";
      else if(systematicList[aa]=="jer_up" || systematicList[aa]=="jer_down")
	signal_name="data/"+plotLists::DATE+"/"+signal_base[qq]+"_jer.root";
      else
	signal_name = "data/"+plotLists::DATE+"/"+signal_base[qq]+"_"+systematicList[aa]+".root";
      TFile* signal_file = TFile::Open(signal_name.c_str());
      
      for (int d=0; d<plotNames.size(); d++){
	TH1D* signal_hist=0;
	std::string histName = plotNames[d];
	
	size_t pos=histName.find("1BIncl");
	if(pos != std::string::npos){ // for 1b tag inclusive combine 1bexcl, 2bincl
	  std::string plotname_base=histName.substr(0,pos);
	  std::string plotname_ending=histName.substr(pos+6);
	  
	  signal_hist=tools::combineHists((TH1D*)signal_file->Get((plotname_base+plotLists::CATEGORIES[1]+plotname_ending).c_str()), (TH1D*)signal_file->Get((plotname_base+plotLists::CATEGORIES[2]+plotname_ending).c_str()), 1., 1.);
	  if(signal_hist){
	    double norm_factor = (plotLists::GetNormalization(signal_base[qq], systematicList[aa])==0.) ? plotLists::LUMI/plotLists::GetNormalization(signal_base[qq], "nominal") : plotLists::LUMI/plotLists::GetNormalization(signal_base[qq], systematicList[aa]);
	    signal_hist->Scale(norm_factor);   
	  }   
	}else{
	  signal_hist = (TH1D*)signal_file->Get((histName).c_str());
	  if(signal_hist){
	    double norm_factor = (plotLists::GetNormalization(signal_base[qq], systematicList[aa])==0.) ? plotLists::LUMI/plotLists::GetNormalization(signal_base[qq], "nominal") : plotLists::LUMI/plotLists::GetNormalization(signal_base[qq], systematicList[aa]);
	    signal_hist->Scale(norm_factor);   
	  }
	}
	
	if(signal_hist){
	  int numBins = signal_hist->GetNbinsX();
	  signalValues[d][aa][qq]=signal_hist->Integral(1,numBins);
	  //	  std::cout << "signal values. plot: " << d << " and systematic: " << systematicList[aa] << " and signal: " << qq << " and value: " << signalValues[d][aa][qq] << std::endl;
	}else{
	  signalValues[d][aa][qq]=0.;
	}
	
      } // end this loop over plots
      delete signal_file;
    }
  } // end loops over signal files
  
  for (int d=0; d<plotNames.size(); d++){
    std::string outputFileName="output/tex/systematicTables/systematics_grouped_forPaper"+plotNames[d];
    printSystematicTables(valuesForTable[d], combinedValuesForTable[d], signalValues[d], background_names, outputFileName);
    
  } // end loop over plots
  
  std::cout << "********** Done! **********" << std::endl;
}


void printSystematicTables(std::vector<std::vector<double> > values, std::vector<double> combinedValues, std::vector<std::vector<double> > signalValues, std::vector<std::string> background_names, std::string outputFileName){

  std::string saveName = outputFileName+".tex";
  int numBins=15;
  int precision=2;

  std::cout << "writing systematic file: " << saveName << std::endl;

  /*
  std::vector<std::string> backgroundsToPrint;
  for(int u=0; u<background_names.size(); u++){
    if(background_names[u]=="WZ")
      backgroundsToPrint.push_back("Diboson");
    else if(isDiboson(background_names[u]))
      continue;
    else if(background_names[u]=="Z+bottom")
      backgroundsToPrint.push_back("Z+jets");
    else if(isZjets(background_names[u]))
      continue;
    else
      backgroundsToPrint.push_back(background_names[u]);
  }
  */

  ofstream myfile;
  myfile.open(saveName.c_str());
  //  myfile << "\\begin{table}[htdp] \n";
  myfile << "\\tiny \n";
  myfile << "\\noindent\\makebox[\\textwidth]{";
  myfile << "\\begin{tabular}{";
  for(int i=0; i<=background_names.size()+4; i++){
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
  myfile << " Total bkg. & BBS & TTS & TsZt";
  myfile << " \\\\ \n";
  myfile << "\\hline \n";

  std::vector<std::string> systList;
  systList.push_back("Luminosity");
  systList.push_back("Cross-section");
  systList.push_back("Jet Reco.");
  systList.push_back("$b$-tagging");
  systList.push_back("$e$ Reco.");
  systList.push_back("$\\mu$ Reco.");


  // values[a][b], a is systematic, b is background
  // combinedValues[a], a is systematic  
  double allBack_XS_up=0;
  double allBack_XS_down=0;
  for(int j=0; j<systList.size(); j++){
    myfile << systList[j] << " & ";
    std::vector<std::string> systToUse;
    std::cout << "syst list j: " << systList[j] << std::endl;
    for(int i=0; i<background_names.size(); i++){

      if(values[0][i]==0) continue;
      double up=0;
      double down=0;
      if(systList[j]=="Cross-section"){
	std::cout << "background name: " << background_names[i] << " and indices: " << std::endl;
	std::vector<int> indices=correspondingSystIndex(background_names[i], systList[j]);
	for(int n=0; n<indices.size(); n++){
	  int index = indices[n];
	  std::cout << "index: " << index << std::endl;
	  //std::cout << "systematic value: " << values[index][i] << " and nominal: " << values[0][i] << std::endl;
	    up+=values[index][i]-values[0][i];
	    allBack_XS_up+=values[index][i]-values[0][i];
	    down+=values[0][i]-values[index+1][i];
	    allBack_XS_down+=values[0][i]-values[index+1][i];
	    //std::cout << "background up: " << allBack_XS_up << " and all background down: " << allBack_XS_down << std::endl;
	}
	if(indices.size()>0){
	  up=100.*up/(values[0][i]);
	  down=100.*down/(values[0][i]);
	}else{
	  up=0.;
	  down=0.;
	}
      }else{
	std::vector<int> indices=correspondingSystIndex(background_names[i], systList[j]);
	up=0;
	down=0;
	for(int n=0; n<indices.size(); n++){
	  int index = indices[n];
	  std::vector<double> errors=tools::getUpAndDownErrors(values[index][i], values[index+1][i], values[0][i], "");
	  up+=errors[0];
	  down+=errors[1];
	}
	up=100.*up/values[0][i];
	down=100.*down/values[0][i];
      }
      if(up < .1) up=0.;
      if(down < .1) down=0.;
      myfile << std::setprecision(precision) << getLarger(up,down) << " & ";
    } // end loop over backgrounds

    // total background
    double totalUp=0;
    double totalDown=0;
    if(systList[j]=="Cross-section"){
      totalUp=100*allBack_XS_up/combinedValues[0];
      totalDown=100*allBack_XS_down/combinedValues[0];
    }else{
      std::vector<int> indices=correspondingSystIndex("", systList[j]);
      totalUp=0;
      totalDown=0;
      for(int n=0; n<indices.size(); n++){
	int index = indices[n];
	std::vector<double> comb_err=tools::getUpAndDownErrors(combinedValues[index], combinedValues[index+1], combinedValues[0], "");
	    totalUp+=comb_err[0];
	    totalDown+=comb_err[1];
      }
      totalUp=100.*totalUp/combinedValues[0];
      totalDown=100.*totalDown/combinedValues[0];
    }
    if(totalUp<.01) totalUp=0;
    if(totalDown<.01) totalDown=0.;
    myfile << std::setprecision(precision) << getLarger(totalUp, totalDown) << " & ";

    // signal samples
    int specialIndex1=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "jeff_up")-plotLists::SYSTEMATICS.begin();
    int specialIndex2=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "jer_up")-plotLists::SYSTEMATICS.begin();
    for(int ab=0; ab<3; ab++){
      double up=0;
      double down=0;
      if(systList[j]=="Luminosity"){
	up = 2.8;
	down = 2.8;
      }else if(systList[j]=="Cross-section"){
	up = 0;
	down = 0;
      }else{
	std::vector<int> indices=correspondingSystIndex("", systList[j]);
	for(int n=0; n<indices.size(); n++){
	  int index = indices[n];
	  if(index==specialIndex1){
	    std::vector<double> errors=tools::getUpAndDownErrors(signalValues[index][ab], signalValues[0][ab], signalValues[0][ab], "jeff_up");
	    up+=errors[0];
	    down+=errors[1];
	  }else if(index==specialIndex2){
	    std::vector<double> errors=tools::getUpAndDownErrors(signalValues[index][ab], signalValues[0][ab], signalValues[0][ab], "jer_up");
	    up+=errors[0];
	    down+=errors[1];
	  }else{
	    std::vector<double> errors=tools::getUpAndDownErrors(signalValues[index][ab], signalValues[index+1][ab], signalValues[0][ab], "");
	    up+=errors[0];
	    down+=errors[1];
	  }
	}
	up=100.*up/signalValues[0][ab];
	down=100.*down/signalValues[0][ab];
      }
      if(up < .1) up=0.;
      if(down < .1) down=0.;
      if(ab<2)
	myfile << std::setprecision(precision) << getLarger(up,down) << " & ";
      else
	myfile << std::setprecision(precision) << getLarger(up,down);
    }


    myfile << " \\\\ \n";
  } // end loop over list of systematics

  myfile << "\\hline \n";
  myfile << "\\hline \n";
  myfile << "\\end{tabular}} \n";
  // myfile << "\\end{table} \n";
  myfile.close();

}

// returns up indices
std::vector<int> correspondingSystIndex(std::string backgroundName, std::string systToPrint){

  std::vector<int> tmp;
  tmp.resize(0);
  int numSyst=plotLists::SYSTEMATICS.size();
  
  if(systToPrint=="Cross-section"){
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
    }else if(backgroundName=="Other bkg."){
      int pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "singleTopS_XS_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
      pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "singleTopT_XS_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
      pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "singleTopWt_XS_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
      pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "diboson_scale_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
      pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "ttbar_XS_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
    }else if(backgroundName=="WZ"){
      int pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "diboson_scale_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
    }
    } else { // not cross-section systematic
    if(systToPrint=="$b$-tagging"){
      int pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "btag_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
      pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "ctautag_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
      pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "mistag_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
    }else if(systToPrint=="$e$ Reco."){
      int pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "eer_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
      pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "ees_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
      pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "el_idSF_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
      pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "el_recSF_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
      pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "el_trigSF_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
    }else if(systToPrint=="$\\mu$ Reco."){
      int pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "mu_idSF_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
      pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "mu_recSF_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
      pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "mu_trigSF_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
      pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "muid_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
      pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "mums_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
    }else if(systToPrint=="Jet Reco."){
      int pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "jes_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
      pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "jvfsf_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
      pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "jeff_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
      pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "jer_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
    }else if(systToPrint=="Luminosity"){
      int pos=std::find(plotLists::SYSTEMATICS.begin(), plotLists::SYSTEMATICS.end(), "lumi_up")-plotLists::SYSTEMATICS.begin();
      if(pos<numSyst)
	tmp.push_back(pos);
    }

  }

  return tmp;

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

double getLarger(double up, double down){
  if(up>down) return up;
  else return down;
}
