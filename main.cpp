#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include "techmap.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
#include "base/abc/abc.h"
extern void  Abc_Start();
extern void  Abc_Stop();
extern Abc_Ntk_t * Io_ReadBlifAsAig(char *, int);
#ifdef __cplusplus
}
#endif


int main(int argc, char **argv)
{
	// input circuit name.
	char *circuit;
	Abc_Ntk_t *ntk;

	if(argc == 3)
	{
		cout << "Reading " << argv[1] << " with technology library " << argv[2] << "...\n";
	}
	else
	{
		cout << "Usage: ./main [.blif] [.lib]\n";
		return 1;
	}

	// << Setup ABC >>
	Abc_Start();
	
    //<< Read blif file >>
	circuit = argv[1];
	ntk = Io_ReadBlifAsAig(circuit, 1);

  	if(ntk == NULL) 
  	{
  		cout << "Use .blif as input file" << endl;
  		Abc_Stop();
  		return 1; 
  	}

  	
  	unordered_map<string, Node> 				umapNodes;
  	vector<Node> 								vecNodes;
  	vector<Tech>								TechLib;
  	unordered_map<string, mappedNode> 			network;
  	double										total_power;

  	umapNodes 		= 		parser(iterate_ntk(ntk)); //cout << iterate_ntk(ntk) << endl;
  	umapNodes 		= 		set_FOs_level(umapNodes);
  	vecNodes 		=		sort_nodes_by_level(umapNodes); //print_vecNodes(vecNodes);
  	TechLib 		=		read_TechLib(argv[2]); //print_TechLib(TechLib);
  	TechLib 		=		set_PIs_POs_tech(TechLib); //print_TechLib(TechLib);
  	
  	network			=		build_PIs(network, vecNodes, TechLib); //cout << "PI:" << network.size() << endl;
  	network			=		build_POs(network, vecNodes, TechLib); //cout << "PI+PO:" << network.size() << endl; 
  	network			=		build_internalNodes(network, umapNodes, TechLib); //cout << "PI+PO+internal:" << network.size() << endl;
  	total_power		=		get_total_power(network);

  	
  	// cout << umapNodes.size() << "\n";
  	// cout << vecNodes.size() << "\n";
  	
  	print_network(network);
  	// cout << "total power:" << total_power << endl;


	Abc_NtkDelete(ntk);

	// << End ABC >>
	Abc_Stop();
	
	return 0;
}