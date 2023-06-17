#include <techmap.h>


unordered_map<string, mappedNode> network;


string iterate_ntk(Abc_Ntk_t * pNtk)
{
	if(!Abc_NtkIsStrash(pNtk))
	{
		Abc_Print(-1, "This command is only applicable to strashed networks.\n");
		return 0;
	}
	char * s = (char *)malloc(sizeof(char) * 1000000);
	s[0] = '\0';
	Abc_Obj_t * pObj;
	int i;
	Abc_NtkForEachObj(pNtk, pObj, i)
	{
		sprintf(s + strlen(s), "%s %d", Abc_ObjName(pObj), pObj->Level);
		Vec_Ptr_t * vNodes = Abc_NodeGetFaninNames( pObj );
		int Fanin_size = Abc_ObjFaninNum(pObj);
		for(int j=0; j<Fanin_size; ++j)
		{
			sprintf(s + strlen(s), " %s ", (char *)vNodes->pArray[j]);
			if(j == 0) sprintf(s + strlen(s), "%d", pObj->fCompl0);
			else 	   sprintf(s + strlen(s), "%d", pObj->fCompl1);
		}
		sprintf(s + strlen(s), " ;\n");
	}
	return move(string(s));
}

unordered_map<string, Node> parser(string s)
{	
	unordered_map<string, Node> umapNodes;
	string node_name;

	stringstream ss;
	string line;
	ss << s;
	
	while (getline(ss, line, ';')) 
	{
		Node node;
        if (line.empty()) continue;

        istringstream iss(line);
        string word;
        int stage = 0;
        while (iss >> word) 
        {
        	switch(stage)
        	{
        	case 0:
        		node_name = word;
        		node.name = node_name;
        		stage++;
        		break;
        	case 1:
        		node.level = stoi(word);
        		stage++;
        		break;
        	case 2:
        		node.fanin0 = word;
        		stage++;
        		break;
        	case 3:
        		node.phase0 = stoi(word);
        		stage++;
        		break;
        	case 4:
        		node.fanin1 = word;
        		stage++;
        		break;
        	case 5:
        		node.phase1 = stoi(word);
        		stage++;
        		break;  
        	} 
        }
        umapNodes.insert(make_pair(node_name, node));
    }
    return move(umapNodes);
}

int set_FOs_level(unordered_map<string, Node> &umapNode)
{
	int max_level = 0;
	for(auto &component : umapNode)
	{
		if(component.second.level > max_level)
			max_level = component.second.level;
	}
	max_level += 1;
	for(auto &component : umapNode)
	{
		if(component.second.level == 0 && component.second.fanin0 != "-1")
			component.second.level = max_level;
	}

	return max_level;
}

bool level_cmp(const Node& a, const Node& b) 
{
    return a.level < b.level;
}

vector<Node> sort_nodes_by_level(unordered_map<string, Node> &umapNode)
{
	vector<Node> sorted_nodes;

	sorted_nodes.reserve(umapNode.size());
	for(auto &component : umapNode)
	{
		sorted_nodes.push_back(component.second);
	}
	sort(sorted_nodes.begin(), sorted_nodes.end(), level_cmp);
	return move(sorted_nodes);
}

void print_vecNodes(vector<Node> &vecNodes)
{
	for(auto &component : vecNodes)
	{
		cout << component.name << " " << component.level << " ";
    	if(component.fanin0 != "-1") cout << " " << component.fanin0 << " " << component.phase0 << " ";
    	if(component.fanin1 != "-1") cout << " " << component.fanin1 << " " << component.phase1 << " ";
    	cout << ";\n";
	}
}

vector<Tech> read_TechLib(char * input_lib)
{
	fstream fs;
	string buffer;
	vector<Tech> TechLib;


	fs.open(input_lib, fstream::in); 

	if(!fs.is_open())
	{
		cout << "error!" << endl;
		exit(1);
	}

	while(fs >> buffer)
	{
		Tech temp;
		temp.name = buffer;
		fs >> buffer >> buffer;
		temp.timing[0] = stod(buffer);
		fs >> buffer;
		temp.timing[1] = stod(buffer);
		fs >> buffer >> buffer;
		temp.power = stod(buffer);
		TechLib.push_back(temp);
	}

	for(auto i=0; i<TechLib.size(); ++i)
	{
		for(auto j=i+1; j<TechLib.size(); ++j)
		{
			if(TechLib[j].name[0] == TechLib[i].name[0] &&
			   TechLib[j].timing[0] >= TechLib[i].timing[0] &&
			   TechLib[j].timing[1] >= TechLib[i].timing[1] &&
			   TechLib[j].power >= TechLib[i].power)
			{
				TechLib.erase(TechLib.begin()+j);
			}
		}
	}
	return move(TechLib);
}

void print_TechLib(vector<Tech> &TechLib)
{
	for(auto &t : TechLib)
	{
		cout << t.name << "\n";
    	cout << t.timing[0] << " " << t.timing[1] << "\n";
    	cout << t.power << "\n\n";
	}
}

pair<int, int> fastest_Tech(vector<Tech> &TechLib)
{
	double fastestINV[2] = {1000, 1000};
	double fastestNAND[2] = {1000, 1000};
	int INV;
	int NAND;
	for(auto i=0; i<TechLib.size(); ++i)
	{
		if(TechLib[i].name[0] == 'I')
		{
			if(TechLib[i].timing[0] < fastestINV[0] && TechLib[i].timing[1] < fastestINV[1]) 
			{
				INV = i;
				fastestINV[0] = TechLib[i].timing[0];
				fastestINV[1] = TechLib[i].timing[1];
			}
		}
		else if(TechLib[i].name[0] == 'N')
		{
			if(TechLib[i].timing[0] < fastestNAND[0] && TechLib[i].timing[1] < fastestNAND[1]) 
			{
				NAND = i;
				fastestNAND[0] = TechLib[i].timing[0];
				fastestNAND[1] = TechLib[i].timing[1];
			}
		}
	}
	return {INV, NAND};
}

vector<Tech> set_PIs_POs_tech(vector<Tech> &TechLib)
{
	Tech PI;
	PI.name = "PI";
	PI.timing[0] = 0;
	PI.timing[1] = 0;
	PI.power = 0;

	Tech PO;
	PO.name = "PO";
	PO.timing[0] = 0;
	PO.timing[1] = 0;
	PO.power = 0;

	TechLib.push_back(PI);
	TechLib.push_back(PO);

	return move(TechLib);
}


unordered_map<string, mappedNode> build_PIs(unordered_map<string, mappedNode> &network, vector<Node> &vecNodes, vector<Tech> &TechLib)
{
	int level = 0;
	mappedNode PI;
	PI.tech = &TechLib[TechLib.size()-2];
	PI.timing = 0;
	PI.delay = 0;
	PI.mode = 0;

	for(int i=0; i<vecNodes.size(); ++i)
	{
		if(vecNodes[i].level != level)
			break;	

		network.insert({vecNodes[i].name, PI});
	}

	return move(network);
}

unordered_map<string, mappedNode> build_POs(unordered_map<string, mappedNode> &network, vector<Node> &vecNodes, vector<Tech> &TechLib)
{
	int level = vecNodes[vecNodes.size()-1].level;
	mappedNode PO;
	PO.tech = &TechLib[TechLib.size()-1];
	PO.timing = 0;
	PO.delay = 0;
	PO.mode = 0;

	for(int i=vecNodes.size()-1; i>=0; --i)
	{
		if(vecNodes[i].level != level)
			break;
		
		network.insert({vecNodes[i].name, PO});
	}

	return move(network);
}

void set_connection_between(string this_node_name, string that_node_name, int phase, unordered_map<string, mappedNode> &network, vector<Tech> &TechLib, int INV, int NAND, stack<pair<string, mappedNode>> &container)
{
	if(that_node_name != "-1")
	{
		if(network.find(that_node_name) == network.end())
		{
			mappedNode that_node;
			that_node.tech = &TechLib[NAND];
			that_node.inv = &TechLib[INV];
			that_node.timing = 0;
			that_node.mode = 0;
			tuple<bool, string, Tech*> FO = { phase, this_node_name, &TechLib[INV] };
			that_node.FOs.push_back(FO);
			network.insert({that_node_name, that_node});
			container.push({that_node_name, that_node});
		}
		else
		{
			mappedNode &that_node = network.at(that_node_name);
			tuple<bool, string, Tech*> FO = { phase, this_node_name, &TechLib[INV] };
			that_node.FOs.push_back(FO);
		}		
	}
}

pair<string, bool> get_fanin_name_and_phase(string this_node_name, unordered_map<string, Node> &umapNode, int path)
{
	string name;
	bool phase;

	if(path == 0)
	{
		name = umapNode.at(this_node_name).fanin0;
		if(name != "-1")
		{
			Node that_node = umapNode.at(name);
			if(that_node.level != 0) phase = umapNode.at(this_node_name).phase0 ^ 1;
			else phase = umapNode.at(this_node_name).phase0;
		}	
	}
	else if(path == 1)
	{
		name = umapNode.at(this_node_name).fanin1;
		if(name != "-1")
		{
			Node that_node = umapNode.at(name);
			if(that_node.level != 0) phase = umapNode.at(this_node_name).phase1 ^ 1;
			else phase = umapNode.at(this_node_name).phase1;
		}
	}
	return {name, phase};
}

unordered_map<string, mappedNode> build_internalNodes(unordered_map<string, mappedNode> &network, unordered_map<string, Node> &umapNode, vector<Tech> &TechLib)
{
	auto [INV, NAND] = fastest_Tech(TechLib);
	cout << "Fastest INV:" << TechLib[INV].name << "   Fastest NAND:" << TechLib[NAND].name << endl;
	stack<pair<string, mappedNode>> container;
	for(auto &node : network)
	{
		container.push(node);
	}
	while(!container.empty())
	{
		pair<string, mappedNode> node = container.top();
		container.pop();
		auto [fanin0_name, phase0] = get_fanin_name_and_phase(node.first, umapNode, 0);
		set_connection_between(node.first, fanin0_name, phase0, network, TechLib, INV, NAND, container);
		auto [fanin1_name, phase1] = get_fanin_name_and_phase(node.first, umapNode, 1);
		set_connection_between(node.first, fanin1_name, phase1, network, TechLib, INV, NAND, container);
	}
	return move(network);
}

void print_network(unordered_map<string, mappedNode> &network, vector<Node> &vecNodes)
{
	for(auto &Node : vecNodes)
	{
		mappedNode node = network.at(Node.name);
		cout << Node.name << " " << node.tech->name << endl;
		cout << "level:" << Node.level << endl;
		cout << "mode:" << node.mode << endl;
		cout << "timing:" << node.timing << "  delay:" << node.delay << endl;
		for(auto &FO : node.FOs)
		{
			cout << get<0>(FO) << " " << get<1>(FO) << " " << get<2>(FO)->name << endl;
		}
		cout << endl;
	}
}

double get_total_power(unordered_map<string, mappedNode> &network)
{
	double power = 0;

	int NAND_count = 0;
	int INV_count = 0;

	for(auto &the_pair : network)
	{
		mappedNode &node = the_pair.second;
		if(node.tech->name == "PO") continue;
		if(node.tech->name != "PI") 
		{
			NAND_count++;
			power += node.tech->power;
		}
		
		for(auto &the_tuple : node.FOs)
		{
			bool &INV = get<0>(the_tuple);
			Tech *INV_tech = get<2>(the_tuple);

			if(INV)
			{
				power += INV_tech->power;
				INV_count++;
			} 
		}
	}
	return move(power);
}

pair<int, int> get_PO_PI_count(unordered_map<string, mappedNode> &network)
{
	int PO_count = 0;
	int PI_count = 0;

	for(auto &the_pair : network)
	{
		mappedNode &node = the_pair.second;
		if(node.tech->name == "PI") 
			PI_count++;
		if(node.tech->name == "PO") 
			PO_count++;
	}
	return {PO_count, PI_count};
}

pair<int, int> get_gate_count(unordered_map<string, mappedNode> &network)
{
	double power = 0;

	int NAND_count = 0;
	int INV_count = 0;

	for(auto &the_pair : network)
	{
		mappedNode &node = the_pair.second;
		if(node.tech->name[0] == 'N') 
			NAND_count++;

		for(auto &the_tuple : node.FOs)
		{
			bool &INV = get<0>(the_tuple);

			if(INV)
			{
				INV_count++;
			} 
		}
		if(node.mode == 1)
		{
			INV_count++;
		}
	}
	return {NAND_count, INV_count};
}

double get_initial_delay(unordered_map<string, mappedNode> &network, vector<Node> &vecNodes, int FO_level)
{
	double initial_delay = 0;
	for(auto &Node : vecNodes)
	{
		if(Node.level == 0 || Node.level == FO_level) continue;

		mappedNode &node = network.at(Node.name);
		vector<tuple<bool, string, Tech*>> &FOs = node.FOs;

		for(auto &the_tuple : FOs)
		{
			bool phase = get<0>(the_tuple);
			string FO_name = get<1>(the_tuple);
			Tech * INV_tech = get<2>(the_tuple);

			mappedNode &FO = network.at(FO_name);

			if(FO.mode == 1)
				FO.delay = FO.tech->timing[0] + FO.tech->timing[1] * 1;
			else
				FO.delay = FO.tech->timing[0] + FO.tech->timing[1] * FOs.size();

			double t = 0;
			if(phase == 1)
				t += INV_tech->timing[0] + INV_tech->timing[1] * 1;
			t += node.timing;
			t += FO.delay;
			if(node.mode == 1)	
				t += node.inv->timing[0] + node.inv->timing[1] * node.FOs.size();

			if(t > FO.timing)
				FO.timing = t;

			if(t >= initial_delay)
				initial_delay = t;
		}
	}
	return initial_delay;
}

int random_int_generator(int lower, int bound)
{
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> distrib(lower, bound);
	return distrib(gen);
}

void change_nand_mode(string gate_name, unordered_map<string, mappedNode> &network)
{
    network[gate_name].mode ^= 1;

    for(auto &FO : network[gate_name].FOs)
	{
		get<0>(FO) ^= 1;
	}
}

void change_inv_tech(string gate_name, unordered_map<string, mappedNode> &network, vector<Tech> &TechLib, pair<int, int> tech_range)
{
    int rd;
    do
    	rd = random_int_generator(tech_range.first, tech_range.second);
    while(network[gate_name].inv != &TechLib[rd]);

    network[gate_name].inv = &TechLib[rd];
}

void change_phase_tech(string gate_name, unordered_map<string, mappedNode> &network, vector<Tech> &TechLib, pair<int, int> tech_range)
{
    int FO_count = network[gate_name].FOs.size()-1;
    int FO = random_int_generator(0, FO_count);
    
    int tech = random_int_generator(tech_range.first, tech_range.second);

    while(get<2>(network[gate_name].FOs[FO]) == &TechLib[tech])
    {
    	tech = random_int_generator(tech_range.first, tech_range.second);
    }

   	get<2>(network[gate_name].FOs[FO]) = &TechLib[tech];
}

void change_nand_tech(string gate_name, unordered_map<string, mappedNode> &network, vector<Tech> &TechLib, pair<int, int> tech_range)
{
    int tech = random_int_generator(tech_range.first, tech_range.second);
    while(network[gate_name].tech == &TechLib[tech])
    {
    	tech = random_int_generator(tech_range.first, tech_range.second);
    }

    network[gate_name].tech = &TechLib[tech];
}

void select_neighbor(int num, string gate_name, unordered_map<string, mappedNode> &network, vector<Tech> &TechLib, pair<int, int> tech_inv_range, pair<int, int> tech_nand_range)
{
	switch(num)
	{
	case 0:
		if(network[gate_name].mode == 0)
		{
			int neighbor = random_int_generator(1, 3);
			select_neighbor(neighbor, gate_name, network, TechLib, tech_inv_range, tech_nand_range);
		}
		else
			change_inv_tech(gate_name, network, TechLib, tech_inv_range);
		break;
	case 1:
		change_nand_mode(gate_name, network);
		break;
	case 2:
		change_phase_tech(gate_name, network, TechLib, tech_inv_range);
		break;
	case 3:
		change_nand_tech(gate_name, network, TechLib, tech_nand_range);
		break;
	default:
		break;
	}
}

pair<int, int> get_range(vector<Tech> &TechLib, string tech)
{
	pair<int, int> range;
	bool out = 1;

	for(int i=0; i<TechLib.size(); ++i)
	{
		if(tech[0] == TechLib[i].name[0] && out)
		{
			range.first = i;
			out = 0;
		}
		if(tech[0] == TechLib[i].name[0])
		{
			range.second = i;
		}
	}
	return range;
}

bool if_pi_or_po(unordered_map<string, mappedNode> &network, string gate_name)
{
	if(network[gate_name].tech->name == "PI" || network[gate_name].tech->name == "PO")
		return true;
	return false;
}

void slack(unordered_map<string, mappedNode> &network, vector<Node> &vecNodes, double initial_delay)
{
	for(int i=vecNodes.size()-1; i>=0; --i)
	{
		string node_name = vecNodes[i].name;

		mappedNode &node   = network.at(node_name);
		if(node.tech->name == "PO")
		{
			node.required_time = initial_delay;
		}
		else
		{
			double min_required_time = RAND_MAX;
			for(auto &the_tuple : node.FOs)
			{
				mappedNode &FO = network[get<1>(the_tuple)];
				double FO_required_time = FO.required_time;
				bool phase = get<0>(the_tuple);
				double temp;
				if(phase)
					temp = FO_required_time - (get<2>(the_tuple)->timing[0] + get<2>(the_tuple)->timing[1]);
				else
					temp = FO_required_time;
				if(temp < min_required_time)
						min_required_time = temp;
			}
			if(node.mode == 1)
			{
				min_required_time = min_required_time - (node.inv->timing[0] + node.inv->timing[1] * node.FOs.size());
			}
			node.required_time = min_required_time;
		}
	}
	for(auto &v : vecNodes)
	{
		mappedNode &node = network.at(v.name);
		if(node.mode == 1)
			node.arrival_time = node.timing - (node.inv->timing[0] + node.inv->timing[1] * node.FOs.size());
		else
			node.arrival_time = node.timing;
		if(node.arrival_time < 0 || node.tech->name == "PI")
			node.arrival_time = 0;
		node.slack = node.required_time - node.arrival_time;
	}
}

void print_slack(unordered_map<string, mappedNode> &network, vector<Node> &vecNodes)
{
	for(auto &v : vecNodes)
	{
		mappedNode &node = network.at(v.name);
		cout << node.required_time << "-" << node.arrival_time << "   slack:" << node.slack << endl;
	}
}

bool logic_and_all_slack(unordered_map<string, mappedNode> &network)
{
	for(auto &pair : network)
	{
		if(&pair.second.slack < 0)
			return false;
	}
	return true;
}

pair<double, double> SA(unordered_map<string, mappedNode> &network, vector<Node> &vecNodes, vector<Tech> &TechLib, double initial_delay, double initial_power, int FO_level)
{
	int count = 0;
	int gate_count = vecNodes.size() - 1;

	double temperature = 30;
	double freeze = 0.01;
	double k = 0.9999;

	cout << "SA, T=30, Freeze=0.01, k=0.9999" << endl;

	double local_power = initial_power;
	double local_delay = initial_delay;

	pair<int, int> tech_inv_range = get_range(TechLib, "INV");
	pair<int, int> tech_nand_range = get_range(TechLib, "NAND");

	while(temperature > freeze)
	{
		count++;
		if(count % 10000 == 0)
		{
			cout << count/10000 << "0k." << flush;
		}

		int neighbor = random_int_generator(0, 3);
		int gate_num = random_int_generator(0, gate_count);
		string gate_name = vecNodes[gate_num].name;

		if(if_pi_or_po(network, gate_name) == true) continue;

		double neighbor_delay;
		double neighbor_power;

		unordered_map<string, mappedNode> old_network = network;

		select_neighbor(neighbor, gate_name, network, TechLib, tech_inv_range, tech_nand_range);

		neighbor_delay = get_initial_delay(network, vecNodes, FO_level);
		neighbor_power = get_total_power(network);

		slack(network, vecNodes, initial_delay);
		if(neighbor_delay > initial_delay || logic_and_all_slack(network) == 0)
		{
			network = old_network;
		}
		else if(neighbor_power-local_power == 0)
		{
			network = old_network;
			// continue;
		}
		else
		{
			if(neighbor_power <= local_power)
			{
				local_delay = neighbor_delay;
				local_power = neighbor_power;				
			}
			else
			{
				double random = static_cast <double> (rand()+1) / static_cast <double> (RAND_MAX);
				if( random < exp( -(neighbor_power-local_power) / temperature) )
				{
					local_delay = neighbor_delay;
					local_power = neighbor_power;					
				}
				else
				{
					network = old_network;
				}
			}
		}
		// cout << "neighbor power:" << neighbor_power  << "  local power:" << local_power << endl;	
		// cout << "diff:" << neighbor_power-local_power << "   temperature:" << temperature << endl;
		temperature *= k;
	}
	
	return {local_delay, local_power};
}

void output_mbench(vector<Node> &vecNodes, unordered_map<string, mappedNode> &network, vector<Tech> &TechLib, double initial_delay, double original_power, double optimized_power, string output)
{
	unordered_map<string, vector<string>> and_gate;

	fstream f(output, ios::out);
	f << "# Initial delay:" << initial_delay << endl;
	f << "# Original power:" << original_power << endl;
	f << "# Optimized power:" << optimized_power << endl;
	for(auto &n : vecNodes)
	{
		mappedNode &node = network[n.name];
		if(node.tech->name == "PI")
		{
			f << "INPUT(" << n.name << ")" << endl;
		}
		else if(node.tech->name == "PO")
		{
			f << "OUTPUT(" << n.name << ")" << endl;
		}

		vector<string> v;
		and_gate.insert({n.name, v});
	}

	int gate_num = 1;

	for(auto &pair : network)
	{
		string node_name = pair.first;
		mappedNode &node = pair.second;
		Tech *tech 	     = node.tech;
		if(node.mode == 1)
		{
			string INV = "Gate" + to_string(gate_num++);
			int min_required_time = RAND_MAX;
			Tech *INV_tech = node.inv;

			for(auto &FO : node.FOs)
			{
				bool phase = get<0>(FO);
				string FO_name = get<1>(FO);
				Tech *phase_tech = get<2>(FO);

				if(phase)
				{
					Tech *phase_tech = get<2>(FO);
					if(network[FO_name].tech->name != "PO")
					{
						string phase_name = "Gate" + to_string(gate_num++);	
						double phase_slack = (network[FO_name].required_time-network[FO_name].delay) - (node.arrival_time+phase_tech->timing[0]+phase_tech->timing[1]+node.inv->timing[0]+node.inv->timing[1]*node.FOs.size());	

						f << std::left << setw(10) << phase_name << " = " << phase_tech->name << "(" << INV << ")";
						f << std::right << setw(10) << "#" << phase_slack << endl;

						vector<string> &fanin = and_gate.at(FO_name);
						fanin.push_back(phase_name);			
					}
					else
					{
						string phase_name = "Gate" + to_string(gate_num++);	
						double phase_slack = (network[FO_name].required_time-network[FO_name].delay) - (node.arrival_time+phase_tech->timing[0]+phase_tech->timing[1]+node.inv->timing[0]+node.inv->timing[1]*node.FOs.size());

						f << std::left << setw(10) << phase_name << " = " << phase_tech->name << "(" << INV << ")";
						f << std::right << setw(10) << "#" << phase_slack << endl;
					}
					if(network[FO_name].required_time-network[FO_name].delay-(phase_tech->timing[0]+phase_tech->timing[1]) < min_required_time)
						min_required_time = network[FO_name].required_time-network[FO_name].delay-(phase_tech->timing[0]+phase_tech->timing[1]);
				}
				else
				{
					if(network[FO_name].tech->name != "PO")
					{
						vector<string> &fanin = and_gate.at(FO_name);
						fanin.push_back(INV);
					}
					else
					{
						f << std::left << setw(10) << FO_name << " = " << INV;
						f << std::right << setw(10) << "#" << network[FO_name].slack << endl;
					}
					if(network[FO_name].required_time-network[FO_name].delay < min_required_time)
						min_required_time = network[FO_name].required_time-network[FO_name].delay;
				}
			}
			double INV_slack = min_required_time - (node.arrival_time+node.inv->timing[0]+node.inv->timing[1]*node.FOs.size());
			f << std::left << setw(10) << INV << " = " << INV_tech->name << "(" << node_name << ")";
			f << std::right << setw(10) << "#" << INV_slack << endl;
		}
		else
		{
			for(auto &FO : node.FOs)
			{
				bool phase = get<0>(FO);
				string FO_name = get<1>(FO);
				Tech *phase_tech = get<2>(FO);

				if(phase)
				{
					if(network[FO_name].tech->name != "PO")
					{
						string phase_name = "Gate" + to_string(gate_num++);			
						double phase_slack = (network[FO_name].required_time-network[FO_name].delay) - (node.arrival_time+phase_tech->timing[0]+phase_tech->timing[1]);	

						f << std::left << setw(10) << phase_name << " = " << phase_tech->name << "(" << node_name << ")" ;
						f << std::right << setw(10) << "#" << phase_slack << endl;

						vector<string> &fanin = and_gate.at(FO_name);
						fanin.push_back(phase_name);
					}
					else
					{
						string phase_name = "Gate" + to_string(gate_num++);	
						double phase_slack = (network[FO_name].required_time-network[FO_name].delay) - node.arrival_time;	

						f << std::left << setw(10) << phase_name << " = " << phase_tech->name << "(" << node_name << ")";
						f << std::right << setw(10) << "#" << phase_slack << endl;
					}
				}
				else
				{
					if(network[FO_name].tech->name != "PO")
					{
						vector<string> &fanin = and_gate.at(FO_name);
						fanin.push_back(node_name);
					}
					else
					{
						f << std::left << setw(10) << FO_name << " = " << node_name;
						f << std::right << setw(10) << "#" << network[FO_name].slack << endl;
					}
				}	
			}
		}
	}
	for(auto &pair : and_gate)
	{
		string gate_name = pair.first;
		vector<string> fanin = pair.second;

		
		if(fanin.size() != 2)
			continue;
		string instance = "Gate" + to_string(gate_num++);
		f << std::left << setw(10) << gate_name << " = " << network[gate_name].tech->name << "(" << fanin[0] << ", " << fanin[1] << ")";
		f << std::right << setw(10) << "#" << network[gate_name].slack << endl;
	}
}
