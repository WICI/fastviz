/*
 * Functions to translate std containers to igraph graphs.
 */


#ifndef PMS_STD_TO_IGRAPH_HPP
#define PMS_STD_TO_IGRAPH_HPP

#include <iostream>

#include <igraph.h>

using namespace std;


void print_to_stdout(igraph_vector_t &v) {
	size_t n = igraph_vector_size(&v);
	for (size_t i = 0; i < n; i++) cout<<VECTOR(v)[i]<<" ";
}

void print_to_stdout(igraph_matrix_t &adjmatrix) {
	long ncol = igraph_matrix_ncol(&adjmatrix);
	long nrow = igraph_matrix_ncol(&adjmatrix);
	for (int i=0; i<ncol; i++) {
		for (int j=0; j<nrow; j++) cout<<MATRIX(adjmatrix,i,j)<<" ";
	}
}

template <class T0>
void stdv_to_vectort(T0 &v1, igraph_vector_t &v2) {
	size_t n = v1.size();
	igraph_vector_resize(&v2, n);
	for (size_t i = 0; i < n; i++) VECTOR(v2)[i] = v1[i];
}

void adjmatrix_to_adjlist( vector <vector <double> > &vv,
		vector <long> &v, vector <double> &w) {
	for (int i=0; i<vv.size(); i++)
		for (int j=0; j<vv[i].size(); j++)
			if (vv[i][j]) if (i!=j) {
				v.push_back(i);
				v.push_back(j);
				w.push_back(vv[i][j]);
			}
}

// void sub_adjmatrix_to_adjlist( vector <vector <double> > &vv,
// 		vector <unsigned> &nodes, vector <long> &v, vector <double> &w) {
// 	for (int i=0; i<nodes.size(); i++)
// 		for (int j=0; j<nodes.size(); j++) {
// 			unsigned pos1 = nodes[i];
// 			unsigned pos2 = nodes[j];
// 			if (vv[pos1][pos2]) if (pos1!=pos2) {
// 				v.push_back(pos1);
// 				v.push_back(pos2);
// 				w.push_back(vv[pos1][pos2]);
// 			}
// 		}
// }

void vv_to_igraph( vector <vector <double> > &vv,
		igraph_t &g, igraph_vector_t &weights,
		const igraph_bool_t directed=false ) {
	unsigned nnodes =  vv.size();
	vector <long> v1;
	vector <double> w;
	adjmatrix_to_adjlist(vv, v1, w);
	stdv_to_vectort(w, weights);
	size_t n2xlinks = v1.size();
	igraph_vector_t v2;
	igraph_vector_init(&v2, 0);
	igraph_vector_resize(&v2, n2xlinks);
	for (size_t i = 0; i < n2xlinks; i++) VECTOR(v2)[i] = v1[i];
	// cout<<"net:"<<endl;
	// print_to_stdout(v2);
	// cout<<endl;
	igraph_create( &g, &v2, 0, directed);
	igraph_vector_destroy(&v2);
}

// void sub_vv_to_igraph( vector <vector <double> > &vv,
// 		vector <unsigned> &nodes,
// 		igraph_t &g, igraph_vector_t &weights,
// 		const igraph_bool_t directed=false ) {
// 	unsigned nnodes =  vv.size();
// 	vector <long> v1;
// 	vector <double> w;
// 	sub_adjmatrix_to_adjlist(vv, nodes, v1, w);
// 	stdv_to_vectort(w, weights);
// 	size_t n2xlinks = v1.size();
// 	igraph_vector_t v2;
// 	igraph_vector_init(&v2, 0);
// 	igraph_vector_resize(&v2, n2xlinks);
// 	for (size_t i = 0; i < n2xlinks; i++) VECTOR(v2)[i] = v1[i];
// 	// cout<<"subnet:"<<endl;
// 	// print_to_stdout(v2);
// 	// cout<<endl;
// 	igraph_create( &g, &v2, 0, directed);
// 	igraph_vector_destroy(&v2);
// }

// not tested
void vv_to_weigthedigraph(vector <vector <double> > &vv,
		igraph_t &graph,
		igraph_adjacency_t mode=IGRAPH_ADJ_UNDIRECTED,
		const char* attr=NULL,
		igraph_bool_t loops=false) {
	unsigned nnodes =  vv.size();
	igraph_matrix_t adjmatrix;
	igraph_matrix_init( &adjmatrix, nnodes, nnodes);
	for (int i=0; i<nnodes; i++)
		for (int j=0; j<nnodes; j++) MATRIX(adjmatrix,i,j) = vv[i][j];
	igraph_weighted_adjacency( &graph, &adjmatrix, mode, attr, loops);
	igraph_matrix_destroy( &adjmatrix);
}


#endif
