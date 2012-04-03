#include "menu.hpp"
#include "gecko.h"
#include <stdlib.h>

// Returns a list of games which starts with the specified (partial) gameId
// We can enhance the code in this file later on to support more search features
// Using a search class as argument or something like that
deque<dir_discHdr> CMenu::_searchGamesByID(string gameId)
{
	deque<dir_discHdr> retval;
	for (GameListIter itr = m_gameList.begin(); itr != m_gameList.end(); itr++)
		if (gameId == (*itr).id) retval.push_back(*itr);

	return retval;
}
/*
deque<dir_discHdr> CMenu::_searchGamesByTitle(wchar_t letter)
{
	deque<dir_discHdr> retval;
	for (GameListIter itr = m_gameList.begin(); itr != m_gameList.end(); itr++)
		if ((*itr).title[0] == letter)
			retval.push_back(*itr);

	return retval;
}

deque<dir_discHdr> CMenu::_searchGamesByType(const char type)
{
	deque<dir_discHdr> retval;
	for (GameListIter itr = m_gameList.begin(); itr != m_gameList.end(); itr++)
		if ((*itr).id[0] == type)
			retval.push_back(*itr);

	return retval;
}

deque<dir_discHdr> CMenu::_searchGamesByRegion(const char region)
{
	deque<dir_discHdr> retval;
	for (GameListIter itr = m_gameList.begin(); itr != m_gameList.end(); itr++)
		if ((*itr).id[3] == region)
			retval.push_back(*itr);

	return retval;
} */