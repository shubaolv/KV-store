#ifndef CACHE_H_
#define CACHE_H_

#include <iostream>
#include <map>
#include <algorithm>
#include <list>
#include <cstring>
#include <unordered_map>
using namespace std;
struct Node
{
	string key;
	string value;
};
class LRUCache
{
	private:
		int maxSize ;
		list<Node> cacheList;
		unordered_map<string, list<Node>::iterator > mp;
		///map<string, list<Node>::iterator > mp;
	
	public:
		LRUCache(int capacity) 
		{
			maxSize = capacity;
		}

		string Get(string key) 
		{
			unordered_map<string, list<Node>::iterator >::iterator it = mp.find(key);
			///map<string, list<Node>::iterator >::iterator it = mp.find(key);
			if(it==mp.end()) 
			{
				return "error:None!";
			}
			else 
			{
				list<Node>::iterator listIt = mp[key];
				Node newNode;
				newNode.key = key;
				newNode.value = listIt->value;
				cacheList.erase(listIt);      
				cacheList.push_front(newNode);
				mp[key] = cacheList.begin();
			}
			return cacheList.begin()->value;
		}

		void Set(string key, string value) 
		{
			unordered_map<string, list<Node>::iterator >::iterator it = mp.find(key);
			///map<string, list<Node>::iterator >::iterator it = mp.find(key);
			if(it==mp.end())
			{
				if(cacheList.size()==maxSize)
				{
					mp.erase(cacheList.back().key);    
					cacheList.pop_back();
				}
				Node newNode;
				newNode.key = key;
				newNode.value = value;
				cacheList.push_front(newNode);
				mp[key] = cacheList.begin();
			}
			else 
			{
				list<Node>::iterator listIt = mp[key];
				cacheList.erase(listIt);          
				Node newNode;
				newNode.key = key;
				newNode.value = value;
				cacheList.push_front(newNode);
				mp[key] = cacheList.begin();
			}
		}
};
#endif

