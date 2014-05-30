#include "plotLists.h"
#include "backgroundPredictions.h"
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

std::string rename(std::string oldName);
bool systematicDoesNotApply(std::string background, std::string syst);

int main(){

  bool plotSystematics=true;

  // global variable to detach histograms from files so files can be closed
  TH1::AddDirectory(kFALSE);

  std::vector<std::string> plotNames;
  plotNames.push_back("HT_regA_");
  plotNames.push_back("HT_regB_");
  plotNames.push_back("HT_regC_");
  plotNames.push_back("charge_regA_");
  plotNames.push_back("HT_regD_");

  // get list of systematics
  std::vector<std::string> systematicList;
  if(plotSystematics){ 
    for(int i=0; i<plotLists::SYSTEMATICS.size(); i++){
      if(plotLists::SYSTEMATICS[i]!="jeff_down" && plotLists::SYSTEMATICS[i]!="jer_down")
	systematicList.push_back(plotLists::SYSTEMATICS[i]);
    }
  }else systematicList.push_back("nominal");
  
  std::cout << "********** Starting To Get Data Plots **********" << std::endl;
 
  // open data file
  std::string data_name = "data/"+plotLists::DATE+"/data_nominal.root";
  TFile* data_file = TFile::Open(data_name.c_str());

  // save data output for limit setting
  TFile *f = new TFile("data/forLimitSetting/data.root","recreate");
  f->cd();
  for(int a=0; a< plotNames.size(); a++){
    TH1D* data_forLimit=0;
    data_forLimit = (TH1D*)data_file->Get(plotNames[a].c_str());
    if(data_forLimit){
      tools::fixOverflow(data_forLimit);
      std::cout << "data plot name: " << plotNames[a] << std::endl;
      std::string name = rename(plotNames[a]);
      std::cout << "new name: " << name << std::endl;
      data_forLimit->SetName(name.c_str());
      data_forLimit->Write();
    }
  }
  if(f)
    delete f;
  delete data_file;

  std::cout << "********** Starting To Get Background Plots **********" << std::endl;
  
  // each systematic broken down by background samples
  // bkgHistsForLimits[a][b][c] a is the systematic, b is the plot, c is the background
  std::vector<std::vector<std::vector<TH1D*> > > bkgHistsForLimits;
  bkgHistsForLimits.resize(systematicList.size());
  
  std::vector<std::string> backgroundNames;
  std::vector<TH1D*> nominalTotalPredictions;

  // get background predictions for nominal and each systematic
  for(int i=0; i<systematicList.size(); i++){
    backgroundPredictions* backgrounds = new backgroundPredictions(systematicList[i], plotNames);
    bkgHistsForLimits[i]=backgrounds->GetAllPredictions();
    if(i==0){ // nominal case
      backgroundNames = backgrounds->GetBackgroundNames();
      nominalTotalPredictions=backgrounds->GetTotalPredictions();
    }
    delete backgrounds;
  }

  std::cout << "size of backgrounds: " << backgroundNames.size() << std::endl;

  // save background files (nominal and systematics in different files with the same histogram names)
  for(int b=0; b<backgroundNames.size(); b++){
    std::cout << "background: " << backgroundNames[b] << std::endl;
    for(int c=0; c< systematicList.size(); c++){
      if(systematicList[c].find("singleTopT")!=std::string::npos) continue;
      if(systematicList[c].find("singleTopWt")!=std::string::npos) continue;
      if(systematicList[c].find("ttZ_XS")!=std::string::npos) continue;
      if(systematicList[c].find("ttWW_XS")!=std::string::npos) continue;
      if(systematicDoesNotApply(backgroundNames[b], systematicList[c]))continue;
      std::string nameWithSyst;
      if(systematicList[c]=="jeff_up")
	nameWithSyst="data/forLimitSetting/"+backgroundNames[b]+"_jeff.root";
      else if(systematicList[c]=="jer_up")
	nameWithSyst="data/forLimitSetting/"+backgroundNames[b]+"_jer.root";
      else if(systematicList[c]=="singleTopS_XS_up")
	nameWithSyst="data/forLimitSetting/"+backgroundNames[b]+"_singleTop_XS_up.root";
      else if(systematicList[c]=="singleTopS_XS_down")
	nameWithSyst="data/forLimitSetting/"+backgroundNames[b]+"_singleTop_XS_down.root";
      else if(systematicList[c]=="ttW_XS_up")
	nameWithSyst="data/forLimitSetting/"+backgroundNames[b]+"_ttbarX_XS_up.root";
      else if(systematicList[c]=="ttW_XS_down")
	nameWithSyst="data/forLimitSetting/"+backgroundNames[b]+"_ttbarX_XS_down.root";
      else
	nameWithSyst="data/forLimitSetting/"+backgroundNames[b]+"_"+systematicList[c]+".root";
      TFile *f = new TFile(nameWithSyst.c_str(),"recreate");
      f->cd();
      for(int d=0; d<plotNames.size(); d++){
	if(systematicList[c].find("singleTopS")!=std::string::npos || systematicList[c].find("ttW_XS")!=std::string::npos){
	  std::string name;
	  if(bkgHistsForLimits[c][d][b]) name = bkgHistsForLimits[c][d][b]->GetName();
	  else if (bkgHistsForLimits[c+2][d][b]) name = bkgHistsForLimits[c+2][d][b]->GetName();
	  else if (bkgHistsForLimits[c+4][d][b]) name = bkgHistsForLimits[c+4][d][b]->GetName();
	  else continue;
	  name = rename(name);
	  TH1D* tmp;
	  tmp=tools::combineHists(bkgHistsForLimits[c][d][b], bkgHistsForLimits[c+2][d][b], 1., 1.);
	  tmp=tools::combineHists(tmp, bkgHistsForLimits[c+4][d][b], 1., 1.);
	  tmp=tools::combineHists(tmp, bkgHistsForLimits[0][d][b], 1., -2.);
	  tmp->SetName(name.c_str());
	  tmp->Write();
	}
	if(bkgHistsForLimits[c][d][b]){
	  std::string name = bkgHistsForLimits[c][d][b]->GetName();
	  name = rename(name);
	  bkgHistsForLimits[c][d][b]->SetName(name.c_str());
	  bkgHistsForLimits[c][d][b]->Write();
	}
      } // end loop over plots
      if(f)
	delete f;
    } // end loop over systematics
  }// end loop over background files
  
  std::cout << "********** Starting To Get Signal Plots **********" << std::endl;

  // making list of signals
  std::vector<std::string> signal_base;
  for(int i=1; i<14; i++){
    int massPoint = 400+50*i;
    std::stringstream ss;
    ss << massPoint;
    std::string mass = ss.str();
    signal_base.push_back("MadGraphPythia8_AU2CTEQ6L1_TsZt_M"+mass+"L2_Pt20");
  }
  for(int i=0; i<=10; i++){
    int massPoint=350+50*i;
    std::stringstream ss;
    ss << massPoint;
    std::string mass = ss.str();
    signal_base.push_back("ProtosPythia_AUET2B_MSTW2008LO_TTS_M"+mass+"_1lep");
    signal_base.push_back("ProtosPythia_AUET2B_MSTW2008LO_BBS_M"+mass+"_1lep");
  }
  
  // save output for limit setting (signal)
  for(int isig=0; isig<signal_base.size(); isig++){
    std::cout << "signal base: " << signal_base[isig] << std::endl;
    std::string signal_name = "data/"+plotLists::DATE+"/"+signal_base[isig]+"_nominal.root";
    TFile *signal_nominal_file = TFile::Open(signal_name.c_str());

    for(int pp=0; pp<systematicList.size(); pp++){
      if(systematicList[pp].find("_XS_")!=std::string::npos || systematicList[pp].find("_scale_")!=std::string::npos)
	continue;
      std::string outputNameWithSyst="";
      if(systematicList[pp]=="jeff_up")
	outputNameWithSyst="data/forLimitSetting/"+signal_base[isig]+"_jeff.root";
      else if(systematicList[pp]=="jer_up")
	outputNameWithSyst="data/forLimitSetting/"+signal_base[isig]+"_jer.root";
      else
	outputNameWithSyst="data/forLimitSetting/"+signal_base[isig]+"_"+systematicList[pp]+".root";
      TFile *f = new TFile(outputNameWithSyst.c_str(),"recreate");
      TFile* syst_file=0;
      if(systematicList[pp]!="nominal" && systematicList[pp].find("lumi")==std::string::npos){
	std::string signal_syst_name = "data/"+plotLists::DATE+"/"+signal_base[isig]+"_"+systematicList[pp]+".root";
	if(systematicList[pp].find("jeff")!=std::string::npos)
	  signal_syst_name = "data/"+plotLists::DATE+"/"+signal_base[isig]+"_jeff.root";
	if(systematicList[pp].find("jer")!=std::string::npos)
	  signal_syst_name = "data/"+plotLists::DATE+"/"+signal_base[isig]+"_jer.root";
	syst_file = TFile::Open(signal_syst_name.c_str());
      }
      for(int mm=0; mm<plotNames.size(); mm++){

	// for single production signals
	if(signal_base[isig].find("TsZt")!=std::string::npos){
	  std::string histHT=plotNames[mm];
	  TH1D* signal_HT;
	  if(pp==0){
	    signal_HT = (TH1D*)signal_nominal_file->Get(histHT.c_str());
	    signal_HT->Scale(plotLists::LUMI/plotLists::GetNormalization(signal_base[isig], "nominal"));
	  }else{
	    if(systematicList[pp].find("lumi")!=std::string::npos){
	       signal_HT = (TH1D*)signal_nominal_file->Get(histHT.c_str());
	       signal_HT->Scale(plotLists::GetSystScaleFactor(systematicList[pp]));
	    }else{
	      if(syst_file && syst_file->IsOpen())
		signal_HT = (TH1D*)syst_file->Get(histHT.c_str());
	    }
	    double norm_factor = (plotLists::GetNormalization(signal_base[isig], systematicList[pp])==0.) ? plotLists::LUMI/plotLists::GetNormalization(signal_base[isig], "nominal") : plotLists::LUMI/plotLists::GetNormalization(signal_base[isig], systematicList[pp]);
	    signal_HT->Scale(norm_factor);
	  }
	  tools::fixOverflow(signal_HT);
	  if(signal_HT){
	    std::string name=signal_HT->GetName();
	    name = rename(name);
	    signal_HT->SetName(name.c_str());
	    f->cd();	
	    signal_HT->Write();
	  }
	}else{ // for pair production signals
	  for(int idecay=0; idecay<6; idecay++){
	    std::string histVLQdecay=plotNames[mm];
	    std::string decay = "";
	    switch(idecay){
	    case 0:
	      decay="_ZZ_";
	      break;
	    case 1:
	      decay="_WW_";
	      break;
	    case 2:
	      decay="_ZW_";
	      break;
	    case 3:
	      decay="_HH_";
	      break;
	    case 4:
	      decay="_ZH_";
	      break;
	    case 5:
	      decay="_WH_";
	      break;
	    }
	    //std::cout << "hist w decay: " << histVLQdecay << " and decay: " << decay << std::endl;
	    histVLQdecay+=decay;
	    //std::cout << "after replace: " << histVLQdecay << std::endl;
	    TH1D* signal_vlq_decay=0;
	    if(pp==0){
	      signal_vlq_decay = (TH1D*)signal_nominal_file->Get(histVLQdecay.c_str());
	      if(signal_vlq_decay)
		signal_vlq_decay->Scale(plotLists::LUMI/plotLists::GetNormalization(signal_base[isig], "nominal")); 
	    }else{
	      if(systematicList[pp].find("lumi")!=std::string::npos){
		signal_vlq_decay = (TH1D*)signal_nominal_file->Get(histVLQdecay.c_str());
		signal_vlq_decay->Scale(plotLists::GetSystScaleFactor(systematicList[pp]));
	      }else
		signal_vlq_decay = (TH1D*)syst_file->Get(histVLQdecay.c_str());
	      double norm_factor = (plotLists::GetNormalization(signal_base[isig], systematicList[pp])==0.) ? plotLists::LUMI/plotLists::GetNormalization(signal_base[isig], "nominal") : plotLists::LUMI/plotLists::GetNormalization(signal_base[isig], systematicList[pp]);
	      if(signal_vlq_decay){
		signal_vlq_decay->Scale(norm_factor); 
	      }
	    }
	    tools::fixOverflow(signal_vlq_decay);
	    if(signal_vlq_decay){
	      std::string newName = rename(histVLQdecay);
	      signal_vlq_decay->SetName(newName.c_str());
	      f->cd();		     
	      signal_vlq_decay->Write();
	    }
	  } // end loop over different decays
	}
      }// end loop over plots
	if(syst_file)
	  delete syst_file;
	delete f;
    } // end loop over systematics
    delete signal_nominal_file;
  } // end loop over signals

std::cout << "********** Done! **********" << std::endl;
}


std::string rename(std::string oldName){

  std::string newName=oldName;

  if(oldName=="HT_regA_")
    newName="HT_regA";
  if(oldName=="HT_regB_")
    newName="HT_regB";
  if(oldName=="HT_regC_")
    newName="HT_regC";
  if(oldName=="HT_regD_")
    newName="HT_regD";
  if(oldName=="charge_regA_")
    newName="charge_regA";
  
  std::vector<std::string> decays;
  decays.push_back("ZZ");
  decays.push_back("WW");
  decays.push_back("ZW");
  decays.push_back("HH");
  decays.push_back("ZH");
  decays.push_back("WH");

  for(int i=0; i<decays.size(); i++){
    if(oldName=="HT_regA__"+decays[i]+"_")
      newName="HT_regA_"+decays[i];
    if(oldName=="HT_regB__"+decays[i]+"_")
      newName="HT_regB_"+decays[i];
   if(oldName=="HT_regC__"+decays[i]+"_")
      newName="HT_regC_"+decays[i];
   if(oldName=="HT_regD__"+decays[i]+"_")
      newName="HT_regD_"+decays[i];
   if(oldName=="charge_regA__"+decays[i]+"_")
     newName="charge_regA_"+decays[i];
  }

   
    return newName;
}

bool systematicDoesNotApply(std::string background, std::string syst){

  if(syst.find("XS")==std::string::npos && syst.find("scale")==std::string::npos)
    return false;
  if(syst.find("singleTop")!=std::string::npos){
    if(background=="SingleTop") return false;
    else return true;
  }else if(syst.find("ttW")!=std::string::npos){
    if(background=="ttbarX") return false;
    else return true;
  }else if(syst.find("ttbar_")!=std::string::npos){
    if(background=="ttbar") return false;
    else return true;
  }else if(syst.find("diboson_scale")!=std::string::npos){
    if(background=="WW_Sherpa") return false;
    else if(background=="ZZ_Sherpa") return false;
    else if(background=="WZ_Sherpa") return false;
    else return true;
  }

  return false;
}
