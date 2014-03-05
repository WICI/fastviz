# export BOOST_SRC=~/scitools/
# export BOOST_BIN=~/scitools/boost_1_53_0/lib
# export JSON_SRC=~/scitools/jsoncpp-src-0.5.0/include
# export JSON_LIBMT=~/scitools/jsoncpp-src-0.5.0/libs/linux-gcc-4.2.4/libjson_linux-gcc-4.2.4_libmt
# export CPPNET_SRC=~/scitools/cpp-netlib-0.9.4
# export CPPNET_BIN=~/scitools/cpp-netlib-0.9.4/build/libs/network/src


CC = g++
CXX = g++
RM = /bin/rm

# -Wall
CXXFLAGS += -O3 -std=c++11 -I. -I$(BOOST_SRC) -I$(CPPNET_SRC) -I $(JSON_SRC) \
			   -pthread -I/opt/local/include/igraph

LDFLAGS += -L$(BOOST_BIN) -Wl,-rpath $(BOOST_BIN) \
          -L$(JSON_BIN) -Wl,-rpath $(JSON_BIN) \
          -L$(CPPNET_BIN) -L/opt/local/lib

LDLIBS  += -lboost_program_options -lboost_filesystem -lboost_regex -lboost_chrono \
          -lboost_date_time -lboost_system -lboost_thread \
          -lcppnetlib-client-connections -lcppnetlib-uri \
          -lcppnetlib-server-parsers -lssl -lcrypto \
          -l$(JSON_LIBMT) -lm -ligraph

OBJS =	util/format_time.o util/pace_checker.o \
			viz/net_collector_timewindow.o viz/link.o
# 	viz/net_collector.o viz/viz_selector.o viz/node.o \
# 	viz/client.o viz/client_gephi_light.o \
# 	viz/client_gephi.o

objs: $(OBJS)

visualize_tweets_finitefile: $(OBJS)

all: $(OBJS) visualize_tweets_finitefile

clean:
	find . -name '*.o' -delete
	find . -name '*~' -delete
	$(RM) -f visualize_tweets_finitefile
