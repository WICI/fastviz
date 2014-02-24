#!/bin/bash

function print_description {
   echo "This script serves as a launcher of the software"
   echo ""
   echo "Synopis:"
   echo "   ./run.sh whattodo={test, demo-diffnets, demo-movies, gephi}"
   echo ""
   echo "Please specify what you want to do:"
   echo "   test - creates a differential network file data/test.json"
   echo "          for the dynamic network stored in data/test.sdnet"
   echo "          then it creates a movie for this network in movies/test.avi"
   echo "   demo-diffnets - creates differential network files in data/"
   echo "                  for all demo networks stored in data/*.sdnet"
   echo "   demo-movies - creates movies in movies/"
   echo "                for all demo networks files stored in data/*.json"
   echo "   gephi - visualizes the dynamic network directly in Gephi"
   echo "           requires preparation steps described in README.md"
}

if [ "$1" == "" ]; then
   print_description
else
   . vars.sh
   export JSON_BIN=$(dirname ${JSON_LIBMT})
   export JSON_LIBMT=$(basename ${JSON_LIBMT} | sed 's/^lib//' )
   if [[ "$OSTYPE" != *"darwin"* ]]; then
      export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$JSON_BIN:$BOOST_BIN
   else
      export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$JSON_BIN:$BOOST_BIN
   fi

   mkdir logs > /dev/null 2>&1

   case "$1" in
   "test" )
      echo "===================================================================="
      echo "Running a short test of the algorithm producing the differential network updates"
      ./visualize_tweets_finitefile --verbose 2 --input data/test.sdnet --timecontraction 100 --output data/test
      if [ $? -ne 0 ]; then
         echo "There is some problem, please check the instructions, exiting."
         exit 1
      else
         echo "First tool finished successfully"
         echo ""
      fi
      echo "===================================================================="
      echo "Running a short test of the python script producing the movies"
      python scripts/DynamicGraph_wici.py data/test.json
      if [ $? -ne 0 ]; then
         echo "There is some problem, please check the instructions, exiting."
         exit 1
      else
         echo "Second tool finished successfully"
         echo ""
      fi
      ;;
   "demo-diffnets" )
      echo -n "Launched generation of differential network files "

      net=osama; echo -n "data/$net.sdnet "
      if [ -f data/$net.sdnet.gz ]; then gunzip -c data/$net.sdnet.gz > data/$net.sdnet; fi
      time ./visualize_tweets_finitefile --verbose 2 --input data/$net.sdnet --timecontraction 500 --maxvisualized 50 --forgetconst 0.6 --edgemin 0.95 --forgetevery 40 --output "data/${net}" --label1 "death of" --label2 "Osama bin Laden" > logs/diffnet_$net.log &

      net=superbowl; echo -n "data/$net.sdnet "
      if [ -f data/$net.sdnet.gz ]; then gunzip -c data/$net.sdnet.gz > data/$net.sdnet; fi
      time ./visualize_tweets_finitefile --verbose 2 --input data/$net.sdnet --timecontraction 3600 --maxvisualized 50 --forgetconst 0.6 --edgemin 10.0 --forgetevery 20 --output "data/${net}" --label1 "hashtags during the" --label2 "#superbowl" > logs/diffnet_$net.log &

      net=patents_full; echo -n "data/$net.sdnet "
      if [ -f data/$net.sdnet.gz ]; then gunzip -c data/$net.sdnet.gz > data/$net.sdnet; fi
      time ./visualize_tweets_finitefile --verbose 2 --input data/$net.sdnet --timecontraction $((3600*24*400)) --maxvisualized 50 --forgetconst 0.65 --edgemin 20 --forgetevery 10 --output "data/${net}" --label1 "words from" --label2 "patent titles" > logs/diffnet_$net.log &

      net=imdb; echo -n "data/$net.sdnet "
      if [ -f data/$net.sdnet.gz ]; then gunzip -c data/$net.sdnet.gz > data/$net.sdnet; fi
      time ./visualize_tweets_finitefile --verbose 2 --input data/$net.sdnet --timecontraction $((3600*24*365*3)) --maxvisualized 80 --forgetconst 0.75 --edgemin 10 --forgetevery 10 --output "data/${net}" --weighttype 2 --label1 "plot keywords" --label2 "of movies" > logs/diffnet_$net.log &

      echo
      echo "Please wait until finished..."
      wait
      echo "Finished. Differential networks in json format are saved in the 'data' subdirectory and logs are saved in the 'logs' subdirectory."
      ;;
   "demo-movies" )
      echo "Launching generation of movies."
      for myfile in data/*.json; do
         python scripts/DynamicGraph_wici.py $myfile
      done
      echo "Finished. Movies are saved in the 'movies' subdirectory."
      ;;
   "allmethods-movies" )
      echo "Launching generation of movies."
      for myfile in data/*_{fastviz,timewindow}.json; do
         python scripts/DynamicGraph_wici.py $myfile
      done
      echo "Finished. Movies are saved in the 'movies' subdirectory."
      ;;
   "allmethods-diffnets" )
      echo -n "Launched generation of differential network files "

      net=osama; echo -n "data/$net.sdnet "
      vtc=500; vfc=0.9; vtw=$(echo "scale=0; $vtc*$vfc*$vfc*$vfc"|bc -l)
      sharedoptions="--verbose 2 --input data/$net.sdnet --timecontraction $vtc"
      if [ -f data/$net.sdnet.gz ]; then gunzip -c data/$net.sdnet.gz > data/$net.sdnet; fi
      time ./visualize_tweets_finitefile --viztype fastviz $sharedoptions --maxvisualized 50 --forgetconst $vfc --edgemin 0.95 --forgetevery 10 --output "data/${net}_fastviz" --label1 "death of" --label2 "Osama bin Laden" > logs/diffnet_${net}_fastviz.log &
      time ./visualize_tweets_finitefile $sharedoptions --maxvisualized 50 --viztype timewindow --maxstored 10000 --timewindow $vtw --edgemin 0.95 --output "data/${net}_timewindow" --label1 "death of" --label2 "Osama bin Laden" > logs/diffnet_${net}_timewindow.log

      net=superbowl; echo -n "data/$net.sdnet "
      vtc=3600; vfc=0.8; vtw=$(echo "$vtc*$vfc*$vfc*$vfc"|bc -l)
      sharedoptions="--verbose 2 --input data/$net.sdnet --timecontraction $vtc"
      if [ -f data/$net.sdnet.gz ]; then gunzip -c data/$net.sdnet.gz > data/$net.sdnet; fi
      time ./visualize_tweets_finitefile --viztype fastviz $sharedoptions --maxvisualized 50 --forgetconst $vfc --edgemin 10.0 --forgetevery 10 --output "data/${net}_fastviz" --label1 "hashtags during the" --label2 "#superbowl" > logs/diffnet_${net}_fastviz.log &
      time ./visualize_tweets_finitefile $sharedoptions --maxvisualized 50 --viztype timewindow --maxstored 10000 --timewindow $vtw --edgemin 10.0 --output "data/${net}_timewindow" --label1 "hashtags during the" --label2 "#superbowl" > logs/diffnet_${net}_timewindow.log

      net=patents_full; echo -n "data/$net.sdnet "
      vtc=$((3600*24*400)); vfc=0.65; vtw=$(echo "$vtc*$vfc*$vfc*$vfc"|bc -l)
      sharedoptions="--verbose 2 --input data/$net.sdnet --timecontraction $vtc"
      if [ -f data/$net.sdnet.gz ]; then gunzip -c data/$net.sdnet.gz > data/$net.sdnet; fi
      time ./visualize_tweets_finitefile --viztype fastviz $sharedoptions --maxvisualized 50 --forgetconst $vfc --edgemin 20 --forgetevery 10 --output "data/${net}_fastviz" --label1 "words from" --label2 "patent titles" > logs/diffnet_${net}_fastviz.log &
      time ./visualize_tweets_finitefile $sharedoptions --maxvisualized 50 --viztype timewindow --maxstored 10000 --timewindow $vtw --edgemin 20 --output "data/${net}_timewindow" --label1 "words from" --label2 "patent titles" > logs/diffnet_${net}_timewindow.log

      net=imdb; echo -n "data/$net.sdnet "
      vtc=$((3600*24*365*3)); vfc=0.75; vtw=$(echo "$vtc*$vfc*$vfc*$vfc"|bc -l)
      sharedoptions="--verbose 2 --input data/$net.sdnet --timecontraction $vtc"
      if [ -f data/$net.sdnet.gz ]; then gunzip -c data/$net.sdnet.gz > data/$net.sdnet; fi
      time ./visualize_tweets_finitefile --viztype fastviz $sharedoptions --maxvisualized 80 --forgetconst $vfc --edgemin 10 --forgetevery 10 --output "data/${net}_fastviz" --weighttype 2 --label1 "plot keywords" --label2 "of movies" > logs/diffnet_${net}_fastviz.log &
      time ./visualize_tweets_finitefile $sharedoptions --maxvisualized 80 --viztype timewindow --maxstored 10000 --timewindow $vtw 0.75 --edgemin 10 --output "data/${net}_timewindow" --weighttype 2 --label1 "plot keywords" --label2 "of movies" > logs/diffnet_${net}_timewindow.log

      echo
      echo "Please wait until finished..."
      wait
      echo "Finished. Differential networks in json format are saved in the 'data' subdirectory and logs are saved in the 'logs' subdirectory."
      ;;
   "debug" )
      echo -n "Launched generation of differential network files "

      net="debug"; echo -n "data/$net.sdnet "
      if [ -f data/$net.sdnet.gz ]; then gunzip -c data/$net.sdnet.gz > data/$net.sdnet; fi
      time ./visualize_tweets_finitefile --verbose 3 --viztype fastviz --input data/$net.sdnet --timecontraction 30 --maxvisualized 50 --forgetconst 1.0 --edgemin 0.95 --forgetevery 40 --output "data/${net}_fastviz"
      time ./visualize_tweets_finitefile --verbose 3 --viztype timewindow --input data/$net.sdnet --timecontraction 30 --maxvisualized 50 --timewindow 2000 --edgemin 0.95 --output "data/${net}_timewindow"

      echo
      echo "Please wait until finished..."
      wait
      echo "Finished. Differential networks in json format are saved in the 'data' subdirectory and logs are saved in the 'logs' subdirectory."
      ;;
   "gephi" )
      if [ "$2" == "" -o "$3" == "" -o "$4" == "" ]; then
         echo "Synopis:"
         echo "   ./run.sh gephi input_file server_ip_address time_contraction"
         echo "Example:"
         echo "   ./run.sh gephi data/osama.sdnet http://localhost 200"
         exit 1
      fi
      echo "Launching direct graph streaming to Gephi."
      ./visualize_tweets_finitefile --verbose 2 --input $2 --server $3 --timecontraction $4
      ;;
    * )
      echo "Option not recognized."
      echo "Please try again using command line arguments specified below"
      echo ""
      print_description
      ;;
   esac


fi













