#!/bin/bash

. vars.sh

if [ ! -d "$BOOST_SRC" ]; then
   echo "Boost include path does not exist"
   echo "Please input correct BOOST_SRC in the file vars.sh"
   exit
fi
if [ ! -d "$BOOST_BIN" ]; then
   echo "Boost path to its binaries does not exist"
   echo "Please input correct BOOST_BIN in the file vars.sh"
   exit
fi
if [ ! -d "$JSON_SRC" ]; then
   echo "Json-cpp location does not exist"
   echo "Please input correct JSON_SRC in the file vars.sh"
   exit
fi
if [ ! -f "${JSON_LIBMT}.a" -a ! -f "${JSON_LIBMT}.so" ]; then
   echo "Libraries "${JSON_LIBMT}".{a,so} do not exist"
   echo "Please input correct JSON_LIBMT in the file vars.sh"
   exit
else
   export JSON_BIN=$(dirname ${JSON_LIBMT})
   export JSON_LIBMT=$(basename ${JSON_LIBMT} | sed 's/^lib//' )
   echo $JSON_BIN
   echo $JSON_LIBMT
fi

cd ./src
make clean
make visualize_tweets_finitefile || { echo 'Compilation failed' ; exit 1; }
mv visualize_tweets_finitefile ..
cd ..

echo "========================================================================="
echo "The compilation has finished. Now you can launch run.sh"
echo ""
echo "In case you want to run standalone visualize_tweets_finitefile, check if the necessary libraries are in your linking path, or add them before launching the visualize_tweets_finitefile using:"
if [[ "$OSTYPE" != *"darwin"* ]]; then
   echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:$JSON_BIN:$BOOST_BIN"
else
   echo "export DYLD_LIBRARY_PATH=\$DYLD_LIBRARY_PATH:$JSON_BIN:$BOOST_BIN"
fi

