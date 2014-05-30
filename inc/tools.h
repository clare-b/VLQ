#ifndef TOOLS_H
#define TOOLS_H

#include <vector>
#include <string>
#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include "TGraphAsymmErrors.h"

class tools{

   private:
      
   public:
      
      static int setColor(std::string sample);
      static bool exists(std::string file_name);
      static TH1D* combineHists(TH1D* hist_1, TH1D* hist_2, double scale_1, double scale_2);
      static TH2D* combineHists2D(TH2D* hist_1, TH2D* hist_2, double scale_1, double scale_2);
      static void fixOverflow(TH1D* hist);
      static void checkRebin(std::string plotName, TH1D*& hist);
      static TGraphAsymmErrors* getRatio(TGraphAsymmErrors* errorsOnBackground, TH1D* totalBackground);
      static TH1D* getSymmetricOpposite(TH1D* someSyst, TH1D* nominal);
      static std::vector<double> getUpAndDownErrors(double input_up, double input_down, double nominal, std::string syst);
      static TH1D* getHistFromFile(TFile* file, std::string histName);
      static void sigFigs(double& num, double& error, int sigFigsOnError);
};

#endif
