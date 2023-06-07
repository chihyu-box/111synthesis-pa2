#include <string>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <stack>
#include <tuple>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
#include "base/abc/abc.h"
#ifdef __cplusplus
}
#endif

struct Tech
{
	string name;
	double timing[2];
	double power;
};

struct Node
{
	string name;
	int level;
	string fanin0 = "-1";
	string fanin1 = "-1";
	int phase0 = -1;
	int phase1 = -1;
};

struct mappedNode
{
	Tech * tech;
	vector<tuple<bool, string, Tech*>> FOs; // <phase, FO's name, inv's tech>
	double timing;
	bool mode = 0;
	Tech * inv;
};

string 										iterate_ntk(Abc_Ntk_t * pNtk);
unordered_map<string, Node> 				parser(string s);
unordered_map<string, Node> 				set_FOs_level(unordered_map<string, Node> &umapNode);
vector<Node> 								sort_nodes_by_level(unordered_map<string, Node> &umapNode);
void 										print_vecNodes(vector<Node> &vecNodes);
vector<Tech> 								read_TechLib(char * input_lib);
void 										print_TechLib(vector<Tech> &TechLib);
vector<Tech> 								set_PIs_POs_tech(vector<Tech> &TechLib);
unordered_map<string, mappedNode> 			build_PIs(unordered_map<string, mappedNode> &network, vector<Node> &vecNodes, vector<Tech> &TechLib);
unordered_map<string, mappedNode>			build_POs(unordered_map<string, mappedNode> &network, vector<Node> &vecNodes, vector<Tech> &TechLib);
unordered_map<string, mappedNode>			build_internalNodes(unordered_map<string, mappedNode> &network, unordered_map<string, Node> &umapNode, vector<Tech> &TechLib);
void										print_network(unordered_map<string, mappedNode> &network);
double										get_total_power(unordered_map<string, mappedNode> &network);
