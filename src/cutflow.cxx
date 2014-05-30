#include "tools.h"
#include "plotLists.h"
#include "AtlasStyle.h"

#include "TH1.h"
#include "TFile.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <iomanip>

int main() {

   std::cout << "********** Starting Cutflow **********" << std::endl;

   bool normalize = true; // if you want it rescaled to expected events
   bool doDetailed = false; // if you want the cutflow for each file, not grouped into categories
   std::string suffix = "_2jets"; // change if you want cutflow for a control region or more specific channel

   TH1::AddDirectory(kFALSE); // global variable to detach histograms from files so files can be closed

   std::vector<std::vector<TH1D*> > histograms;
   histograms.resize( plotLists::CATEGORIES.size());
   std::vector<std::string> processName;
   
   // make directories for output to go into
   int status = mkdir("output/tex/", S_IRWXU);
   for(int w=0; w<plotLists::CATEGORIES.size(); w++){
      std::string dirName = "output/tex/"+plotLists::CATEGORIES[w];
      status = mkdir(dirName.c_str(), S_IRWXU);
   }
   
   // run over files + signal + data
   std::vector<std::string> allFilenames;// = plotLists::FILENAMES;
   allFilenames.push_back("data");
   allFilenames.push_back("MadGraphPythia8_AU2CTEQ6L1_TsZt_M650L2_Pt20");
   allFilenames.push_back("ProtosPythia_AUET2B_MSTW2008LO_BBS_M650_1lep");
   allFilenames.push_back("ProtosPythia_AUET2B_MSTW2008LO_TTS_M650_1lep");
   for (int z=0; z<plotLists::FILENAMES.size(); z++){
     allFilenames.push_back(plotLists::FILENAMES[z]);
   }
   std::vector<std::string> namesForCutflow;

   for (int a=0; a<allFilenames.size(); a++){
      std::string name="data/"+plotLists::DATE+"/"+allFilenames[a]+"_nominal.root";
      // check if file exists (if it's not there then none passed and it doesn't need to be included)
      if(!tools::exists(name))
         std::cout << "Skipping file: " << allFilenames[a] << std::endl;
      else{
         TFile* file = TFile::Open(name.c_str());

	 if(allFilenames[a]=="ProtosPythia_AUET2B_MSTW2008LO_TTS_M650_1lep"){
	   TH1D* runNumHist=(TH1D*)file->Get("runNumber_pretag");
	   std::cout << "integral of run hist: " << runNumHist->Integral()*plotLists::LUMI/plotLists::GetNormalization(allFilenames[a], "nominal") << std::endl;
	   std::cout << "Normalization: " << plotLists::LUMI/plotLists::GetNormalization(allFilenames[a], "nominal");
	 }

	 bool hist_found = false;
            
         std::string process = plotLists::GetFileSampleName(allFilenames[a]);
	 if(process=="")
	   process = "data";
	 if(process=="WZ_Sherpa" || process=="ZZ_Sherpa" || process=="WW_Sherpa")
	   process = "Sherpa_Diboson";
         int numForSample=-1;
         if(!doDetailed){
	   for (int c=0; c<processName.size(); c++){
	     if(process == processName[c])
	       numForSample = c;
	   } // loop over process names
	 }

	 double norm_factor = 1.;
	 if(normalize){
	   if(allFilenames[a].find("data") == std::string::npos){ // if it's data then don't normalize
	     // get number of events in the file (to use for normalization)
	     norm_factor = plotLists::LUMI/plotLists::GetNormalization(allFilenames[a], "nominal");
	     std::cout << "file: "<< allFilenames[a] <<" norm factor: " << norm_factor << std::endl;
	   }
	 }
	 
         for(int b=0; b<plotLists::CATEGORIES.size(); b++){
	   if(plotLists::CATEGORIES[b]!="1BIncl"){
	     std::string cutflowName;
	     if(normalize)
	       cutflowName="cutflow_"+plotLists::CATEGORIES[b]+suffix;
	     else
	       cutflowName="cutflow_unw_"+plotLists::CATEGORIES[b]+suffix;
	     TH1D* hist = (TH1D*)file->Get(cutflowName.c_str());
	     if(hist){
	       hist_found = true;
               if(normalize)
		 hist->Scale(norm_factor);
               TH1D* hist_2 = (TH1D*)hist->Clone();
	       
               if(numForSample==-1) // means no entry for this sample has been created yet
		 histograms[b].push_back(hist_2);
               else // entry has been created, so add to existing entry
		 histograms[b][numForSample]->Add(hist_2);
	     } // end if(hist)
	     else
	       std::cout << "cutflow histogram not found" << std::endl;
	   }// end check to see if this is 1BIncl
         } // end loop over plots
         if(numForSample==-1 && hist_found){
	   processName.push_back(process);
	   namesForCutflow.push_back(allFilenames[a]);
	   std::cout << "adding process: " << process << std::endl;
	 }
	 delete file;
      } // end check to see if file exists
   } // end loop over files

   // construct histograms for 1bIncl by adding 1bexcl and 2bincl
   for(int s=0; s<histograms[0].size(); s++){
     TH1D* temp = (TH1D*)histograms[1][s]->Clone("new_1bexcl");
     histograms[3].push_back(temp); // add 1bexcl
     for(int r=0; r<histograms[2][s]->GetNbinsX(); r++){
       if(r>5){
	 double binContent=histograms[2][s]->GetBinContent(r);
	 binContent+=histograms[1][s]->GetBinContent(r); // add 2bincl for relevant bins
	 histograms[3][s]->SetBinContent(r,binContent);
       } // end if it's cut 5 or greater
     } // end loop over bins
   } // end loop over histograms for different files

   int cutPrecision=2;
   for(int c=0; c<histograms[0].size(); c++){ // should be a loop over all files
      for (int d=0; d<plotLists::CATEGORIES.size(); d++){
         ofstream myfile;
         std::string outputName = "";
         if(doDetailed)outputName="output/tex/"+plotLists::CATEGORIES[d]+"/cutflow_"+namesForCutflow[c]+".tex";
	 else outputName="output/tex/"+plotLists::CATEGORIES[d]+"/cutflow_"+processName[c]+".tex";

	 std::cout << "outputing cutflow for: " << outputName << std::endl;
	 myfile.open(outputName.c_str());
         myfile << "\\begin{table}[htdp] \n";
         myfile << "\\begin{center} \n";
         myfile << "\\footnotesize \n";
         myfile << "\\begin{tabular}{|c|c|c|c|c|} \n";
         myfile << "& Expected Events & Cumulative \\% & Relative \\% \\\\ \n";
         myfile << "\\hline \n";
         for(int e=1; e<=plotLists::CUTNAMES.size()-1; e++){
           if(e==1)
	     myfile << plotLists::CUTNAMES[e-1] << " & " << std::setprecision(cutPrecision) << std::fixed << histograms[d][c]->GetBinContent(e) << "$\\pm$" << histograms[d][c]->GetBinError(e) << " & 100.0 & 100.0 \\\\ \n";
           else if(histograms[d][c]->GetBinContent(e)==0)
              myfile << plotLists::CUTNAMES[e-1] << " & " << histograms[d][c]->GetBinContent(e) << "$\\pm$" << histograms[d][c]->GetBinError(e) << " & " << histograms[d][c]->GetBinContent(e)*100.0/histograms[d][c]->GetBinContent(1) << " & 0.0 " << "\\\\ \n";
           else
	     myfile << plotLists::CUTNAMES[e-1] << " & "<< std::setprecision(cutPrecision) << std::fixed << histograms[d][c]->GetBinContent(e) << "$\\pm$" << histograms[d][c]->GetBinError(e) << " & " << histograms[d][c]->GetBinContent(e)*100.0/histograms[d][c]->GetBinContent(1) << " & " << histograms[d][c]->GetBinContent(e)*100.0/histograms[d][c]->GetBinContent(e-1) << "\\\\ \n";
          }
         myfile << "\\end{tabular} \n";
         myfile << "\\end{center} \n";
         myfile << "\\end{table} \n";
         myfile.close();
      }
   } // end loops creating cutflow output
   
   for(int ntags=0; ntags<=2; ntags++){
     // now create csv file for output to excel spreadsheet
     ofstream csv_file;
     std::string csv_name="";
     if(ntags==0)
       csv_name="output/cutflow_0tags.csv";
     else if(ntags==1)
       csv_name="output/cutflow_1tag.csv";
     else if(ntags==2)
       csv_name="output/cutflow_2tags.csv";
     
     csv_file.open(csv_name.c_str());
     
     for(int m=0; m<histograms[0].size(); m++){
       csv_file << processName[m] << ',';
       for(int n=1; n<=5; n++){
	   csv_file << histograms[ntags][m]->GetBinContent(n) << ","; // here print cuts after requiring 2 b-tagged jets
       }
       csv_file << " \n";
     }
     
     csv_file.close();
   }
   
   std::cout << "********** Done! **********" << std::endl;
}
