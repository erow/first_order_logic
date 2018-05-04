#pragma once
#include "Function.hpp"
// 析取式 连接
#include <strstream>

static std::ostream& operator<<(std::ostream& out, const Function::Variable& s) {
		return out << s.str;
}

class Sentence :public vector<Function> {
public:
	template<class ...Args>
	Sentence(Args... args):vector<Function>(args...){}
	Sentence(std::initializer_list<Function> l):vector<Function>(l){}
	Sentence():vector<Function>() {}
	void print_to( std::ostream& cout) const {
		const Sentence& s = *this;
		if (s.size() == 0)
			return;
		cout << (s[0].negated ? "~" : "") << s[0].name << "(";
		cout << s[0].args[0];
		for (size_t j = 1; j < s[0].args.size(); j++) {
			cout << "," << s[0].args[j];
		}
		cout << ")";
		for (size_t i = 1; i < s.size(); i++) {
			cout << "|" << (s[i].negated ? "~" : "") << s[i].name << "(";
			cout << s[i].args[0];
			for (size_t j = 1; j < s[i].args.size(); j++) {
				cout << "," << s[i].args[j];
			}
			cout << ")";
		}
	}

	void substitute(const Function::Equation& eq)
	{
		for (auto& function : *this) {
			function.substitute(eq);
		}
	}

	size_t hashcode() const{
		size_t signature = 0;
		for (auto&fun:*this) {
				size_t tmp =  fun.hashcode();
				signature^= tmp;
		}
		return signature;
	}

	//删除重复和永真式， PVP <=> P PV~P <=> True
	void factorize() {
		vector<Function> adds;
		for (auto it = begin(); it != end(); ) {			
			auto fun = *it;
			auto nfun = fun.negate();
			
			for (auto it1 = it+1; it1 != end(); ) {
				if (*it1 == fun ) {
					// f(1) | f(1) <=> f(1)
					it1=erase(it1);
				}
				else if (*it1 == nfun)
				{
					// ~f(1) | f(1) <=> True
					it1 = erase(it1);
					it = erase(it);
					goto endfun;
				}
				else if (it1->match(fun)) {
					// f(x) | f(1) <=> f(x)
					auto var = fun;
					for (size_t i = 0; i < var.args.size(); i++)
					{
						if (!var.args[i].isVariable())
							var.args[i] = it1->args[i];
					}
					it1 = erase(it1);
					it = erase(it);
					adds.push_back(var);
					goto endfun;
				}
				// ~f(x) | f(1) 
				// f(x) | ~f(1) 
				else
					++it1;
			}
			++it;
			endfun:
				{}
		}
		insert(end(), adds.begin(), adds.end());
	}
	static bool merge(Sentence&a, Sentence&b, Sentence& c) {
		//(aVP)^(bV~P) <=> aVb
		for (auto it = a.begin(); it != a.end(); ++it) {
			auto nfun = it->negate();
			for (auto it1 = b.begin(); it1 != b.end(); ++it1) {
				if (nfun.match(*it1)) {
					vector<Function::Equation> eqs;
					for (size_t i = 0; i < nfun.args.size();++i) {
						auto eq = nfun.args[i].solve(it1->args[i]);
						if (eq.first != "")
							eqs.push_back(eq);
					}
					c.insert(c.end(), a.begin(), it);
					c.insert(c.end(), it+1,a.end());
					c.insert(c.end(), b.begin(), it1);
					c.insert(c.end(), it1 + 1, b.end());

					for (auto&eq : eqs)
						c.substitute(eq);
					return true;
				}
			}
		}
		return false;
	}
	/*CNF negate() const {
		CNF result;
		for (auto& function : *this) {
			result.push_back(function.negate());
		}
		return result;
	}*/
};
#include <iostream>
using namespace std;
//合取范式，合取式连接  A1 & A2...
class CNF :public vector<Sentence> {
public:
	template<class ...Args>
	CNF(Args... args) :vector<Sentence>(args...) {}
	CNF(std::initializer_list<Sentence> l) :vector<Sentence>(l) {}
	CNF():vector<Sentence>(){}
	CNF negateCNF() {
		CNF result;
		for (auto &sen : *this) {
			if (result.empty()) {
				for (auto& pre : sen)
					result.push_back(Sentence{ pre.negate() });
			}
			else
			{
				CNF tmp;
				for (auto& pre : sen) {
					auto t = result;
					for (auto &r : t) {
						r.push_back(pre.negate());
					}
					tmp.insert(tmp.end(), t.begin(), t.end());
				}
				result = tmp;
			}
		}
		for (auto it = result.begin(); it != result.end(); )
		{
			it->factorize();
			if (it->empty())
				it = result.erase(it);
			else
				++it;
		}
		return result;
	}
	//long maxQuery = 20;
	//typedef size_t hash_sentence;
	//map<hash_sentence, std::pair<size_t, size_t>> steps;
	//size_t cur_id = 0;
	//map<hash_sentence, size_t> sentence_id;
	//bool query() {
	//	double finishTime = maxQuery;
	//	while (!empty()&& maxQuery-->0)
	//	{
	//		Sentence sen = *rbegin();
	//		pop_back();
	//		
	//		size_t n = size();
	//		for (size_t i=0;i<n; ++i )
	//		{
	//			Sentence c;
	//			auto& item = operator[](i);
	//			if (Sentence::merge(sen, item, c)) {
	//				cout << "merge: (" << sentence_id[sen.hashcode()]<<" ,"<< sentence_id[item.hashcode()]<<" )"<< endl;
	//				cout<< "   -"; sen.print_to(cout); cout << endl;
	//				cout << "   -"; item.print_to(cout); cout << endl;
	//				/*cout << "into:" << endl;
	//				c.print_to(cout); cout << endl;*/
	//				if (c.empty())
	//					return false;
	//				else {
	//					c.factorize();
	//					if (!c.empty())
	//					{
	//						insert_back(c);
	//						cout << "factorize: " << sentence_id[c.hashcode()] << "   -";
	//						c.print_to(cout); cout << endl;
	//					}
	//				}
	//			}
	//		}
	//	}
	//	return true;
	//}
	//void insert_back(const Sentence& _Val)
	//{	// insert element at end, provide strong guarantee
	//	emplace_back(_Val);
	//	sentence_id[_Val.hashcode()] = cur_id++;
	//}
	//
};