#ifndef BACKGROUNDPREDICTIONS_H
#define BACKGROUNDPREDICTIONS_H

#include "TH1.h"
#include <vector>

//**********************************// 
// a class to provide histograms    //
// with predictions for total and   //
// individual backgrounds           //
//**********************************//

class backgroundPredictions{

   public:
  backgroundPredictions(std::string syst, std::vector<std::string> plotNames, std::string lepType="", bool rebin=false);
      ~backgroundPredictions();
      
      std::vector<TH1D*> GetTotalPredictions();
      std::vector<std::vector<TH1D*> > GetAllPredictions();
      std::vector<std::string> GetBackgroundNames();
      
   private:
      void Init(std::vector<std::string> plotNames, bool rebin);
      void CalculatePredictions(std::string syst, std::vector<std::string> plotNames, std::string lepType="");
      std::vector<TH1D*> m_totalPredictions;
      std::vector<std::vector<TH1D*> > m_allPredictions;
      std::vector<std::string> m_backgrounds;
      std::vector<std::string> m_fileNames;
      bool m_rebin;

};

#endif
