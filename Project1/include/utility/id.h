#ifndef ID_H_INCLUDED
#define ID_H_INCLUDED

/** @file */

#include <cstdio>
#include <climits>
#include <string>
#include <queue>

using namespace std;

/** A class for managing unique IDs */
class cIDManager {
	private:
		queue<unsigned int> freeID; /**< Queue of returned IDs */

		unsigned int baseID; /**< To prevent the IDs from dropping below when being returned */
		unsigned int currentID; /**< Next ID to be given out */

		bool consumeQueue; /**< This signals that IDs have reached their max and freeIDs should be used */

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

		/** Get a low ID
		 *
		 *	When generating process PID's we use the regular getID
		 *	so that process IDs continue to grow. This choice was mainly
		 *	to prevent confusion when process 1 terminated and the next
		 *	one to start had pid = 1. For functions which use vectors and need
		 *	an ID system, it is more efficient to maintain a smaller window
		 *	to keep the array small. This method provides this by preferring
		 *	to return IDs from the freeID queue. Therefore, if there is an ID
		 *	availabe in freeID then the total range of IDs will not grow after
		 *	this function call.
		 */
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

		/** See what the next low ID would be
		 *
		 *	There is not longer any purpose for this functino but I
		 *	left it here for the potential functionality. The intention
		 *	was to improve performance in the process logger to determine
		 *	if the next ID would be right after the previous low ID. That way,
		 *	if we had variable length records we wouldn't have to search from
		 *	the beginning.
		 */
		unsigned int nextLowID();

		/** How many IDs have been given out
		 *
		 *	Returns the number of IDs which have been reserved
		 */
		unsigned int reservedIDs();
};

#endif // ID_H_INCLUDED
