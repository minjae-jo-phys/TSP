#define BOOST_TEST_MODULE HLTSpanningForestTest
#include <boost/test/included/unit_test.hpp>
#include <memory>
#include <set>
#include <map>
#include <utility>
#include "HDTSpanningForest.hpp"

using namespace std;
using namespace Snu::Cnrc::HDTSpanningForest;

struct MyNode : public HDTSpanningForestNode {
	
	MyNode()
	: HDTSpanningForestNode() {
	}
	
	//to test with a minimal restricted class
	MyNode(const MyNode&) = delete;
	
	//to test with a minimal restricted class
	MyNode(MyNode&&) = delete;
	
	//to test with a minimal restricted class
	MyNode& operator=(const MyNode&) = delete;
	
	//to test with a minimal restricted class
	MyNode& operator=(MyNode&&) = delete;
	
	//for convenience of test
	bool operator==(const MyNode& other) const {
		return this == &other;
	}
	
	//for convenience of test
	bool operator!=(const MyNode& other) const {
		return this != &other;
	}
	
	unsigned int name;
};

struct MyEdge : public HDTSpanningForestEdge {
};

using HDT = HDTSpanningForestAlgorithm<MyNode, MyEdge>;

BOOST_AUTO_TEST_CASE(TestSingleNodeGraph) {
	MyNode n;
	n.name = 1234;
	BOOST_REQUIRE(HDT::hasPath(n, n));
	BOOST_REQUIRE(HDT::isClusterRep(n));
	BOOST_REQUIRE(HDT::findClusterRep(n) == n);
	
	{
		HDT::Cluster c = HDT::cluster(n);
		BOOST_REQUIRE(c.size() == 1);
		HDT::Cluster::iterator itr = c.begin();
		BOOST_REQUIRE(*itr == n);
		BOOST_REQUIRE(itr->name == 1234);
		itr->name = 4321;
		BOOST_REQUIRE(*itr++ == n);
		BOOST_REQUIRE(itr == c.end());
	}
	
	{
		HDT::ConstCluster c = HDT::cluster(static_cast<const MyNode&>(n));
		BOOST_REQUIRE(c.size() == 1);
		HDT::Cluster::const_iterator itr = c.begin();
		BOOST_REQUIRE(*itr == n);
		BOOST_REQUIRE(itr->name == 4321);
		BOOST_REQUIRE(*itr++ == n);
		BOOST_REQUIRE(itr == c.end());
	}
}

set<const MyNode*> toNodeSet(const MyNode& n) {
	set<const MyNode*> s;
	for(const MyNode& m : HDT::cluster(n)) {
		BOOST_REQUIRE(s.insert(&m).second);
	}
	return s;
}

BOOST_AUTO_TEST_CASE(TestTwoNodesGraph) {
	MyNode nodes[2];
	MyEdge edge;
	BOOST_REQUIRE(! HDT::hasPath(nodes[0], nodes[1]));
	BOOST_REQUIRE(HDT::createEdge(nodes[0], nodes[1], edge));
	BOOST_REQUIRE(HDT::hasPath(nodes[0], nodes[1]));
	BOOST_REQUIRE(HDT::clusterSize(nodes[0]) == 2);
	BOOST_REQUIRE(HDT::clusterSize(nodes[1]) == 2);
	BOOST_REQUIRE(toNodeSet(nodes[0]) == set<const MyNode*>({nodes, nodes + 1}));
	BOOST_REQUIRE(toNodeSet(nodes[1]) == set<const MyNode*>({nodes, nodes + 1}));
	BOOST_REQUIRE(HDT::deleteEdge(edge));
	BOOST_REQUIRE(! HDT::hasPath(nodes[0], nodes[1]));
	BOOST_REQUIRE(toNodeSet(nodes[0]) == set<const MyNode*>({nodes}));
	BOOST_REQUIRE(toNodeSet(nodes[1]) == set<const MyNode*>({nodes + 1}));
	BOOST_REQUIRE(HDT::clusterSize(nodes[0]) == 1);
	BOOST_REQUIRE(HDT::clusterSize(nodes[1]) == 1);
}

BOOST_AUTO_TEST_CASE(TestNameConflict) {
	
	struct MyNode : public HDTSpanningForestNode {
		int levelNodes;
	};
	
	struct MyEdge : public HDTSpanningForestEdge {
		double node1;
		double node2;
		double level;
		double isTreeEdge;
		double levelEdges;
	};
	
	using HDT = HDTSpanningForestAlgorithm<MyNode, MyEdge>;

	MyNode nodes[2];
	MyEdge edge;
	
	BOOST_REQUIRE(! HDT::hasPath(nodes[0], nodes[1]));
	BOOST_REQUIRE(HDT::createEdge(nodes[0], nodes[1], edge));
	BOOST_REQUIRE(HDT::hasPath(nodes[0], nodes[1]));
	BOOST_REQUIRE(HDT::cluster(nodes[0]).size() == 2);
	BOOST_REQUIRE(HDT::clusterSize(nodes[1]) == 2);
	BOOST_REQUIRE(HDT::isClusterRep(HDT::findClusterRep(nodes[0])));
	BOOST_REQUIRE(HDT::deleteEdge(edge));
	BOOST_REQUIRE(! HDT::hasPath(nodes[0], nodes[1]));
	BOOST_REQUIRE(HDT::cluster(static_cast<const MyNode&>(nodes[0])).size() == 1);
	BOOST_REQUIRE(HDT::cluster(static_cast<const MyNode&>(nodes[1])).size() == 1);
	BOOST_REQUIRE(HDT::isClusterRep(HDT::findClusterRep(static_cast<const MyNode&>(nodes[0]))));
}

MyNode* makeNodes(unsigned int n) {
	MyNode* const nodes = new MyNode[n];
	for(unsigned int i = 0;  i < n; ++i) {
		nodes[i].name = i;
	}
	return nodes;
}

void assertRangeConnected(
	const MyNode* const nodes, 
	const unsigned int from, 
	const unsigned to
) {
	const MyNode& r = *find_if(nodes + from, nodes + to, [](const MyNode& n) {
		return HDT::isClusterRep(n);
	});
	set<const MyNode*> ss;
	for(unsigned int i = from; i < to; ++i) {
		ss.insert(nodes + i);
	}
	for(unsigned int i = from; i < to; ++i) {
		if(nodes[i] != r) {
			BOOST_REQUIRE(! HDT::isClusterRep(nodes[i]));
		
		BOOST_REQUIRE(HDT::findClusterRep(nodes[i]) == r);}
		for(unsigned int j = from; j < to; ++j) {
			BOOST_REQUIRE(HDT::hasPath(nodes[i], nodes[j]));
		}
		BOOST_REQUIRE(HDT::cluster(nodes[i]).size() == to- from);
		BOOST_REQUIRE(HDT::clusterSize(nodes[i]) == to -from);
		BOOST_REQUIRE(toNodeSet(nodes[i]) == ss);
	}
}

void assertRangeNotConnected(
	const MyNode* const nodes, 
	const unsigned int from1, 
	const unsigned int to1,
	const unsigned int from2, 
	const unsigned int to2
) {
	for(unsigned int i = from1; i < to1; ++i) {
		for(unsigned int j = from2; j < to2; ++j) {
			BOOST_REQUIRE(! HDT::hasPath(nodes[i], nodes[j]));
		}
	}
}

BOOST_AUTO_TEST_CASE(TestManyNodesGraph) {
	
	using NodeNamePair = pair<unsigned int, unsigned int>;
	
	for(unsigned int n = 3; n < 32; ++n) {
		unique_ptr<MyNode[]> nodes(makeNodes(n));
		map<NodeNamePair, unique_ptr<MyEdge>> edges;
		for(unsigned int i = 0; i < n; ++i) {
			for(unsigned int j = i / 2; j < i; ++j) {
				edges[NodeNamePair(i, j)] = unique_ptr<MyEdge>(new MyEdge());
				bool merged = HDT::createEdge(nodes[i], nodes[j], *edges[NodeNamePair(i, j)]);
				if(j == i / 2) {
					BOOST_REQUIRE(merged);
					assertRangeConnected(nodes.get(), 0, i + 1);
				} else {
					BOOST_REQUIRE(! merged);
				}
			}
		}
		assertRangeConnected(nodes.get(), 0, n);
		for(unsigned int i = 0; i < n; ++i) {
			for(unsigned int j = i / 2; j < i; ++j) {
				if(i != j + 1) {
					BOOST_REQUIRE(! HDT::deleteEdge(*edges[NodeNamePair(i, j)]));
					edges.erase(NodeNamePair(i, j));
					assertRangeConnected(nodes.get(), 0, n);
				}
			}
		}
		for(unsigned int i = 0; i < n - 1; ++i) {
			BOOST_REQUIRE(HDT::deleteEdge(*edges[NodeNamePair(i + 1, i)]));
			BOOST_REQUIRE(HDT::cluster(nodes[i]).size() == 1);
			BOOST_REQUIRE(HDT::clusterSize(nodes[i]) == 1);
			BOOST_REQUIRE(toNodeSet(nodes[i]) == set<const MyNode*>{nodes.get() + i});
			assertRangeConnected(nodes.get(), i + 1, n);
			BOOST_REQUIRE(HDT::cluster(nodes[i + 1]).size() == n - i - 1);
			BOOST_REQUIRE(HDT::clusterSize(nodes[i + 1]) == n - i - 1);
		}
	}
}