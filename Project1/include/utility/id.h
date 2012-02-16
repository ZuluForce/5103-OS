#ifndef ID_H_INCLUDED
#define ID_H_INCLUDED

#include <cstdio>
#include <climits>
#include <string>
#include <queue>

using namespace std;

class cIDManager {
	private:
		queue<unsigned int> freeID;

		unsigned int baseID;
		unsigned int currentID;

		bool consumeQueue;

	public:
		/** Creates a new ID Manager object
		 *
		 *	Default start ID is 0.
		 */
		cIDManager(unsigned int startID = 0);
		~cIDManager();

		/** Reserves a unique ID
		 *
		 *	Unique is in the sense that no one else
		 *	is currently using it but it may have been
		 *	used previously.\nIDs are distributed in increasing
		 *	order until UINT_MAX is reached. After this is reached,
		 *	IDs are given from the queue of returned IDs. If this
		 *	queue is empty then an exception is thrown.
		 */
		unsigned int getID();
		unsigned int getLowID();

		/** Returns an ID to the manager
		 *
		 *	If the ID is not equal to the one last given then
		 *	it is added to a 'free queue'. If it is equal to the
		 *	last one reserved then the ID counter is simply decremented
		 *	If this last case happens, it causes cIDManager::getID to stop
		 *	consuming from the queue and return this newly availabe ID.
		 */
		void returnID(unsigned int id);

		unsigned int reservedIDs();
};

#endif // ID_H_INCLUDED
