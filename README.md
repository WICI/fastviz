Fast visualization of relevant portions of large dynamic networks
===================

Introduction
------------

This project is a collection of two tools for visualization of large dynamic 
networks, that perform the following functions, respectively:

  *  From a chronological sequence of graph links in form of sdnet files
     produce differential updates to a subgraph of the network delegated
     fro visualization in form of JSON events.
     (``src/visualize_tweets_finitefile.cpp``)
  *  Produce movies of evolving graphs from a feed of JSON events
     (``scripts/DynamicGraph_wici.py``).

In addition, the first tool can send the network updates directly to Gephi
Streaming API, that visualizes them interactively.

Dependencies
------------

The tools have been tested under Linux and Mac OS systems. In future they can
be easily ported to Windows, however, this has not been tested yet.

The first tool is written in C++ and needs to be compiled.
The second tool is a Python script, that requires Python 2.6 or higher.

Before proceeding to next point of these instructions please check that
all the required dependencies specified below are present on your system.

The first tool has the following dependencies on the external libraries and 
command line tools:
  *  Boost library (http://www.boost.org/), version 1.44 or higher.
  *  JsonCpp library (http://www.boost.org/), version 0.5.0 or higher.
  *  cpp-netlib library (http://cpp-netlib.org), version 0.9.4 or higher.
  *  Gephi (https://gephi.org/), with installed plugin Graph Streaming API,
     it can be installed internally from Gephi

The second tool has the following dependencies on the external libraries and 
command line tools:
  *  igraph library (http://igraph.sourceforge.net/), with igraph Python 
     extension module, version 6.0 or higher.
  *  Cairo library (http://cairographics.org/), it is a prerequisite of igraph
     to be able to plot graphs, any version that works with the igraph installed
     is fine.
  *  Python bindings for Cairo (http://cairographics.org/pycairo/).
  *  mencoder command line tool (http://en.wikipedia.org/wiki/Mencoder)
     It is a standard tool present in many modern systems and available in
     standard repositories.


Configuring and building
------------------------

To configure the system before the compilation of the first tool one needs to
enter paths to the corresponding installed libraries in the configuration file 
``vars.sh`` present in the parent directory of the project.

After inputting the paths to the configuration file, in order to compile the 
code run:

    ./compile.sh

This command will compile and copy the executable visualize_tweets_finitefile
to the parent directory of the project. Note, that before running 
``visualize_tweets_finitefile`` one needs to tell the linker where the compiled 
libraries are. The simplest way of achiving it is by following the instructions 
that are printed after ``compile.sh`` is successfully finished. The other
possibility is to use ``run.sh`` for launching the tools. This small script
configures the paths itself.

Running Tests
-------------

Both the tools can be tested by running:

    ./run.sh test

The first tool can be tested by running from the parent directory:

    ./visualize_tweets_finitefile --input data/test.sdnet --output data/test

The second tool does not require installation and can be launched from the
parent directory of the project:

    python scripts/DynamicGraph_wici.py data/test.json


Re-creating the demo movies
---------------------------

The script ``run.sh`` has been created to automatize the recreation of the 
demo movie and to store the values of the parameters used for their generation. 

To launch the first tool in order to convert the demo .sdnet files that are 
located in ``data`` to ``json`` files:
     
    ./run demo-diffnets
     
To launch the second tool in order to create movies from the ``json``files 
stored in the ``data`` subdir and save them as ``avi`` in the ``movies`` 
subdir:
     
    ./run demo-movies


Launching interactive visualizations
------------------------------------

Before launching this visualizations one needs to run server in the Graph
Streaming API in Gephi. To do this launch Gephi, start new project
and in the panel called ``Streaming`` select ``Server``, and run it. 
To make the visualization look better in the ``Labels`` panels turn on 
node labels, select option ``Size`` proportional to ``Node size``, and 
with the slider reduce the size of labels by half. Finally, get a
graph layout running (e.g. Fruchterman) in the panel called
``Layout``.

To launch stream the visualization directly to Gephi for the selected 
``json`` file:
     
    ./run gephi json_file server_ip_address time_contraction
     
The parameter time_contraction is important here. If is low, e.g. 100, 
for a dataset that has a time span of 100 years, then the visualization 
will last 1 year, needless to say way to long. Please pick it carefully.
For an idea of what value should it be set to please look into
the values of this parameter set for different datasets inside
of the script ``run.sh``.






