#pragma once
#include "Sentence.hpp"
#include <algorithm>
#include <stack>
#include <iostream>
#include <fstream>
using namespace std;
using std::stack;
class CNFGenerator {
	// according to https://github.com/vritvij/First-Order-Logic-Inference-Engine
private:
	struct node {
		node *parent, *left, *right;
		string data;

		node() : parent(nullptr), left(nullptr), right(nullptr), data("") {}
	};

	static bool isOperator(string &s) {
		return (s == "(" || s == ")" || s == "~" ||
			s == "&" || s == "|" || s == "=>");
	}

	static int operatorPrecedence(string &op) {
		/*if (op == "(" || op == ")")
		return 5;*/
		if (op == "~")
			return 4;
		if (op == "&")
			return 3;
		if (op == "|")
			return 2;
		if (op == "=>")
			return 1;
		return 0;
	}

	static node *deepCopy(node *root) {
		if (root == nullptr)
			return nullptr;

		node *temp = new node;
		temp->data = root->data;
		temp->left = deepCopy(root->left);
		if (temp->left != nullptr)
			temp->left->parent = temp;
		temp->right = deepCopy(root->right);
		if (temp->right != nullptr)
			temp->right->parent = temp;

		return temp;
	}

	string createExpressionString(node *root) {
		if (root == nullptr)
			return "";

		if (root->data == "~")
			return (root->data + " " + createExpressionString(root->left));
		else
			return (createExpressionString(root->left) + " " + root->data + " " +
				createExpressionString(root->right));
	}

	static node *createExpressionTree(vector<string> expression) {
		stack<string> op;
		stack<node *> result;

		for (int i = 0; i < expression.size(); i++) {
			string temp = expression[i];
			if (isOperator(temp)) {
				if (op.empty() || (temp != ")" && op.top() == "(")) {
					op.push(temp);
				}
				else if (temp == "(") {
					op.push(temp);
				}
				else if (temp == ")") {
					while (op.top() != "(") {
						addToStack(op.top(), result);
						op.pop();
					}
					op.pop();
				}
				else if (operatorPrecedence(temp) > operatorPrecedence(op.top())) {
					op.push(temp);
				}
				else if (operatorPrecedence(temp) <= operatorPrecedence(op.top())) {
					while (!op.empty() && operatorPrecedence(temp) <= operatorPrecedence(op.top())) {
						addToStack(op.top(), result);
						op.pop();
					}
					op.push(temp);
				}
			}
			else {
				addToStack(temp, result);
			}
		}

		while (!op.empty()) {
			addToStack(op.top(), result);
			op.pop();
		}

		return result.top();
	}

	vector<string> createExpressionList(node *root) {
		vector<string> result, temp;
		if (root == nullptr)
			return result;

		temp = createExpressionList(root->left);
		if (!temp.empty())
			result.insert(result.end(), temp.begin(), temp.end());

		result.push_back(root->data);

		temp = createExpressionList(root->right);
		if (!temp.empty())
			result.insert(result.end(), temp.begin(), temp.end());

		return result;
	}

	static void deleteExpressionTree(node *root) {
		if (root == nullptr)
			return;

		deleteExpressionTree(root->left);
		deleteExpressionTree(root->right);
		delete root;
	}

	static vector<string> tokenize(string s) {
		s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
		vector<string> result;

		const char *exp = s.c_str();
		unsigned length = (unsigned)s.length();

		for (unsigned i = 0; i < length; i++) {
			if (exp[i] == '(') {
				result.push_back("(");
			}
			else if (exp[i] == ')') {
				result.push_back(")");
			}
			else if (exp[i] == '~') {
				result.push_back("~");
			}
			else if (exp[i] == '&') {
				result.push_back("&");
			}
			else if (exp[i] == '|') {
				result.push_back("|");
			}
			else if (exp[i] == '=' && exp[i + 1] == '>') {
				result.push_back("=>");
				i++;
			}
			else {
				auto j = i + 1;
				while (exp[j] != ')') { j++; };
				string temp(exp, i, j - i + 1);
				result.push_back(temp);
				i = j;
			}
		}

		return result;
	}

	static void addToStack(string &op, stack<node *> &operandStack) {
		//create node for the operator or operand
		node *root = new node;
		root->data = op;

		if (isOperator(op)) {
			//if node is an operator process the operator based on unary or binary operation
			if (op == "~") {
				//get node for the operand
				node *operand = operandStack.top();
				operand->parent = root;
				//pop the operand stack
				operandStack.pop();
				//assign the operand to the left of the root operator
				root->left = operand;
				root->right = nullptr;
			}
			else {
				//get node for second operand
				node *operand2 = operandStack.top();
				operand2->parent = root;
				operandStack.pop();
				//get node for first operand
				node *operand1 = operandStack.top();
				operand1->parent = root;
				operandStack.pop();
				//assign the operand to the left and right of the root operator
				root->left = operand1;
				root->right = operand2;
			}
		}
		operandStack.push(root);
	}

	static node *negate(node *root) {
		if (root == nullptr)
			return nullptr;

		if (root->left == nullptr && root->right == nullptr) {
			//If leaf node is encountered i.e operand encountered
			if (root->data[0] != '~') {
				//premise is positive
				root->data = "~" + root->data;
			}
			else {
				//premise is negative
				root->data = root->data.substr(1);
			}
		}
		else {
			//operator encountered
			if (root->data == "~") {
				//If a "~" operator node is encountered
				node *t = root;
				if (root->parent == nullptr) {
					//if root doesnt have a parent then make child the root
					root = root->left;
					root->parent = nullptr;
				}
				else {
					if (root->parent->left == root) {
						//if "~" is on the left side of parent
						root->parent->left = root->left;
					}
					else {
						//if "~" is on the right side of parent
						root->parent->right = root->left;
					}
					//make the roots parent, the child's parent
					root->left->parent = root->parent;
					root = root->left;
					//delete the "~" node
				}
				delete t;
				return root;
			}
			else if (root->data == "&") {
				root->data = "|";
			}
			else if (root->data == "|") {
				root->data = "&";
			}
		}

		root->left = negate(root->left);
		root->right = negate(root->right);

		return root;
	}

	static void removeImplications(node *root) {
		if (root == nullptr)
			return;
		removeImplications(root->left);
		removeImplications(root->right);
		if (root->data == "=>") {
			//A => B ------> ~A | B
			negate(root->left);
			root->data = "|";
		}
	}

	static node *resolveNegations(node *root) {
		if (root == nullptr)
			return nullptr;

		while (root->data == "~") {
			root = negate(root); //removes the "~" sign only
			root = negate(root); //performs the actual negation
		}

		root->left = resolveNegations(root->left);
		root->right = resolveNegations(root->right);

		return root;
	}

	static node *distribute(node *parent, node *child) {
		node *grandparent = parent->parent;
		node *leftBranch1, *leftBranch2, *rightBranch1, *rightBranch2;
		bool isParentLeftOfGrandparent = (grandparent != nullptr) ? (grandparent->left == parent) : false;

		if (parent->left == child) {
			//if child is the left child of the parent
			leftBranch1 = child->left;
			leftBranch2 = child->right;
			rightBranch1 = parent->right;
			rightBranch2 = deepCopy(parent->right);
		}
		else {
			//If child is the right child of the parent
			leftBranch1 = parent->left;
			leftBranch2 = deepCopy(parent->left);
			rightBranch1 = child->left;
			rightBranch2 = child->right;
		}

		//delete parent
		delete parent;

		//create and initialize new left and right parent
		node *leftNode = new node;
		leftNode->data = "|";
		leftNode->left = leftBranch1;
		leftNode->right = rightBranch1;
		leftNode->parent = child;

		node *rightNode = new node;
		rightNode->data = "|";
		rightNode->left = leftBranch2;
		rightNode->right = rightBranch2;
		rightNode->parent = child;

		//Make left node parent of *branch1
		leftBranch1->parent = leftNode;
		rightBranch1->parent = leftNode;

		//Make right node parent of *branch2
		leftBranch2->parent = rightNode;
		rightBranch2->parent = rightNode;

		//Make child point to new left and right nodes
		child->left = leftNode;
		child->right = rightNode;

		//Make child grandparents child
		child->parent = grandparent;
		if (grandparent != nullptr) {
			//If child has a grandparent
			if (isParentLeftOfGrandparent) {
				grandparent->left = child;
			}
			else {
				grandparent->right = child;
			}
		}
		return child;
	}

	static node *distributeOrOverAnd(node *root) {
		if (root == nullptr)
			return nullptr;

		//If root is an operand, return without modification
		if (!isOperator(root->data))
			return root;

		//If root is "|"...
		if (root->data == "|") {
			bool distributed = false;
			//...and its left child is "&"
			if (root->left->data == "&") {
				//distribute | over & on the left child
				root = distribute(root, root->left);
				distributed = true;
			}

			//...and its right child is "&"
			if (root->right->data == "&") {
				//distribute | over & on the right child
				root = distribute(root, root->right);
				distributed = true;
			}

			//If distribution has taken place
			if (distributed) {
				if (root->parent == nullptr) {
					//If root is the absolute root of the tree, test children for possible distributions
					return distributeOrOverAnd(root);
				}
				else {
					//Bubble up the control flow
					return root;
				}
			}
		}

		//Store original left and right children
		node *left = root->left;
		node *right = root->right;
		/*
		*  If after calling distributeOrOverAnd on the left or right subtree,
		*  the left or right subtree changes then we need to run again for possible distributions in the children
		*/
		root->left = distributeOrOverAnd(root->left);
		if (left != root->left) {
			//If left child changes then return the result of rerunning over root
			return distributeOrOverAnd(root);
		}
		root->right = distributeOrOverAnd(root->right);
		if (right != root->right) {
			//If right child changes then return the result of rerunning over root
			return distributeOrOverAnd(root);
		}

		return root;
	}

	static vector<node *> splitSentenceOverAnd(node *root) {
		vector<node *> result, temp;
		if (root == nullptr)
			return result;

		if (root->data != "&") {
			result.push_back(root);
			return result;
		}

		temp = splitSentenceOverAnd(root->left);
		if (!temp.empty()) {
			result.insert(result.end(), temp.begin(), temp.end());
		}
		temp = splitSentenceOverAnd(root->right);
		if (!temp.empty()) {
			result.insert(result.end(), temp.begin(), temp.end());
		}

		return result;
	}

	static Sentence createCNFSentence(node *root) {
		Sentence result, temp;
		if (root->left == nullptr && root->right == nullptr) {
			Function t = Function(root->data);
			result.push_back(t);
			return result;
		}

		temp = createCNFSentence(root->left);
		if (!temp.empty())
			result.insert(result.end(), temp.begin(), temp.end());

		temp = createCNFSentence(root->right);
		if (!temp.empty())
			result.insert(result.end(), temp.begin(), temp.end());

		return result;
	}

public:
	static vector<Sentence> negateCNFSentence(Sentence s) {
		vector<Sentence> result;
		for (int i = 0; i < s.size(); i++) {
			result.emplace_back(Sentence{ s[i].negate() });
		}
		return result;
	}

	static CNF convertToCNFSentences(const string &s) {
		//Create expression tree from tokenized string
		node *expressionRoot = createExpressionTree(tokenize(s));
		//cout << "Tokenized Expression \t\t:\t " << createExpressionString(expressionRoot) << endl;

		//Remove implications
		removeImplications(expressionRoot);
		//cout << "Expression without Implications :\t " << createExpressionString(expressionRoot) << endl;

		//Resolve negations
		expressionRoot = resolveNegations(expressionRoot);
		//cout << "Expression without Negations \t:\t " << createExpressionString(expressionRoot) << endl;

		//Distribute | over &
		expressionRoot = distributeOrOverAnd(expressionRoot);
		//cout << "After Distribution of | over & \t:\t " << createExpressionString(expressionRoot) << endl;

		//Split sentences over &
		vector<node *> sentences = splitSentenceOverAnd(expressionRoot);
		//cout << "New Sentences in CNF" << endl;

		Sentence temp;
		CNF result;
		for (int i = 0; i < sentences.size(); i++) {
			//Convert to CNF Sentence
			temp = createCNFSentence(sentences[i]);
			//Factorize the CNF Sentence
			//Removes redundant predicates
			//A(x) | A(x) becomes A(x)
			temp.factorize();
			//Add to result
			if(!temp.empty())
				result.push_back(temp);
		}

		//Delete the expression tree
		deleteExpressionTree(expressionRoot);

		return result;
	}
};

class KnowledgeBase {
	
	CNF facts;
	CNFGenerator gen;

	typedef size_t hash_sentence;
	typedef map<hash_sentence, std::pair<size_t, size_t>> step_map;

	map<hash_sentence, size_t> sentence_id;
	void parse_fun(vector<size_t>& ans, size_t p, step_map& steps) {
		if (p < facts.size())
		{
			ans.push_back(p);
		}
		else
		if (steps.count(p) > 0)
		{
			auto t = steps[p];
			parse_fun(ans, t.first, steps);
			parse_fun(ans, t.second, steps);
		}
	}
	string print_fun(size_t p, step_map& steps) {
		if (p < facts.size())
			return to_string(p);
		if (steps.count(p) > 0)
		{
			auto t = steps[p];
			return "(" + print_fun(t.first, steps) + " ," + print_fun(t.second, steps) + ")";
		}
		return "Q";
	}
public:
	//消解法 无法在有限步内给出真命题的答案。但如果是一个假命题，那么可以在有限步内发现矛盾
	long maxQuery = 200;
	step_map query(const string& query) {
		auto tmp_id = facts.size();
		long finishTime = maxQuery;
		auto q = gen.convertToCNFSentences(query);
		auto negquery = q.negateCNF();
		CNF db = facts;
		auto temp_sentence_id = sentence_id;
		for(auto&t:negquery)
		{
			temp_sentence_id[t.hashcode()] = tmp_id++;
			db.push_back(t);
		}
		//反证法
		return (resolution_reasoning(db, temp_sentence_id, maxQuery));
		
	}
	// if steps contain -1 ,it's a false proposition.
	static step_map resolution_reasoning(CNF& db, map<hash_sentence, size_t>& temp_sentence_id,int finishTime) {
		//std::ostringstream cout;
		step_map steps;
		auto tmp_id = db.size();
		while (!db.empty() && finishTime-->0)
		{
			//不断生产子表达式，陷入局部子问题。
			/*Sentence sen = *db.rbegin();
			db.pop_back();*/
			Sentence sen;
			if (rand() % 2) {
				sen = *db.rbegin();
				db.pop_back();
			}
			else {
				sen = *db.begin();
				db.erase(db.begin());
			}

			size_t n = db.size();
			for (size_t i = 0; i<n; ++i)
			{
				Sentence c;
				auto& item = db[i];
				if (Sentence::merge(sen, item, c)) {
					auto id_a = temp_sentence_id[sen.hashcode()];
					auto id_b = temp_sentence_id[item.hashcode()];
					cout << "merge: (" << id_a << " ," << id_b << " )" << endl;
					cout << "   -"; sen.print_to(cout); cout << endl;
					cout << "   -"; item.print_to(cout); cout << endl;
					/*cout << "into:" << endl;
					c.print_to(cout); cout << endl;*/
					auto m = pair<size_t, size_t>{ id_a,id_b };
					if (c.empty())
					{
						steps[-1] = m;
						return steps;
					}
					else {
						c.factorize();
						if (!c.empty())
						{
							db.push_back(c);
							steps[tmp_id] = m;
							temp_sentence_id[c.hashcode()] = tmp_id++;
							cout << "factorize: " << temp_sentence_id[c.hashcode()] << "   -";
							c.print_to(cout); cout << endl;
						}
					}
				}
			}
		}

		return steps;
	}
	void print(ostream& out, step_map& steps) {
		if (steps.count(-1) == 0)
			return;
		out << print_fun(-1, steps);
	}

	//这个顺序可能是错的
	vector<size_t> parse_step(step_map& steps)
	{
		vector<size_t> ans;
		if (steps.count(-1) == 0)
			return ans;
		auto t = steps[-1];
		parse_fun(ans, t.first, steps);
		parse_fun(ans, t.second, steps);
		return ans;
	}
	pair<size_t,size_t> insert(const string& known)
	{	
		pair<size_t, size_t> bound;
		bound.first = facts.size();
		auto cnf = gen.convertToCNFSentences(known);
		for (auto& t : cnf)
		{
			sentence_id[t.hashcode()] = facts.size();
			facts.push_back(t);
		}
		bound.second = facts.size();
		return bound;
	}
};