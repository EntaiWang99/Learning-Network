#pragma warning(disable: 4305 4267 4018) 
#pragma warning(disable: 6385 6386)
#include <iostream>
#include <fstream>
#include <list> 
#include <omp.h>
#include <algorithm>
#include <time.h>
#include <functional>
#include <stdio.h>   
#include <math.h>

#include <numeric> 
#include <stack>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <iomanip>
#define _MAX_MEMORY_BLOCKS 20

using namespace std;


class Assignment {
public:
	Assignment()
	{
		g_number_of_memory_blocks = 4;
		total_demand_volume = 0.0;
		g_origin_demand_array = NULL;

		g_number_of_threads = 1;
		g_number_of_K_paths = 20;

		g_number_of_links = 0;

		g_number_of_nodes = 0;
		g_number_of_zones = 0;

		g_pFileDebugLog = NULL;
		assignment_mode = 0;  // default is UE
	}

	~Assignment()
	{

	}
	int g_number_of_threads;
	int g_number_of_K_paths;
	int assignment_mode;
	int g_number_of_memory_blocks;

	std::map<int, int> g_internal_node_to_seq_no_map;  // hash table, map external node number to internal node sequence no. 
	std::map<int, int> g_zoneid_to_zone_seq_no_mapping;// from integer to integer map zone_id to zone_seq_no
	std::map<string, int> g_road_link_id_map;


	float*** g_origin_demand_array;

	//StatisticOutput.cpp
	float total_demand_volume;

	int g_LoadingStartTimeInMin;
	int g_LoadingEndTimeInMin;

	std::map<string, int> demand_period_to_seqno_mapping;
	std::map<string, int> agent_type_2_seqno_mapping;

	int g_number_of_links;
	int g_number_of_nodes;
	int g_number_of_zones;
	int g_number_of_demand;

	FILE* g_pFileDebugLog = NULL;

};
Assignment assignment;

class CNode
{
public:
	CNode()
	{
		zone_id = -1;
		node_seq_no = -1;
	}

	int node_seq_no;  // sequence number 
	int node_id;      //external node number 
	int zone_id = -1;

	double x;
	double y;

	std::vector<int> m_outgoing_link_seq_no_vector;
	std::vector<int> m_incoming_link_seq_no_vector;

	std::vector<int> m_to_node_seq_no_vector;
	std::map<int, int> m_to_node_2_link_seq_no_map;

};

class CLink
{
public:
	CLink()  // construction 
	{

	}
	int from_node_seq_no;
	int to_node_seq_no;
	int link_seq_no;
	string link_id;
	float lenth;
	float volume;
};

class CDemand
{
public:
	CDemand()  
	{

	}
	int from_zone_seq_no;
	int to_zone_seq_no;
	int demand;
};

std::vector<CNode> g_node_vector;
std::vector<CLink> g_link_vector;
std::vector<CDemand> g_demand_vector;

class CCSVParser
{
public:
	char Delimiter;
	bool IsFirstLineHeader;
	ifstream inFile;
	string mFileName;
	vector<string> LineFieldsValue;
	vector<string> Headers;
	map<string, int> FieldsIndices;
	vector<int> LineIntegerVector;

public:
	void  ConvertLineStringValueToIntegers()
	{
		LineIntegerVector.clear();
		for (unsigned i = 0; i < LineFieldsValue.size(); i++)
		{
			std::string si = LineFieldsValue[i];
			int value = atoi(si.c_str());
			if (value >= 1)
				LineIntegerVector.push_back(value);
		}
	}
	vector<string> GetHeaderVector()
	{
		return Headers;
	}

	bool m_bDataHubSingleCSVFile;
	string m_DataHubSectionName;
	bool m_bLastSectionRead;
	bool m_bSkipFirstLine;  // for DataHub CSV files

	CCSVParser(void)
	{
		Delimiter = ',';
		IsFirstLineHeader = true;
		m_bSkipFirstLine = false;
		m_bDataHubSingleCSVFile = false;
		m_bLastSectionRead = false;
	}

	~CCSVParser(void)
	{
		if (inFile.is_open()) inFile.close();
	}

	bool OpenCSVFile(string fileName, bool b_required)
	{
		mFileName = fileName;
		inFile.open(fileName.c_str());

		if (inFile.is_open())
		{
			if (IsFirstLineHeader)
			{
				string s;
				std::getline(inFile, s);
				vector<string> FieldNames = ParseLine(s);

				for (size_t i = 0; i < FieldNames.size(); i++)
				{
					string tmp_str = FieldNames.at(i);
					size_t start = tmp_str.find_first_not_of(" ");

					string name;
					if (start == string::npos)
					{
						name = "";
					}
					else
					{
						name = tmp_str.substr(start);
					}
					FieldsIndices[name] = (int)i;
				}
			}

			return true;
		}
		else
		{
			if (b_required)
			{
				cout << "File " << fileName << " does not exist. Please check." << endl;
			}
			return false;
		}
	}

	void CloseCSVFile(void)
	{
		inFile.close();
	}

	bool ReadRecord()
	{
		LineFieldsValue.clear();

		if (inFile.is_open())
		{
			string s;
			std::getline(inFile, s);
			if (s.length() > 0)
			{
				LineFieldsValue = ParseLine(s);
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	vector<string> ParseLine(string line)
	{
		vector<string> SeperatedStrings;
		string subStr;

		if (line.length() == 0)
			return SeperatedStrings;

		istringstream ss(line);

		if (line.find_first_of('"') == string::npos)
		{

			while (std::getline(ss, subStr, Delimiter))
			{
				SeperatedStrings.push_back(subStr);
			}

			if (line.at(line.length() - 1) == ',')
			{
				SeperatedStrings.push_back("");
			}
		}
		else
		{
			while (line.length() > 0)
			{
				size_t n1 = line.find_first_of(',');
				size_t n2 = line.find_first_of('"');

				if (n1 == string::npos && n2 == string::npos) //last field without double quotes
				{
					subStr = line;
					SeperatedStrings.push_back(subStr);
					break;
				}

				if (n1 == string::npos && n2 != string::npos) //last field with double quotes
				{
					size_t n3 = line.find_first_of('"', n2 + 1); // second double quote

					//extract content from double quotes
					subStr = line.substr(n2 + 1, n3 - n2 - 1);
					SeperatedStrings.push_back(subStr);

					break;
				}

				if (n1 != string::npos && (n1 < n2 || n2 == string::npos))
				{
					subStr = line.substr(0, n1);
					SeperatedStrings.push_back(subStr);
					if (n1 < line.length() - 1)
					{
						line = line.substr(n1 + 1);
					}
					else // comma is the last char in the line string, push an empty string to the back of vector
					{
						SeperatedStrings.push_back("");
						break;
					}
				}

				if (n1 != string::npos && n2 != string::npos && n2 < n1)
				{
					size_t n3 = line.find_first_of('"', n2 + 1); // second double quote
					subStr = line.substr(n2 + 1, n3 - n2 - 1);
					SeperatedStrings.push_back(subStr);
					size_t idx = line.find_first_of(',', n3 + 1);

					if (idx != string::npos)
					{
						line = line.substr(idx + 1);
					}
					else
					{
						break;
					}
				}
			}

		}

		return SeperatedStrings;
	}

	template <class T> bool GetValueByFieldName(string field_name, T& value, bool NonnegativeFlag = true, bool required_field = true)
	{

		if (FieldsIndices.find(field_name) == FieldsIndices.end())
		{
			if (required_field)
			{
				cout << "Field " << field_name << " in file " << mFileName << " does not exist. Please check the file." << endl;

			}
			return false;
		}
		else
		{
			if (LineFieldsValue.size() == 0)
			{
				return false;
			}

			int size = (int)(LineFieldsValue.size());
			if (FieldsIndices[field_name] >= size)
			{
				return false;
			}

			string str_value = LineFieldsValue[FieldsIndices[field_name]];

			if (str_value.length() <= 0)
			{
				return false;
			}

			istringstream ss(str_value);

			T converted_value;
			ss >> converted_value;

			if (/*!ss.eof() || */ ss.fail())
			{
				return false;
			}

			if (NonnegativeFlag && converted_value < 0)
				converted_value = 0;

			value = converted_value;
			return true;
		}
	}


	bool GetValueByFieldName(string field_name, string& value)
	{
		if (FieldsIndices.find(field_name) == FieldsIndices.end())
		{
			return false;
		}
		else
		{
			if (LineFieldsValue.size() == 0)
			{
				return false;
			}

			unsigned int index = FieldsIndices[field_name];
			if (index >= LineFieldsValue.size())
			{
				return false;
			}
			string str_value = LineFieldsValue[index];

			if (str_value.length() <= 0)
			{
				return false;
			}

			value = str_value;
			return true;
		}
	}
};

void g_ReadInputData(Assignment& assignment)
{
	// Step 1：读取“node.csv”文件
	int internal_node_seq_no = 0;
	std::map<int, int> zone_id_to_node_id_mapping;

	CCSVParser parser;
	if (parser.OpenCSVFile("node.csv", true))
	{
		while (parser.ReadRecord())
		{
			int node_id;
			if (parser.GetValueByFieldName("node_id", node_id) == false)
				continue;

			//scan the node list, this node has not been defined.
			if (assignment.g_internal_node_to_seq_no_map.find(node_id) != assignment.g_internal_node_to_seq_no_map.end())
			{
				continue; //has been defined
			}
			assignment.g_internal_node_to_seq_no_map[node_id] = internal_node_seq_no;

			CNode node;
			node.node_id = node_id;
			node.node_seq_no = internal_node_seq_no;
			int zone_id = -1;
			parser.GetValueByFieldName("zone_id", zone_id);

			if (zone_id >= 1)
			{
				if (zone_id_to_node_id_mapping.find(zone_id) == zone_id_to_node_id_mapping.end())
				{
					zone_id_to_node_id_mapping[zone_id] = node_id;
					node.zone_id = zone_id;
				}
				else
				{
					cout << "warning: zone_id " << zone_id << " have been defined more than once." << endl;
				}
			}
			internal_node_seq_no++;
			g_node_vector.push_back(node);
			assignment.g_number_of_nodes++;
		}
		cout << "number of nodes = " << assignment.g_number_of_nodes << endl;
		parser.CloseCSVFile();
	}

	// step 2: 读取“road_link.csv”文件
	CCSVParser parser_link;
	if (parser_link.OpenCSVFile("road_link.csv", true))
	{
		int internal_link_seq_no = 0;
		while (parser_link.ReadRecord())  // if this line contains [] mark, then we will also read field headers.
		{
			int from_node_id;
			int to_node_id;
			if (parser_link.GetValueByFieldName("from_node_id", from_node_id) == false)
				continue;
			if (parser_link.GetValueByFieldName("to_node_id", to_node_id) == false)
				continue;

			string linkID;
			parser_link.GetValueByFieldName("road_link_id", linkID);

			int lenth;
			parser_link.GetValueByFieldName("length", lenth);
			
			// add the to node id into the outbound (adjacent) node list
			if (assignment.g_internal_node_to_seq_no_map.find(from_node_id) == assignment.g_internal_node_to_seq_no_map.end())
			{
				cout << "Error: from_node_id " << from_node_id << " in file road_link.csv is not defined in node.csv." << endl;
				continue; //has not been defined
			}
			if (assignment.g_internal_node_to_seq_no_map.find(to_node_id) == assignment.g_internal_node_to_seq_no_map.end())
			{
				cout << "Error: to_node_id " << to_node_id << " in file road_link.csv is not defined in node.csv." << endl;
				continue; //has not been defined
			}

			if (assignment.g_road_link_id_map.find(linkID) != assignment.g_road_link_id_map.end())
			{
				cout << "Error: road_link_id " << linkID.c_str() << " has been defined more than once. Please check road_link.csv." << endl;
				continue; //has not been defined
			}
			int internal_from_node_seq_no = assignment.g_internal_node_to_seq_no_map[from_node_id];  // map external node number to internal node seq no. 
			int internal_to_node_seq_no = assignment.g_internal_node_to_seq_no_map[to_node_id];

			CLink link;

			link.from_node_seq_no = internal_from_node_seq_no;
			link.to_node_seq_no = internal_to_node_seq_no;
			link.link_seq_no = assignment.g_number_of_links;
			link.to_node_seq_no = internal_to_node_seq_no;
			link.lenth = lenth;
			link.link_id = linkID;

			assignment.g_road_link_id_map[link.link_id] = 1;
			internal_link_seq_no++;

			g_node_vector[internal_from_node_seq_no].m_outgoing_link_seq_no_vector.push_back(link.link_seq_no);
			g_node_vector[internal_to_node_seq_no].m_incoming_link_seq_no_vector.push_back(link.link_seq_no);


			g_link_vector.push_back(link);
			assignment.g_number_of_links++;
		}
		parser_link.CloseCSVFile();
		cout << "number of links = " << assignment.g_number_of_links << endl;	
	}


	// Step 3: 读取“demand.csv”文件
	CCSVParser parser_demand;
	if (parser_demand.OpenCSVFile("demand.csv", true)) {
		while (parser_demand.ReadRecord()) {
			int from_zone_id;
			int to_zone_id;
			int number_of_agents;
			if (parser_demand.GetValueByFieldName("from_zone_id", from_zone_id) == false)
				continue;
			if (parser_demand.GetValueByFieldName("to_zone_id", to_zone_id) == false)
				continue;
			if (parser_demand.GetValueByFieldName("number_of_agents", number_of_agents) == false)
				continue;

			CDemand demand;
			demand.from_zone_seq_no = assignment.g_internal_node_to_seq_no_map[from_zone_id];
			demand.to_zone_seq_no = assignment.g_internal_node_to_seq_no_map[to_zone_id];
			demand.demand = number_of_agents;

			g_demand_vector.push_back(demand);
			assignment.g_number_of_demand++;
		}
		cout << "number of demands = " << assignment.g_number_of_demand << endl;
	}
};

template <typename T>
vector<size_t> sort_indexes_e(vector<T>& v)
{
	vector<size_t> idx(v.size());
	iota(idx.begin(), idx.end(), 0);
	sort(idx.begin(), idx.end(),
		[&v](size_t i1, size_t i2) {return v[i1] < v[i2]; });
	return idx;
}

class NetworkForSUE
{
public:
	std::vector<int> origin_node_vector;
	std::vector<int> destination_node_vector;
	std::vector<int> demand_volume_vector;
	int** sub_network_link_for_each_od;
	int** sub_network_node_for_each_od;
	double** sub_network_volume_for_each_od;
	int* node_number_for_sub_network;
	int* link_number_for_sub_network;

	int b_Dial = 1;

	int m_g_number_of_links = assignment.g_number_of_links;
	int m_g_number_of_nodes = assignment.g_number_of_nodes;


	void AllocateMemorySUE(int number_of_nodes, int number_of_links) {
		sub_network_link_for_each_od = new int* [demand_volume_vector.size()];
		sub_network_node_for_each_od = new int* [demand_volume_vector.size()];
		sub_network_volume_for_each_od = new double* [demand_volume_vector.size()];

		link_number_for_sub_network = new int[demand_volume_vector.size()]();
		node_number_for_sub_network = new int[demand_volume_vector.size()]();
	}


	void Dial_subnetwork(float** adj_matrix) {
		float* r_distance = new float[assignment.g_number_of_nodes];
		float* s_distance = new float[assignment.g_number_of_nodes];
		double* link_likelihood = new double[assignment.g_number_of_links];
		double* link_weights_list = new double[assignment.g_number_of_links];
		double* link_label_list = new double[assignment.g_number_of_links];
		double* link_flow_list = new double[assignment.g_number_of_links];

		for (int od = 0; od < origin_node_vector.size(); od++) {
			int origin_node = origin_node_vector[od];
			int destination_node = destination_node_vector[od];
			int demand_volume = demand_volume_vector[od];
			for (int j = 0; j < assignment.g_number_of_links; j++) {
				link_likelihood[j] = 0.0;
				link_weights_list[j] = 0.0;
				link_label_list[j] = 0.0;
				link_flow_list[j] = 0.0;
			}
			for (int j = 0; j < assignment.g_number_of_nodes; j++) {
				r_distance[j] = 0.0;
				s_distance[j] = 0.0;
			}

			//Step 1：
			for (int i = 0; i < assignment.g_number_of_nodes; i++) {
				r_distance[i] = adj_matrix[origin_node][i];
			}
			for (int i = 0; i < assignment.g_number_of_nodes; i++) {
				s_distance[i] = adj_matrix[i][destination_node];
			}

			for (int i = 0; i < assignment.g_number_of_links; i++) {
				int from_node = g_link_vector[i].from_node_seq_no;
				int to_node = g_link_vector[i].to_node_seq_no;
				if ((r_distance[from_node] < r_distance[to_node]) & (s_distance[from_node] > s_distance[to_node])) {
					link_likelihood[i] = exp((float)b_Dial * (r_distance[to_node] - r_distance[from_node] - (float)g_link_vector[i].lenth));
					if (link_likelihood[i] > 65535)
						link_likelihood[i] = 0;
				}
			}

			//Step 2:
			vector<float> r_distance_array(assignment.g_number_of_nodes);
			for (int i = 0; i < r_distance_array.size(); i++)
			{
				r_distance_array[i] = r_distance[i];
			}
			vector<size_t> r_index_list;
			r_index_list = sort_indexes_e(r_distance_array);
			vector<float> s_distance_array(assignment.g_number_of_nodes);
			for (int i = 0; i < s_distance_array.size(); i++)
			{
				s_distance_array[i] = s_distance[i];
			}
			vector<size_t> s_index_list;
			s_index_list = sort_indexes_e(s_distance_array);


			for each (int node_list_no in r_index_list) {
				CNode node = g_node_vector[node_list_no];
				if (node.node_seq_no == destination_node)
					break;
				if (node.node_seq_no == origin_node) {
					for each (int next_link in g_node_vector[node_list_no].m_outgoing_link_seq_no_vector) {
						link_weights_list[next_link] = link_likelihood[next_link];
						link_label_list[next_link] = 1.0;
					}
				}
				else {
					float current_label = 1.0;
					for each (int pre_link in g_node_vector[node_list_no].m_incoming_link_seq_no_vector) {
						if (link_likelihood[pre_link] > 0) {
							current_label = current_label * link_label_list[pre_link];
						}
					}
					if (current_label == 1.0) {
						for each (int next_link in g_node_vector[node_list_no].m_outgoing_link_seq_no_vector) {
							float sum_w = 0.0;
							for each (int pre_link in g_node_vector[node_list_no].m_incoming_link_seq_no_vector) {
								sum_w += link_weights_list[pre_link];
							}
							link_weights_list[next_link] = link_likelihood[next_link] * sum_w;
							link_label_list[next_link] = 1.0;
						}
					}
				}
			}

			//Step 3:
			memset(link_label_list, 0.0, sizeof(link_label_list));
			for each (int node_list_no in s_index_list) {
				CNode node = g_node_vector[node_list_no];
				if (node.node_seq_no == origin_node)
					break;
				if (node.node_seq_no == destination_node) {
					float sum_w_pre = 0.0;
					for each (int pre_link in g_node_vector[node_list_no].m_incoming_link_seq_no_vector) {
						sum_w_pre += link_weights_list[pre_link];
					}
					for each (int pre_link in g_node_vector[node_list_no].m_incoming_link_seq_no_vector) {
						if (link_weights_list[pre_link] == 0) {
							link_flow_list[pre_link] = 0.0;
							link_label_list[pre_link] = 1.0;
						}
						else {
							link_flow_list[pre_link] = (link_weights_list[pre_link] / sum_w_pre) * demand_volume;
							link_label_list[pre_link] = 1.0;
						}
					}
				}
				else {
					float current_label = 1.0;
					float sum_flow = 0.0;
					for each (int next_link in g_node_vector[node_list_no].m_outgoing_link_seq_no_vector) {
						if (link_weights_list[next_link] > 0) {
							current_label = current_label * link_label_list[next_link];
							sum_flow += link_flow_list[next_link];
						}
					}
					if (current_label == 1.0) {
						float sum_w_pre = 0.0;
						for each (int pre_link in g_node_vector[node_list_no].m_incoming_link_seq_no_vector) {
							sum_w_pre += link_weights_list[pre_link];
						}
						for each (int pre_link in g_node_vector[node_list_no].m_incoming_link_seq_no_vector) {
							if (link_weights_list[pre_link] == 0) {
								link_flow_list[pre_link] = 0.0;
								link_label_list[pre_link] = 1.0;
							}
							else {
								link_flow_list[pre_link] = (link_weights_list[pre_link] / sum_w_pre) * sum_flow;
								link_label_list[pre_link] = 1.0;
							}
						}
					}
				}
			}

			//1. Update link numbers for each od
			link_number_for_sub_network[od] = 0;
			for (int i = 0; i < assignment.g_number_of_links; i++) {
				if (link_flow_list[i] != 0) {
					link_number_for_sub_network[od]++;
				}
			}

			//2. Update link volume for each od
			int count = 0;
			sub_network_volume_for_each_od[od] = new double[link_number_for_sub_network[od]];
			for (int j = 0; j < m_g_number_of_links; j++) {
				if (link_flow_list[j] != 0) {
					sub_network_volume_for_each_od[od][count] = link_flow_list[j];
					count++;
				}
			}

			//3.Update link index for each od
			count = 0;
			sub_network_link_for_each_od[od] = new int[link_number_for_sub_network[od]];
			for (int j = 0; j < m_g_number_of_links; j++) {
				if (link_flow_list[j] != 0) {
					sub_network_link_for_each_od[od][count] = j;
					count++;
				}
			}

			//4. Update globle link voulme for each od
			for (int i = 0; i < link_number_for_sub_network[od]; i++) {
				g_link_vector[sub_network_link_for_each_od[od][i]].volume += sub_network_volume_for_each_od[od][i];
			}
		}
		delete[] r_distance;
		delete[] s_distance;
		delete[] link_likelihood;
		delete[] link_weights_list;
		delete[] link_label_list;
		delete[] link_flow_list;
	}
};
std::vector<NetworkForSUE*> g_NetworkForSUE_vector;

void g_assign_computing_tasks_to_memory_blocks_SUE(Assignment& assignment) {
	NetworkForSUE* PointerMatrxSUE[_MAX_MEMORY_BLOCKS];
	int i = 0;
	for (int demand_no = 0; demand_no < assignment.g_number_of_demand; demand_no++)
	{
		if (g_demand_vector[demand_no].demand != 0)
		{
			if (i < assignment.g_number_of_memory_blocks) {
				NetworkForSUE* p_NetworkForSUE = new NetworkForSUE();
				p_NetworkForSUE->origin_node_vector.push_back(g_demand_vector[demand_no].from_zone_seq_no);
				p_NetworkForSUE->destination_node_vector.push_back(g_demand_vector[demand_no].to_zone_seq_no);
				p_NetworkForSUE->demand_volume_vector.push_back(g_demand_vector[demand_no].demand);
				p_NetworkForSUE->AllocateMemorySUE(assignment.g_number_of_nodes, assignment.g_number_of_links);
				PointerMatrxSUE[i] = p_NetworkForSUE;
				g_NetworkForSUE_vector.push_back(p_NetworkForSUE);
				i++;
			}
			else {
				int memory_block_no = i % assignment.g_number_of_memory_blocks;
				NetworkForSUE* p_NetworkForSUE = PointerMatrxSUE[memory_block_no];
				p_NetworkForSUE->origin_node_vector.push_back(g_demand_vector[demand_no].from_zone_seq_no);
				p_NetworkForSUE->destination_node_vector.push_back(g_demand_vector[demand_no].to_zone_seq_no);
				p_NetworkForSUE->demand_volume_vector.push_back(g_demand_vector[demand_no].demand);
				p_NetworkForSUE->AllocateMemorySUE(assignment.g_number_of_nodes, assignment.g_number_of_links);
				i++;
			}
		}
	}

}

void fopen_ss(FILE** file, const char* fileName, const char* mode)
{
	*file = fopen(fileName, mode);
}

void g_output_RLSUE_result(Assignment& assignment)
{
	cout << "writing result.csv.." << endl;
	FILE* g_pFileLinkMOE = NULL;
	fopen_ss(&g_pFileLinkMOE, "result.csv", "w");
	if (g_pFileLinkMOE == NULL)
	{
		cout << "File result.csv cannot be opened." << endl;
	}
	else
	{
		fprintf(g_pFileLinkMOE, "link_id,from_node_id,to_node_id,volume");
		fprintf(g_pFileLinkMOE, "\n");

		for (int j = 0; j < assignment.g_number_of_links; j++) {
			fprintf(g_pFileLinkMOE, "%s,%d,%d,%.3f",
				g_link_vector[j].link_id.c_str(),
				g_node_vector[g_link_vector[j].from_node_seq_no].node_id,
				g_node_vector[g_link_vector[j].to_node_seq_no].node_id,
				g_link_vector[j].volume);
			fprintf(g_pFileLinkMOE, "\n");
		}
		fclose(g_pFileLinkMOE);
	}
}

float** floyd_SPP(Assignment assignment) {
	float** adj_matrix;
	adj_matrix = new float* [assignment.g_number_of_nodes];
	for (int i = 0; i < assignment.g_number_of_nodes; i++) {
		adj_matrix[i] = new float[assignment.g_number_of_nodes];
	}
	for (int i = 0; i < assignment.g_number_of_nodes; i++) {
		for (int j = 0; j < assignment.g_number_of_nodes; j++)
		{
			adj_matrix[i][j] = 65535;
		}
	}
	for (int i = 0; i < assignment.g_number_of_links; i++) {
		adj_matrix[g_link_vector[i].from_node_seq_no][g_link_vector[i].to_node_seq_no] = g_link_vector[i].lenth;
	}

	for (int u = 0; u < assignment.g_number_of_nodes; ++u) {
		for (int v = 0; v < assignment.g_number_of_nodes; ++v) {
			for (int w = 0; w < assignment.g_number_of_nodes; ++w) {
				if (adj_matrix[v][w] > adj_matrix[v][u] + adj_matrix[u][w]) {
					adj_matrix[v][w] = adj_matrix[v][u] + adj_matrix[u][w];
				}
			}
		}
		adj_matrix[u][u] = 0;
	}
	for (int i = 0; i < assignment.g_number_of_nodes; i++) {
		for (int j = 0; j < assignment.g_number_of_nodes; j++) {
			if (adj_matrix[j][i] < 65535)
				adj_matrix[i][j] = adj_matrix[j][i];
			if (adj_matrix[i][j] < 65535)
				adj_matrix[j][i] = adj_matrix[i][j];
		}
	}
	return adj_matrix;
}


int main()
{
	// 1.计时开始
	clock_t startTime, endTime;
	startTime = clock();

	// 2.读取数据，包括node,link,demand
	g_ReadInputData(assignment);

	// 3.准备所有node间最短径路，使用Floyd方法实现(Dijstra亦可)
	float** adj_matrix;
	adj_matrix = floyd_SPP(assignment);

	// 4.重置所有link流量为0
	for (int i = 0; i < assignment.g_number_of_links; i++) {
		g_link_vector[i].volume = 0.0;
	}

	// 5.准备并行计算工作台(将所有OD均分至工作台)
	g_assign_computing_tasks_to_memory_blocks_SUE(assignment);

	// 6.每个工作台中分别执行Dial算法
#pragma omp parallel for 
	for (int ProcessID = 0; ProcessID < g_NetworkForSUE_vector.size(); ProcessID++) {
		g_NetworkForSUE_vector[ProcessID]->Dial_subnetwork(adj_matrix);
	}

	// 7.输出结果
	g_output_RLSUE_result(assignment);

	// 8. 释放Floyd矩阵内存空间
	for (int i = 0; i < assignment.g_number_of_nodes; i++)
	{
		delete[] adj_matrix[i];
	}
	delete[] adj_matrix;

	// 9.计时停止
	endTime = clock();
	cout << "The run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
	
	system("pause");
	return 1;
}