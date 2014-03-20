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

function get_shared_opts {
   if [ ! -f data/$net.wdnet ]; then
      gunzip -c data/$net.wdnet.gz > data/$net.wdnet;
   fi
   local shared_opts="--verbose 2 --inputformat weighted --input data/$net.wdnet"
   shared_opts+=" --timecontraction $vtc --edgemin $edgemin"
   echo $shared_opts
}

function get_label1 {
   if [ ! -f data/$net.wdnet ]; then
      gunzip -c data/$net.wdnet.gz > data/$net.wdnet;
   fi
   local label=""
   if [ "$net" == "osama" ]; then label="death of"; fi
   if [ "$net" == "superbowl" ]; then label="hashtags during the"; fi
   if [ "$net" == "patents_full" ]; then label="words from"; fi
   if [ "$net" == "imdb" ]; then label="plot keywords"; fi
   echo $label
}

function get_label2 {
   if [ ! -f data/$net.wdnet ]; then
      gunzip -c data/$net.wdnet.gz > data/$net.wdnet;
   fi
   local label=""
   if [ "$net" == "osama" ]; then label="Osama bin Laden"; fi
   if [ "$net" == "superbowl" ]; then label="#superbowl"; fi
   if [ "$net" == "patents_full" ]; then label="US patent titles"; fi
   if [ "$net" == "imdb" ]; then label="of movies"; fi
   echo $label
}


function run_fastviz {
   local shared_opts="$(get_shared_opts) $1"
   local label1="$(get_label1)"
   local label2="$(get_label2)"
   local viztype_opts="--viztype fastviz --forgetconst $vfc"
   local output_suffix="_fastviz"
   if [[ $1 == *"--hide_singletons off"* ]]; then
      output_suffix+="_withsingletons";
   fi
   viztype_opts+=" --output data/${net}${output_suffix}"
   ./visualize_tweets_finitefile $shared_opts $viztype_opts \
      --label1 "$label1" --label2 "$label2" \
      > logs/diffnet_${net}${output_suffix}.log
}

function run_timewindow {
   local shared_opts="$(get_shared_opts) $1"
   local label1="$(get_label1)"
   local label2="$(get_label2)"
   local output_suffix="_timewindow"
   local vtwmultip=1;
   if [[ $1 == "wide" ]];
      then
         local vtwmultip=$1;
         output_suffix+="_wide";
   fi
   local vtw=$(echo "scale=0; $vtwmultip*$vtc/(1.0-$vfc)/3.0"|bc -l)
   local viztype_opts="--viztype timewindow --maxstored 2000 --timewindow $vtw"
   if [[ $1 == *"--hide_singletons off"* ]]; then
      output_suffix+="_withsingletons";
   fi
   viztype_opts+=" --output data/${net}${output_suffix}"
   ./visualize_tweets_finitefile $shared_opts $viztype_opts \
      --label1 "$label1" --label2 "$label2" \
      > logs/diffnet_${net}${output_suffix}.log
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

      echo
      echo "Finished. Differential networks in json format are saved in the 'data' subdirectory and logs are saved in the 'logs' subdirectory."
      ;;
   "demo-movies" )
      echo "Launching generation of movies."
      for myfile in data/*{timewindow,fastviz}.json; do
         python scripts/DynamicGraph_wici.py $myfile
      done
      echo "Finished. Movies are saved in the 'movies' subdirectory."
      ;;
   "allmethods-movies" )
      echo "Launching generation of movies."
      for myfile in data/*_fastviz_nosingletons.json; do
         python scripts/DynamicGraph_wici.py $myfile
      done
      echo "Finished. Movies are saved in the 'movies' subdirectory."
      ;;
   "allmethods-diffnets" )
      echo -n "Launched generation of differential network files "

      net="osama"; vtc=500; vfc=0.9; edgemin=0.95
      echo -n "data/$net.wdnet "
      run_fastviz
      # run_fastviz "--hide_singletons off"
      # run_timewindow
      # run_timewindow "wide"
      wait

      net="superbowl"; vtc=3600; vfc=0.8; edgemin=10.0
      echo -n "data/$net.wdnet "
      run_fastviz
      # run_fastviz "--hide_singletons off"
      # run_timewindow
      # run_timewindow "wide"
      wait

      net="imdb"; vtc=$((3600*24*365*3)); vfc=0.75; edgemin=10
      echo -n "data/$net.wdnet "
      run_fastviz
      # run_fastviz "--hide_singletons off"
      # run_timewindow
      # run_timewindow "wide"
      wait

      net="patents_full"; vtc=$((3600*24*400)); vfc=0.65; edgemin=20
      echo -n "data/$net.wdnet "
      run_fastviz
      # run_fastviz "--hide_singletons off"
      # run_timewindow
      # run_timewindow "wide"
      wait

      echo
      echo "Finished. Differential networks in json format are saved in the 'data' subdirectory and logs are saved in the 'logs' subdirectory."
      ;;
   "debug-fastviz" )
      echo -n "Launched generation of differential network files "
      net="debug"; echo -n "data/$net.sdnet "
      # verbose 3 prints graph properties
      # verbose 4 prints adjeciency matrix and lists
      time ./visualize_tweets_finitefile --verbose 4 --viztype fastviz --input data/$net.sdnet --timecontraction $((30*100)) --maxvisualized 5 --forgetconst 1.0 --edgemin 0.95 --forgetevery 40 --output "data/${net}_fastviz"
      ;;
   "debug-timewindow" )
      echo -n "Launched generation of differential network files "
      net="debug"; echo -n "data/$net.sdnet "
      # verbose 3 prints graph properties
      # verbose 4 prints adjeciency matrix and lists
      time ./visualize_tweets_finitefile --verbose 4 --viztype timewindow --input data/$net.sdnet --timecontraction $((30*100)) --maxvisualized 50 --timewindow 30 --edgemin 0.95 --output "data/${net}_timewindow"
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
      ./visualize_tweets_finitefile --verbose 2 --input $2 --server $3\
          --timecontraction $4
      ;;
    * )
      echo "Option not recognized."
      echo "Please try again using command line arguments specified below"
      echo ""
      print_description
      ;;
   esac


fi













