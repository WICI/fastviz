
# BOOST_SRC   = /home/pms/scitools/boost_1_53_0/include
# BOOST_BIN   = /home/pms/scitools/boost_1_53_0/lib
# JSON_SRC    = /home/pms/scitools/jsoncpp-src-0.5.0/include
# JSON_LIBMT    = libjson_linux-gcc-4.2.4_libmt
# JSON_BIN    = /home/pms/scitools/jsoncpp-src-0.5.0/libs/linux-gcc-4.2.4
# # CPPNET_LOC  = /home/pms/scitools/cpp-netlib-0.8.1
# # CPPNET_LOC  = /home/pms/scitools/cpp-netlib-0.9.4

CC = g++
CXX = g++
RM = /bin/rm

# -Wall
CXXFLAGS = -O3 -I. -I$(BOOST_SRC) -I $(JSON_SRC)
#  -I$(CPPNET_LOC)

LDFLAGS = -L$(BOOST_BIN) -Wl,-rpath $(BOOST_BIN) \
          -L$(JSON_BIN) -Wl,-rpath $(JSON_BIN)

LDLIBS  = -lboost_program_options -lboost_filesystem -lboost_regex \
          -lboost_date_time -lboost_system -l$(JSON_LIBMT)

OBJS =	util/format_time.o util/pace_checker.o \
	viz/net_collector.o viz/viz_selector.o viz/node.o \
	viz/client.o viz/client_gephi_light.o
# 	viz/client_gephi.o

all: $(OBJS) visualize_tweets_finitefile

visualize_tweets_finitefile: $(OBJS)

test: $(OBJS)

clean:
	find . -name '*.o' -delete
	find . -name '*~' -delete
	$(RM) -f visualize_tweets_finitefile
