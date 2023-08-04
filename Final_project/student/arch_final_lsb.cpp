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
	fin.open(argv[1], ios::in);   //read cache.org

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
	//close cache.org
	fin.close(); 

	string reference_start;
	string reference_end;
	vector<string> byte_address;
	LL cache_accesses_count = 0;
	//read reference.lst
	fin.open(argv[2], ios::in); 
	//get byte address data
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
	//close reference.lst
	fin.close();

	//calculate offset bit and index bit
	LL Offset_bit_count = (LL)log2(Block_size);
	LL Indexing_bit_count = (LL)log2(Cache_sets);
	
	//calculate block address(binary) & block_address_bits
	vector<string> block_address;
	LL block_address_bits = Address_bits - Offset_bit_count;
	for(int i=0;i<cache_accesses_count;i++){
		string tmp_block_address = byte_address[i].substr(0,block_address_bits);
		block_address.push_back(tmp_block_address);
	}

	//calculate address tag(binary) and address index(binary and decimal)
	vector<string> address_tag_binary;
	vector<string> address_index_binary;
	vector<LL> address_index_decimal;
	LL address_tag_bits = block_address_bits - Indexing_bit_count;
	for(LL i=0;i<cache_accesses_count;i++){
		string tmp_tag = block_address[i].substr(0,address_tag_bits);
		address_tag_binary.push_back(tmp_tag);
		string tmp_index = block_address[i].substr(address_tag_bits,Indexing_bit_count);
		address_index_binary.push_back(tmp_index);
		LL tmp_index_decimal = binary_to_decimal(tmp_index);
		address_index_decimal.push_back(tmp_index_decimal);
	}

	//calculate Indexing_bits(binary)
	vector<LL> Indexing_bits;
	for(LL i=0;i<Indexing_bit_count;i++){
		Indexing_bits.push_back(i+Offset_bit_count);
	}
	sort(Indexing_bits.rbegin(),Indexing_bits.rend());

	//create cache_sets
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
