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

unordered_map<string, Node> set_FOs_level(unordered_map<string, Node> &umapNode)
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

	return move(umapNode);
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

void print_network(unordered_map<string, mappedNode> &network)
{
	for(auto &node : network)
	{
		cout << node.first << " " << node.second.tech->name << " " << node.second.tech->timing[0] << " " << node.second.tech->timing[1] << " " << node.second.tech->power << endl;
		for(auto &FO : node.second.FOs)
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
		if(node.tech->name[0] != 'N') continue;
		NAND_count++;
		power += node.tech->power;

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
	cout << "NAND_count:" << NAND_count << "   INV_count:" << INV_count << endl;
	return move(power);
}

double get_initial_delay(unordered_map<string, mappedNode> &network)
{
	for(auto &the_pair : network)
	{
		mappedNode &node = the_pair.second;
		if(node.tech->name != "PI") continue;

		node.timing = 0;
	}
}