#ifndef ID_H_INCLUDED
#define ID_H_INCLUDED

#include <climits>
#include <string>
#include <queue>

using namespace std;

class cIDManager {
	private:
		queue<unsigned int> freeID;

		unsigned int currentID;

		bool consumeQueue;

	public:
		cIDManager(unsigned int startID = 0);
		~cIDManager();

		unsigned int getID();
		void returnID(unsigned int id);
};

#endif // ID_H_INCLUDED
