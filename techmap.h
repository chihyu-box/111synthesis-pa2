#include <string>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <stack>
#include <tuple>
#include <random>
#include <iomanip>

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
	double timing; 							// delay from node to PI
	double delay;  							// delay itself 
	int mode;
	Tech * inv;
	double arrival_time;
	double required_time;
	double slack;
};

string 										iterate_ntk(Abc_Ntk_t * pNtk);
unordered_map<string, Node> 				parser(string s);
int 										set_FOs_level(unordered_map<string, Node> &umapNode);
vector<Node> 								sort_nodes_by_level(unordered_map<string, Node> &umapNode);
void 										print_vecNodes(vector<Node> &vecNodes);
vector<Tech> 								read_TechLib(char * input_lib);
void 										print_TechLib(vector<Tech> &TechLib);
vector<Tech> 								set_PIs_POs_tech(vector<Tech> &TechLib);
unordered_map<string, mappedNode> 			build_PIs(unordered_map<string, mappedNode> &network, vector<Node> &vecNodes, vector<Tech> &TechLib);
unordered_map<string, mappedNode>			build_POs(unordered_map<string, mappedNode> &network, vector<Node> &vecNodes, vector<Tech> &TechLib);
unordered_map<string, mappedNode>			build_internalNodes(unordered_map<string, mappedNode> &network, unordered_map<string, Node> &umapNode, vector<Tech> &TechLib);
void										print_network(unordered_map<string, mappedNode> &network, vector<Node> &vecNodes);
double										get_total_power(unordered_map<string, mappedNode> &network);
pair<int, int> 								get_gate_count(unordered_map<string, mappedNode> &network);
pair<int, int> 								get_PO_PI_count(unordered_map<string, mappedNode> &network);
double 										get_initial_delay(unordered_map<string, mappedNode> &network, vector<Node> &vecNodes, int FO_level);
pair<double, double> 						SA(unordered_map<string, mappedNode> &network, vector<Node> &vecNodes, vector<Tech> &TechLib, double initial_delay, double initial_power, int FO_level);
void 										print_slack(unordered_map<string, mappedNode> &network, vector<Node> &vecNodes);
void										slack(unordered_map<string, mappedNode> &network, vector<Node> &vecNodes, double initial_delay);
void										output_mbench(vector<Node> &vecNodes, unordered_map<string, mappedNode> &network, vector<Tech> &TechLib, double initial_delay, double original_power, double optimized_power, string output);

