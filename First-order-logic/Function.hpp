#pragma once
#include <string>
#include <map>
#include <sstream> 
#include <vector>
#include <numeric>
using std::vector;
using std::string;
using std::map;
using std::hash;
class Function {
public:
	typedef std::pair<string, string> Equation;
	class Variable {
	public:
		//exp <-> exp
		// bound <-> free , free <-> bound
		//support expression only for now
		enum Type
		{
			free,
			expression,
			bound
		};
		Type type;
		// a x+b, x is free variable, a\b is a digital
		string x;
		float a = 0, b = 0;
		//str origin 
		string str;
		
		Equation solve(const Variable& v) const {
			Equation result;
			if (this->isVariable()&&!v.isVariable())
			{
				return solve(v.b);
			}
			else if (!this->isVariable() && v.isVariable())
			{
				return v.solve(b);
			}
			return Equation();
		}
		Equation solve(float n) const {
			if (a == 0)
				return Equation();
			Equation t;
			t.first = x;
			t.second = std::to_string((n - b) / a);
			return t;
		}
		//完全一样
		bool operator==(const Variable&exp) const {
			return type==exp.type&&a == exp.a&&b == exp.b&&x == exp.x;
		}
		//可用变元匹配
		bool match(const Variable&exp) const{
			// bound match free, expression match expression
			if (isVariable() && !exp.isVariable())
				return true;
			if (!isVariable() && exp.isVariable())
				return true;
			if (type == bound&&exp.type == bound)
				return str==exp.str;
			if (type == expression&&exp.type == expression)
			{
				if (a == 0 && exp.a == 0)
					return b == exp.b;
			}
			return false;
		}

		bool isVariable() const{
			if (type == bound) {
				return false;
			}
			if (type == free)
				return true;
			return a!=0;
		}
		Variable(const string& str) {
			this->str = str;
			if (isupper(str[0]))
			{
				type = bound;
				x = str;
			}
			else {
				for (size_t i = 1; i < str.size(); i++)
				{
					if (str[i] == '+' || str[i] == '-' || str[i] == '*' || str[i] == '/')
					{

						auto left = (string(str.begin(), str.begin() + i));
						auto right = (string(str.begin() + i + 1, str.end()));
						x = left;
						auto v = std::atof(right.c_str());
						switch (str[i])
						{
						case '+':
							a = 1; b = v;
							break;
						case '-':
							a = 1; b = -v;
							break;
						case '*':
							a = v; b = 0;
							break;
						case '/':
							a = 1.0 / v; b = 0;
							break;
						default:
							break;
						}
						type = expression;
						return;
					}
				}
				if (isalpha(str[0]))
				{
					x = str;
					a = 1; b = 0;
					type = expression;
				}
				else
				{
					a = 0;
					b = std::atof(str.c_str());
					type = expression;
				}
			}
		}
	};
	typedef vector<Variable> argumentList;
	bool negated;
	string name;
	argumentList args;

	Function negate() const{
		bool s = !negated;
		string n = name;
		argumentList a = args;
		return Function(s, n, a);
	}

	Function(const string &s) {
		const char *t = s.c_str();

		negated = t[0] == '~';

		unsigned int index = 0;
		while (t[index++] != '(') {};

		if (negated)
			name = string(t, 1, index - 2);
		else
			name = string(t, 0, index - 1);

		unsigned int length = (unsigned int)s.length();

		for (unsigned int i = index; i < length; i++) {
			if (t[i] == ',' || t[i] == ')') {
				string temp(t, index, i - index);
				index = i + 1;
				args.emplace_back(Variable(temp));
			}
		}
	}

	Function(bool &s, string &n, argumentList &a) {
		negated = s;
		name = n;
		args = a;
	}

	size_t hashcode() const {
		size_t signature1 = 17;
		signature1 = signature1 * 31 + hash<bool>()(this->negated);
		signature1 = signature1 * 31 + hash<string>()(this->name);
		for (int i = 0; i < this->args.size(); i++) {
			if (this->args[i].type == Variable::bound)
				signature1 = signature1 * 31 + hash<string>()(this->args[i].str);
			else if (this->args[i].type == Variable::free)
				signature1 = signature1 * 31 + hash<string>()("$");
			else
				signature1 = signature1 * 31 + hash<string>()(args[i].str);
		}
		return signature1;
	}

	bool operator==(const Function &p) const {
		if (negated == p.negated&&name == p.name&&args.size()==p.args.size())
		{
			for (size_t i = 0; i < args.size(); ++i)
			{
				if (!(args[i] == p.args[i]))
					return false;
			}
			return true;
		}
		else
			return false;
	}

	bool match(const Function&p) const {
		if (negated == p.negated&&name == p.name&&args.size() == p.args.size())
		{
			for (size_t i = 0; i < args.size(); ++i)
			{
				if (!(args[i].match(p.args[i])))
					return false;
			}
			return true;
		}
		else
			return false;
	}
	void substitute(const Function::Equation& eq)
	{
		for (auto& arg : args) {
			if (arg.x == eq.first)
			{
				if (arg.type == Variable::free) {
					arg=Variable(eq.second);
				}
				else if (arg.type == Variable::expression) {
					arg.x = eq.first;
					auto x=atof(eq.second.c_str());
					arg.b = arg.a*x+arg.b;
					arg.a = 0;
					arg.str = eq.first + "=" + std::to_string(arg.b);
				}
			}
		}
	}
};