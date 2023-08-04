#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <tuple>
#include <set>
#include <string>
#include <string.h>
#include <cmath>
#include <algorithm>
using namespace std;
using LL = long long int;
using block = pair<string,LL>;
using bits_data = pair<double,LL>;
#define Min(x,y) (x < y ? x : y)
#define Max(x,y) (x > y ? x : y)
#define INF (LL) (1e19)

LL string_to_int(string str) {
    LL sum = 0;
    LL i = 0;

    while (isdigit(str[i])) {
        sum = sum * 10 + str[i] - '0';
        i++;
    }
    return sum;
}

LL binary_to_decimal(string str) {
    LL sum = 0;
    LL i = 0;

    while (isdigit(str[i])) {
        sum = sum * 2 + str[i] - '0';
        i++;
    }
    return sum;
}


int main(int argc, char* argv[])
{
	ifstream fin;
	fin.open(argv[1], ios::in); //read cache.org

	ofstream fout; 
	fout.open(argv[3], ios::out); //write index.rpt

	string data;
	string Address_bits_string;
	string Block_size_string;
	string Cache_sets_string;
	string Associativity_string;
	string Address_bits_num;
	string Block_size_num;
	string Cache_sets_num;
	string Associativity_num;
	LL Address_bits;
	LL Block_size;
	LL Cache_sets;
	LL Associativity;
	for(int i=0;i<4;i++){
		getline(fin, data,'\n');
		if(data[1]=='d'){
			size_t found_dash = data.find("_");
			string start = data.substr(0,found_dash);
			string end = data.substr(found_dash+1);
			Address_bits_string = start + " " + end;
			size_t found_space = data.find(" ");
			Address_bits_num = data.substr(found_space+1);
			Address_bits = string_to_int(Address_bits_num);
		}else if(data[1]=='l'){
			size_t found_dash = data.find("_");
			string start = data.substr(0,found_dash);
			string end = data.substr(found_dash+1);
			Block_size_string = start + " " + end;
			size_t found_space = data.find(" ");
			Block_size_num = data.substr(found_space+1);
			Block_size = string_to_int(Block_size_num);
		}else if(data[1]=='a'){
			size_t found_dash = data.find("_");
			string start = data.substr(0,found_dash);
			string end = data.substr(found_dash+1);
			Cache_sets_string = start + " " + end;
			size_t found_space = data.find(" ");
			Cache_sets_num = data.substr(found_space+1);
			Cache_sets = string_to_int(Cache_sets_num);
		}else if(data[1]=='s'){
			Associativity_string = data;
			size_t found_space = data.find(" ");
			Associativity_num = data.substr(found_space+1);
			Associativity = string_to_int(Associativity_num);
		}
	}
	fin.close(); //close cache.org

	string reference_start;
	string reference_end;
	vector<string> byte_address;
	LL cache_accesses_count = 0;
	fin.open(argv[2], ios::in); //read reference.lst
	
	while(getline(fin, data,'\n')){
		if (fin.eof()){
			break;
		}else if (data.size()==0){
			continue;
		}else if(data[1]=='b'){
			reference_start = data;
		}else if(data[1]=='e'){
			reference_end = data;
		}else{
			byte_address.push_back(data);
			cache_accesses_count++;
		}
	}
	fin.close();//close reference.lst

	//calculate offset bit and index bit
	LL Offset_bit_count = (LL)log2(Block_size);
	LL Indexing_bit_count = (LL)log2(Cache_sets);
	
	//calculate block address(binary)
	vector<string> block_address;
	LL block_address_bits = Address_bits - Offset_bit_count;
	for(LL i=0;i<cache_accesses_count;i++){
		string tmp_block_address = byte_address[i].substr(0,block_address_bits);
		block_address.push_back(tmp_block_address);
	}

	//calculate address tag(binary) and address index(binary and decimal)
	vector<int> same_position_address_bits[block_address_bits];
	for(LL i=0;i<cache_accesses_count;i++){
		for(LL j=0;j<block_address_bits;j++){
			same_position_address_bits[j].push_back(block_address[i][j]-'0');
		}
	}
	//calculate correlation
	vector<vector<double>> correlation(block_address_bits,vector<double>(block_address_bits,0));
	double E, D, C;
	for(LL i=0;i<block_address_bits;i++){
		for(LL j=0;j<block_address_bits;j++){
			E = 0;D = 0;C = 0;
			for(LL k=0;k<cache_accesses_count;k++){
				if(same_position_address_bits[i][k]==same_position_address_bits[j][k]){
					E++;
				}else{
					D++;
				}
			}
			C = Min(E,D)/Max(E,D);
			correlation[i][j] = C;

		}
	}
	//calculate quality
	vector<bits_data> quality(block_address_bits);
	double Z, O, Q;
	for(LL i=0;i<block_address_bits;i++){
		Z = 0;O = 0;Q = 0;
		for(LL j=0;j<cache_accesses_count;j++){
			if(same_position_address_bits[i][j]==0){
				Z++;
			}else{
				O++;
			}
		}
		Q = Min(Z,O)/Max(Z,O);
		bits_data tmp_bits_data = bits_data(Q, i);
		quality[i]= tmp_bits_data;
	}
	//use quality and correlation to select index bits
	LL select_bits_count = Indexing_bit_count;
	vector<bits_data> tmp_quality;
	vector<LL> select_bits_array(block_address_bits);
	for(LL i=0;i<block_address_bits;i++){
			tmp_quality.push_back(quality[i]);
	}
	while(select_bits_count){
		set<bits_data> tmp_quality_search;
		for(auto &i:tmp_quality){
			tmp_quality_search.insert(i);
		}

		bits_data max = *(tmp_quality_search.rbegin());

		LL max_index = max.second;
		select_bits_array[max_index]=1;

		vector<bits_data>::iterator tmp_max_it;
		for(auto it=tmp_quality.begin();it!=tmp_quality.end();it++){
			if(*it==max){
				tmp_max_it = it;
			}
		}
		tmp_quality.erase(tmp_max_it);

		for(auto &i : tmp_quality){
			i.first *= correlation[i.second][max_index];
		}

		select_bits_count--;
	}
	//use select bits array to judge which bits belong to tag or index
	vector<string> address_tag_binary(cache_accesses_count);   
	vector<string> address_index_binary(cache_accesses_count); 
	LL address_tag_bits = block_address_bits - Indexing_bit_count;
	for(LL i=0;i<cache_accesses_count;i++){
		for(LL j=0;j<block_address_bits;j++){
			char ch = block_address[i][j];
			if(select_bits_array[j]==1){
				address_index_binary[i].push_back(ch);    
			}else{
				address_tag_binary[i].push_back(ch);  
			}
		}
	}
	//turn address index from binary to decimal
	vector<LL> address_index_decimal;
	for(LL i=0;i<cache_accesses_count;i++){
		LL tmp_index_decimal = binary_to_decimal(address_index_binary[i]);
		address_index_decimal.push_back(tmp_index_decimal);
	}
	//use select bits array to judge the indexing bits number
	vector<LL> Indexing_bits; 
	for(LL i=0;i<block_address_bits;i++){
		if(select_bits_array[i]==1){
			Indexing_bits.push_back(Address_bits-1-i);
		}
	}

	//create cache_sets to simulate cache replacement(LRU)
	vector<block> cache_sets[Cache_sets];
	vector<string> hit_miss_result;
	LL total_cache_miss_count = 0;
	for(LL i=0;i<cache_accesses_count;i++){
		LL current_index_set_size = cache_sets[address_index_decimal[i]].size();
		//if set is empty,push block(tag,address index) into the set,record cache miss
		if(current_index_set_size==0){
			block tmp_block = pair<string,int>(address_tag_binary[i],i);
			cache_sets[address_index_decimal[i]].push_back(tmp_block);
			hit_miss_result.push_back("miss");
			total_cache_miss_count++;
		}else if(current_index_set_size<Associativity){
			//if set is not empty but not full
			
			int tag_find = 0;
			//if the tag is in the set,update the block's address index,record cache hit
			for(auto &b : cache_sets[address_index_decimal[i]]){
				if(b.first==address_tag_binary[i]){
					tag_find = 1;
					b.second = i;
					hit_miss_result.push_back("hit");
					break;
				}
			}
			//if the tag is not in the set,push block(tag,address index) into the set,record cache miss
			if(tag_find == 0){
				block tmp_block = pair<string,int>(address_tag_binary[i],i);
				cache_sets[address_index_decimal[i]].push_back(tmp_block);
				hit_miss_result.push_back("miss");
				total_cache_miss_count++;
			}
		}else if(current_index_set_size==Associativity){
			//if set is full
			int tag_find = 0;
			//if the tag is in the set,update the block's address index,record cache hit
			for(auto &b : cache_sets[address_index_decimal[i]]){
				if(b.first==address_tag_binary[i]){
					tag_find = 1;
					b.second = i;
					hit_miss_result.push_back("hit");
					break;
				}
			}
			//if the tag is not in the set,try to find the blocks in the set whose address is smallest
			//the block which has smallest address should be updated its tag and address index,record cache miss
			if(tag_find == 0){
				
				LL min_index = INF;
				block tmp_block = pair<string,LL>(address_tag_binary[i],i);
				
				for(auto &b : cache_sets[address_index_decimal[i]]){
					min_index = Min(min_index, b.second);
				}

				for(auto &b : cache_sets[address_index_decimal[i]]){
					if(b.second == min_index){
						b = tmp_block;
					}
				}

				hit_miss_result.push_back("miss");
				total_cache_miss_count++;
			}


		}
	}	

	//output
	fout << Address_bits_string << "\n";
	fout << Cache_sets_string << "\n";
	fout << Associativity_string << "\n";
	fout << Block_size_string << "\n";
	fout << "\n";
	fout << "Indexing bit count: " << Indexing_bit_count << "\n";
	fout << "Indexing bits: ";
	if(Indexing_bit_count==0){
		fout << "\n";
	}else{
		for(LL i=0;i<Indexing_bit_count;i++){
			fout << Indexing_bits[i] << ( (i==Indexing_bit_count-1) ? "\n" : " ");
		}
	}
	fout << "Offset bit count: " << Offset_bit_count << "\n";
	fout << "\n";
	fout << reference_start << "\n";
	for(LL i=0;i<cache_accesses_count;i++){
		fout << byte_address[i] << " " << hit_miss_result[i] << "\n";
	}
	fout << reference_end << "\n" ;
	fout << "\n";
	fout << "Total cache miss count: " << total_cache_miss_count << "\n";
	
	fout.close(); //close index.rpt

	return 0;

}