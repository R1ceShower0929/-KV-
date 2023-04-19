#pragma once
#include<iostream>
#include<fstream>
#include<mutex>
#include<cstring>
#include<string>
#include<cmath>
#include<cstdlib>
#include<vector>

#define STORE_FILE "./dumpFile"
std::mutex mtx;
std::string delimiter = ":";


template<typename K,typename V>
class Node
{
public:
	Node();
	~Node();
	Node(K k, V v, int);
	K get_key() const;
	V get_value() const;
	void set_value(V v);

	Node<K, V>** forward_;
	int node_level_;
private:
	K k_;
	V v_;
};

template<typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level)
{
	this->k_ = k;
	this->v_ = v;
	this->node_level_ = level;
	this->forward_ = new Node<K, V>* [level + 1];
	memset(forward_, 0, sizeof(Node<K, V>*) * (level + 1));
}

template<typename K,typename V>
Node<K, V>::~Node()
{
	delete[] forward_;
}

template<typename K,typename V>
K Node<K, V>::get_key() const
{
	return this->k_;
}

template<typename K, typename V>
V Node<K, V>::get_value() const
{
	return this->v_;
}

template<typename K, typename V>
void Node<K, V>::set_value(V v)
{
	this->v_ = v;
}

template<typename K, typename V>
class SkipList
{
public:
	SkipList(int);
	~SkipList();
	int insert_element(K, V);
	void delete_element(K);
	bool search_element(K);
	void display_list();
	int list_size();
	Node<K, V>* create_node(K, V, int);

	void dump_file();
	void load_file();

	int get_random_level();

private:
	void get_key_value_from_str(const std::string& str, std::string* k,std::string* v);
	bool is_valid_str(const std::string& str);
private:
	int max_level_;
	int skiplist_level;
	Node<K, V>* header_;
	std::ifstream file_reader_;
	std::ofstream file_writer_;
	int element_num_;
};

template<typename K,typename V>
SkipList<K,V>::SkipList(int level)
{
	this->max_level_ = level;
	this->skiplist_level = 0;
	this->element_num_ = 0;
	K k = {};
	V v = {};
	this->header_ = new Node<K, V>(k, v, level);
}

template<typename K, typename V>
SkipList<K, V>::~SkipList()
{
	if (file_reader_.is_open()) {
		file_reader_.close();
	}
	if (file_writer_.is_open()) {
		file_writer_.close();
	}
	delete header_;
}

template<typename K, typename V>
int SkipList<K, V>::list_size()
{
	return element_num_;
}

template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(const K k,const V v, int level)
{
	Node<K, V>* node = new Node<K, V>(k, v, level);
	return node;
}

template<typename K, typename V>
bool SkipList<K, V>::search_element(K k)
{
	std::cout << "search_element----------------" << std::endl;
	Node<K, V>* cur = header_;

	for (int i = skiplist_level; i >= 0; --i) {
		while (cur->forward_[i] && cur->forward_[i]->get_key() < k) {
			cur = cur->forward_[i];
		}
	}
	cur = cur->forward_[0];
	if (cur && cur->get_key() == k) {
		std::cout << "found key:" << k << ",value:" << cur->get_value() << std::endl;
		return true;
	}
	std::cout << "not found key:" << k << std::endl;
	return false;
}

template<typename K, typename V>
int SkipList<K, V>::insert_element(const K k,const V v)
{
	mtx.lock();
	Node<K, V>* cur = header_;
	std::vector<Node<K, V>*> update(max_level_ + 1);
	//memset(update, 0, sizeof(Node<K, V>*) * (max_level_ + 1));
	for (int i = skiplist_level; i >= 0; --i) {
		while (cur->forward_[i] && cur->forward_[i]->get_key() < k) {
			cur = cur->forward_[i];
		}
		update[i] = cur;
	}
	cur = cur->forward_[0];
	if (cur && cur->get_key() == k) {
		std::cout << "key: " << k << " exists" << std::endl;
		mtx.unlock();
		return 1;
	}

	if (!cur || cur->get_key() != k) {
		int random_level = get_random_level();
		if (random_level > skiplist_level) {
			for (int i = skiplist_level + 1; i <= random_level; ++i) {
				update[i] = header_;
			}
			skiplist_level = random_level;
		}
		Node<K, V>* node = create_node(k, v, random_level);
		for (int i = 0; i <= random_level; ++i) {
			node->forward_[i] = update[i]->forward_[i];
			update[i]->forward_[i] = node;
		}
		std::cout << "successfully inserted key:" << k << " " << ",value:" << v << std::endl;
		++element_num_;
	}
	mtx.unlock();
	return 0;
}

template<typename K,typename V>
int SkipList<K, V>::get_random_level()
{
	int k = 0;
	while (rand() % 2) {
		++k;
	}
	return k > max_level_ ? max_level_ : k;
}

template<typename K, typename V>
void SkipList<K, V>::delete_element(K k)
{
	mtx.lock();
	Node<K, V>* cur = header_;
	std::vector<Node<K, V>*> update(max_level_ + 1);
	//memset(update, 0, sizeof(Node<K, V>*) * (max_level_ + 1));
	for (int i = skiplist_level; i >= 0; --i) {
		while (cur->forward_[i] && cur->forward_[i]->get_key() < k) {
			cur = cur->forward_[i];
		}
		update[i] = cur;
	}
	cur = cur->forward_[0];
	if (cur && cur->get_key() == k) {
		for (int i = 0; i <= skiplist_level; ++i) {
			if (update[i]->forward_[i] != cur) {
				break;
			}
			update[i]->forward_[i] = cur->forward_[i];
		}

		while (skiplist_level > 0 && header_->forward_[skiplist_level] == NULL) {
			--skiplist_level;
		}
		std::cout << "successfully delete key:" << k << std::endl;
		--element_num_;
	}
	else {
		std::cout << "can't find key " << k << " ,delete failed" << std::endl;
	}
	mtx.unlock();
}

template<typename K,typename V>
void SkipList<K, V>::display_list()
{
	std::cout << "\n******SkipList******" << std::endl;
	for (int i = skiplist_level; i >= 0; --i) {
		std::cout << "level " << i << ":";
		Node<K, V>* node = header_->forward_[i];
		while (node) {
			std::cout << "key:" << node->get_key() << ", value:" << node->get_value() << ";";
			node = node->forward_[i];
		}
		std::cout << std::endl;
	}
}

template<typename K, typename V>
void SkipList<K, V>::dump_file()
{
	std::cout << "\ndump_file--------" << std::endl;
	file_writer_.open(STORE_FILE);
	Node<K, V>* node = header_->forward_[0];

	while (node) {
		file_writer_ << node->get_key() << ":" << node->get_value() << "\n";
		std::cout<<"д�룺"<< node->get_key() << ":" << node->get_value() << ";\n";
		node = node->forward_[0];
	}

	file_writer_.flush();
	file_writer_.close();
}

template<typename K, typename V>
void SkipList<K, V>::load_file()
{
	std::cout << "\nload_file--------" << std::endl;
	file_reader_.open(STORE_FILE);
	std::string s;
	std::string* key = new std::string();
	std::string* value = new std::string();
	while (getline(file_reader_, s)) {
		get_key_value_from_str(s, key, value);
		if (key->empty() || value->empty()) {
			continue;
		}
		insert_element(*key, *value);
		std::cout << "key" << *key << ",value:" << *value << std::endl;
	}
	file_reader_.close();
}

template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_str(const std::string& str, std::string* key, std::string* value)
{
	if (!is_valid_str(str)) {
		return;
	}
	auto pos = str.find(delimiter);
	*key = str.substr(0, str.find(pos));
	*value = str.substr(pos + 1, str.length());
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid_str(const std::string& str)
{
	if (str.empty()) {
		return false;
	}
	if (str.find(delimiter) == std::string::npos) {
		return false;
	}
	return true;
}