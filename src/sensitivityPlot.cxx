#include "plotLists.h"
#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TLatex.h"
#include "AtlasStyle.h"

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

std::string returnDecay(int i);
void PrintPlot(TH1D* hist, std::string name);
void Print2DPlots(std::vector<TH2D*> hists, std::string channel);
double reweight(TH1D* histo, std::vector<float> w);

int main(int argc,char** argv){

  // TTS, 650 mass point
  std::string signal_channel=argv[1]; //"T" or "B"
  float Step=0.01;
  int nmix = (int)(1/Step + 1);

  std::vector<TH2D*> trianglePlots;


  for(int i=0; i<=10; i++){
    int mass=350+i*50;
    std::cout << "mass: " << mass << std::endl;
    std::stringstream convert;
    convert << mass;
    std::string massStr=convert.str();

    TFile* signal_file = TFile::Open(("data/March4/ProtosPythia_AUET2B_MSTW2008LO_"+signal_channel+signal_channel+"S_M"+massStr+"_1lep_nominal.root").c_str());

    TH1D* decayHist_regB_zPt = new TH1D("decayHist_regB_zPt", "decayHist_regB_zPt", 6, 0, 6);
    std::vector<TH1D*> histos_regB_zPt(6);
    
    for(int i=0; i<6; i++){
      histos_regB_zPt[i]= (TH1D*)signal_file->Get(("HT_regB__"+returnDecay(i)+"_").c_str());
      histos_regB_zPt[i]->Scale(plotLists::LUMI/plotLists::GetNormalization("ProtosPythia_AUET2B_MSTW2008LO_"+signal_channel+signal_channel+"S_M"+massStr+"_1lep","nominal"));
      decayHist_regB_zPt->SetBinContent(i+1,histos_regB_zPt[i]->Integral());
      std::cout << "decay: " << i << " and integral: " << histos_regB_zPt[i]->Integral() << std::endl;
    }
    
    int status = mkdir("output/pdf", S_IRWXU);
    status = mkdir("output/pdf/sensitivityPlots", S_IRWXU);
    PrintPlot(decayHist_regB_zPt, "output/pdf/sensitivityPlots/simpleHist_regB_ZPt_"+signal_channel+".pdf");
    
    // then make a triangle
    
    TH2D* trianglePlot;
    if(signal_channel=="B") trianglePlot= new TH2D("2dhist", ("VL"+signal_channel+"("+massStr+"); BR ("+signal_channel+"->Wt); BR("+signal_channel+"->Hb)").c_str(), nmix, 0, 1, nmix, 0, 1);
    else trianglePlot= new TH2D("2dhist", ("VL"+signal_channel+"("+massStr+"); BR ("+signal_channel+"->Wb); BR("+signal_channel+"->Ht)").c_str(), nmix, 0, 1, nmix, 0, 1);
    double Wbr=0;
    double Hbr=0;
    double Zbr=0;
    
    double nEvents;
    for(int j=0; j<nmix; j++){
      Wbr=j*Step;
      for(int k=0; k<nmix; k++){
	Hbr=k*Step;
	if(Wbr+Hbr > 1.) continue;
	Zbr=1-(Wbr+Hbr);
	std::vector<float> br;
	br.push_back(Zbr);
	br.push_back(Wbr);
	br.push_back(Hbr);
	nEvents= reweight(decayHist_regB_zPt, br);
	trianglePlot->SetBinContent(j, k, nEvents);
      }
    }
    
    trianglePlots.push_back(trianglePlot);


  } // end loop over signal masses

  Print2DPlots(trianglePlots, signal_channel);
  
}

std::string returnDecay(int i){
  if(i==0) return "ZZ";
  else if (i==1) return "WW";
  else if (i==2) return "ZW";
  else if (i==3) return "HH";
  else if (i==4) return "ZH";
  else if (i==5) return "WH";

  std::cout << "Error: int does not represent any decay!" << std::endl;
  return "";
}

void PrintPlot(TH1D* hist, std::string name){

  SetAtlasStyle();

  TCanvas* c1 = new TCanvas();
  for(int i=1; i<7; i++){
    hist->GetXaxis()->SetBinLabel(i, returnDecay(i-1).c_str());
    //std::cout << "bin content: " << hist->GetBinContent(i) << std::endl;
  }
  hist->DrawNormalized();
  c1->SaveAs(name.c_str());

}

void Print2DPlots(std::vector<TH2D*> hists, std::string channel){

  //SetAtlasStyle();
  TCanvas* c1 = new TCanvas("c","c",800,800);
  TCanvas* c2 = new TCanvas("c2","c2",800,800);
  c1->Divide(3,4,0.01,0.01);
  int less=0;
  for(int i=0; i<hists.size(); i++){
    int pad=i+1;
    c1->cd(pad);
    c1->GetPad(pad)->SetRightMargin(.15);
    c1->GetPad(pad)->SetTopMargin(.1);
    if(pad==3){
      TLatex* text = new TLatex(0.1,0.5,"Sensitivity to ");
      TLatex* text1 = new TLatex(0.1,0.35,(channel+channel+" production").c_str());
      //TLatex* text2 = new TLatex(0.1,0.2,label.c_str());
      text->SetTextAlign(11);
      text->SetTextSize(0.1);
      text1->SetTextAlign(11);
      text1->SetTextSize(0.1);
      //text2->SetTextAlign(11);
      //text2->SetTextSize(0.1);
      text->Draw();
      text1->Draw("same");
      //text2->Draw("same");
      less += 1;
    }else{
      hists[i-less]->SetContour(100);
      hists[i-less]->SetStats(0);
      std::string text3 = hists[i-less]->GetTitle();
      hists[i-less]->SetTitle(0);
      hists[i-less]->Draw("COLZ");
      std::cout << "title: " << text3 << std::endl;
      TLatex* text3_latex = new TLatex(0.2,1.05,text3.c_str());
      text3_latex->SetTextSize(.09);
      text3_latex->Draw("same");
    }

  }
  c1->cd();
  c1->SaveAs(("output/pdf/sensitivityPlots/signal_sensitivity_"+channel+".pdf").c_str());

}

double reweight(TH1D* histo, std::vector<float> w){

  double globalScaling=1./9.;

  double n=0;
  // ZZ
  n += histo->GetBinContent(1)*w[0]*w[0]/globalScaling;
  // WW
  n += histo->GetBinContent(2)*w[1]*w[1]/globalScaling;
  // ZW
  n += 2*histo->GetBinContent(3)*w[0]*w[1]/(2*globalScaling);
  // HH
  n += histo->GetBinContent(4)*w[2]*w[2]/globalScaling;
  // ZH
  n += 2*histo->GetBinContent(5)*w[0]*w[2]/(2*globalScaling);
  // WH
  n += 2*histo->GetBinContent(6)*w[1]*w[2]/(2*globalScaling);
  
  return n;
}
