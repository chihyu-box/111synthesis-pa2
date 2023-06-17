#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <chrono>
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

	if(argc == 4)
	{
		cout << "Reading " << argv[1] << " with technology library " << argv[2] << "...\n";
	}
	else
	{
		cout << "Usage: ./main [.blif] [.lib] [.mbench]\n";
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
  	int   										FO_level;
  	vector<Node> 								vecNodes;
  	vector<Tech>								TechLib;
  	unordered_map<string, mappedNode> 			network;
  	double										initial_power;
  	double    									initial_delay;
  	
  	umapNodes 		= 		parser(iterate_ntk(ntk)); //cout << iterate_ntk(ntk) << endl;
  	FO_level 		= 		set_FOs_level(umapNodes);
  	vecNodes 		=		sort_nodes_by_level(umapNodes); //print_vecNodes(vecNodes);
  	TechLib 		=		read_TechLib(argv[2]); //print_TechLib(TechLib);
  	TechLib 		=		set_PIs_POs_tech(TechLib); //print_TechLib(TechLib);
  	
  	network			=		build_PIs(network, vecNodes, TechLib); //cout << "PI:" << network.size() << endl;
  	network			=		build_POs(network, vecNodes, TechLib); //cout << "PI+PO:" << network.size() << endl; 
  	network			=		build_internalNodes(network, umapNodes, TechLib); //cout << "PI+PO+internal:" << network.size() << endl;
  	initial_power	=		get_total_power(network); //cout << "Initial power:" << initial_power << endl;
	initial_delay   =		get_initial_delay(network, vecNodes, FO_level); //cout << "Initial delay:" << initial_delay << endl;

  	auto [initial_nand, initial_inv] 	= 		get_gate_count(network);
  	auto [PO_count, PI_count] 			= 		get_PO_PI_count(network);
  	auto start_time 					= 		chrono::high_resolution_clock::now();
  	auto [local_delay, local_power] 	= 		SA(network, vecNodes, TechLib, initial_delay, initial_power, FO_level);
  	auto end_time 						= 		chrono::high_resolution_clock::now();
  	auto [local_nand, local_inv] 		= 		get_gate_count(network);
  	chrono::duration<double> diff 		= 		end_time-start_time;

  	
  	cout << "\n\nSA took:     " << diff.count()  << "s\n" << endl;

  	cout << std::left << setw(15) << "PO:" << std::left << setw(10) << PO_count << std::left << setw(15) << "PI:" << PI_count << endl;
  	cout << std::left << setw(15) << "Initial NAND:" << std::left << setw(10) << initial_nand << std::left << setw(15) << "Initial INV:" << initial_inv << endl;
  	cout << std::left << setw(15) << "local   NAND:" << std::left << setw(10) << local_nand   << std::left << setw(15) << "local   INV:" << local_inv << endl;

  	cout << std::left << setw(15) << "Initial power:" << std::left << setw(10) << initial_power << std::left << setw(15) << "Initial delay:" << initial_delay << endl;
  	cout << std::left << setw(15) << "local   power:" << std::left << setw(10) << local_power   << std::left << setw(15) << "local   delay:" << local_delay   <<endl;
  
  	// print_slack(network, vecNodes);

  	output_mbench(vecNodes, network, TechLib, initial_delay, initial_power, local_power, argv[3]);

  	//print_network(network, vecNodes);

	Abc_NtkDelete(ntk);

	// << End ABC >>
	Abc_Stop();
	
	return 0;
}