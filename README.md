Fast filtering and animation of large dynamic networks
======================================================

Introduction
------------

This project is a collection of two tools that filter and visualize large dynamic
networks. More specifically, the tools perform the following functions:

  * The filtering tool. From a chronological sequence of weighthed graph links produce differential updates to a subgraph of the network delegated for visualization in a format of JSON events.
  * The visualizing tool. Produce movies of evolving graphs from a feed of the JSON events.

In addition, the filtering tool can send the network updates directly to Gephi
Streaming API, which visualizes them interactively.

The filtering tool can be understood as a sliding time-window method with an exponential decay that is limited in computational complexity and memory usage. The method is introduced, described, and analyzed in the publication that is available at http://arxiv.org/abs/1308.0309, using datasets released within this project. In case you decide to use our method or datasets we kindly request that you cite the publication.

Dependencies
------------

The tools have been tested under Linux and Mac OS systems.

The filtering tool is written in C++ and needs to be compiled. The visualizing tool is a Python script requiring Python 2.6 or higher.

Before proceeding to next point of these instructions please check that all the required dependencies specified below are present on your system.

The filtering tool has the following dependencies on the external libraries and
command line tools:
  *  Boost library (http://www.boost.org/), version 1.44 or higher.
  *  JsonCpp library (http://www.boost.org/), version 0.5.0 or higher.
  *  cpp-netlib library (http://cpp-netlib.org), version 0.9.4 or higher.
  *  Gephi (https://gephi.org/), with installed plugin Graph Streaming API,
     it can be installed internally from Gephi
  *  igraph library (http://igraph.sourceforge.net/).

The visualizing tool has the following dependencies on the external libraries and
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

To configure the system before the compilation of the filtering tool one needs to
enter paths to the corresponding installed libraries in the configuration file
``vars.sh`` present in the parent directory of the project. After inputting the paths to the configuration file, in order to compile the
code run:

    ./compile.sh

This command will compile and copy the executable visualize_tweets_finitefile
to the parent directory of the project. Note that before running
``visualize_tweets_finitefile`` one needs to tell the linker where the compiled
libraries are. The simplest way of achiving it is by following the instructions
that are printed after ``compile.sh`` is successfully finished. The other
possibility is to use ``run.sh`` for launching the tools. This small script
configures the paths itself.


Running Tests
-------------

Both the tools can be tested by running:

    ./run.sh test

The filtering tool can be tested by running from the parent directory:

    ./visualize_tweets_finitefile --input data/test.sdnet --output data/test

The visualizing tool does not require installation and can be launched from the
parent directory of the project:

    python scripts/DynamicGraph_wici.py data/test.json


Input format
------------
The dynamic network that is given as the input to the filtering tool can have multiple edges and it can be either weigthed or unweigthed. The input file has to be sorted in chronological order with the epoch time used as time stamps. The input files have the following format for each of its lines:

    t1 n1 n2 w1
    t2 n1 n3 n4 w2
    ...

Where `t1` is an epoch time, `n1` stands for node 1, `n2` stands for node 2, and `w1` is the corresponding weight of the connection(s).

  *  Weigthed links - files with the extension ``wdnet``, to run the filtering method for this format use the ``--weigthed`` flag
  *  Unweigthed links  - files with the extension ``sdnet``, the same format, except the weights are not stored in the files

One cas see examples of ``wdnet`` and  ``sdnet`` input files in the directory ``data``.


Re-creating the demo movies
---------------------------

The tools are released together with four datasets that reside in the directory
``data``. For each of the datasets a demo movie has been created using the tools:
  *  Osama bin Laden's death on Twitter (http://youtu.be/z7goOblZbcI)
  *  Super Bowl on Twitter (http://youtu.be/uwHNtb5QZlE)
  *  IMDB movie keywords (http://youtu.be/rN5qWSTOgkM)
  *  US patent title words (http://youtu.be/3zR-GjgxKWE)

A detailed description of the demo movies is available the full publication
(available at http://arxiv.org/abs/1308.0309).

The script ``run.sh`` has been created to automatize the recreation of the
demo movie and to store the values of the parameters used for their generation.

To launch the filtering tool in order to convert the demo ``sdnet`` files that are
located in the directory ``data`` to ``json`` files:

    ./run.sh demo-diffnets

To launch the visualizing tool in order to create movies from the ``json``files
stored in the directory ``data`` and save them as ``avi`` in the directory
``movies``:

    ./run.sh demo-movies

Creating your own movies
------------------------

You can use the tools to create your own movies of dynamic networks. To learn how to set the parameters of the tools please see Appendix B of our publication (available at http://arxiv.org/abs/1308.0309). The parameters of the filtering tool are to be provided as arguments to ``visualize_tweets_finitefile`` (run ``visualize_tweets_finitefile -h`` for details), while the parameters of the visualizing tool are stored in the configuration file [scripts/Constants.py](scripts/Constants.py).

Launching interactive visualizations
------------------------------------

Before launching interactive visualizations one needs to run server in the Graph
Streaming API in Gephi. To do this launch Gephi, start new project
and in the panel called ``Streaming`` select ``Server``, and run it.
To make the visualization look better in the ``Labels`` panels turn on
node labels, select option ``Size`` proportional to ``Node size``, and
with the slider reduce the size of labels by half. Finally, get a
graph layout running (e.g., Fruchterman) in the panel called
``Layout``.

To launch stream the visualization directly to Gephi for the selected
``json`` file:

    ./run.sh gephi json_file server_ip_address time_contraction

The parameter time_contraction is important here. If is low (e.g., 100)
for a dataset that has a time span of 100 years, then the visualization
will last 1 year, needless to say way to long. To learn how to set the parameters of the tools please see Appendix B of our publication (available at http://arxiv.org/abs/1308.0309).






