#include "tools.h"
#include "TColor.h"

#include <TMath.h>
#include <iostream>
#include <fstream>
#include <map>


// *************************** //
// function to set the color   //
// of a histogram depending    //
// on the background process   //
// *************************** //
int tools::setColor(std::string sample){
   
  double color=0;
  //TColor* root_color = new TColor();
  // can get other colors using root_color->GetColor(r,g,b)
  // where r,g,b are integers between 0 and 255
  // more colors here: http://www-cdf.fnal.gov/~bernd/rootcolors.png
  // kWhite =0,   kBlack =1,   kGray=920, kRed   =632, kGreen =416, kBlue=600, kYellow=400, kMagenta=616, kCyan=432,kOrange=800, kSpring=820, kTeal=840, kAzure =860, kViolet =880, kPink=900
  if(sample=="Signal") color=2; // red
  else if(sample=="ttbar") color=0; // white
  else if(sample=="SingleTop") color=225; 
  else if(sample=="TZ") color=9;
  else if(sample=="WJets") color=55; // dark blue
  else if(sample=="Zjets_Alpgen_cc") color=66; // cyan
  else if(sample=="Zjets_Alpgen_bb") color=66; // cyan
  else if(sample=="Zjets_Alpgen_light") color=66; // cyan
  else if(sample=="Zjets_Sherpa") color=66;
  else if(sample=="Zjets_Sherpa_light") color=413;
  else if(sample=="Zjets_Sherpa_bb") color=801;
  else if(sample=="Zjets_Sherpa_cc") color=630;
  else if(sample=="Diboson") color=225; 
  else if(sample=="Diboson_Sherpa") color=225; 
  else if(sample=="WZ_Sherpa") color=600; 
  else if(sample=="WW_Sherpa") color=599; 
  else if(sample=="ZZ_Sherpa") color=598; 
  else if(sample=="Triboson") color=222;
  else if(sample=="ttbarX") color=920;
  return color;
  
}

// *************************** //
// function to determine if a  //
// file exists                 //
// *************************** //
bool tools::exists(std::string file_name){
   return ( access( file_name.c_str(), F_OK ) != -1 );
}

// ************************ //
// this is a function that  //
// combines two histograms  //
// safely without assuming  //
// that either exists or    //
// has entries              //
// ************************ //

TH1D* tools::combineHists(TH1D* hist_1, TH1D* hist_2, double scale_1, double scale_2){
  TH1D* hist;
  bool getH1=true;
  bool getH2=true;

  if(!hist_1) getH1=false;
  if(!hist_2) getH2=false;

  /*
  bool scale=true;
  
  // only check to see if the entries are zero if the histogram exists
  if(getH1)
    if(hist_1->GetEntries()==0.) getH1=false;
  if(getH2)
    if(hist_2->GetEntries()==0.) getH2=false;
  */

  if(getH1 && getH2){
    hist_1->Scale(scale_1);
    hist_2->Scale(scale_2);
    hist = hist_1;
    hist->Add(hist_2);
  }
  else if(getH1){
    hist_1->Scale(scale_1);
    hist = hist_1;
  }
  else if(getH2){
    hist_2->Scale(scale_2);
    hist = hist_2;
  }
  else if(!getH1 && !getH2){ // in this case both histograms exist and are empty
    hist = hist_1; // so just return one of the empty histograms
  }
  return hist;
}

// ************************ //
// this is a function that  //
// combines two histograms  //
// safely without assuming  //
// that either exists or    //
// has entries (2D version  //
// ************************ //

TH2D* tools::combineHists2D(TH2D* hist_1, TH2D* hist_2, double scale_1, double scale_2){

  TH2D* hist;
  bool getH1=true;
  bool getH2=true;
  if(!hist_1) getH1=false;
  if(!hist_2) getH2=false;

  // only check to see if the entries are zero if the histogram exists
  if(getH1)
    if(hist_1->GetEntries()==0.) getH1=false;
  if(getH2)
    if(hist_2->GetEntries()==0.) getH2=false;

  if(getH1 && getH2){
    hist_1->Scale(scale_1);
    hist_2->Scale(scale_2);
    hist = hist_1;
    hist->Add(hist_2);
  }
  else if(getH1){
    hist_1->Scale(scale_1);
    hist = hist_1;
  }
  else if(getH2){
    hist_2->Scale(scale_2);
    hist = hist_2;
  }
  else if(!getH1 && !getH2){ // in this case both histograms exist and are empty
    hist = hist_1; // so just return one of the empty histograms
  }
  return hist;
}

// ********************************* //
// function to add overflow and      //
// underflow to the next bin in the  //
// histogram so that it can be seen  //
// when plotted                      //
// ********************************* //

void tools::fixOverflow(TH1D* hist){

  if(hist){
    int numBins = hist->GetNbinsX();
    double getOverflow = hist->GetBinContent(numBins+1);
    double getUnderflow = hist->GetBinContent(0);
    double getOverflowError = hist->GetBinError(numBins+1);
    double getUnderflowError = hist->GetBinError(0);
    hist->SetBinContent(0, 0.);
    hist->SetBinContent(numBins+1, 0.);
    hist->SetBinError(0, 0.);
    hist->SetBinError(numBins+1, 0.);
    double binContent_low = hist->GetBinContent(1);
    double binContent_high = hist->GetBinContent(numBins);
    double binError_low = hist->GetBinError(1);
    double binError_high = hist->GetBinError(numBins);
    hist->SetBinContent(1, binContent_low+getUnderflow);
    hist->SetBinContent(numBins, binContent_high+getOverflow);
    hist->SetBinError(1, sqrt(pow(getUnderflowError,2)+pow(binError_low,2)));
    hist->SetBinError(numBins, sqrt(pow(getOverflowError,2)+pow(binError_high,2)));
  }
  
}

// ********************************* //
// function to check if a histogram  //
// needs to be rebinned and if it    //
// does then rebins it               //
// ********************************* //

void tools::checkRebin(std::string plotName, TH1D*& hist){

  if(hist){
    // 200 bins from 0 to 2000 in original binning
    if(plotName.substr(0,3)=="ht_")
      hist->Rebin(20);
    // 80 bins from 0 to 800
    if(plotName.substr(0,5)=="Z_pt_"){
      hist->Rebin(8);
      // now just need to combine last 4 bins into two
      double bins[]={0., 80., 160., 240., 320., 400., 480., 640., 800.};
      const char* name = hist->GetName();
      TH1D* hist_new = (TH1D*)hist->Rebin(8, "rebin", bins);
      hist_new->SetBinContent(7, hist_new->GetBinContent(7)/2.);
      hist_new->SetBinContent(8, hist_new->GetBinContent(8)/2.);
      hist = hist_new;
      //      std::cout << "number of bins in hist: " << hist->GetNbinsX() << std::endl;
      hist->SetName(name);
    } 
  } 
}


// **************************************** //
// function to divide TGraphAsymmErrors     //
// by a histogram in order to obtain the    //
// TGraphAsymmErrors for the ratio          //
// **************************************** //
TGraphAsymmErrors* tools::getRatio(TGraphAsymmErrors* errorsOnBackground, TH1D* totalBackground){

  TGraphAsymmErrors* tmp = new TGraphAsymmErrors(totalBackground->GetNbinsX());

   for(int bin_i=1; bin_i<=totalBackground->GetNbinsX(); bin_i++){
     double x_val;
     double y_val;
     errorsOnBackground->GetPoint(bin_i-1, x_val, y_val);
     double binContent= totalBackground->GetBinContent(bin_i);
     //     if(fabs(y_val-binContent)>.01) std::cout << "Error: value of TGraphAsymmErrors is not equal to the background bin content. graph: " << y_val << " and background bin: " << binContent << std::endl;
     if(binContent==0.){
            tmp->SetPoint(bin_i-1, x_val, 0.);
	    tmp->SetPointError(bin_i-1, errorsOnBackground->GetErrorXlow(bin_i-1), errorsOnBackground->GetErrorXhigh(bin_i-1), 0., 0.); 
     }else{
       tmp->SetPoint(bin_i-1, x_val, y_val/totalBackground->GetBinContent(bin_i));
       tmp->SetPointError(bin_i-1, errorsOnBackground->GetErrorXlow(bin_i-1), errorsOnBackground->GetErrorXhigh(bin_i-1), errorsOnBackground->GetErrorYlow(bin_i-1)/totalBackground->GetBinContent(bin_i), errorsOnBackground->GetErrorYhigh(bin_i-1)/totalBackground->GetBinContent(bin_i));
     }
   }

   tmp->SetFillStyle(3354);
   tmp->SetFillColor(kBlack);
   tmp->SetLineWidth(0);
   tmp->SetMarkerSize(0);


   return tmp;

}


// **************************************** //
// function to get the symmetrized          //
// error based on the error in one          //
// direction and the nominal histogram      //
// **************************************** //
TH1D* tools::getSymmetricOpposite(TH1D* someSyst, TH1D* nominal){

  if(!nominal){
    //std::cout << "Error: nominal does not exist so can't symmetrize systematic" << std::endl;
    // if nominal doesn't exist then just return empty histogram
    TH1D* tmp = (TH1D*) someSyst->Clone();
    for(int i=0; i<=someSyst->GetNbinsX()+1; i++){
      tmp->SetBinContent(i,0);
    }
    return tmp;
  }
  TH1D* tmp = (TH1D*)nominal->Clone();
  for(int ibin=1; ibin<=tmp->GetNbinsX(); ibin++){
    double difference  = someSyst->GetBinContent(ibin) - nominal->GetBinContent(ibin);
    double newSystBinContent = nominal->GetBinContent(ibin);
    newSystBinContent=newSystBinContent-difference;
    tmp->SetBinContent(ibin, newSystBinContent);
    tmp->SetBinError(ibin, someSyst->GetBinError(ibin));
  }

  return tmp;

}

// **************************************** //
// function which determines the actual     //
// up and down errors given an "up" and     //
// "down" value                             //
// tmp[0] is the actual up error and        //
// tmp[1] is the actual down error          //
// **************************************** //
std::vector<double> tools::getUpAndDownErrors(double input_up, double input_down, double nominal, std::string syst){

  std::vector<double> tmp;
  tmp.resize(2, 0.);

  // expect input systematics only to be "up" variants becuase we should only run over one (either up or down)
  if(syst.find("_down")!=std::string::npos)
    std::cout << "Error: found down systematic. Only expecting up" << std::endl;

  double up_diff=input_up-nominal;
  double down_diff=input_down-nominal;

  if(syst=="jeff_up" || syst=="jer_up"){ // symmetrized systematics
    tmp[0]=up_diff;
    tmp[1]=up_diff;
    return tmp;
  }else{ // unsymmetrized systematics
    if(up_diff>=0.){
      if(down_diff>0. && down_diff>up_diff){ // both above nominal, down bigger
	tmp[0]=down_diff;
	return tmp;
      }else if(down_diff>0.){// both above nominal, up bigger
	tmp[0]=up_diff;
	return tmp;
      }else{ // up above nominal, down below
	tmp[0]=up_diff;
	tmp[1]=fabs(down_diff);
	return tmp;
      }
    }else{
      if(down_diff<0. && fabs(down_diff)>fabs(up_diff)){ // both below nominal, down bigger
	tmp[1]=fabs(down_diff);
	return tmp;
      }else if(down_diff<0.){ // both below nominal, up bigger
	tmp[1]=fabs(up_diff);
	  return tmp;
      }else{ // up below nominal, down above
	tmp[0]=down_diff;
	tmp[1]=fabs(up_diff);
	return tmp;
      }
    }
  } // end unsymmetrized systematics

  return tmp;

}

// **************************************** //
// function which returns a histogram       //
// from a given file. It knows that if      //
// "1BIncl" is in the histogram name it     //
// it should get the histogram by combining //
// "1BExcl" with "2BIncl"                   //
// **************************************** //
TH1D* tools::getHistFromFile(TFile* file, std::string histName){

  TH1D* tmp=0;

  size_t pos=histName.find("1BIncl");
  if(pos==std::string::npos){ // not 1BIncl
    tmp=(TH1D*) file->Get(histName.c_str());
    return tmp;
  }else{ // 1BIncl
    std::string plotname_base=histName.substr(0,pos);
    std::string plotname_ending=histName.substr(pos+6);
    tmp=tools::combineHists((TH1D*)file->Get((plotname_base+"1BExcl"+plotname_ending).c_str()), (TH1D*)file->Get((plotname_base+"2BIncl"+plotname_ending).c_str()), 1., 1.);
    return tmp;
  }

}

// **************************************** //
// function which returns takes in a number //
// and the error on that number and         //
// returns them with the correct number of  //
// sig figs                                 //
// **************************************** //

void tools::sigFigs(double& num, double& error, int sigFigsOnError){

  // convert error to the correct number of sig figs
  double multiplier = pow(10.0, sigFigsOnError-ceil(log10(fabs(error))));
  error = floorf(error*multiplier+0.5)/multiplier;  
  
  // now convert num to the same number of decimal places
  num = floorf(num*multiplier+0.5)/multiplier;

}
