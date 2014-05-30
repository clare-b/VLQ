#ifndef PLOTLISTS_H
#define PLOTLISTS_H

#include <vector>
#include <string>
#include <map>

class plotLists{

   private:
   
   public:
      
      static const std::string DATE;
      static const double LUMI;
      static const bool DEBUG;
      static const bool ONLYALLZREW;
      
      static const std::vector<std::vector<std::string> > LOOKUPTABLE;
      static std::vector<std::vector<std::string> > LookUpTable(void);
      
      static const std::vector<std::string> CATEGORIES;
      static std::vector<std::string> Categories(void);
      
      static const std::vector<std::string> ZPTREWCATEGORIES;
      static std::vector<std::string> ZptRewCategories(void);

      static const std::vector<std::string> JETCATEGORIES;
      static std::vector<std::string> JetCategories(void);

      static const std::vector<std::string> SYSTEMATICS;
      static std::vector<std::string> Systematics(void);
      
      static double GetSystScaleFactor(std::string systName);

      static std::map<std::string, std::string> GetSystNameMap();
      static const std::map<std::string, std::string> SYST_NAME_MAP;

      static const std::vector<std::string> PLOTNAMES;
      static std::vector<std::string> plotNames(void);
      
      static const std::vector<std::string> PLOTNAMES_E;
      static std::vector<std::string> plotNamesE(void);

      static const std::vector<std::string> PLOTNAMES_M;
      static std::vector<std::string> plotNamesM(void);

      static const std::vector<std::string> SAMPLES;
      static std::vector<std::string> samples(void);

      static const std::vector<std::string> SAMPLES_TO_PLOT;
      static std::vector<std::string> samplesToPlot(void);
 
      static const std::vector<std::string> SAMPLES_FOR_RESCALING;
      static std::vector<std::string> samplesForRescaling(void);
     
      static const std::vector<std::string> FILENAMES;
      static std::vector<std::string> fileNames(void);

      static const std::vector<std::string> FILENAMES_FOR_PLOTTING;
      static std::vector<std::string> fileNamesForPlotting(void);
      
      static const std::vector<std::string> CUTNAMES;
      static std::vector<std::string> cutNames(void);
      
      static std::string GetFileSampleName(std::string fileName);
      
      static double GetNormalization(std::string fileName, std::string syst);
      
      static std::string GetXAxisName(std::string plotName);
      
      static bool isSignalRegion(std::string plotName);

      static bool useLogScale(std::string plotName);

      static int includeWhichSignals(std::string plotName);

      static std::string GetLegendName(std::string name);

      static const std::vector<std::string> CUTSFORYIELDS;
      static std::vector<std::string> cutsForYields(void);

      static std::string getFilenameForSyst(std::string syst, std::string file);
      static bool scaleForSyst(std::string syst, std::string file);

};


#endif
