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
      ./visualize_tweets_finitefile --input data/test.sdnet --timecontraction 100 --output data/test
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
      echo "Launched generation of differential network files."
      
      net=osama
      time ./visualize_tweets_finitefile --input data/$net.sdnet --timecontraction 500 --maxvisualized 50 --forgetcons 0.6 --edgemin 0.95 --forgetevery 40 --output data/$net > logs/diffnet_$net.log &

      net=superbowl
      time ./visualize_tweets_finitefile --input data/$net.sdnet --timecontraction 3600 --maxvisualized 50 --forgetcons 0.6 --edgemin 10.0 --forgetevery 20 --output data/$net --keyword "#superbowl" > logs/diffnet_$net.log &

      net=patents
      time ./visualize_tweets_finitefile --input data/$net.sdnet --timecontraction $((3600*24*7)) --maxvisualized 80 --forgetcons 0.85 --edgemin 2 --forgetevery 20 --output data/$net > logs/diffnet_$net.log &

      net=imdb
      time ./visualize_tweets_finitefile --input data/$net.sdnet --timecontraction $((3600*24*365*3)) --maxvisualized 80 --forgetcons 0.75 --edgemin 10 --forgetevery 10 --output data/${net} --scoretype 2 > logs/diffnet_$net.log &
      
      ;;
   "demo-movies" )
      echo "Launching generation of movies."
      for myfile in data/*.json; do
         python scripts/DynamicGraph_wici.py $myfile
      done
      ;;
   "gephi" )
      if [ "$2" == "" || "$3" == "" || "$4" == "" ]; then
         echo "Synopis:"
         echo "   ./run.sh gephi json_file server_ip_address time_contraction"
         echo "Example:"
         echo "   ./run.sh gephi data/osama localhost 500"
         exit 1
      fi
      echo "Launching direct graph streaming to Gephi."
      ./visualize_tweets_finitefile --input $2 --server $3 --timecontraction $4
      ;;
    * )
      echo "Option not recognized."
      echo "Please try again using command line arguments specified below"
      echo ""
      print_description
      ;;
   esac


fi













