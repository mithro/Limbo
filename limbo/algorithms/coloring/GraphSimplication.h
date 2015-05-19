/*************************************************************************
    > File Name: GraphSimplication.h
    > Author: Yibo Lin
    > Mail: yibolin@utexas.edu
    > Created Time: Mon May 18 15:55:09 2015
 ************************************************************************/

#ifndef LIMBO_ALGORITHMS_GRAPHSIMPLICATION_H
#define LIMBO_ALGORITHMS_GRAPHSIMPLICATION_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <boost/graph/graph_concepts.hpp>
#include <boost/property_map/property_map.hpp>

namespace limbo { namespace algorithms { namespace coloring {

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::pair;

template <typename GraphType>
class GraphSimplication
{
	public:
		typedef GraphType graph_type;
		typedef typename boost::graph_traits<graph_type>::vertex_descriptor graph_vertex_type;
		typedef typename boost::graph_traits<graph_type>::edge_descriptor graph_edge_type;
		typedef typename boost::graph_traits<graph_type>::vertex_iterator vertex_iterator;
		typedef typename boost::graph_traits<graph_type>::adjacency_iterator adjacency_iterator;
		typedef typename boost::graph_traits<graph_type>::edge_iterator edge_iterator;

		/// constructor 
		GraphSimplication(graph_type const& g) : m_graph (g), m_vParent(boost::num_vertices(g)), m_vChildren(boost::num_vertices(g))
		{
			graph_vertex_type v = 0; 
			for (typename vector<graph_vertex_type>::iterator it = m_vParent.begin(); it != m_vParent.end(); ++it, ++v)
				(*it) = v;
			v = 0;
			for (typename vector<vector<graph_vertex_type> >::iterator it = m_vChildren.begin(); it != m_vChildren.end(); ++it, ++v)
				it->push_back(v);
#ifdef DEBUG_GRAPHSIMPLIFICATION
			assert(m_vParent.size() == boost::num_vertices(m_graph));
			assert(m_vChildren.size() == boost::num_vertices(m_graph));
#endif
		}
		/// copy constructor is not allowed 
		GraphSimplication(GraphSimplication const& rhs);

		vector<graph_vertex_type> const& parents() const {return m_vParent;}
		vector<vector<graph_vertex_type> > const& children() const {return m_vChildren;}

		/// for a structure of K4 with one fewer edge 
		/// suppose we have 4 vertices 1, 2, 3, 4
		/// 1--2, 1--3, 2--3, 2--4, 3--4, vertex 4 is merged to 1 
		/// this strategy only works for 3-coloring 
		void merge_subK4()
		{
			// when applying this function, be aware that other merging strategies may have already been applied 
			// so m_vParent is valid 
			//
			// merge iteratively 
			bool merge_flag; // true if merge occurs
			do 
			{
				merge_flag = false;
				// traverse the neighbors of vertex 1 to find connected vertex 2 and 3 
				vertex_iterator vi1, vie1;
				for (boost::tie(vi1, vie1) = boost::vertices(m_graph); vi1 != vie1; ++vi1)
				{
					graph_vertex_type v1 = *vi1;
					// only track unmerged vertices 
					if (this->merged(v1)) continue;
					// find vertex 2 by searching the neighbors of vertex 1
					vector<graph_vertex_type> const& vChildren1 = m_vChildren.at(v1);
					for (typename vector<graph_vertex_type>::const_iterator vic1 = vChildren1.begin(); vic1 != vChildren1.end(); ++vic1)
					{
#ifdef DEBUG_GRAPHSIMPLIFICATION
						cout << vic1-vChildren1.begin() << endl;
#endif
						graph_vertex_type vc1 = *vic1;
						adjacency_iterator vi2, vie2;
						for (boost::tie(vi2, vie2) = boost::adjacent_vertices(vc1, m_graph); vi2 != vie2; ++vi2)
						{
							// skip stitch edges 
							pair<graph_edge_type, bool> e12 = boost::edge(vc1, *vi2, m_graph);
							assert(e12.second);
							if (boost::get(boost::edge_weight, m_graph, e12.first) < 0) continue;

							graph_vertex_type v2 = this->parent(*vi2);
							// find vertex 3 by searching the neighbors of vertex 2 
							vector<graph_vertex_type> const& vChildren2 = m_vChildren.at(v2);
							for (typename vector<graph_vertex_type>::const_iterator vic2 = vChildren2.begin(); vic2 != vChildren2.end(); ++vic2)
							{
								graph_vertex_type vc2 = *vic2;
								adjacency_iterator vi3, vie3;
								for (boost::tie(vi3, vie3) = boost::adjacent_vertices(vc2, m_graph); vi3 != vie3; ++vi3)
								{
									// skip stitch edges 
									pair<graph_edge_type, bool> e23 = boost::edge(vc2, *vi3, m_graph);
									assert(e23.second);
									if (boost::get(boost::edge_weight, m_graph, e23.first) < 0) continue;

									graph_vertex_type v3 = this->parent(*vi3);
									// skip v1, v1 must be a parent  
									if (v3 == v1) continue;
									// only connected 1 and 3 are considered 
									if (!this->connected_conflict(v1, v3)) continue;

									// find vertex 4 by searching the neighbors of vertex 3  
									vector<graph_vertex_type> const& vChildren3 = m_vChildren.at(v3);
									for (typename vector<graph_vertex_type>::const_iterator vic3 = vChildren3.begin(); vic3 != vChildren3.end(); ++vic3)
									{
										graph_vertex_type vc3 = *vic3;
										adjacency_iterator vi4, vie4;
										for (boost::tie(vi4, vie4) = boost::adjacent_vertices(vc3, m_graph); vi4 != vie4; ++vi4)
										{
											// skip stitch edges 
											pair<graph_edge_type, bool> e34 = boost::edge(vc3, *vi4, m_graph);
											assert(e34.second);
											if (boost::get(boost::edge_weight, m_graph, e34.first) < 0) continue;

											graph_vertex_type v4 = this->parent(*vi4);
											// skip v1 or v2, v1 and v2 must be parent  
											if (v4 == v1 || v4 == v2) continue;
											// vertex 2 and vertex 4 must be connected 
											// vertex 1 and vertex 4 must not be connected (K4)
											if (!this->connected_conflict(v2, v4) || this->connected_conflict(v1, v4)) continue;
											// merge vertex 4 to vertex 1 
											m_vChildren[v1].insert(m_vChildren[v1].end(), m_vChildren[v4].begin(), m_vChildren[v4].end());
											m_vChildren[v4].resize(0); // clear and shrink to fit 
											m_vParent[v4] = v1;
											merge_flag = true;
										}
									}
								}
							}
						}
					}
				}
			} while (merge_flag);
		}
		void write_graph_dot(string const& filename) const 
		{
			std::ofstream dot_file(filename.c_str());
			dot_file << "graph D { \n"
				<< "  randir = LR\n"
				<< "  size=\"4, 3\"\n"
				<< "  ratio=\"fill\"\n"
				<< "  edge[style=\"bold\",fontsize=200]\n" 
				<< "  node[shape=\"circle\",fontsize=200]\n";

			//output nodes 
			vertex_iterator vi1, vie1;
			for (boost::tie(vi1, vie1) = boost::vertices(m_graph); vi1 != vie1; ++vi1)
			{
				graph_vertex_type v1 = *vi1;
				if (m_vChildren[v1].empty()) continue;

				dot_file << "  " << v1 << "[shape=\"circle\"";
				//output coloring label
				dot_file << ",label=\"" << v1 << ":(";
				for (typename vector<graph_vertex_type>::const_iterator it1 = m_vChildren[v1].begin(); it1 != m_vChildren[v1].end(); ++it1)
				{
					if (it1 != m_vChildren[v1].begin())
						dot_file << ",";
					dot_file << *it1;
				}
				dot_file << ")\"";
				dot_file << "]\n";
			}

			//output edges
			for (boost::tie(vi1, vie1) = boost::vertices(m_graph); vi1 != vie1; ++vi1)
			{
				graph_vertex_type v1 = *vi1;
				if (this->merged(v1)) continue;

				vertex_iterator vi2, vie2;
				for (boost::tie(vi2, vie2) = boost::vertices(m_graph); vi2 != vie2; ++vi2)
				{
					graph_vertex_type v2 = *vi2;
					if (this->merged(v2)) continue;
					if (v1 >= v2) continue;

					// if two hyper vertices are connected by conflict edges, 
					// there is no need to consider stitch edges 
					if (this->connected_conflict(v1, v2)) 
						dot_file << "  " << v1 << "--" << v2 << "[color=\"black\",style=\"solid\",penwidth=3]\n";
					else if (this->connected_stitch(v1, v2))
						dot_file << "  " << v1 << "--" << v2 << "[color=\"black\",style=\"dashed\",penwidth=3]\n";
				}
			}
			dot_file << "}";
			dot_file.close();

			char buf[256];
			sprintf(buf, "dot -Tpdf %s.dot -o %s.pdf", filename.c_str(), filename.c_str());
			system(buf);
		}
	protected:
		/// \return the root parent 
		/// i.e. the vertex that is not merged 
		graph_vertex_type parent(graph_vertex_type v) const 
		{
			while (v != m_vParent.at(v))
				v = m_vParent.at(v);
			return v;
		}
		/// \return true if two vertices (parents) are connected by conflict edges 
		bool connected_conflict(graph_vertex_type v1, graph_vertex_type v2) const 
		{
			graph_vertex_type vp1 = this->parent(v1);
			graph_vertex_type vp2 = this->parent(v2);
			vector<graph_vertex_type> const& vChildren1 = m_vChildren.at(vp1);
			vector<graph_vertex_type> const& vChildren2 = m_vChildren.at(vp2);
			for (typename vector<graph_vertex_type>::const_iterator vic1 = vChildren1.begin(); vic1 != vChildren1.end(); ++vic1)
				for (typename vector<graph_vertex_type>::const_iterator vic2 = vChildren2.begin(); vic2 != vChildren2.end(); ++vic2)
				{
					pair<graph_edge_type, bool> e12 = boost::edge(*vic1, *vic2, m_graph);
					// only count conflict edge 
					if (e12.second && boost::get(boost::edge_weight, m_graph, e12.first) >= 0) return true;
				}
			return false;
		}
		/// \return true if two vertices (parents) are connected by stitch edges 
		bool connected_stitch(graph_vertex_type v1, graph_vertex_type v2) const 
		{
			graph_vertex_type vp1 = this->parent(v1);
			graph_vertex_type vp2 = this->parent(v2);
			vector<graph_vertex_type> const& vChildren1 = m_vChildren.at(vp1);
			vector<graph_vertex_type> const& vChildren2 = m_vChildren.at(vp2);
			for (typename vector<graph_vertex_type>::const_iterator vic1 = vChildren1.begin(); vic1 != vChildren1.end(); ++vic1)
				for (typename vector<graph_vertex_type>::const_iterator vic2 = vChildren2.begin(); vic2 != vChildren2.end(); ++vic2)
				{
					pair<graph_edge_type, bool> e12 = boost::edge(*vic1, *vic2, m_graph);
					// only count conflict edge 
					if (e12.second && boost::get(boost::edge_weight, m_graph, e12.first) < 0) return true;
				}
			return false;
		}
		/// \return true if current vertex is merged 
		bool merged(graph_vertex_type v1) const 
		{
			bool flag = m_vChildren.at(v1).empty();
#ifdef DEBUG_GRAPHSIMPLIFICATION
			if (!flag) assert(v1 == m_vParent.at(v1));
			else assert(v1 != m_vParent.at(v1));
#endif
			return flag;
		}
		graph_type const& m_graph;
		vector<graph_vertex_type> m_vParent; ///< parent vertex of current vertex 
		vector<vector<graph_vertex_type> > m_vChildren; ///< children verices of current vertex, a vertex is the child of itself if it is not merged  
};

}}} // namespace limbo // namespace algorithms // namespace coloring 

#endif
