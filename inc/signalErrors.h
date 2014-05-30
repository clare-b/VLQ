#ifndef SIGNALERRORS_H
#define SIGNALERRORS_H

#include "TH1.h"
#include <vector>

//**********************************//
// a class to get the total         //
// systematic error up and down     //
// for a particular signal in a     //
// particular histogram             //
//**********************************//

class signalErrors{
  
 public:
  signalErrors(std::string signal, std::string plotName, TH1D* nominalHist);
  ~signalErrors();
  
  double GetSystUp();
  double GetSystDown();
  
 private:
  void Init();
  
  void CalculateSyst(std::string signal, std::string plotName, TH1D* nominalHist);
  double m_systUp;
  double m_systDown;
  
};

#endif
