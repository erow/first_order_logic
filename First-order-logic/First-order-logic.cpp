#include"CNFGenerator.hpp"
using namespace std;

int main() {
	int NQ = 0, NS = 0;
	string tempString;
	ifstream InputFile("input.txt");
	KnowledgeBase KB;
	if (InputFile.is_open()) {
		getline(InputFile, tempString);
		NS = stoi(tempString);

		for (int i = 0; i < NS; i++) {
			getline(InputFile, tempString);
			auto t=KB.insert(tempString);
			cout << tempString << " :" << t.first << " ," << t.second << endl;
		}
		InputFile.close();
	}
	else {
		cout << "Input file failed to load" << endl;
	}
	string query = "f(6)";

	for (;;) {
		std::getline(cin, query);
		if (query == "q")
			break;

		auto ans = KB.query(query);
		cout << string(ans.count(-1) ? "TRUE" : "FALSE") << endl;
		KB.print(cout, ans);
		
		auto vec=KB.parse_step(ans);
		cout << endl ;
		if (!vec.empty())
			for (auto i : vec) {
				if (i == 0)
					cout << "+3";
				if (i == 1)
					cout << "+4";
				/*if (i == 4)
					cout << "f(1)";*/
			}
		cout << endl;
	}

	return 0;
}