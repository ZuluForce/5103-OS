#include "utility/id.h"

cIDManager::cIDManager(unsigned int startID) {
	//fprintf(stderr, "Max unsigned int: %ud\n", UINT_MAX);
	baseID = startID;
	currentID = startID;

	consumeQueue = false;

	//printf("Initialized id manager: consumeQueue = %s\n", consumeQueue ? "true" : "false");

	return;
}

cIDManager::~cIDManager() {

	return;
}

unsigned int cIDManager::getID() {
	//printf("getID Called: freeID.size() = %d	currentID = %u	consumeQueue = %s\n",
	//		freeID.size(), currentID, consumeQueue ? "true" : "false");
	if ( consumeQueue ) {
		if ( freeID.size() > 0 ) {
			unsigned int newID = freeID.front();
			freeID.pop();

			//fprintf(stderr, "Returning id %f from queue\n", newID);
			return newID;
		} else {
			/* This means the currentID is at the max and
			 * the queue is empty. All out of IDs */
			 printf("cIDManager has run out of IDs: CurrentID = %u\n", currentID);
			 throw ((string) "cIDManager has run out of IDs");
		}
	} else {
		if ( currentID >= UINT_MAX ) {
			consumeQueue = true;

			//fprintf(stderr, "Returning id %d: consumeQueue = true\n", currentID);
			return currentID;
		} else {

			//fprintf(stderr, "Returning id %d\n", currentID + 1);
			return currentID++;
		}
	}
}

unsigned int cIDManager::getLowID() {
	if ( !freeID.empty() ) {
		unsigned int newID = freeID.front();
		freeID.pop();
		return newID;
	}

	return getID();
}

void cIDManager::returnID(unsigned int id) {
	if ( consumeQueue ) {
		if ( id == currentID )
			consumeQueue = false;
		else
			freeID.push(id);
	} else {
		if  ( id == currentID - 1)
				--currentID;
		else
			freeID.push(id);
	}
}

unsigned int cIDManager::reservedIDs() {
	if ( currentID == UINT_MAX )
		return currentID - baseID - freeID.size() + 1;

	return currentID - baseID - freeID.size();
}
