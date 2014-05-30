#include "plotLists.h"
#include "tools.h"

#include <iostream>
#include <fstream>
#include <map>

#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>

#include <stdlib.h>

// *************************************** //
// This is a class that keeps track of     //
// lots of messy hard-coded variable       //
// names, plot names, file names, etc.     //
// *************************************** //


const std::string plotLists::DATE = "May28";
const double plotLists::LUMI = 20276.9; // 20.2769 fb^-1
const bool plotLists::DEBUG=false;

// converts look up table to a vector of strings
std::vector<std::vector< std::string> > plotLists::LookUpTable(void){
   std::vector< std::vector< std::string > > tmp;
   std::ifstream infile;
   std::string LUT_name = "data/LookUpTable.txt";
   if(!tools::exists(LUT_name))
       std::cout << "ERROR: Can't find look up table" << std::endl;
   else{
      infile.open("data/LookUpTable.txt");
      std::string line;
      std::string sample="";
      while(!infile.eof()){
         getline(infile,line);
         // ensure you are not getting empty lines
         if(line != ""){
            std::vector<std::string> tmp_2;
            iter_split(tmp_2, line, boost::algorithm::first_finder("     "));
            tmp.push_back(tmp_2);
         }
      }
      infile.close();
   }
   return tmp;
}
const std::vector<std::vector< std::string> > plotLists::LOOKUPTABLE = plotLists::LookUpTable();

// produces a list of the b-tag categories considered
std::vector<std::string> plotLists::Categories(void){
   std::vector<std::string> tmp;
   tmp.push_back("0BExcl");
   tmp.push_back("1BExcl");
   tmp.push_back("2BIncl");
   tmp.push_back("1BIncl"); // this is 1BExcl+2BIncl

   return tmp;
}
const std::vector<std::string> plotLists::CATEGORIES = plotLists::Categories();

// produces a list of the jet categories considered
std::vector<std::string> plotLists::JetCategories(void){
   std::vector<std::string> tmp;
   /*
   if(plotLists::ONLY0JETREGION)
     tmp.push_back("0jets");
   else if(plotLists::ONLY2JETREGION)
     tmp.push_back("2jets");
   else{
   */
     tmp.push_back("0jets");
     tmp.push_back("1jet");
     tmp.push_back("2jets");
     //   }
   return tmp;
}
const std::vector<std::string> plotLists::JETCATEGORIES = plotLists::JetCategories();

// produces a list of all systematics
std::vector<std::string> plotLists::Systematics(void){
//    std::cout << "... building list of systematics" << std::endl;
   std::vector<std::string> tmp;

   tmp.push_back("nominal");
  
   // scale systematics
   tmp.push_back("lumi_up");
   tmp.push_back("lumi_down");
   
   tmp.push_back("ttbar_XS_up");
   tmp.push_back("ttbar_XS_down");

   tmp.push_back("singleTopS_XS_up");
   tmp.push_back("singleTopS_XS_down");
   tmp.push_back("singleTopT_XS_up");
   tmp.push_back("singleTopT_XS_down");
   tmp.push_back("singleTopWt_XS_up");
   tmp.push_back("singleTopWt_XS_down");

   tmp.push_back("ttW_XS_up");
   tmp.push_back("ttW_XS_down");
   tmp.push_back("ttZ_XS_up");
   tmp.push_back("ttZ_XS_down");
   tmp.push_back("ttWW_XS_up");
   tmp.push_back("ttWW_XS_down");

   // diboson scale systematic
   tmp.push_back("diboson_scale_up");
   tmp.push_back("diboson_scale_down");
   
   // detector systematics
   tmp.push_back("btag_up");
   tmp.push_back("btag_down");
   tmp.push_back("ctautag_up");
   tmp.push_back("ctautag_down");
   tmp.push_back("mistag_up");
   tmp.push_back("mistag_down");
   tmp.push_back("eer_up");
   tmp.push_back("eer_down");
   tmp.push_back("ees_up");
   tmp.push_back("ees_down");
   tmp.push_back("el_idSF_up");
   tmp.push_back("el_idSF_down");
   tmp.push_back("el_recSF_up");
   tmp.push_back("el_recSF_down");
   tmp.push_back("el_trigSF_up");
   tmp.push_back("el_trigSF_down");
   tmp.push_back("jes_up");
   tmp.push_back("jes_down");
   tmp.push_back("jvfsf_up");
   tmp.push_back("jvfsf_down");
   tmp.push_back("mu_idSF_up");
   tmp.push_back("mu_idSF_down");
   tmp.push_back("mu_recSF_up");
   tmp.push_back("mu_recSF_down");
   tmp.push_back("mu_trigSF_up");
   tmp.push_back("mu_trigSF_down");
   tmp.push_back("muid_up");
   tmp.push_back("muid_down");
   tmp.push_back("mums_up");
   tmp.push_back("mums_down");
   
   // root files only exist as "jeff" and "jer"
   tmp.push_back("jeff_up");
   tmp.push_back("jeff_down");
   tmp.push_back("jer_up");
   tmp.push_back("jer_down");
   
   return tmp;
}
const std::vector<std::string> plotLists::SYSTEMATICS = plotLists::Systematics();

double plotLists::GetSystScaleFactor(std::string systName){
   if(systName=="lumi_up")
     return 1+0.028;
   if(systName=="lumi_down")
     return 1-0.028;
   if(systName=="ttbar_XS_up")
     return 1+0.06;
   if(systName=="ttbar_XS_down")
     return 1-0.064;
   if(systName=="singleTopS_XS_up")
     return 1+0.039;
   if(systName=="singleTopS_XS_down")
     return 1-0.039;
   if(systName=="singleTopT_XS_up")
     return 1+0.039;
   if(systName=="singleTopT_XS_down")
     return 1-0.022;
   if(systName=="singleTopWt_XS_up")
     return 1+0.068;
   if(systName=="singleTopWt_XS_down")
     return 1-0.068;
   if(systName=="ttW_XS_up")
     return 1+0.3;
   if(systName=="ttW_XS_down")
     return 1-0.3;
   if(systName=="ttZ_XS_up")
     return 1+0.3;
   if(systName=="ttZ_XS_down")
     return 1-0.3;
   if(systName=="ttWW_XS_up")
     return 1+0.38;
   if(systName=="ttWW_XS_down")
     return 1-0.26;

   return 1.;
}

std::map<std::string,  std::string> plotLists::GetSystNameMap(){

  std::map<std::string, std::string> tmp;
  std::string tempString;

  for(int i=0; i<plotLists::SYSTEMATICS.size(); i++){
    tempString=plotLists::SYSTEMATICS[i];
    if(plotLists::SYSTEMATICS[i].find("lumi")!=std::string::npos)
      tmp[tempString]="Luminosity";
    else if(tempString.find("_XS")!=std::string::npos || tempString.find("scale") !=std::string::npos)
      tmp[tempString]="Cross-section";
    else if(tempString.find("btag")!=std::string::npos)
      tmp[tempString]="B-tag SF";
    else if(tempString.find("ctautag")!=std::string::npos)
      tmp[tempString]="C-tag SF";
    else if(tempString.find("mistag")!=std::string::npos)
      tmp[tempString]="Miss-tag SF";
    else if(tempString.find("eer")!=std::string::npos)
      tmp[tempString]="EER";
    else if(tempString.find("ees")!=std::string::npos)
      tmp[tempString]="EES";
    else if(tempString.find("el_idSF")!=std::string::npos)
      tmp[tempString]="Electron ID SF";
    else if(tempString.find("el_recSF")!=std::string::npos)
      tmp[tempString]="Electron rec.";
    else if(tempString.find("el_trigSF")!=std::string::npos)
      tmp[tempString]="Electron trig SF";
    else if(tempString.find("jes")!=std::string::npos)
      tmp[tempString]="JES";
    else if(tempString.find("jvfsf")!=std::string::npos)
      tmp[tempString]="JVF SF";
    else if(tempString.find("mu_idSF")!=std::string::npos)
      tmp[tempString]="Muon ID";
    else if(tempString.find("mu_recSF")!=std::string::npos)
      tmp[tempString]="Muon rec SF";
    else if(tempString.find("mu_trigSF")!=std::string::npos)
      tmp[tempString]="Muon trig SF";
    else if(tempString.find("muid")!=std::string::npos)
      tmp[tempString]="MuID";
    else if(tempString.find("mums")!=std::string::npos)
      tmp[tempString]="MuMS";
    else if(tempString.find("jer")!=std::string::npos)
      tmp[tempString]="JER";
    else if(tempString.find("jeff")!=std::string::npos)
      tmp[tempString]="JEFF";
  }
 
  return tmp;
}

const std::map<std::string, std::string> plotLists::SYST_NAME_MAP = plotLists::GetSystNameMap();

// produces a list of the names of all plots to be made from text file
std::vector<std::string> plotLists::plotNames(void){

  std::vector<std::string> tmp;

  std::ifstream infile;
  std::string LUT_name = "data/"+plotLists::DATE+"/plotNames.txt";
   if(!tools::exists(LUT_name))
       std::cout << "ERROR: Can't find look up table for plot names" << std::endl;
   else{
     infile.open(LUT_name.c_str());
      std::string line;
      std::string sample="";
      while(!infile.eof()){
         getline(infile,line);
         // ensure you are not getting empty lines
         if(line != ""){
	   //	   if(line.find("ht_2_afterZpt__fwd__1BIncl_2jets")!=std::string::npos)
	     tmp.push_back(line);
	 }
      }
      infile.close();
   }
   
   return tmp;
}
const std::vector<std::string> plotLists::PLOTNAMES = plotLists::plotNames();


// produces a list of the names of plots to be made for specific channels
std::vector<std::string> plotLists::plotNamesE(void){

  std::vector<std::string> tmp;
  std::string line;

  for(int j=0; j<plotLists::PLOTNAMES.size(); j++){
    line = plotLists::PLOTNAMES[j];
    if(line.find("2lep") == std::string::npos && line.find("3lep") == std::string::npos)
      tmp.push_back(line);
  }

   return tmp;
}
const std::vector<std::string> plotLists::PLOTNAMES_E = plotLists::plotNamesE();

// produces a list of the names of plots to be made for specific channels
std::vector<std::string> plotLists::plotNamesM(void){

  std::vector<std::string> tmp;
  std::string line;

  for(int j=0; j<plotLists::PLOTNAMES.size(); j++){
    line = plotLists::PLOTNAMES[j];
    if(line.find("2lep") == std::string::npos && line.find("3lep") == std::string::npos){
      if(line.find("_notTight_") == std::string::npos)
	tmp.push_back(line);
    }
  }

   return tmp;
}
const std::vector<std::string> plotLists::PLOTNAMES_M = plotLists::plotNamesM();

// produces a vector with the names of all samples being used 
std::vector<std::string> plotLists::samples(void){
//    std::cout << "... building list of samples to use" << std::endl;
   std::vector<std::string> tmp;
   
   tmp.push_back("Signal");
   //   tmp.push_back("BBS_600");
   //   tmp.push_back("TTS_600");
   tmp.push_back("Zjets_Alpgen_light");
   tmp.push_back("Zjets_Alpgen_bb");
   tmp.push_back("Zjets_Alpgen_cc");
   tmp.push_back("Zjets_Sherpa_MENLOPS");
   tmp.push_back("Zjets_Sherpa_light");
   tmp.push_back("Zjets_Sherpa_light_sliced");
   tmp.push_back("Zjets_Sherpa_bb");
   tmp.push_back("Zjets_Sherpa_bb_sliced");
   tmp.push_back("Zjets_Sherpa_cc");
   tmp.push_back("Zjets_Sherpa_cc_sliced");  
   tmp.push_back("Diboson_Alpgen");
   //tmp.push_back("Diboson_Sherpa");
   tmp.push_back("WW_Sherpa");
   tmp.push_back("WZ_Sherpa");
   tmp.push_back("ZZ_Sherpa");
   tmp.push_back("ttbar");
   tmp.push_back("SingleTop");
   //   tmp.push_back("TZ");
   tmp.push_back("WJets");
   tmp.push_back("ttbarX");
   tmp.push_back("Triboson");
   
   return tmp;
}
const std::vector<std::string> plotLists::SAMPLES = plotLists::samples();

// produces a vector with the names of all samples being used for plotting
// should be in the order you want them stacked
std::vector<std::string> plotLists::samplesToPlot(void){
//    std::cout << "... building list of samples to use in plots" << std::endl;
   std::vector<std::string> tmp;
   
   tmp.push_back("Triboson");
   tmp.push_back("ttbarX");
   //   tmp.push_back("TZ");  
   tmp.push_back("SingleTop");
   tmp.push_back("WW_Sherpa");
   tmp.push_back("WZ_Sherpa");
   tmp.push_back("ZZ_Sherpa");
   tmp.push_back("ttbar");  
   tmp.push_back("WJets");
   tmp.push_back("Zjets_Sherpa_light");
   tmp.push_back("Zjets_Sherpa_light_sliced"); // added to previous for plotting
   tmp.push_back("Zjets_Sherpa_bb");
   tmp.push_back("Zjets_Sherpa_bb_sliced"); // added to previous for plotting
   tmp.push_back("Zjets_Sherpa_cc");
   tmp.push_back("Zjets_Sherpa_cc_sliced"); // added to previous for plotting
   
   return tmp;
}
const std::vector<std::string> plotLists::SAMPLES_TO_PLOT = plotLists::samplesToPlot();

// produces a vector with the names of all samples being used for plotting
// should be in the order you want them stacked
std::vector<std::string> plotLists::samplesForRescaling(void){
//    std::cout << "... building list of samples to use in plots" << std::endl;
   std::vector<std::string> tmp;
   
   tmp.push_back("SingleTop");
   tmp.push_back("WW_Sherpa");
   tmp.push_back("ttbar");  
   tmp.push_back("Zjets_Sherpa_cc");
   tmp.push_back("Zjets_Sherpa_bb");
   tmp.push_back("Zjets_Sherpa_light");
   
   return tmp;
}
const std::vector<std::string> plotLists::SAMPLES_FOR_RESCALING = plotLists::samplesForRescaling();


// produces a vector with the name of all background files being used
// note that signal and data are not being used here
std::vector<std::string> plotLists::fileNames(void){
   std::vector<std::string> tmp; // the final vector to return
   std::vector<std::string> entry; // one line with sample, file, normalization
   // loop through LOOKUPTABLE
   for(int i=0; i<plotLists::LOOKUPTABLE.size(); i++){
      entry = plotLists::LOOKUPTABLE[i];
      if (std::find(plotLists::SAMPLES.begin(), plotLists::SAMPLES.end(), entry[0]) != plotLists::SAMPLES.end()){
	if(entry[0] != "Signal" && entry[0].substr(0,3) != "TTS" && entry[0].substr(0,3) != "BBS"){
	    bool alreadyInVector = false;
	    for(int j=0; j<tmp.size(); j++){
	      if(tmp[j]==entry[1]) alreadyInVector = true;
	    }
	    if(!alreadyInVector) tmp.push_back(entry[1]);
	} // end check to see if it is signal
      } // end checking sample name
   } // end loop over look up table
   
   return tmp;
}
const std::vector<std::string> plotLists::FILENAMES = plotLists::fileNames();

// produces a vector with the name of all background files used for plotting/total predictions
std::vector<std::string> plotLists::fileNamesForPlotting(void){
  std::vector<std::string> tmp;
  for(int i=0; i<plotLists::FILENAMES.size(); i++){
    std::string sampleName=plotLists::GetFileSampleName(plotLists::FILENAMES[i]);
    for(int j=0;j<plotLists::SAMPLES_TO_PLOT.size(); j++){
      if(sampleName==plotLists::SAMPLES_TO_PLOT[j]) 
	tmp.push_back(plotLists::FILENAMES[i]);
    }
  }
  return tmp;
}
const std::vector<std::string> plotLists::FILENAMES_FOR_PLOTTING = plotLists::fileNamesForPlotting();

// produces a vector with the name of all cuts in cutflow
std::vector<std::string> plotLists::cutNames(void){
//    std::cout << "... building list of cuts made" << std::endl;
   std::vector<std::string> tmp;
   tmp.push_back("initial");
   tmp.push_back("Z mass");
   tmp.push_back("$\\geq$ 2 jets");
   tmp.push_back("b-tags");
   tmp.push_back("$p_{T}$ of Z");
   tmp.push_back("3 leptons");
   tmp.push_back("tight 3rd lepton");
   tmp.push_back("$H_{T}$");
   
   return tmp;
}
const std::vector<std::string> plotLists::CUTNAMES = plotLists::cutNames();

// Gets the process (Signal, Diboson, etc.) from the filename
std::string plotLists::GetFileSampleName(std::string fileName){
//    std::cout << "... getting the sample name for a file" << std::endl;
   std::vector<std::string> entry; // one line with sample, file, normalization
   // loop through LOOKUPTABLE
   for(int i=0; i<plotLists::LOOKUPTABLE.size(); i++){
      entry = plotLists::LOOKUPTABLE[i];
      // check filename
      if(entry[1]==fileName)
         return entry[0];
   }
   
   return "";
}

// Get normalization
double plotLists::GetNormalization(std::string fileName, std::string syst){
//    std::cout << "... getting the normalization for a file" << std::endl;
   std::vector<std::string> entry; // one line with sample, file, normalization
   // loop through LOOKUPTABLE
   for(int i=0; i<plotLists::LOOKUPTABLE.size(); i++){
      entry = plotLists::LOOKUPTABLE[i];
      // check filename
      if(entry[1]==fileName && entry[2]==syst)
	return atof(entry[3].c_str());
   }
   
   return 0.;
}

// Converts the saved name of the plot to a more reasonable x axis name
// purely cosmetic, also make one of these for the legend names
std::string plotLists::GetXAxisName(std::string plotName){
//    std::cout << "... getting the x axis name for a plot" << std::endl;
   if(plotName.find("numBjets") != std::string::npos){
      return "b-tagged jet multiplicity";
   }else if(plotName.find("numFwdJets") != std::string::npos){
     return "Forward jet multiplicity";
   }else if(plotName.find("numJets") != std::string::npos){
      return "Central jet multiplicity";
   }else if(plotName.find("isTight") != std::string::npos){
     return "3rd lepton isTight";
   }else if(plotName.find("3rdLep_pt") != std::string::npos){
     return "3rd lepton p_{T} [GeV]";
   }else if(plotName.find("d0sig") != std::string::npos){
     return "#sigma(d_{0}) [mm]";
   }else if(plotName.find("d0_") != std::string::npos){
     return "d_{0} [mm]";
   }else if(plotName.find("z0_") != std::string::npos){
     return "z_{0} [mm]";
   }else if(plotName.find("num_lep") != std::string::npos){
      return "Lepton multiplicity";
   }else if(plotName.find("etcone20rel") != std::string::npos){
      return "etcone20/E";
   }else if(plotName.find("etcone20") != std::string::npos){
     return "etcone20 [GeV]";
   }else if(plotName.find("ptcone30rel") != std::string::npos){
     return "ptcone30/p_{T}";
   }else if(plotName.find("ptcone30") != std::string::npos){
     return "ptcone30 [GeV]";
   }else if(plotName.find("delR_3rdLep_jet") != std::string::npos){
     return "#Delta R between the third lepton and the closest jet";
   }else if(plotName.find("Z_mass") != std::string::npos){
      return "Z boson mass [GeV]";
   }else if(plotName.find("Z_pt") != std::string::npos){
      return "p_{T}(Z) [GeV]";
   }else if(plotName.find("_pt") != std::string::npos){
      return "p_{T} [GeV]";
   }else if(plotName.find("_phi") != std::string::npos){
      return "#phi";
   }else if(plotName.find("_eta") != std::string::npos){
      return "#eta";
   }else if(plotName.find("_mass") != std::string::npos){
      return "mass [GeV]";
   }else if(plotName.find("_E") != std::string::npos){
      return "energy [GeV]";
   }else if(plotName.find("cutflow_") != std::string::npos){
      return "cut number";
   }else if(plotName.find("MET_") != std::string::npos){
      return "E_{T}^{miss} [GeV]";
   }else if(plotName.find("MWT_") != std::string::npos){
      return "m_{T}(W) [GeV]";
   }else if(plotName.find("ht_") != std::string::npos){
     return "H_{T}(jets+leptons) [GeV]";
     //return "H_{T}=#sum p_{T}^{jet} + #sum p_{T}^{lepton} [GeV]";
   }else if(plotName.find("htAndMet_") != std::string::npos){
     return "H_{T}+E_{T}^{miss} [GeV]";
   }else if(plotName.find("top_b_and_W_delta_R_") != std::string::npos){
      return "#Delta R between the b-quark and W boson from the top decay";
   }

   return plotName;
}

// bool that tells us if we should be blinded or not
// we should be blinded if we are in the 1 or 2 tag category
// only after the cut on the pt of the Z boson
bool plotLists::isSignalRegion(std::string plotName){

  // hack to unblind
  return false;
  
   if(plotName.find("0BExcl") != std::string::npos){
      return false;
   }else if(plotName.find("notTight") != std::string::npos){
     return false;
   }else if(plotName.find("2jets") != std::string::npos){
     return true;
   }else if(plotName.find("0jets") != std::string::npos){
     return false;
   }else if(plotName.find("1jet") != std::string::npos){
     return false;
   }else if(plotName.find("_1_") != std::string::npos){
     return false;
   }else if(plotName.find("_2_") != std::string::npos){
     return false;
   }
   
   return true;
}

// bool that tell us if the plot should be in log scale or not
bool plotLists::useLogScale(std::string plotName){
  
  if(plotName.find("numBjets") != std::string::npos){
      return true;
  }else if(plotName.find("num_lep") != std::string::npos){
      return true;
  }else if(plotName.find("numFwdJets") != std::string::npos){
      return true;
  }else if(plotName.find("Z_pt_") != std::string::npos){
      return true;
  }else if(plotName.find("Lep_pt_1_") != std::string::npos){
      return true;
  }else if(plotName.find("Lep_pt_2_") != std::string::npos){
      return true;
  }else if(plotName.find("Lep_pt_3_") != std::string::npos){
      return true;
  }else if(plotName.find("Lep_pt_4_") != std::string::npos){
      return true;
  }else if(plotName.find("ptcone30") != std::string::npos){
      return true;
  }

  return false;
}

// int to tell us which signals to include
// 0 is none
// 1 is only single production
// 2 is only pair production (both)
// 3 is all of them
int plotLists::includeWhichSignals(std::string plotName){

  // if jet requirement hasn't been made then don't bother plotting signals
  if(plotName.find("2jets") == std::string::npos){
      return 0;   
  }
  // if 2 jet requirement is made but not 1B-tag inclusive then only plot pair production
  else if(plotName.find("1BIncl") == std::string::npos){
      return 2;
  }
  // now see if pertains to single production (includes Fwd or fwd)
  else if(plotName.find("fwd") != std::string::npos){
      return 3;
  }else if(plotName.find("Fwd") != std::string::npos){
      return 3;
  }
  else 
    return 2;

}


// returns the name of a sample to be used in the legend
std::string plotLists::GetLegendName(std::string name){
   std::string tmp = "";
   
   if(name.find("Triboson") != std::string::npos){
     return "Triboson";
   }else if(name.find("ttbarX") != std::string::npos){
     return "t#bar{t}+X";
   }else if(name.find("TZ") != std::string::npos){
     return "tZ";
   }else if(name.find("SingleTop") != std::string::npos){
     return "Single Top";
   }else if(name.find("Diboson_Sherpa") != std::string::npos){
     return "Diboson";
   }else if(name.find("WW_Sherpa") != std::string::npos){
     return "WW";
   }else if(name.find("WZ_Sherpa") != std::string::npos){
     return "WZ";
   }else if(name.find("ZZ_Sherpa") != std::string::npos){
     return "ZZ";
   }else if(name.find("ttbar") != std::string::npos){
     return "t#bar{t}";
   }else if(name.find("WJets") != std::string::npos){
     return "W+jets";
   }else if(name.find("Zjets_Sherpa_cc") != std::string::npos){
     return "Z+charm";
   }else if(name.find("Zjets_Sherpa_bb") != std::string::npos){
     return "Z+bottom";
   }else if(name.find("Zjets_Sherpa_light") != std::string::npos){
     return "Z+light";
   }

   return tmp;
}


// produces a vector with the names of all the cuts/categories
// that I want a yield table for
std::vector<std::string> plotLists::cutsForYields(void){
   std::vector<std::string> cuts;
   std::vector<std::string> cats;
   std::vector<std::string> jets;
   std::vector<std::string> btags;
   std::vector<std::string> tmp;
   
   cuts.push_back("_1_afterZMass_");
   cuts.push_back("_2_afterZpt_");
   //   cuts.push_back("_3_afterTight");  
   //   cuts.push_back("_4_afterHt");
   
   cats.push_back("");
   //cats.push_back("_tight_");
   //cats.push_back("_2lep_");
   //cats.push_back("_3lep_");
   cats.push_back("_iso_");
   cats.push_back("_fwd_");
   //cats.push_back("_fakes_");

   for(int n=0; n<plotLists::CATEGORIES.size(); n++){
     btags.push_back("_"+plotLists::CATEGORIES[n]);
   }
   btags.push_back("");
   
   for(int p=0; p<plotLists::JETCATEGORIES.size(); p++){
     jets.push_back("_"+plotLists::JETCATEGORIES[p]);
   }
   jets.push_back("");

   bool skip=false;

   for(int i=0; i<cuts.size(); i++){
     for(int j=0; j<cats.size(); j++){
       for(int k=0; k<btags.size(); k++){
	 for(int m=0; m<jets.size(); m++){
	   if((btags[k]!="" && jets[m]!="") || btags[k]==""){
	     skip = false;
	     if(k==1 && m==0) skip = true; // skip 1 b-tag but 0 jets
	     if(k==2 && m==0) skip = true; // skip >=1 b-tag but 0 jets
	     if(k==3 && m==0) skip = true; // skip >=2 b-tags but 0 jets
	     if(k==2 && m==1) skip = true; // skip >=2 b-tags but 1 jet
	     if(!skip)
	       tmp.push_back(cuts[i]+cats[j]+btags[k]+jets[m]);
	     
	   }
	 }
       }
     }
   }

   return tmp;
}
const std::vector<std::string> plotLists::CUTSFORYIELDS = plotLists::cutsForYields();


std::string plotLists::getFilenameForSyst(std::string syst, std::string file){
  std::string tmp="data/"+plotLists::DATE+"/"+file;

  if(syst.find("diboson_scale")!=std::string::npos){
    if(file.find("Sherpa_CT10_l")!=std::string::npos)
      tmp=tmp+"_"+syst+".root";
    else
      tmp=tmp+"_nominal.root";
  }else if(syst.find("jeff")!=std::string::npos){
    tmp=tmp+"_jeff.root";
    return tmp;
  }else if(syst.find("jer")!=std::string::npos){
    tmp=tmp+"_jer.root";
    return tmp;
  }else if(syst.find("lumi")!=std::string::npos){
    tmp=tmp+"_nominal.root";
    return tmp;
  }else if(syst.find("_XS_")!=std::string::npos){
    tmp=tmp+"_nominal.root";
    return tmp;
  }else{
    tmp=tmp+"_"+syst+".root";
    return tmp;
  }
}

bool plotLists::scaleForSyst(std::string syst, std::string file){
  
  if(syst.find("lumi")!=std::string::npos)
   return true;

  if(syst.find("_XS_")==std::string::npos)
    return false;
  
  else if(syst.find("ttbar_XS")!=std::string::npos && file=="PowhegPythia_P2011C_ttbar")
    return true;
  else if(syst.find("singleTopS_XS")!=std::string::npos && file.find("McAtNloJimmy_AUET2CT10_SingleTopSChan")!=std::string::npos)
    return true;
  else if(syst.find("singleTopT_XS")!=std::string::npos && file.find("AcerMCPythia_AUET2BCTEQ6L1_singletop_tchan")!=std::string::npos)
    return true;
  else if(syst.find("singleTopWt_XS")!=std::string::npos && file=="McAtNloJimmy_AUET2CT10_SingleTopWtChanIncl")
    return true;
  else if(syst.find("diboson_XS")!=std::string::npos && file.find("Sherpa_CT10_l")!=std::string::npos)
    return true;
  else if(syst.find("ttW_XS")!=std::string::npos && file.find("MadGraphPythia_AUET2BCTEQ6L1_ttbarW")!=std::string::npos)
    return true;
  else if(syst.find("ttZ_XS")!=std::string::npos && file.find("MadGraphPythia_AUET2BCTEQ6L1_ttbarZ")!=std::string::npos)
    return true;
  else if(syst.find("ttWW_XS")!=std::string::npos && file=="MadgraphPythia_AUET2B_CTEQ6L1_ttbarWW")
    return true;

  return false;
}
