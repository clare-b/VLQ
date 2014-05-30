#include "drawPlots.h"
#include "tools.h"
#include "plotLists.h"
#include "AtlasStyle.h"

#include <TAxis.h>
#include <TLatex.h>
#include <TLegend.h>
#include <THStack.h>
#include <TF1.h>

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cmath>
#include <sstream>

// *************************************** //
// this is a function that takes a set of  //
// histograms and draws them on a canvas   //
// in a stack, returning the canvas        //
// just for MC- no ratio and no data       //
// *************************************** //
TCanvas* drawPlots::plot_MC_noRatio(std::vector<TH1D*> histos, std::vector<std::string> names, std::string axisName, TH1D* signal){

   const unsigned int MAINPLOT_WIDTH  = 800;
   const unsigned int MAINPLOT_HEIGHT = 600;
   
   TCanvas* canvas = new TCanvas("canvas","canvas",0,0,MAINPLOT_WIDTH,MAINPLOT_HEIGHT);
   
   canvas->SetMargin(0.,0.,0.,0.);
   canvas->cd();
   
   double main_y_max = -99;
   double main_y_min = -99;
   double main_x_max = -99;
   double main_x_min = -99;
   for(int a=0; a<histos.size(); a++){
      GetAxisLimits(histos[a], main_x_min, main_x_max, main_y_min, main_y_max);
   }

   // create main pad
   TPad* mainPad = new TPad("main","main",0.,0.,1.,1.);
   mainPad->SetMargin(0.15,0.05,0.15,.05); // left, right, bottom, top
   mainPad->Draw();
   mainPad->cd();
   
   SetAtlasStyle();

   THStack* Stack = new THStack();
   // fix title here, also not showing on plot
   std::string title = std::string(";") + axisName + ";Events";
   Stack->SetTitle(title.c_str());
   
   for(int b=0; b<histos.size(); b++){
      histos[b]->SetLineColor(1);
      histos[b]->SetFillColor(tools::setColor(names[b]));
      Stack->Add(histos[b]);
   }
   
   main_y_max = main_y_max*1.5;
   Stack->SetMaximum(main_y_max);
   Stack->Draw("hist");
   
   signal->SetLineStyle(2);
   signal->SetLineColor(kRed);
   signal->Draw("hist same");
   
   TLegend* leg = new TLegend(0.7,0.7,0.93,0.9);
   leg->SetFillStyle(0);
   leg->SetBorderSize(0);
   
   leg->AddEntry(signal, "Signal", "f");
   for(int q=0; q<names.size(); q++){
      leg->AddEntry(histos[q], names[q].c_str(), "f");
   }
   leg->Draw("lpe");
   
   ATLAS_LABEL(0.2,0.8,1);
   char text[]="#sqrt{s}=8 TeV";
   myText(0.2,0.7,1,text);
   
   return canvas;
}

// *************************************** //
// this is a function that takes a set of  //
// histograms and draws them on a canvas   //
// in a stack, returning the canvas        //
// just for MC- no ratio and no data.      //
// This one includes syst and stat or just //
// stat errors                             //
// *************************************** //
TCanvas* drawPlots::plot_MC_noRatio_wError(std::vector<TH1D*> histos, std::vector<std::string> names, std::string axisName, TH1D* signal, TH1D* errDown, TH1D* errUp){

   const unsigned int MAINPLOT_WIDTH  = 800;
   const unsigned int MAINPLOT_HEIGHT = 600;
   
   
   TCanvas* canvas = new TCanvas("canvas","canvas",0,0,MAINPLOT_WIDTH,MAINPLOT_HEIGHT);
   
   canvas->SetMargin(0.,0.,0.,0.);
   canvas->cd();
   
   double main_y_max = -99;
   double main_y_min = -99;
   double main_x_max = -99;
   double main_x_min = -99;
   for(int a=0; a<histos.size(); a++){
      GetAxisLimits(histos[a], main_x_min, main_x_max, main_y_min, main_y_max);
   }
   
   // create main pad
   TPad* mainPad = new TPad("main","main",0.,0.,1.,1.);
   mainPad->SetMargin(0.15,0.05,0.15,.05); // left, right, bottom, top
   mainPad->Draw();
   mainPad->cd();
   
   SetAtlasStyle();

   THStack* Stack = new THStack();
   std::string title = std::string(";") + axisName + ";Events";
   Stack->SetTitle(title.c_str());
   
   for(int b=0; b<histos.size(); b++){
      histos[b]->SetLineColor(1);
      histos[b]->SetFillColor(tools::setColor(names[b]));
      Stack->Add(histos[b]);
   }
   
   main_y_max = main_y_max*1.5;
   Stack->SetMaximum(main_y_max);
   Stack->Draw("hist");
   
   errDown->Draw("E2same");
   errUp->Draw("E2Same");
   
   signal->SetLineStyle(2);
   signal->SetLineColor(kRed);
   signal->Draw("hist same");
   
   TLegend* leg = new TLegend(0.7,0.7,0.93,0.9);
   leg->SetFillStyle(0);
   leg->SetBorderSize(0);
   leg->AddEntry(signal, "Signal", "f");
   for(int q=0; q<histos.size(); q++){
      leg->AddEntry(histos[q], names[q].c_str(), "f");
   }
   leg->Draw("lpe");
   
   ATLAS_LABEL(0.2,0.85,1);
   char text[]="#sqrt{s}=8 TeV";
   myText(0.2,0.7,1,text);
   
   return canvas;
}


// *************************************** //
// this is a function that takes a set of  //
// histograms and draws them on a canvas   //
// in a stack, returning the canvas.       //
// It also plots the signal as a dashed    //
// line and the data with a ratio at the   //
// the bottom of data/allBackgrounds       //
// *************************************** //
TCanvas* drawPlots::plotAll(std::vector<TH1D*> histos, std::vector<std::string> names, std::string axisName, TH1D* signal, TH1D* data, TH1D* errDown, TH1D* errUp, bool isSignalRegion, bool doLogAxis){
         
   const unsigned int MAINPLOT_WIDTH  = 800;
   const unsigned int MAINPLOT_HEIGHT = 500;
   const unsigned int RATIOPLOT_HEIGHT = 125;
   const unsigned int OFFSET = 10;
   
  //LUKE: Title offset and title size are proportional in 
  //root, so you have to change them together, 
  //this makes me a sad panda 
   const double RATIOPLOT_YAXIS_TITLE_OFFSET = 0.4;
   const double RATIOPLOT_YAXIS_TITLE_SIZE   = 0.14;
   const double RATIOPLOT_YAXIS_TICK_LENGTH  = 0.01;
   const double RATIOPLOT_YAXIS_LABEL_SIZE   = 0.15;
  
   const double RATIOPLOT_XAXIS_TITLE_OFFSET = 0.9;
   const double RATIOPLOT_XAXIS_TITLE_SIZE   = 0.2;
   const double RATIOPLOT_XAXIS_TICK_LENGTH  = 0.05;
   const double RATIOPLOT_XAXIS_LABEL_SIZE   = 0.2;
   
   const double CANVAS_HEIGHT = MAINPLOT_HEIGHT+RATIOPLOT_HEIGHT+OFFSET;
   
   TCanvas* canvas = new TCanvas("canvas","canvas",0,0,MAINPLOT_WIDTH,CANVAS_HEIGHT);
   
   canvas->SetMargin(0.,0.,0.,0.);
   canvas->Clear();
   canvas->cd();
   
   // create main pad
   const double mainPad_ylow  = (CANVAS_HEIGHT - MAINPLOT_HEIGHT)/CANVAS_HEIGHT;
   const double mainPad_yhigh = 1.;
  
   const double pad_margin_left = 0.15;
   const double pad_margin_right  = 0.05;
   
   double main_y_max = -99;
   double main_y_min = -99;
   double main_x_max = -99;
   double main_x_min = -99;
   for(int a=0; a<histos.size(); a++){
     if(histos[a]){
       if(histos[a]->GetEntries()> 0.){
	 GetAxisLimits(histos[a], main_x_min, main_x_max, main_y_min, main_y_max);
       }
     }
   }
   
   if(data){
     int maxBinData = data->GetMaximumBin();
     double dataYmax = data->GetBinContent(maxBinData);
     if(dataYmax > main_y_max) main_y_max = dataYmax;
   }

   // create main pad
   TPad* mainPad = new TPad("main","main",0.,mainPad_ylow,1.,mainPad_yhigh);
   mainPad->SetMargin(pad_margin_left,pad_margin_right,0.,.05); // left, right, bottom, top
   mainPad->Draw();
   mainPad->cd();

   SetAtlasStyle();

   if(doLogAxis)mainPad->SetLogy();

   // clone signal or data because one should exist for each plot
   TH1D* allBackgrounds;
   bool gotSomething=false;
   if(signal)
     allBackgrounds = (TH1D*)signal->Clone("all_backgrounds");
   else if(data)
     allBackgrounds = (TH1D*)data->Clone("all_backgrounds");
   else{
     for(int b=0; b<histos.size(); b++){
       if(histos[b] && !gotSomething){
	 allBackgrounds = (TH1D*)histos[b]->Clone("all_backgrounds");
	 gotSomething=true;
       }
     }  
   }

   // set all bins to zero so that you can add all histograms to it
   for(int i=0; i<=allBackgrounds->GetNbinsX()+1; i++)
      allBackgrounds->SetBinContent(i,0);
   
   THStack* Stack = new THStack();
   std::string title = std::string(";") + axisName + ";Events";
   Stack->SetTitle(title.c_str());
   
   for(int b=0; b<histos.size(); b++){
     if(histos[b]){
       if(histos[b]->GetEntries() > 0.){
	 histos[b]->SetLineColor(1);
	 histos[b]->SetFillColor(tools::setColor(names[b]));
	 Stack->Add(histos[b]);
	 for(int j=0; j<=allBackgrounds->GetNbinsX()+1; j++){
	   double binContent=allBackgrounds->GetBinContent(j)+histos[b]->GetBinContent(j);
	   allBackgrounds->SetBinContent(j,binContent);
	 }
       }
     }
   }

   TH1D* backgroundsForRatio = (TH1D*)allBackgrounds->Clone("bkgds_for_ratio");
   
   if(doLogAxis){ 
     main_y_max = main_y_max*10;
     if(Stack->GetMinimum()>.1) Stack->SetMinimum(.1);
   }
   else main_y_max = main_y_max*1.4;
   Stack->SetMaximum(main_y_max);
   Stack->Draw("hist");
   errDown->Draw("E2same");
   errUp->Draw("E2Same");
   
   // make error histograms for the ratio plot
   TH1D* ratioErrDown = (TH1D*) errDown->Clone("ratio_error_down");
   TH1D* ratioErrUp = (TH1D*) errUp->Clone("ratio_error_up");

   ratioErrDown->Divide(backgroundsForRatio);
   ratioErrUp->Divide(backgroundsForRatio);

   if(!isSignalRegion && data)
      data->Draw("e same");

   if(signal){
     signal->SetLineStyle(2);
     signal->SetLineColor(kRed);
     signal->SetLineWidth(5);
     signal->Draw("hist same");
   }

   TLegend* leg = new TLegend(0.8,0.65,0.95,0.9);
   leg->SetFillStyle(0);
   leg->SetBorderSize(0);
   
   if(signal)
     leg->AddEntry(signal, "Signal", "l");
   if(!isSignalRegion)
     leg->AddEntry(data, "Data");
   for(int q=0; q<histos.size(); q++){
     if(histos[q]){
       if(histos[q]->GetEntries() > 0.)
	 leg->AddEntry(histos[q], names[q].c_str(), "f");
     }
   }
   leg->Draw("lpe");
   
   ATLAS_LABEL(0.2,0.85,1);
   char text[]="#sqrt{s}=8 TeV";
   myText(0.6,0.75,1,.04, text);
   char text_L[]="#int L dt = 20.3 fb^{-1}";
   myText(0.6, 0.85,1, .04, text_L);
   char inText[100];
   int t;
   if(signal)
     t=sprintf(inText, "N_{Signal} = %.1f", signal->Integral(0,signal->GetNbinsX()+1));
   else
     t=sprintf(inText, "N_{Signal} = %.1f", 0.);
   myText(0.2, 0.75, 1, .04, inText);
   double back_int = allBackgrounds->Integral(0,allBackgrounds->GetNbinsX()+1);
   if(back_int > 10000.)
     t=sprintf(inText, "N_{Bkgd} = %.3e", back_int);
   else
     t=sprintf(inText, "N_{Bkgd} = %.1f", back_int);
   myText(0.2, 0.7, 1, .04, inText);
   if(!isSignalRegion){
     if(data)
       t=sprintf(inText, "N_{Data} = %.0f", data->Integral(0,data->GetNbinsX()+1));
     else
       t=sprintf(inText, "N_{Data} = %.0f", 0.);
     myText(0.2, 0.65, 1, .04, inText);
   }
   
   canvas->cd();
   
   // Draw Ratio plot
   double ratio_max = 1.6;
   double ratio_min = 0.4;
  
   const double x_axis_size = 0.4;
   const double ratio_pad_height = RATIOPLOT_HEIGHT/CANVAS_HEIGHT;
   
   double pad_xlow = 0.,pad_xhigh = 1.;
   double pad_ylow = OFFSET/CANVAS_HEIGHT,pad_yhigh = ratio_pad_height;
   
   const char* pad_name = "pad";
   TPad* pad4ratio = new TPad(pad_name,pad_name,pad_xlow,pad_ylow,pad_xhigh,pad_yhigh);
   
   pad4ratio->SetMargin(pad_margin_left,pad_margin_right,x_axis_size,0.);
   pad4ratio->Draw();
   pad4ratio->cd();

   TH1D* ratioPlot;
   if(data) ratioPlot = (TH1D*)data->Clone("ratio");
   else if(signal) ratioPlot = (TH1D*)signal->Clone("ratio"); // if there is no data then use signal for axis 
   ratioPlot->SetTitle("");
   ratioPlot->GetYaxis()->SetTitle("Data/MC");
   ratioPlot->GetXaxis()->SetTitle(axisName.c_str());
   if(data){
     ratioPlot->Divide(allBackgrounds);
     // here change ratio_min and ratio_max if the ratio plot is quite flat
     double maxDeviation=0;
     double tempDev=0;
     for(int ibin=1; ibin<=allBackgrounds->GetNbinsX(); ibin++){
       tempDev = std::abs(ratioPlot->GetBinContent(ibin)-1.);
       if(tempDev > maxDeviation) maxDeviation = tempDev;
     }
     if(maxDeviation < 0.1){
       ratio_max = 1.12;
       ratio_min = 0.88;
     }
     else if(maxDeviation < 0.2){
       ratio_max = 1.25;
       ratio_min = .75;
     }
     ratioPlot->SetMinimum(ratio_min);
     ratioPlot->SetMaximum(ratio_max);
   }
   ratioPlot->GetXaxis()->SetLabelSize(RATIOPLOT_XAXIS_LABEL_SIZE);
   ratioPlot->GetXaxis()->SetTickLength(RATIOPLOT_XAXIS_TICK_LENGTH);
   ratioPlot->GetYaxis()->SetLabelSize(RATIOPLOT_YAXIS_LABEL_SIZE);
   ratioPlot->GetYaxis()->SetTickLength(RATIOPLOT_YAXIS_TICK_LENGTH);
   ratioPlot->GetYaxis()->SetNdivisions(3);
   ratioPlot->GetYaxis()->SetTitleSize(RATIOPLOT_YAXIS_TITLE_SIZE);
   ratioPlot->GetYaxis()->SetTitleOffset(RATIOPLOT_YAXIS_TITLE_OFFSET);
   ratioPlot->GetXaxis()->SetTitleSize(RATIOPLOT_XAXIS_TITLE_SIZE);
   ratioPlot->GetXaxis()->SetTitleOffset(RATIOPLOT_XAXIS_TITLE_OFFSET);
   
   if(!isSignalRegion && data){
     ratioPlot->Draw();
     // plot horizontal line at y=1
     TF1* horizontal = new TF1("horizontal","pol1",-10000,10000);
     horizontal->SetParameter(0,1.);
     horizontal->SetParameter(1,0.);
     horizontal->SetLineColor(kBlack);
     horizontal->SetLineStyle(2);
     horizontal->SetLineWidth(1);
     horizontal->Draw("same");
     ratioErrDown->Draw("E2same");
     ratioErrUp->Draw("E2same");
   }else if(isSignalRegion){
     ratioPlot->Draw("axis");
     char text[]="DATA IS BLINDED HERE";
     myText(0.4,0.6,1,.2,text);
   }else{ // in this case it is not the signal region but there is no data
     ratioPlot->Draw("axis");
     char text[]="NO DATA";
     myText(0.4,0.6,1,.2,text);
     }
   
   return canvas;
}

// *************************************** //
// this is a function that takes a set of  //
// histograms and draws them on a canvas   //
// in a stack, returning the canvas.       //
// It also plots the signal as a dashed    //
// line and the data with a ratio at the   //
// the bottom of data/allBackgrounds       //
//                                         //
// This one is supposed to match the plots //
// make by JP                              //
// *************************************** //
TCanvas* drawPlots::plotAll_VLQ(std::vector<TH1D*> histos, std::vector<std::string> names, std::string axisName, std::vector<TH1D*> signal, TH1D* data, TGraphAsymmErrors* err, bool isSignalRegion, bool doLogAxis, std::string channel){

   const unsigned int CANVAS_WIDTH  = 720;
   const unsigned int CANVAS_HEIGHT = 750;
   const double RATIOPLOT_YAXIS_TITLE_OFFSET = 0.75;
   const double RATIOPLOT_YAXIS_TITLE_SIZE   = 0.11;
   const double RATIOPLOT_YAXIS_LABEL_SIZE   = 0.09;  
   const double RATIOPLOT_XAXIS_TITLE_OFFSET = 1.6;
   const double RATIOPLOT_XAXIS_TITLE_SIZE   = 0.11;
   const double RATIOPLOT_XAXIS_LABEL_SIZE   = 0.09;
   
   TCanvas* canvas = new TCanvas("canvas","canvas",0,0,CANVAS_WIDTH,CANVAS_HEIGHT);
   
   canvas->SetMargin(0.,0.,0.,0.);
   canvas->Clear();
   canvas->cd();
   
   SetAtlasStyle();
   gStyle->SetHistLineWidth(1.);
   
   // create histogram to store all backgrounds
   TH1D* allBackgrounds;
   bool gotSomething=false;
   for(int c=0; c<signal.size(); c++){
     if(signal[c]){
       allBackgrounds = (TH1D*)signal[c]->Clone("all_backgrounds");
       gotSomething=true;
     }
   }
   if(!gotSomething && data)
     allBackgrounds = (TH1D*)data->Clone("all_backgrounds");
   if(!gotSomething){
     for(int b=0; b<histos.size(); b++){
       if(histos[b] && !gotSomething){
	 allBackgrounds = (TH1D*)histos[b]->Clone("all_backgrounds");
	 gotSomething=true;
       }
     }
   }

   if(!gotSomething){
     std::cout << "Error: could not find any background, data, or signal for this plot" << std::endl;
     return canvas;
   }
   
   // set all bins to zero
   for(int i=0; i<=allBackgrounds->GetNbinsX()+1; i++)
     allBackgrounds->SetBinContent(i,0);
   
   // make stack of backgrounds, fill backgrounds histogram
   THStack* Stack = new THStack();
   for(int b=0; b<histos.size(); b++){
     if(histos[b]){
       if(histos[b]->GetEntries() > 0.){
	 histos[b]->SetLineColor(1);
	 histos[b]->SetFillColor(tools::setColor(names[b]));
	 Stack->Add(histos[b]);
	 for(int j=0; j<=allBackgrounds->GetNbinsX()+1; j++){
	   double binContent=allBackgrounds->GetBinContent(j)+histos[b]->GetBinContent(j);
	   allBackgrounds->SetBinContent(j,binContent);
	 }
       }
     }
   } 
   TH1D* backgroundsForRatio = (TH1D*)allBackgrounds->Clone("bkgds_for_ratio");

   // create main pad
   const double mainPad_ylow  = 0.3;
   const double mainPad_yhigh = 0.95;
   const double mainPad_xlow = 0.;
   const double mainPad_xhigh = 0.95;
   const double pad_margin_left = 0.2;
   const double pad_margin_right  = 0.02;
   double main_y_max = -99;
   double main_y_min = -99;

   TPad* mainPad = new TPad("main","main",mainPad_xlow,mainPad_ylow,mainPad_xhigh,mainPad_yhigh);
   mainPad->SetMargin(pad_margin_left,pad_margin_right,0.,.05); // left, right, bottom, top
   mainPad->Draw();
   mainPad->cd();

   // find max y
   if(allBackgrounds){
     int maxBinBkg = allBackgrounds->GetMaximumBin();
     double bkgYmax = allBackgrounds->GetBinContent(maxBinBkg);
     main_y_max = bkgYmax;
   }
   if(data){
     int maxBinData = data->GetMaximumBin();
     double dataYmax = data->GetBinContent(maxBinData);
     if(dataYmax > main_y_max) main_y_max = dataYmax;
   }
   if(main_y_max < .002) main_y_max = .1;

   if(doLogAxis){
     mainPad->SetLogy();
     main_y_min = 0.007;
     main_y_max*=600;
   }else{
     main_y_min = 0.001;
     main_y_max*=1.6;
   }

   // draw axis
   allBackgrounds->SetMaximum(main_y_max);
   allBackgrounds->SetMinimum(main_y_min);
   std::string title = std::string(";") + axisName + ";events/bin";
   allBackgrounds->SetTitle(title.c_str());   
   allBackgrounds->GetYaxis()->SetTitleOffset(1.4);
   allBackgrounds->GetYaxis()->SetTitleSize(0.06);
   allBackgrounds->GetXaxis()->SetTitleOffset(1.2);
   allBackgrounds->GetYaxis()->SetLabelSize(0.04);
   allBackgrounds->GetXaxis()->SetLabelSize(0.04);
   allBackgrounds->Draw("hist");

   // draw stack with error
   Stack->Draw("hist same");
   if(err){
     err->Draw("E2 same");
   }

   // make error histograms for the ratio plot
   TGraphAsymmErrors* ratioErr = tools::getRatio(err, backgroundsForRatio);

   // draw data
   if(!isSignalRegion && data)
      data->Draw("e same");

   // draw signal
   if(signal[0]){
     signal[0]->SetLineStyle(2);
     signal[0]->SetLineColor(kRed);
     signal[0]->SetLineWidth(5);
     signal[0]->Draw("hist same");
   }
   if(signal.size() > 1){
     if(signal[1]){
       signal[1]->SetLineStyle(2);
       signal[1]->SetLineColor(kOrange+7);
       signal[1]->SetLineWidth(4);
       signal[1]->Draw("hist same");
     }
     if(signal.size() > 2){
       if(signal[2]){
	 signal[2]->SetLineStyle(2);
	 signal[2]->SetLineColor(kPink-6);
	 signal[2]->SetLineWidth(4);
	 signal[2]->Draw("hist same");
       }
     }
   }
   allBackgrounds->Draw("axis same");

   // draw legend
   float leg_height = 0.45;
   float leg_width = 0.2;
   float leg_xoffset = 0.74;
   float leg_yoffset = 0.7;
   
   TLegend* leg = new TLegend(leg_xoffset,leg_yoffset-leg_height/2,leg_xoffset+leg_width,leg_yoffset+leg_height/2);
   leg->SetFillColor(0);
   leg->SetFillStyle(0);
   leg->SetBorderSize(0);
   leg->SetTextFont(52);
   leg->SetTextSize(0.033);
   
   // currently assuming first signal is Tbq (single T production), second is TTS, third is BBS
   if(signal[0])
     leg->AddEntry(signal[0], "Tbq (650 GeV)", "l");
   if(signal.size()>1){
     if(signal[1])
       leg->AddEntry(signal[1], "TT_{S} (650 GeV)", "l");
     if(signal[2])
       leg->AddEntry(signal[2], "BB_{S} (650 GeV)", "l");
   }
   if(!isSignalRegion)
     leg->AddEntry(data, "data 2012");
   for(int q=0; q<histos.size(); q++){
     if(histos[q]){
       if(histos[q]->GetEntries() > 0.)
	 leg->AddEntry(histos[q], plotLists::GetLegendName(names[q]).c_str(), "f");
     }
   }
   leg->Draw("lpe");
   
   ATLAS_LABEL(0.24,0.86,1);
   myText(0.36, 0.86, 1, .05, "Internal");
   char text[]="#sqrt{s}=8 TeV";
   myText(0.55,0.77,1,.04, text);
   char text_L[]="#int L dt = 20.3 fb^{-1}";
   myText(0.5, 0.84,1, .04, text_L);
   if(channel!=""){
     char chan_txt[channel.size()-1];
     strcpy(chan_txt, (channel.substr(1)).c_str());
     myText(0.5, 0.7, 1, .06, chan_txt);
   }
   char inText[100];
   int t;
   if(signal[0])
     t=sprintf(inText, "N_{Tbq} = %.1f", signal[0]->Integral(0,signal[0]->GetNbinsX()+1));
   else
     t=sprintf(inText, "N_{Tbq} = %.1f", 0.);
   myText(0.24, 0.7, 1, .04, inText);
   if(signal.size()>1){
     if(signal[1])
       t=sprintf(inText, "N_{TTS} = %.1f", signal[1]->Integral(0,signal[1]->GetNbinsX()+1));
     else
       t=sprintf(inText, "N_{TTS} = %.1f", 0.);
     myText(0.24, 0.65, 1, .04, inText);
     if(signal.size()>2){
       if(signal[2])
	 t=sprintf(inText, "N_{BBS} = %.1f", signal[2]->Integral(0,signal[2]->GetNbinsX()+1));
       else
	 t=sprintf(inText, "N_{BBS} = %.1f", 0.);
       myText(0.24, 0.6, 1, .04, inText);
     }
   }

   double back_int = allBackgrounds->Integral(0,allBackgrounds->GetNbinsX()+1);
   if(back_int > 10000.)
     t=sprintf(inText, "N_{B} = %.3e", back_int);
   else
     t=sprintf(inText, "N_{B} = %.1f", back_int);
   myText(0.24, 0.75, 1, .04, inText);
   if(!isSignalRegion){
     if(data)
       t=sprintf(inText, "N_{D} = %.0f", data->Integral(0,data->GetNbinsX()+1));
     else
       t=sprintf(inText, "N_{D} = %.0f", 0.);
     myText(0.24, 0.8, 1, .04, inText);
   }
   
   canvas->cd();
   
   // Draw Ratio plot
   double ratio_max = 1.6;
   double ratio_min = 0.4;
  
   double pad_xlow = 0.;
   double pad_xhigh = 0.95;
   double pad_ylow = 0.0;
   double pad_yhigh = 0.3;
   
   const char* pad_name = "pad";
   TPad* pad4ratio = new TPad(pad_name,pad_name,pad_xlow,pad_ylow,pad_xhigh,pad_yhigh);

   pad4ratio->SetMargin(pad_margin_left,pad_margin_right,0.46,0.);
   pad4ratio->Draw();
   pad4ratio->cd();

   TH1D* ratioPlot;
   if(data)
     ratioPlot = (TH1D*)data->Clone("ratio");
   else if (signal[0]) 
     ratioPlot = (TH1D*)signal[0]->Clone("ratio"); // just for getting the axis

   ratioPlot->SetTitle("");
   ratioPlot->Divide(allBackgrounds);
   ratioPlot->GetYaxis()->SetTitle("Data/MC");
   ratioPlot->GetXaxis()->SetTitle(axisName.c_str());

   if (data){
     // here change ratio_min and ratio_max if the ratio plot is quite flat
     double maxDeviation=0;
     double tempDev=0;
     for(int ibin=1; ibin<=allBackgrounds->GetNbinsX(); ibin++){
       tempDev = std::abs(ratioPlot->GetBinContent(ibin)-1.);
       if(tempDev > maxDeviation) maxDeviation = tempDev;
     }
     if(maxDeviation < 0.1){
       ratio_max = 1.12;
       ratio_min = 0.88;
     }
     else if(maxDeviation < 0.2){
       ratio_max = 1.25;
       ratio_min = .75;
     }
     ratioPlot->SetMinimum(ratio_min);
     ratioPlot->SetMaximum(ratio_max);
   }     

   if(ratioPlot->GetXaxis()->GetNdivisions() > ratioPlot->GetNbinsX())
     ratioPlot->GetXaxis()->SetNdivisions(ratioPlot->GetNbinsX());
   if(ratioPlot->GetXaxis()->GetNdivisions() > 11)
     ratioPlot->GetXaxis()->SetNdivisions(11);
   ratioPlot->GetXaxis()->SetLabelSize(RATIOPLOT_XAXIS_LABEL_SIZE);
   ratioPlot->GetYaxis()->SetLabelSize(RATIOPLOT_YAXIS_LABEL_SIZE);
   ratioPlot->GetYaxis()->SetNdivisions(3);
   ratioPlot->GetYaxis()->SetTitleSize(RATIOPLOT_YAXIS_TITLE_SIZE);
   ratioPlot->GetYaxis()->SetTitleOffset(RATIOPLOT_YAXIS_TITLE_OFFSET);
   ratioPlot->GetXaxis()->SetTitleSize(RATIOPLOT_XAXIS_TITLE_SIZE);
   ratioPlot->GetXaxis()->SetTitleOffset(RATIOPLOT_XAXIS_TITLE_OFFSET);
   
   if(!isSignalRegion && data){
     ratioPlot->Draw();
     // plot horizontal line at y=1
     TF1* horizontal = new TF1("horizontal","pol1",-10000,10000);
     horizontal->SetParameter(0,1.);
     horizontal->SetParameter(1,0.);
     horizontal->SetLineColor(kBlack);
     horizontal->SetLineStyle(2);
     horizontal->SetLineWidth(1);
     horizontal->Draw("same");
     ratioErr->Draw("E2 same");
   }else if(isSignalRegion){
     ratioPlot->Draw("axis");
     char text[]="DATA IS BLIND HERE";
     myText(0.4,0.6,1,.15,text);
   }else{ // in this case it is not the signal region but there is no data
     ratioPlot->Draw("axis");
     char text[]="NO DATA";
     myText(0.4,0.6,1,.15,text);
   }
   
   return canvas;
}

// *************************************** //
// this is a function that takes a set of  //
// histograms and draws them on a canvas   //
// in a stack, returning the canvas.       //
// It also plots the signal as a dashed    //
// line and the data with a ratio at the   //
// the bottom of data/allBackgrounds       //
//                                         //
// This one is supposed to make plots for  //
// the paper draft                         //
// *************************************** //
TCanvas* drawPlots::plotAll_VLQ_paperStyle(std::vector<TH1D*> histos, std::vector<std::string> names, std::string axisName, std::vector<TH1D*> signal, TH1D* data, TGraphAsymmErrors* err, bool doLogAxis, std::string extraTag){

   const unsigned int CANVAS_WIDTH  = 720;
   const unsigned int CANVAS_HEIGHT = 750;
   const double RATIOPLOT_YAXIS_TITLE_OFFSET = 0.75;
   const double RATIOPLOT_YAXIS_TITLE_SIZE   = 0.11;
   const double RATIOPLOT_YAXIS_LABEL_SIZE   = 0.09;  
   const double RATIOPLOT_XAXIS_TITLE_OFFSET = 1.6;
   const double RATIOPLOT_XAXIS_TITLE_SIZE   = 0.11;
   const double RATIOPLOT_XAXIS_LABEL_SIZE   = 0.09;
   
   TCanvas* canvas = new TCanvas("canvas","canvas",0,0,CANVAS_WIDTH,CANVAS_HEIGHT);
   
   canvas->SetMargin(0.,0.,0.,0.);
   canvas->Clear();
   //canvas->cd();
   
   SetAtlasStyle();
   gStyle->SetHistLineWidth(1.);
   gStyle->SetErrorX(0);

   err->SetLineColor(kWhite);
   err->SetLineWidth(0);

   // create histogram to store all backgrounds
   TH1D* allBackgrounds;
   bool gotSomething=false;
   for(int c=0; c<signal.size(); c++){
     if(signal[c]){
       allBackgrounds = (TH1D*)signal[c]->Clone("all_backgrounds");
       gotSomething=true;
     }
   }

   if(!gotSomething && data){
     allBackgrounds = (TH1D*)data->Clone("all_backgrounds");
     gotSomething=true;
   }
   if(!gotSomething){
     for(int b=0; b<histos.size(); b++){
       if(histos[b] && !gotSomething){
	 allBackgrounds = (TH1D*)histos[b]->Clone("all_backgrounds");
	 gotSomething=true;
       }
     }
   }

   if(!gotSomething){
     std::cout << "Error: could not find any background, data, or signal for this plot" << std::endl;
     return canvas;
   }
   
   // set all bins to zero
   for(int i=0; i<=allBackgrounds->GetNbinsX()+1; i++)
     allBackgrounds->SetBinContent(i,0);

   // go through histos and combine the histograms that need to be combined
   // WZ stays separate, ttV stays separate, all others become "Other bkg"
   TH1D* otherBkgs=0; TH1D* ttbarX=0; TH1D* WZ_Sherpa=0;
   for(int cc=0; cc<histos.size(); cc++){
     if(histos[cc]){
       for(int j=0; j<=allBackgrounds->GetNbinsX()+1; j++){
	 double binContent=allBackgrounds->GetBinContent(j)+histos[cc]->GetBinContent(j);
	 allBackgrounds->SetBinContent(j,binContent);
       }
       
       if(names[cc]=="ttbarX")
	 ttbarX=(TH1D*)histos[cc]->Clone("ttbarX_new");
       else if(names[cc]=="WZ_Sherpa")
	 WZ_Sherpa=(TH1D*)histos[cc]->Clone("WZ_new");
       else{

	 if(!otherBkgs)
	   otherBkgs=(TH1D*)histos[cc]->Clone("other_bkg_new");
	 else
	   otherBkgs->Add(histos[cc]);
       }
     }
   }

   // make stack of backgrounds, fill backgrounds histogram
   THStack* Stack = new THStack();
   if(ttbarX){
     ttbarX->SetLineColor(kGray);
     ttbarX->SetFillColor(kGray);
     Stack->Add(ttbarX);
   }
   if(WZ_Sherpa){
     WZ_Sherpa->SetLineColor(kBlue-9);
     WZ_Sherpa->SetFillColor(kBlue-9);
     Stack->Add(WZ_Sherpa);
   }
   if(otherBkgs){
     otherBkgs->SetLineColor(kOrange-2);
     otherBkgs->SetFillColor(kOrange-2);
     Stack->Add(otherBkgs);
   }

   TH1D* backgroundsForRatio = (TH1D*)allBackgrounds->Clone("bkgds_for_ratio");

   // create main pad
   const double mainPad_ylow  = 0.3;
   const double mainPad_yhigh = 0.95;
   const double mainPad_xlow = 0.;
   const double mainPad_xhigh = 0.95;
   const double pad_margin_left = 0.2;
   const double pad_margin_right  = 0.02;
   double main_y_max = -99;
   double main_y_min = -99;

   TPad* mainPad = new TPad("main","main",mainPad_xlow,mainPad_ylow,mainPad_xhigh,mainPad_yhigh);
   mainPad->SetMargin(pad_margin_left,pad_margin_right,0.,.05); // left, right, bottom, top
   mainPad->Draw();
   mainPad->cd();

   // find max y
   if(allBackgrounds){
     int maxBinBkg = allBackgrounds->GetMaximumBin();
     double bkgYmax = allBackgrounds->GetBinContent(maxBinBkg);
     main_y_max = bkgYmax;
   }
   if(data){
     int maxBinData = data->GetMaximumBin();
     double dataYmax = data->GetBinContent(maxBinData);
     if(dataYmax > main_y_max) main_y_max = dataYmax;
   }
   if(main_y_max < .002) main_y_max = .1;

   if(doLogAxis){
     mainPad->SetLogy();
     main_y_min = 0.07;
     main_y_max*=60;
   }else{
     main_y_min = 0.001;
     main_y_max*=1.6;
   }

   // draw axis
   allBackgrounds->SetMaximum(main_y_max);
   allBackgrounds->SetMinimum(main_y_min);
   std::string title = std::string(";") + axisName + ";Events";
   std::stringstream binw; 
   
   binw<<allBackgrounds->GetBinWidth(1);
   std::string width = binw.str();
   if (axisName.find("GeV") != std::string::npos){
     width += " GeV";
     title += " / "+width;
   }
   allBackgrounds->SetTitle(title.c_str());   
   allBackgrounds->GetYaxis()->SetTitleOffset(1.4);
   allBackgrounds->GetYaxis()->SetTitleSize(0.06);
   allBackgrounds->GetXaxis()->SetTitleOffset(1.2);
   allBackgrounds->GetYaxis()->SetLabelSize(0.04);
   allBackgrounds->GetXaxis()->SetLabelSize(0.04);
   allBackgrounds->Draw("hist");

   // draw stack with error
   Stack->Draw("hist same");
   if(err){
     err->Draw("E2 same");
   }

   // make error histograms for the ratio plot
   TGraphAsymmErrors* ratioErr = tools::getRatio(err, backgroundsForRatio);

   // draw data
   if(data)
      data->Draw("e same");

   // draw signal
   if(signal[0]){
     signal[0]->SetLineStyle(2);
     signal[0]->SetLineColor(kRed+2);//kGreen+3 before
     signal[0]->SetLineWidth(5);
     signal[0]->Draw("hist same");
   }
   if(signal.size() > 1){
     if(signal[1]){
       //signal[1]->SetLineStyle(2);
       signal[1]->SetLineColor(kRed+2);
       signal[1]->SetLineWidth(4);
       signal[1]->Draw("hist same");
     }
     if(signal.size() > 2){
       if(signal[2]){
	 //signal[2]->SetLineStyle(2);
	 signal[2]->SetLineColor(kBlue+2);
	 signal[2]->SetLineWidth(4);
	 signal[2]->Draw("hist same");
       }
     }
   }
   allBackgrounds->Draw("axis same");

   // draw legend
   float leg_height = 0.3;
   float leg_width = 0.2;
   float leg_xoffset = 0.74;
   float leg_yoffset = 0.77;
   
   TLegend* leg = new TLegend(leg_xoffset,leg_yoffset-leg_height/2,leg_xoffset+leg_width,leg_yoffset+leg_height/2);
   leg->SetFillColor(0);
   leg->SetFillStyle(0);
   leg->SetBorderSize(0);
   leg->SetTextFont(52);
   leg->SetTextSize(0.033);
   
   // currently assuming first signal is Tbq (single T production), second is TTS, third is BBS
   leg->AddEntry(data, "data 2012");
   leg->AddEntry(otherBkgs, "Other bkg.", "f");
   leg->AddEntry(WZ_Sherpa, "WZ", "f");
   leg->AddEntry(ttbarX, "t#bar{t}+X", "f");
   if(signal.size()>1){
     if(signal[2])
       leg->AddEntry(signal[2], "B#bar{B} (650 GeV)", "l");
     if(signal[1])
       leg->AddEntry(signal[1], "T#bar{T} (650 GeV)", "l");
   }   
   if(signal[0])
     leg->AddEntry(signal[0], "T#bar{b}q (650 GeV)", "l");
   
   
   leg->AddEntry( err, "Uncertainty", "f");


   leg->Draw("lpe");
   
   ATLAS_LABEL(0.24,0.86,1);
   myText(0.36, 0.86, 1, .05, "Internal");
   char text[]="#sqrt{s}=8 TeV";
   myText(0.55,0.77,1,.04, text);
   char text_L[]="#int L dt = 20.3 fb^{-1}";
   myText(0.5, 0.84,1, .04, text_L);
   if(extraTag!=""){
     char tag_txt[extraTag.size()];
     strcpy(tag_txt, extraTag.c_str());
     if(extraTag.size()>15)
       myText(0.7, 0.56, 1, .04, tag_txt);
     else
       myText(0.75, 0.56, 1, .04, tag_txt);
   }
   char text_tri[]="Trilepton";
   myText(0.55, 0.70, 1, .04, text_tri);

   /*
   double back_int = allBackgrounds->Integral(0,allBackgrounds->GetNbinsX()+1);
   int t;
   char inText[100];
   if(back_int > 10000.)
     t=sprintf(inText, "N_{B} = %.3e", back_int);
   else
     t=sprintf(inText, "N_{B} = %.1f", back_int);
   myText(0.24, 0.6, 1, .04, inText);
   if(data)
     t=sprintf(inText, "N_{D} = %.0f", data->Integral(0,data->GetNbinsX()+1));
   else
     t=sprintf(inText, "N_{D} = %.0f", 0.);
   myText(0.24, 0.7, 1, .04, inText);
   */
   canvas->cd();
   
   // Draw Ratio plot
   // 1.6 and .4 are the normal ones
   double ratio_max = 3.2;
   double ratio_min = -.9;
  
   double pad_xlow = 0.;
   double pad_xhigh = 0.95;
   double pad_ylow = 0.0;
   double pad_yhigh = 0.3;
   
   const char* pad_name = "pad";
   TPad* pad4ratio = new TPad(pad_name,pad_name,pad_xlow,pad_ylow,pad_xhigh,pad_yhigh);

   pad4ratio->SetMargin(pad_margin_left,pad_margin_right,0.46,0.);
   pad4ratio->Draw();
   pad4ratio->cd();

   TH1D* ratioPlot;
   if(data)
     ratioPlot = (TH1D*)data->Clone("ratio");
   else if (signal[0]) 
     ratioPlot = (TH1D*)signal[0]->Clone("ratio"); // just for getting the axis
   else if (signal[1])
     ratioPlot = (TH1D*)signal[1]->Clone("ratio");
   else
     return canvas;

   ratioPlot->SetTitle("");
   ratioPlot->Divide(allBackgrounds);
   ratioPlot->GetYaxis()->SetTitle("Data / bkg");
   ratioPlot->GetXaxis()->SetTitle(axisName.c_str());

   if (data){
     
     // here change ratio_min and ratio_max if the ratio plot is quite flat
     double maxDeviation=0;
     double minDeviation=10.;
     double tempDev=0;
     for(int ibin=1; ibin<=allBackgrounds->GetNbinsX(); ibin++){
       double binContent = ratioPlot->GetBinContent(ibin);
       tempDev=0;
       if(binContent>0 && binContent < 10) tempDev = std::abs(binContent-1.);
       if(tempDev > maxDeviation) maxDeviation = tempDev;
       if(tempDev < minDeviation) minDeviation = tempDev;
     }
     if(maxDeviation < 0.6){
       ratio_max = 1.6;
       ratio_min = 0.4;
     }
     else if(maxDeviation < 0.1){
       ratio_max = 1.12;
       ratio_min = 0.88;
     }
     else if(maxDeviation < 0.2){
       ratio_max = 1.25;
       ratio_min = .75;
     }
     if(minDeviation > 1.5){
       if(minDeviation > 2.)
	 ratio_max = 2.7;
       else
	 ratio_max = 2.2;
     }
     ratioPlot->SetMinimum(ratio_min);
     ratioPlot->SetMaximum(ratio_max);
   }     

   if(ratioPlot->GetXaxis()->GetNdivisions() > ratioPlot->GetNbinsX())
     ratioPlot->GetXaxis()->SetNdivisions(ratioPlot->GetNbinsX());
   if(ratioPlot->GetXaxis()->GetNdivisions() > 11)
     ratioPlot->GetXaxis()->SetNdivisions(11);
   ratioPlot->GetXaxis()->SetLabelSize(RATIOPLOT_XAXIS_LABEL_SIZE);
   ratioPlot->GetYaxis()->SetLabelSize(RATIOPLOT_YAXIS_LABEL_SIZE);
   ratioPlot->GetYaxis()->SetNdivisions(3);
   ratioPlot->GetYaxis()->SetTitleSize(RATIOPLOT_YAXIS_TITLE_SIZE);
   ratioPlot->GetYaxis()->SetTitleOffset(RATIOPLOT_YAXIS_TITLE_OFFSET);
   ratioPlot->GetXaxis()->SetTitleSize(RATIOPLOT_XAXIS_TITLE_SIZE);
   ratioPlot->GetXaxis()->SetTitleOffset(RATIOPLOT_XAXIS_TITLE_OFFSET);
   
   if(data){
     ratioPlot->Draw();
     // plot horizontal line at y=1
     TF1* horizontal = new TF1("horizontal","pol1",-10000,10000);
     horizontal->SetParameter(0,1.);
     horizontal->SetParameter(1,0.);
     horizontal->SetLineColor(kBlack);
     horizontal->SetLineStyle(2);
     horizontal->SetLineWidth(1);
     horizontal->Draw("same");
     ratioErr->Draw("E2 same");
   }else{ // there is no data
     ratioPlot->Draw("axis");
     char text[]="NO DATA";
     myText(0.4,0.6,1,.15,text);
   }
   
   return canvas;
}


// *************************************** //
// this is a function that draws all       //
// backrounds as a grey area, and compares //
// the shape of the backgrounds to that of //
// the signal                              //
// This one is supposed to match the plots //
// make by JP (Fig 3 of CONF note)         //
// *************************************** //
TCanvas* drawPlots::plotShape_VLQ(TH1D* bkgd, std::string axisName, std::vector<TH1D*> signal, bool doLogAxis, std::string channel){

  // only drawing shapes so normalize histograms
  std::cout << "background integral: " << bkgd->Integral() << std::endl;
  bkgd->Scale(1./bkgd->Integral());
  for(int ahist=0; ahist<signal.size(); ahist++){
    if(signal[ahist]){
      std::cout << "scaling signal: " << ahist << " with integral: " << signal[ahist]->Integral() << std::endl;
      signal[ahist]->Scale(1./signal[ahist]->Integral());
    }
  }

   const unsigned int CANVAS_WIDTH  = 720;
   const unsigned int CANVAS_HEIGHT = 750;
   TCanvas* canvas = new TCanvas("canvas","canvas",0,0,CANVAS_WIDTH,CANVAS_HEIGHT);
   
   canvas->SetMargin(0.,0.,0.,0.);
   canvas->Clear();
   canvas->cd();

   SetAtlasStyle();
   gStyle->SetHistLineWidth(1.);

   
   // create main pad
   const double mainPad_ylow  = 0.;
   const double mainPad_yhigh = 1.;
   const double mainPad_xlow = 0.;
   const double mainPad_xhigh = 1.;
   const double pad_margin_left = 0.16;
   const double pad_margin_right  = 0.035;
   
   double main_y_max = -99;
   double main_y_min = -99;

   TPad* mainPad = new TPad("main","main",mainPad_xlow,mainPad_ylow,mainPad_xhigh,mainPad_yhigh);
   mainPad->SetMargin(pad_margin_left,pad_margin_right,0.16,.05); // left, right, bottom, top
   mainPad->Draw();
   mainPad->cd();

   // find max y
   int maxBinBkg = bkgd->GetMaximumBin();
   double bkgYmax = bkgd->GetBinContent(maxBinBkg);
   main_y_max = bkgYmax;
   //   std::cout << "background maximum: " << bkgYmax << std::endl;
   if(main_y_max < .002) main_y_max = .1;
   
   if(doLogAxis){
     mainPad->SetLogy();
     main_y_min = 0.0001;
     main_y_max*=500;
   }else{
     main_y_min = 0.00001;
     main_y_max*=1.6;
   }

   // draw axis
   bkgd->SetMaximum(main_y_max);
   bkgd->SetMinimum(main_y_min);
   std::string title = std::string(";") + axisName + ";Fraction of events";
   std::stringstream binw; 
   binw<<bkgd->GetBinWidth(1);
   std::string width = binw.str();
   if (axisName.find("GeV") != std::string::npos) width += " GeV";
   if(bkgd->GetBinWidth(1) != 1) title += " / "+width;
   bkgd->SetTitle(title.c_str());   
   bkgd->GetYaxis()->SetTitleOffset(1.6);
   bkgd->GetXaxis()->SetTitleOffset(1.2);
   bkgd->GetYaxis()->SetLabelSize(0.03);
   bkgd->GetYaxis()->SetTitleSize(0.045);
   bkgd->GetXaxis()->SetTitleSize(0.045);
   bkgd->GetXaxis()->SetLabelSize(0.03);
   bkgd->SetFillStyle(3354);
   bkgd->SetFillColor(kGray+2);
   bkgd->SetLineColor(kGray+2);
   bkgd->Draw("histf");

   // draw signal
   if(signal[0]){
     std::cout << "printing Tbq" << std::endl;
     signal[0]->SetLineStyle(2);
     signal[0]->SetLineColor(kRed+2); // kGreen+3 was being used before
     signal[0]->SetLineWidth(5);
     signal[0]->Draw("hist same");
   }
   if(signal.size() > 1){
     if(signal[1]){
       std::cout << "printing TTS" << std::endl;
       //   signal[1]->SetLineStyle(2);
       signal[1]->SetLineColor(kRed+2);
       signal[1]->SetLineWidth(4);
       signal[1]->Draw("hist same");
     }
     if(signal.size() > 2){
       if(signal[2]){
	 std::cout << "printing BBS" << std::endl;
	 //	 signal[2]->SetLineStyle(2);
	 signal[2]->SetLineColor(kBlue+2);
	 signal[2]->SetLineWidth(4);
	 signal[2]->Draw("hist same");
       }
     }
   }
   bkgd->Draw("axis same");

   // draw legend
   float leg_height = 0.2;
   float leg_width = 0.4;
   float leg_xoffset = 0.65;
   float leg_yoffset = 0.8;
   
   TLegend* leg = new TLegend(leg_xoffset,leg_yoffset-leg_height/2,leg_xoffset+leg_width,leg_yoffset+leg_height/2);
   leg->SetFillColor(0);
   leg->SetFillStyle(0);
   leg->SetBorderSize(0);
   leg->SetTextFont(72);
   leg->SetTextSize(0.026);
   
   // currently assuming first signal is single T, second is TTS
   leg->AddEntry(bkgd,"Background", "f");
   if(signal.size()>1){
     if(signal.size()>2){
       if(signal[2])
	 leg->AddEntry(signal[2], "B#bar{B} (650 GeV)", "l");
     }
     if(signal[1])
       leg->AddEntry(signal[1], "T#bar{T} (650 GeV)", "l");
   }  
   if(signal[0])
     leg->AddEntry(signal[0], "T#bar{b}q (650 GeV)", "l");
   
   leg->Draw("lpe");
   
   ATLAS_LABEL(0.24,0.86,1);
   myText(0.26, 0.81, 1, .04, "Internal");
   char text[]="#sqrt{s}=8 TeV";
   myText(0.48,0.82,1,.03, text);
   char text_L[]="Simulation";
   myText(0.47, 0.87,1, .03, text_L);
   char text_tri[]="Trilepton";
   myText(0.485, 0.77, 1, .03, text_tri);
   if(channel!=""){
     char chan_txt[channel.size()-1];
     strcpy(chan_txt, (channel.substr(1)).c_str());
     if(channel.size()>10)
       myText(0.55, 0.56, 1, .03, chan_txt);
     else
       myText(0.65, 0.56, 1, .03, chan_txt);
   }
   canvas->cd();
   
   return canvas;
}


// *************************************** //
// get max/min values for axis limits      //
// *************************************** //
void drawPlots::GetAxisLimits(TH1D* graph,double& x_min,double& x_max,double& y_min,double& y_max){
   double graphXmax = graph->GetXaxis()->GetXmax();
   if(graphXmax > x_max || x_max == -99) x_max = graphXmax;
   double graphXmin = graph->GetXaxis()->GetXmin();
   if(graphXmin < x_min || x_min == -99) x_min = graphXmin;
   int maxBin = graph->GetMaximumBin();
   double graphYmax = graph->GetBinContent(maxBin);
   if(y_max == -99) y_max = graphYmax;
   else y_max += graphYmax;
   y_min = 0;
}

// *************************************** //
// draw the ATLAS label                    //
// *************************************** //
void drawPlots::ATLAS_LABEL(Double_t x,Double_t y,Color_t color) 
{
   TLatex l; //l.SetTextAlign(12); l.SetTextSize(tsize); 
   l.SetNDC();
   l.SetTextFont(72);
   l.SetTextColor(color);
   l.DrawLatex(x,y,"ATLAS");
}

// *************************************** //
// draw other text (like lumi or energy)    //
// *************************************** //
void drawPlots::myText(Double_t x,Double_t y,Color_t color,Double_t size,char *text) {

  TLatex l; //l.SetTextAlign(12); 
  l.SetTextSize(size); 
  l.SetNDC();
  l.SetTextColor(color);
  l.DrawLatex(x,y,text);
}
void drawPlots::myText(Double_t x,Double_t y,Color_t color,char *text) {

  TLatex l; //l.SetTextAlign(12); 
  //  l.SetTextSize(size); 
  l.SetNDC();
  l.SetTextColor(color);
  l.DrawLatex(x,y,text);
}

// ********************************************* //
// add overflow content to first and last bin    //
// errors are corrected elsewhere                //
// ********************************************* //
void drawPlots::correctForOverflows(TH1D* hist){

  if(hist){
    double underflow_bin = hist->GetBinContent(0);
    if(underflow_bin != 0.){
      double first_bin = hist->GetBinContent(1);
      hist->SetBinContent(1,first_bin+underflow_bin);
      hist->SetBinContent(0,0);
    }
    double overflow_bin = hist->GetBinContent(hist->GetNbinsX()+1);
    if(overflow_bin != 0.){
    double last_bin = hist->GetBinContent(hist->GetNbinsX());
    hist->SetBinContent(hist->GetNbinsX(), overflow_bin+last_bin);
    hist->SetBinContent(hist->GetNbinsX()+1, 0);
    }
  }
  
}
