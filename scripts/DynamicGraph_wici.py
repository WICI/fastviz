'''
Created on Mar 12, 2013
Visualization of the evolution of dynamic graphs
Produces movies of evolving graphs from a feed of JSON events

@author: Luca Maria Aiello 
@author: Przemek Grabowicz
@version: 1.0
'''
#Python standard libraries
import sys, os, math, random, shutil, time, datetime, traceback, json
#iGraph and Cairo plotting
import igraph, cairo
from igraph.layout import Layout

import Constants as const

#random generator initialization
random.seed()

class DynamicGraph():
    """
    An object embedding the evolution of the graph. It receives in input graph update events
    and it generates in output a movie produced with the mencoder tool. 
    
    @param layout_type the graph layout. In the current version, only the Fruchterman-Reingold ('fr') layout is available
    @param width the width of the movie, in pixels
    @param height the height of the movie, in pixels
    @param frames_dir directory where movie frames are saved
    @param movie_dir directory where the movie file is saved
    """
    def __init__ (self, layout_type='fr', width=800, height=600, frames_dir='./frames', movie_dir='./movie'):
        """
        Creates a new instance of DynamicGraph
        """
        #the graph
        self.g = igraph.Graph(0)
        #external nodeId -> node index (igraph id)
        self.nodes_dict={}
        #external edgeId -> edge index (igraph id)
        self.edges_dict={}
        #labelname -> dictionary of its properties
        self.labels_dict={}
        
        #dictionary of newly added nodes. 
        self.new_nodes={}
        self.new_nodes_support={}
        #dictionary of nodes that must be deleted
        self.sentenced_nodes=set()

        #the list of events representing the dynamic of the graph evolution
        self.events=[]
        
        #graph layout [only Fruchterman-Reingold layout is supported in this version!]
        self.supported_layouts = ['fr']
        if layout_type not in self.supported_layouts:
            raise Exception('Layout '+str(layout_type)+' not supported')
        self.layout_type=layout_type
        self.layout=[]
        #weights for force-driven layouts
        self.edge_weights={}
        
        #counter of produced frames
        self.framecount=0
        #node colors
        self.default_node_color='#0000FF'
        self.default_edge_color='#0000FF'
        #boundaries of plot
        self.minX=0.0
        self.maxX=0.0
        self.minY=0.0
        self.maxY=0.0
        #movie resolution
        self.movie_width=width
        self.movie_height=height
        
        #path to frames and movie
        self.path_to_frames=frames_dir
        self.path_to_movie=movie_dir
        self.input_file=None
         
        self.label_offset=0

    def drawclock(self,timestring, ctx, x, y, size=50):
        """
        Draws the clock on the movie
        
        @param timestring the string representing the time, in the format "%Y-%b-%d %H:%M:%S"
        @param ctx ???
        @param x the x position of the clock
        @param y the y position of the clock
        @param size the size of the clock
        """
        PI=math.pi
        dt = datetime.datetime.fromtimestamp(time.mktime((time.strptime(timestring,"%Y-%b-%d %H:%M:%S"))))
        hour = dt.hour
        minutes = dt.minute
        
        #clock face
        yinyang = 1-0.5*(1+math.cos(hour/24.0*2*PI))
        ctx.set_source_rgba( yinyang, yinyang, yinyang, 0.3 )
        radius = size/2 - 0.8
        ctx.arc( size/2.0+x, size/2.0+y, radius, 0, 2*PI)
        ctx.fill()
        ctx.stroke()
    
        #hours, minutes and second
        ctx.move_to( size/2.0+x, size/2.0+y)
        ctx.set_source_rgba(0.1, 0.1, 0.1, 0.5)
        per_hour = (2 * PI) / 12
        dh = (hour * per_hour) + ((per_hour / 60) * minutes)
        dh += 2 * PI / 4    
        ctx.set_line_width(0.05 * radius)
        ctx.rel_line_to(-0.5 * radius * math.cos(dh), -0.5 * radius * math.sin(dh))
        
        ctx.move_to(size/2+x, size/2+y)    
        per_minute = (2 * PI) / 60
        dm = minutes * per_minute
        dm += 2 * PI / 4    
        ctx.rel_line_to(-0.9 * radius * math.cos(dm), -0.9 * radius * math.sin(dm))
        
        ctx.stroke()
        
        #set date
        ctx.move_to( x-600, 90 )
        ctx.select_font_face("sans-serif", cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_BOLD)
        ctx.set_font_size(90)
        ctx.set_source_rgba(1, 1, 1, 0.1)
        ctx.text_path(dt.strftime("%Y-%m-%d"))
        ctx.fill()

    def bliping(self):
        """
        Handles the animation of nodes entering the network
        """
        for nodename in self.new_nodes.keys():
            node_idx=self.nodes_dict[nodename]
            status = self.new_nodes[nodename]
            if status < const.BLIPING_SEQUENCE_LENGTH:
                if nodename in self.new_nodes_support:
                    if self.new_nodes_support[nodename] > self.g.vs[node_idx]['size']:
                        delta = const.BLIPING_DELTA
                    else:
                        delta = -const.BLIPING_DELTA
                else :
                    if status < const.BLIPING_SEQUENCE_LENGTH/2:
                        delta = const.BLIPING_DELTA
                    else:
                        delta = -const.BLIPING_DELTA
                old_node_size=self.g.vs[node_idx]['size']
                new_node_size = min(max(old_node_size+delta,const.MIN_NODE_SIZE),const.MAX_NODE_SIZE)
                #old_label_size = self.g.vs[node_idx]['label_size']
                new_label_size = self.rescale_nodelabel_size(new_node_size)
                #new_label_size = min(max(old_label_size+delta, MIN_LABEL_SIZE), MAX_LABEL_SIZE)
                self.g.vs[node_idx]['size']=new_node_size
                self.g.vs[node_idx]['label_size']=new_label_size
                self.new_nodes[nodename]+=1
            else:
                self.new_nodes.pop(nodename)
    
    def fading(self):
        """
        Handles the animation of nodes churning out
        """
        to_delete=[]
        for nodename in self.sentenced_nodes:
            node_idx=self.nodes_dict[nodename]
            self.g.vs[node_idx]['size']-=const.FADING_DELTA
            self.g.vs[node_idx]['label_size']-=const.FADING_DELTA
            if self.g.vs[node_idx]['size'] <=0 or self.g.vs[node_idx]['label_size'] <=0:
                to_delete.append(nodename)
        for nodename in to_delete:
            self.delete_node(nodename)
            self.sentenced_nodes.remove(nodename)
    
    def rescale_nodelabel_size(self, size):
        """
        Rescales a label size
        @param size: the size to be rescaled 
        """
        newlabelsize=(2*math.sqrt(size))+10
        
        if newlabelsize>const.MAX_LABEL_SIZE:
            newlabelsize=const.MAX_LABEL_SIZE
        if newlabelsize<const.MIN_LABEL_SIZE:
            newlabelsize=const.MIN_LABEL_SIZE
        return newlabelsize
    
    def hexRGB(self,r,g,b):
        """
        Given a color specified by the three RGB components in [0,1],
        produces a string representing the HEX code for that color
        (e.g. '#FFABC1')
        @param r: the red component in [0,1]
        @param g: the green component in [0,1]
        @param b: the blue component in [0,1] 
        """
        if r==None or g==None or b== None:
            raise Exception('R G and B value must be non null')
        r = r if r<=1.0 else 1.0
        r = r if r>=0.0 else 0.0
        g = g if g<=1.0 else 1.0
        g = g if g>=0.0 else 0.0
        b = b if b<=1.0 else 1.0
        b = b if b>=0.0 else 0.0
        rgb='#'
        r=hex(int(r*255)).replace('0x','').upper()
        g=hex(int(g*255)).replace('0x','').upper()
        b=hex(int(b*255)).replace('0x','').upper()
        if len(r) == 1:
            r='0'+r
        if len(g) == 1:
            g='0'+g
        if len(b) == 1:
            b='0'+b
        rgb+=r+g+b
        return rgb
    
    def add_node(self,nodename,properties):
        """
        Add a new node to this DynamicGraph.
        @param nodename: a string representing the id of the node
        @param properties: a dictionary of properties (e.g., 'size'->10)
        """
        if nodename not in self.nodes_dict.keys() or nodename in self.sentenced_nodes:
            if nodename in self.sentenced_nodes:
                self.sentenced_nodes.remove(nodename)
                self.change_node(nodename,properties)
            else :
                #add the new node to the graph and to the data structures
                self.g.add_vertices(1)
                node_idx=self.g.vcount()-1
                self.nodes_dict[nodename]=node_idx
                self.new_nodes[nodename]=const.INITIAL_NEWNODE_STATE
                #default values of the main properties
                self.g.vs[node_idx]['size']=const.MIN_NODE_SIZE
                if 'vertex_color' in const.COLORCODE.keys():
                    self.g.vs[node_idx]['color']=const.COLORCODE['vertex_color']
                self.g.vs[node_idx]['label']=""
                self.g.vs[node_idx]['label_size']=const.MIN_LABEL_SIZE
                if 'label_color' in const.COLORCODE.keys():
                    self.g.vs[node_idx]['label_color']=const.COLORCODE['label_color']
                
                self.change_node(nodename,properties)
                layout_coord = self.generate_random_layout_position()
                self.layout.append(layout_coord)
#        else :
#            print 'Warning! Trying to add an already existent node ' + nodename 
    
    def sentence_node(self,nodename):
        """
        Marks a node for future deletion
        @param nodename the id of the node
        """
        self.sentenced_nodes.add(nodename)
        if nodename in self.new_nodes.keys():
            self.new_nodes.pop(nodename)
    
    def delete_node(self,nodename):
        """
        Deletes a node from the DynamicGraph
        @param nodename the id of the node
        """
        if nodename in self.nodes_dict.keys():
            deleted_nodename_idx=self.nodes_dict[nodename]
            adjacent_edges_idxs=set(self.g.adjacent(deleted_nodename_idx))
            self.g.delete_vertices(deleted_nodename_idx)
            self.nodes_dict.pop(nodename)
            if self.new_nodes.has_key(nodename):
                self.new_nodes.pop(nodename)
            #rescales all the node indexes
            for n in self.nodes_dict.keys():
                if self.nodes_dict[n] > deleted_nodename_idx:
                    self.nodes_dict[n] = self.nodes_dict[n]-1
            #rescale the layout
            del self.layout[deleted_nodename_idx]
            #erase the adjacent edges in the edge dictionary and rescales the indexes
            rescale_count=0
            #reorder the edges dictionary by value
            edges_ordered_by_val = sorted(self.edges_dict.items(), key=lambda x: x[1])
            for e in edges_ordered_by_val:
                name=e[0]
                if self.edges_dict[name] in adjacent_edges_idxs:
                    self.edges_dict.pop(name)
                    self.edge_weights.pop(name)
                    rescale_count+=1
                else:
                    self.edges_dict[name]=self.edges_dict[name]-rescale_count
#        else :
#            print 'Warning! Trying to delete a non existent node ' + nodename 
        
    def change_node(self,nodename,properties):
        """
        Changes the properties of a node in the DynamicGraph
        @param nodename the id of the node
        @param the new dictionary of properties
        """
        #modify the properties
        if nodename in self.nodes_dict:
            node_idx = self.nodes_dict[nodename]
            r=None
            g=None
            b=None
            for pname in properties.keys():
                pvalue=properties[pname]
                if pname == 'r':
                    r=float(pvalue)
                elif pname == 'g':
                    g=float(pvalue)
                elif pname == 'b':
                    b=float(pvalue)
                elif pname == 'size':
                    if nodename not in self.sentenced_nodes:
                        new_size = min(max(float(pvalue),const.MIN_NODE_SIZE), const.MAX_NODE_SIZE)
                        if nodename not in self.new_nodes:
                            self.g.vs[node_idx]['size']=float(new_size)
                            newlabelsize=self.rescale_nodelabel_size(self.g.vs[node_idx]['size'])
                            self.g.vs[node_idx]['label_size']=newlabelsize
                        #else:
                        #    self.new_nodes_support[nodename]=new_size
                elif pname == 'label':
                    self.g.vs[node_idx]['label']=str(pvalue)
                else:
                    self.g.vs[node_idx][str(pname)]=pvalue
            if r!=None and g!=None and b!= None:
                self.g.vs[node_idx]['color']=self.hexRGB(r, g, b)
        else:
            raise Exception("Warning! Trying to modify a non existent node: " + str(nodename))
    
    def add_edge(self,edgename,nodename1,nodename2,properties):
        """
        Adds an edge in the DynamicGraph
        @param edgename the id of the edge
        @param nodename1 the id of the first node
        @param nodename2 the id of the second node
        @param properties the dictionary of edge properties
        """
        if edgename not in self.edges_dict.keys():
            if nodename1!=None and nodename2!=None:
                if nodename1 not in self.nodes_dict.keys():
#                    print "Warning! Trying to add an edge with an inexistent endpoint " + str(nodename1)
                    properties={}
                    properties['label']=""
                    self.add_node(nodename1, properties)
                if nodename2 not in self.nodes_dict.keys():
#                    print "Warning! Trying to add an edge with an inexistent endpoint " + str(nodename2)
                    properties={}
                    properties['label']=""
                    self.add_node(nodename2, properties)
            else:
                raise Exception("Both endpoints should be specified in a edge addition")
            
            n1_idx = self.nodes_dict[nodename1]
            n2_idx = self.nodes_dict[nodename2]
            
            if not self.g.are_connected(n1_idx, n2_idx):
                self.g.add_edges([(n1_idx,n2_idx)])
                edge_idx = self.g.ecount()-1
                self.edges_dict[edgename]=edge_idx
                weight=1
                self.edge_weights[edgename]=weight
                self.change_edge(edgename, properties)
#            else :
#                print 'Warning! Trying to connect two already connected nodes' + nodename1+','+nodename2+' with a new edge ' + edgename 
#        else:
#            print 'Warning! Trying to add an existent edge ' + edgename 
    
    def delete_edge(self,edgename):
        """
        Deletes an edge from the DynamicGraph
        @param nodename the id of the edge
        """
        if edgename in self.edges_dict.keys():
            deleted_edgename_idx=self.edges_dict[edgename]
            self.g.delete_edges(deleted_edgename_idx)
            self.edges_dict.pop(edgename)
            #rescales all the indexes
            for e in self.edges_dict.keys():
                if self.edges_dict[e] > deleted_edgename_idx:
                    self.edges_dict[e] = self.edges_dict[e]-1
            self.edge_weights.pop(edgename)
#        else :
#            print "Warning! Trying to delete a non-existent edge " + str(edgename)
    
    def change_edge(self,edgename,properties):
        """
        Changes the properties of an edge in the DynamicGraph
        @param edgename the id of the edge
        @param properties the new dictionary of properties
        """
        if edgename in self.edges_dict.keys():
            edge_idx=self.edges_dict[edgename]
            r=None
            g=None
            b=None
            for pname in properties.keys():
                pvalue = properties[pname]
                if pname == 'r':
                    r=float(pvalue)
                elif pname == 'g':
                    g=float(pvalue)
                elif pname == 'b':
                    b=float(pvalue)
                elif pname == 'weight':
                    self.g.es[edge_idx]['weight']=float(pvalue)
                    self.g.es[edge_idx]['width']=float(pvalue)
                else:
                    self.g.es[edge_idx][str(pname)]=pvalue
            if r!=None and g!=None and b!= None:
                self.g.es[edge_idx]['color']=self.hexRGB(r, g, b)
#        else :
#            print "Warning! Trying to modify a non-existent edge " + str(edgename)
    
    def add_label(self, labelname, properties):
        """
        Adds a new label
        @labelname the id of the label
        @properties a dictionary of the label properties (x,y,size,color,label)
        """
        if labelname not in self.labels_dict.keys():
            #add the new node to the graph and to the data structures
            self.labels_dict[labelname]={}
            #default values of the main properties
            self.labels_dict[labelname]['x']=50.0
            self.labels_dict[labelname]['y']=50.0+self.label_offset
            self.label_offset+=27
            self.labels_dict[labelname]['size']=const.DEFAULT_LABEL_SIZE
            self.labels_dict[labelname]['color']=const.COLORCODE['text_color']
            if 'label' not in properties.keys():
                self.labels_dict[labelname]['label']=const.DEFAULT_LABEL_TEXT
#                print 'Warning! Empty label added (no "label" attribute found) ' + labelname + '. Using default text.' 
            self.change_label(labelname,properties)
#        else:
#            print 'Warning! Trying to add an already existent label ' + labelname 
    
    def change_label(self, labelname, properties):
        """
        Changes a label
        @param labelname the label id
        @param properties the new dictionary of properties (x,y,size,color,label)
        """
        #modify the properties
        if labelname in self.labels_dict:
            r=None
            g=None
            b=None
            for pname in properties.keys():
                pvalue=properties[pname]
                if pname == 'x':
                    self.labels_dict[labelname]['x']=float(pvalue)
                if pname == 'y':
                    self.labels_dict[labelname]['y']=float(pvalue)
                elif pname == 'r':
                    r=float(pvalue)
                elif pname == 'g':
                    g=float(pvalue)
                elif pname == 'b':
                    b=float(pvalue)
                elif pname == 'size':
                    self.labels_dict[labelname]['size']=float(pvalue)
                elif pname == 'label':
                    self.labels_dict[labelname]['label']=str(pvalue)
                else:
                    self.labels_dict[labelname][str(pname)]=pvalue            
            if r!=None and g!=None and b!= None:
                self.labels_dict[labelname]['color']=self.hexRGB(r, g, b)
#        else:
#            print "Warning! Modifying a inexistent label: " + str(labelname)
    
    def delete_label(self,labelname):
        """
        Deletes a label
        @param labelname the id of the label
        """
        if labelname in self.labels_dict:
            self.labels_dict.pop(labelname)
    
    def update(self, event):
        """
        Updates the graph with the data contained in the event given in input
        Each event is a sequence of JSON objects that are sub-events that are suppose 
        to happen at the same time.
        @param event: a list of parsed JSON objects
        """
        #this handles the case of the empty JSON event {}. In this case nothing is done
        if len(event[0]) == 0:
            return
        
        #the first layout is handled differently
        firstLayout=False
        if len(self.layout) == 0 :
            firstLayout=True
        
        for sub_event in event:
            event_type = sub_event.keys()[0]
            
            if event_type == const.ADD_NODE:
                nodename = sub_event[event_type].keys()[0]
                properties={}
                for k in sub_event[event_type][nodename]:
                    properties[k]=sub_event[event_type][nodename][k]
                self.add_node(nodename, properties)
                                    
            elif event_type == const.DELETE_NODE:
                nodename = sub_event[event_type].keys()[0]
                self.sentence_node(nodename)
                #self.delete_node(nodename)
            
            elif event_type == const.CHANGE_NODE:
                nodename = sub_event[event_type].keys()[0]
                properties={}
                for k in sub_event[event_type][nodename]:
                    properties[k]=sub_event[event_type][nodename][k]
                self.change_node(nodename, properties)
            
            elif event_type == const.ADD_EDGE:
                edgename = sub_event[event_type].keys()[0]
                nodename1=None
                nodename2=None
                directed=None
                properties={}
                for k in sub_event[event_type][edgename]:
                    if k == 'source':
                        nodename1=sub_event[event_type][edgename][k]
                    elif k == 'target':
                        nodename2=sub_event[event_type][edgename][k]
                    if k == 'directed':
                        if sub_event[event_type][edgename][k] == 'true':
                            directed=True
                        else:
                            directed=False
                    else:
                        properties[k]=sub_event[event_type][edgename][k]
                if directed == None:
                    raise Exception('Must specify directed/undirected edges')
                
                self.add_edge(edgename, nodename1, nodename2, properties)
                
            elif event_type == const.DELETE_EDGE:
                edgename = sub_event[event_type].keys()[0]
                self.delete_edge(edgename)
                
            elif event_type == const.CHANGE_EDGE:
                edgename = sub_event[event_type].keys()[0]
                properties={}
                for k in sub_event[event_type][edgename]:
                    properties[k]=sub_event[event_type][edgename][k]
                self.change_edge(edgename, properties)
                
            elif event_type == const.ADD_LABEL:
                labelname = sub_event[event_type].keys()[0]
                properties={}
                for k in sub_event[event_type][labelname]:
                    properties[k]=sub_event[event_type][labelname][k]
                self.add_label(labelname, properties)
                
            elif event_type == const.DELETE_LABEL:
                labelname = sub_event[event_type].keys()[0]
                self.delete_label(labelname)
                
            elif event_type == const.CHANGE_LABEL:
                labelname = sub_event[event_type].keys()[0]
                properties={}
                for k in sub_event[event_type][labelname]:
                    properties[k]=sub_event[event_type][labelname][k]
                self.change_label(labelname, properties)
                
            else:
                raise Exception('Unknown event event_type: '+event_type)
        
        if firstLayout : #produces a new layout
            self.layout = self.g.layout_fruchterman_reingold(maxiter=500, coolexp=1.5,seed=None)
            if len(self.layout)==0:
                self.minX, self.maxX, self.minY, self.maxY = 0.0, 1.0, 0.0, 1.0
            else:
                x, y = zip(*self.layout)
                #handles the case in which there's just one node in the layout
                if min(x) == max(x) and min(y)== max(y):
                    self.minX, self.maxX, self.minY, self.maxY = min(x)-1.0, max(x)+1.0, min(y)-1.0, max(y)+1.0
                else:
                    self.minX, self.maxX, self.minY, self.maxY = min(x), max(x), min(y), max(y)
        
        self.normalize_weights()
    
    def generate_random_layout_position(self):
        if len(self.layout) == 0:
            self.minX, self.maxX, self.minY, self.maxY = 0.0, 1.0, 0.0, 1.0
        else:
            x, y = zip(*self.layout)
            minx, maxx, miny, maxy = min(x), max(x), min(y), max(y)
            if minx < self.minX:
                self.minX=minx
            if maxx > self.maxX:
                self.maxX=maxx
            if miny < self.minY:
                self.minY=miny
            if maxy > self.maxY:
                self.maxY=maxy
        new_coords=[random.uniform(self.minX,self.maxX),random.uniform(self.minY,self.maxY)]  
        return new_coords
    
    def normalize_weights(self):
        for nodename in self.nodes_dict.keys():
            if nodename not in self.sentenced_nodes and nodename not in self.new_nodes:
                node_id=self.nodes_dict[nodename]
                newsize=4*math.sqrt(self.g.vs[node_id]['size'])
                if newsize < const.MIN_NODE_SIZE : newsize = const.MIN_NODE_SIZE
                if newsize > const.MAX_NODE_SIZE : newsize = const.MAX_NODE_SIZE
                self.g.vs[node_id]['size'] = newsize
        for edge in self.edges_dict.values():
            newsize=math.sqrt(self.g.es[edge]['weight'])
            source=self.g.es[edge].source
            target=self.g.es[edge].target
            source_size=self.g.vs[source]['size']
            target_size=self.g.vs[target]['size']
            newsize=min(newsize,source_size,target_size)
            if newsize < const.MIN_EDGE_SIZE : newsize = const.MIN_EDGE_SIZE
            if newsize > const.MAX_EDGE_SIZE : newsize = const.MAX_EDGE_SIZE
            self.g.es[edge]['width'] = newsize

    def insert_boundaries(self,graph_layout):
        """
        Insert a 4 hidden nodes at the four corners of the provided graph layout.
        The four nodes act as a frame for the layout, avoiding the automatic resize of the frames.  
        """
        graph_layout.append([self.minX,self.minY])
        graph_layout.append([self.minX,self.maxY])
        graph_layout.append([self.maxX,self.minY])
        graph_layout.append([self.maxX,self.maxY])
        self.g.add_vertices(4)
        for i in [1,2,3,4]:
            self.g.vs[len(graph_layout)-i]['label']=''
            self.g.vs[len(graph_layout)-i]['shape']='hidden'
    
    def remove_boundaries(self,graph_layout):
        """
        Remove the 4-nodes frame from the provided graph_layout
        """
        n = len(graph_layout)
        for i in [1,2,3,4]:
            self.g.delete_vertices([n-i])
            del graph_layout[n-i]

    def plot(self, target, bgcolor, bbox, *args, **kwds):
        if not isinstance(bbox, igraph.drawing.BoundingBox):
            bbox=igraph.drawing.BoundingBox(bbox)
        surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, int(bbox.width), int(bbox.height))
        result = igraph.drawing.Plot(target=surface, bbox=bbox)
        ctx = result._ctx
        
        cw, ch = bbox.width/60., bbox.height/60.  
        ctx.set_source_rgba(*result._palette.get(bgcolor))
        ctx.rectangle(0, 0, bbox.width, bbox.height)
        ctx.fill()

        result.add(self.g, bbox.contract((cw, ch, cw, ch)), *args, **kwds)
        result.redraw()
        
        for key in self.labels_dict.keys(): #values():
            if (key=="datetime"):
                self.drawclock(self.labels_dict[key]["label"], ctx, bbox.width-105, 5, 100)
            else:
                #position
                ctx.move_to(float(self.labels_dict[key]["x"]),float(self.labels_dict[key]["y"]))
                #color
                col=list(result._palette.get(self.labels_dict[key]["color"]))
                ctx.set_source_rgba( *col )
                #text size
                ctx.set_font_size(self.labels_dict[key]["size"])
                #text
                ctx.text_path(self.labels_dict[key]["label"])
                ctx.fill()
    
        if target is None: result.show()
        if isinstance(target, basestring):
            #result.fname=target
            result.save(target)

    def draw_frame(self,max_iter,max_delta):
        """
        Run the layout algorithm once and draw a single frames of the new node positions 
        """
        new_frame_name = '%08d.png' % self.framecount
        self.framecount+=1
        
        weight_vector=[]
        edges_ordered_by_val = sorted(self.edges_dict.items(), key=lambda x: x[1])
        for e in edges_ordered_by_val:
            edgename=e[0]
            weight_vector.append(self.edge_weights[edgename])
        
        self.layout = self.g.layout_fruchterman_reingold(weights=weight_vector, repulserad=(self.g.vcount()**3), maxiter=max_iter, maxdelta=max_delta, coolexp=1.5, seed=self.layout, area=self.g.vcount()*self.g.vcount() )
        self.insert_boundaries(self.layout)
        visual_style={}
        visual_style["vertex_label_dist"] = [0]*len(self.layout)
        
        if 'vertex_color' in const.COLORCODE.keys():
            for idx in range(self.g.vcount()): self.g.vs[idx]["color"]=const.COLORCODE['vertex_color']
        if 'edge_color' in const.COLORCODE.keys():
            for idx in range(self.g.ecount()): self.g.es[idx]["color"]=const.COLORCODE['edge_color']
        #igraph.drawing.plot(self.g, layout=self.layout, bbox=(0,0,self.movie_width,self.movie_height), target=self.path_to_frames+'/'+new_frame_name, **visual_style)
        self.plot(layout=self.layout, target=self.path_to_frames+'/'+new_frame_name, bbox=(0,0,self.movie_width,self.movie_height), bgcolor=const.COLORCODE['bg_color'], **visual_style )
        
        self.remove_boundaries(self.layout)
        
    #INTERFACE METHODS ------------------------------------------------------------------------------
    
    def read_json_events(self,jsonfile):
        """
        Read the graph events from the provided json file
        @param jsonfile: the file containing the json events 
        """
        events_list=[]
        self.input_file=jsonfile
        infile= open(jsonfile,'r')
        for line in infile.readlines():
            subevents = line.split("\r")
            subevents_list=[]
            for subevent in subevents :
                #deals with possible empty lines
                if subevent != None and subevent != "" and subevent != "\n" and subevent != "\r":
                    obj = json.loads(subevent)
                    subevents_list.append(obj)
            events_list.append(subevents_list)
        self.events=events_list
    
    def make_frames(self, frames_per_event, max_events, layout_max_iterations, layout_max_delta, number_fadeout_frames):
        """
        Produce the movie frames from the events.
        If there are no events to process, nothing is done and None is returned
        @param frames_per_event: how many frames are used for each event in the event list
        @param maxevents: the maximum number of events for which frames are produced
        @param layout_max_iterations: number of iterations of the graph layout algorithm for each frame
        @param layout_max_delta: maximum delta between graph layouts in consecutive frames
        @param number_fadeout_frames: the number of frames after the last event 
        """
        if len(self.events) == 0:
            return None

        eventcount=0
        for event in self.events:
            #update the graph with the new event
            self.update(event)
            eventcount+=1
            #produce frames just for the maximum number of events specified
            if eventcount <= max_events:
                for i in range(0,frames_per_event) :
                    self.bliping()
                    self.fading()
                    self.draw_frame(max_iter=layout_max_iterations,max_delta=layout_max_delta)
            
        for i in range(0,number_fadeout_frames):
            self.bliping()
            self.fading()
            self.draw_frame(max_iter=layout_max_iterations,max_delta=layout_max_delta)
    
    def make_movie(self,frames_per_second,output_file='movie.avi'):
        if self.input_file != None:
            output_file = self.input_file.split('/')[-1].replace('.json','')+'.avi'
        os.system(const.PATH_MENCODER+'mencoder "mf://'+self.path_to_frames+'/*.png" -mf fps='+str(frames_per_second)+' -o '+self.path_to_movie+'/'+output_file+' -ovc lavc -lavcopts vcodec=msmpeg4v2:vbitrate=10000 > std.out 2> std.err')
        return
    
def start(jsonfile, frames_output, movie_output, movie_width=const.MOVIE_WIDTH, movie_height=const.MOVIE_HEIGHT, frames_per_event=const.FRAMES_PER_EVENT, frames_per_second=const.FRAMES_PER_SECOND, max_events=const.MAX_EVENTS, layout_max_iterations=const.LAYOUT_MAX_ITERATIONS, layout_max_delta=const.LAYOUT_MAX_DELTA, number_fadeout_frames=const.NUMBER_FADEOUT_FRAMES):
    """
    Description here...
    
    @param jsonfile path to json file
    @param frames_output the directory in which movie frames are temporarily saved
    @param movie_output the directory in which the movie is saved
    @param frames_per_event how many frames are used for each event in the event list
    @param maxevents: the maximum number of events for which frames are produced
    @param layout_max_iterations: number of iterations of the graph layout algorithm for each frame
    @param layout_max_delta: maximum delta between graph layouts in consecutive frames
    @param number_fadeout_frames: the number of frames after the last event 
    
    """
    try:
        if not os.path.exists(frames_output):
            os.mkdir(frames_output)
        if not os.path.exists(movie_output):
            os.mkdir(movie_output)
        print "Creating DynamicGraph..."
        dGraph = DynamicGraph('fr',movie_width,movie_height,frames_output,movie_output)
        print "Reading JSON file..."
        dGraph.read_json_events(jsonfile)
        print "Producing frames..."
        dGraph.make_frames(frames_per_event,max_events,layout_max_iterations,layout_max_delta,number_fadeout_frames)
        print "Encoding movie..."
        dGraph.make_movie(frames_per_second)
        print "Finished rendering"
    except Exception as e:
        print e
        traceback.print_exc(file=sys.stderr)
        return None
    finally:
        shutil.rmtree(frames_output)
    
if __name__ == "__main__":
    jsonfile=sys.argv[1]
    if len(sys.argv) == 1 or sys.argv[1] == 'help':
        print 'Usage: python DynamicGraph.py json_file [movie_frames_output_dir] [movie_output_dir]'  
    else:  
        frames_output=const.PATH_FRAMES_OUTPUT
        movie_output=const.PATH_MOVIES_OUTPUT
        
        if len(sys.argv)>=4:
            frames_output=sys.argv[2]
            movie_output=sys.argv[3]
        
        frames_output+="_"+os.path.basename(jsonfile)
        
        start(jsonfile, frames_output, movie_output)

