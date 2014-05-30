#ifndef DRAWPLOTS_H
#define DRAWPLOTS_H

#include <TH1.h>
#include <TGraphAsymmErrors.h>
#include <TCanvas.h>
#include <string>

class drawPlots{

   public:
      static TCanvas* plot_MC_noRatio(std::vector<TH1D*> histos, std::vector<std::string> names, std::string axisName, TH1D* signal);
      static TCanvas* plot_MC_noRatio_wError(std::vector<TH1D*> histos, std::vector<std::string> names, std::string axisName, TH1D* signal, TH1D* errDown, TH1D* errUp);
      static TCanvas* plotAll(std::vector<TH1D*> histos, std::vector<std::string> names, std::string axisName, TH1D* signal, TH1D* data, TH1D* errDown, TH1D* errUp, bool isSignalRegion, bool doLogAxis);
      static TCanvas* plotAll_VLQ(std::vector<TH1D*> histos, std::vector<std::string> names, std::string axisName, std::vector<TH1D*> signal, TH1D* data, TGraphAsymmErrors* err, bool isSignalRegion, bool doLogAxis, std::string channel);
      static TCanvas* plotAll_VLQ_paperStyle(std::vector<TH1D*> histos, std::vector<std::string> names, std::string axisName, std::vector<TH1D*> signal, TH1D* data, TGraphAsymmErrors* err, bool doLogAxis, std::string extraTag);
      static TCanvas* plotShape_VLQ(TH1D* bkgd, std::string axisName, std::vector<TH1D*> signal, bool doLogAxis, std::string channel);

   private:
      static void GetAxisLimits(TH1D* graph,double& x_min,double& x_max,double& y_min,double& y_max);
      static void ATLAS_LABEL(Double_t x,Double_t y,Color_t color);
      static void myText(Double_t x,Double_t y,Color_t color,char *text);
      static void myText(Double_t x,Double_t y,Color_t color,Double_t size,char *text);
      static void correctForOverflows(TH1D* hist);

};

#endif
