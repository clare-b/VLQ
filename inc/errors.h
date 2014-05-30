#ifndef ERRORS_H
#define ERRORS_H

#include "TH1.h"
#include "TGraphAsymmErrors.h"
#include <vector>

//**********************************//
// a class to hold histograms which //
// provide the total uncertainties  //
// in each bin                      //
//**********************************//

class errors{

   public:
  errors(std::vector<std::vector<TH1D*> > allInputHists, bool includeSyst, std::vector<std::string> my_plotNames);
      ~errors();
      
      std::vector<TGraphAsymmErrors*> GetErrors();
      
   private:
      void Init();
      
      void CalculateHists(bool includeSyst, std::vector<std::string> my_plotNames);
      std::vector<std::vector<TH1D*> > m_allInputHists;
      std::vector<TGraphAsymmErrors*> m_err;

};

#endif
