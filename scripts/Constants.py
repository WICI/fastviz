'''
Created on Mar 12, 2013

Load configuration variables from file or sets them to default values

@author: Luca Maria Aiello
@author: Przemek Grabowicz
@version: 0.2
'''

#Path to config file
PATH_CONFIG_FILE='config_wici'

#DEFAULT PARAMETERS

#Paths
PATH_MENCODER=''
PATH_FRAMES_OUTPUT='./frames'
PATH_MOVIES_OUTPUT='./movies'

#JSON event markers
ADD_NODE='an'
DELETE_NODE='dn'
CHANGE_NODE='cn'
ADD_EDGE='ae'
DELETE_EDGE='de'
CHANGE_EDGE='ce'
ADD_LABEL='al'
DELETE_LABEL='dl'
CHANGE_LABEL='cl'

MOVIE_WIDTH=1280
MOVIE_HEIGHT=720
#MOVIE_WIDTH=1680
#MOVIE_HEIGHT=1050
FRAMES_PER_EVENT=1
FRAMES_PER_SECOND=30
MAX_EVENTS=3600
LAYOUT_MAX_ITERATIONS=5
LAYOUT_MAX_DELTA=0.1
NUMBER_FADEOUT_FRAMES=60

#Visualization parameters
INITIAL_NEWNODE_STATE=0

MIN_NODE_SIZE=10
MAX_NODE_SIZE=50
MIN_EDGE_SIZE=0
MAX_EDGE_SIZE=50
MIN_LABEL_SIZE=10
MAX_LABEL_SIZE=50

DEFAULT_LABEL_X_START=100
DEFAULT_LABEL_Y_START=100

DEFAULT_LABEL_SIZE=25
DEFAULT_LABEL_TEXT='new_label'

FEW_NODES=30
MEDIUM_NODES=60

BLIPING_SEQUENCE_LENGTH=18
BLIPING_DELTA=2
FADING_DELTA=1

# graytones
COLORCODE={ 'label_color':'#ffffff', 'vertex_color':'#000000', 'edge_color':'#b3b4c3', 'bg_color':'#62626d', 'text_color':'#ffffff33' }
# reddish
#COLORCODE={ 'label_color':'#ffffff', 'vertex_color':'#880000', 'edge_color':'#b3b4c3', 'bg_color':'#92626d', 'text_color':'#ffffff00' }
# bluenight
#COLORCODE={ 'label_color':'#ffffff', 'vertex_color':'#5555ff', 'edge_color':'#5555ff', 'bg_color':'#000000', 'text_color':'#ffffff' }

configuration = {}
try:
    config_file = open(PATH_CONFIG_FILE,'r')
    print 'Reading configuration file: ' + PATH_CONFIG_FILE 
    for line in config_file.readlines():
        line = line.replace('\n','').replace('\r','')
        if not (line == '' or line[0] == '#'):
            try:
                tokens = line.split('=')
                key=tokens[0].strip()
                value=tokens[1].strip()
                configuration[key]=value
            except:
                print 'Error in parsing data from configuration file at following line: "'+line+'"'
                exit(-1)
except:
    print 'No config file found, using default parameters'

for k in configuration:
    v=configuration[k]
    if k == 'path_mencoder':
        PATH_MENCODER=v
    if k == 'path_frames_output':
        PATH_FRAMES_OUTPUT=v
    if k == 'path_movies_output':
        PATH_MOVIES_OUTPUT=v
    if k == 'add_node':
        ADD_NODE=v
    if k == 'delete_node':
        DELETE_NODE=v
    if k == 'change_node':
        CHANGE_NODE=v
    if k == 'add_adge':
        ADD_EDGE=v
    if k == 'delete_edge':
        DELETE_EDGE=v
    if k == 'change_edge':
        CHANGE_EDGE=v
    if k == 'add_label':
        ADD_LABEL=v
    if k == 'delete_label':
        DELETE_LABEL=v
    if k == 'change_label':
        CHANGE_LABEL=v
    if k == 'movie_width':
        MOVIE_WIDTH=int(v)
    if k == 'movie_height':
        MOVIE_HEIGHT=int(v)
    if k == 'frames_per_event':
        FRAMES_PER_EVENT=int(v)
    if k == 'frames_per_second':
        FRAMES_PER_SECOND=int(v)
    if k == 'max_events':
        MAX_EVENTS=int(v)
    if k == 'layout_max_iterations':
        LAYOUT_MAX_ITERATIONS=int(v)
    if k == 'layout_max_delta':
        LAYOUT_MAX_DELTA=float(v)
    if k == 'number_fadeout_frames':
        NUMBER_FADEOUT_FRAMES=int(v)
    if k == 'initial_newnode_state':
        INITIAL_NEWNODE_STATE=int(v)
    if k == 'min_node_size':
        MIN_NODE_SIZE=int(v)
    if k == 'max_node_size':
        MAX_NODE_SIZE=int(v)
    if k == 'min_edge_size':
        MIN_EDGE_SIZE=int(v)
    if k == 'max_edge_size':
        MAX_EDGE_SIZE=int(v)
    if k == 'min_label_size':
        MIN_LABEL_SIZE=int(v)
    if k == 'max_label_size':
        MAX_LABEL_SIZE=int(v)
    if k == 'default_label_x_start':
        DEFAULT_LABEL_X_START=int(v)
    if k == 'default_label_y_start':
        DEFAULT_LABEL_Y_START=int(v)
    if k == 'default_label_size':
        DEFAULT_LABEL_SIZE=int(v)
    if k == 'default_label_text':
        DEFAULT_LABEL_TEXT=v
    if k == 'few_nodes':
        FEW_NODES=int(v)
    if k == 'nmedium_nodes':
        MEDIUM_NODES=int(v)
    if k == 'bliping_sequence':
        BLIPING_SEQUENCE_LENGTH=int(v)
    if k == 'bliping_delta':
        BLIPING_DELTA=int(v)
    if k == 'fading_delta':
        FADING_DELTA=int(v)
    if k == 'label_color':
        COLORCODE['label_color']=v
    if k == 'vertex_color':
        COLORCODE['vertex_color']=v
    if k == 'edge_color':
        COLORCODE['edge_color']=v
    if k == 'bg_color':
        COLORCODE['bg_color']=v
    if k == 'text_color':
        COLORCODE['text_color']=v
        