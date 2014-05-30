#!/usr/bin/env python 

import os

####################################
### script to produce all output ###
### by submitting each request   ###
### as a batch job               ###
####################################

def main():

    executables=['./bin/MasterPlotter ""', './bin/MasterPlotter "_eee"', './bin/MasterPlotter "_eem"', './bin/MasterPlotter "_mme"', './bin/MasterPlotter "_mmm"', './bin/plotShapes', './bin/getPlotsForLimits', './bin/yields', './bin/combinedSystTables', './bin/signalYields "TTS"', './bin/signalYields "BBS"', './bin/signalYields "TsZt"', './bin/cutflow', './bin/sensitivityPlot "T"', './bin/sensitivityPlot "B"', './bin/getPlots ""', './bin/getPlots "_eee"', './bin/getPlots "_eem"', './bin/getPlots "_mme"', './bin/getPlots "_mmm"', './bin/paperCutflow "pair"' , './bin/paperCutflow "single"', './bin/paperSystTables']

    os.system('make')
    myIter=0

    for item in executables:
        subOut = open('sub_%i.sh' %myIter, 'w')
        subOut.write('#!/bin/bash\n')
        subOut.write('\n')
        subOut.write('cd /home/graduate/clare1/work/VLQ \n')
        subOut.write('source setup.sh \n')
        subOut.write('cd vlq_plotting \n')
        subOut.write(item+' \n')
        subOut.close()
    
        cmd = 'qsub -q tier3 sub_%i.sh'%myIter
        os.system(cmd)

        myIter+=1

###########################                                                                                                                                      
if __name__ == "__main__":
    main()
#   
