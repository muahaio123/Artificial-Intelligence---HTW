// YOUR NAME: Thanh Long Le
//
// CS 4318, spring 2021
// Agent Challenge 6: Hunt the wumpus
//
// Rename this file and the function below.  For example, if your agent name
// is Jones, rename this htwAgentSmith.cpp file to htwAgentJones.cpp and the
// htwAgentSmith function below to htwAgentJones.  Complete your agent
// function and turn it in on Blackboard.  Random-number generation is not
// allowed; your agent must be entirely deterministic.  Feel free to create
// other agents--each in a separate .cpp file--for comparison purposes, but
// turn in only one.  Test your agent(s) with
//
//    nice bash htwBuild.bash
//
// and then
//
//    nice ./htwRunSim
//
// Each submitted agent will explore each of many random 4x4 wumpus worlds
// to determine the standings, which will be posted soon after the agents
// are due.

#include "htw.h"
#include <stack>
#include <vector>
#include <iomanip>
#include <algorithm>
using namespace std;

namespace
{
	// If you need to define any new types or functions, put them here in
	// this unnamed namespace.  But no variables allowed!
	
	struct KnowledgeBase
	{
		// Declare and initialize member variables here.
		// No dynamic memory allocation is allowed, but arrays are okay.
		int x = 0, y = 0, pastX = -1, pastY = -1;
		
		int map[4][4] = {0};	// keep a copy of the map that has the percentage of death
		
		int wumpus[4][4] = {0};	// the location of the wumpus
		
		int pit[4][4] = {0};	// the location of the pits
		
		stack<action> path;	// keep the current path as a stack
		
		vector<action> possibleMove;	// keep a list of all possible move: moveW, moveS, moveN, moveE
		
		vector<pair<int, int>> notvisited;	// keep a list of neighbor square that have not visited
		
		bool
			haveGold = false,	// if the player have gold or not
			noNeighbor = false,	// if all the neighboring square are already visited
			backing = false;	// trigger when we are backtracking
		
		action back;
	};
	
	/*----------functions to get the neighboring square----------*/
	KnowledgeBase getMoves(KnowledgeBase kb)	// evaluate all the possible move we can take from where we are right now
	{
		kb.possibleMove = {moveW, moveS, moveN, moveE};
		
		auto where_to_delete = find(kb.possibleMove.begin(), kb.possibleMove.end(), doNothing);	// initialize for placeholder
		
		if (kb.x == 0)	// if current location is at the left most of map -> cannot move West anymore
		{
			where_to_delete = find(kb.possibleMove.begin(), kb.possibleMove.end(), moveW);
			kb.possibleMove.erase(where_to_delete);
		}
		
		if (kb.x == 3)	// right-left -> cannot go move East anymore
		{
			where_to_delete = find(kb.possibleMove.begin(), kb.possibleMove.end(), moveE);
			kb.possibleMove.erase(where_to_delete);
		}
		
		if (kb.y == 0)	// upper most -> cannot move South
		{
			where_to_delete = find(kb.possibleMove.begin(), kb.possibleMove.end(), moveS);
			kb.possibleMove.erase(where_to_delete);
		}
		
		if (kb.y == 3)	// upper most -> cannot move North
		{
			where_to_delete = find(kb.possibleMove.begin(), kb.possibleMove.end(), moveN);
			kb.possibleMove.erase(where_to_delete);
		}
		
		return kb;
	}
	
	// chek for all the not visited neighbor (!= -1)
	// and get the (x, y) of all the neighbor
	KnowledgeBase get_notvisited(KnowledgeBase kb)
	{
		// clear the old neighbors as the player have taken an action
		kb.notvisited.clear();
		
		kb = getMoves(kb);
		
		// a function to check all the possible neighbors that have not been visited (!= -3)
		for (uint i = 0; i < kb.possibleMove.size(); ++i)
		{
			if (kb.possibleMove[i] == moveN && kb.map[kb.x][kb.y + 1] != -3)
				kb.notvisited.push_back(make_pair(kb.x, kb.y + 1));
			
			else if (kb.possibleMove[i] == moveS && kb.map[kb.x][kb.y - 1] != -3)
				kb.notvisited.push_back(make_pair(kb.x, kb.y - 1));
			
			else if (kb.possibleMove[i] == moveW && kb.map[kb.x - 1][kb.y] != -3)
				kb.notvisited.push_back(make_pair(kb.x - 1, kb.y));
			
			else if (kb.possibleMove[i] == moveE && kb.map[kb.x + 1][kb.y] != -3)
				kb.notvisited.push_back(make_pair(kb.x + 1, kb.y));
			
			// erase the move directly if the move is going to an already visited square
			// erase so that the square matches the move direction
			else
			{
				kb.possibleMove.erase(kb.possibleMove.begin() + i);
				--i;
			}
		}
		
		return kb;
	}
	/*----------functions to get the neighboring square----------*/
	
	action oppositeMove(action move)	// return the opposite move of any move
	{
		if (move == moveS)
			return moveN;
		
		if (move == moveN)
			return moveS;
		
		if (move == moveW)
			return moveE;
		
		return moveW;
	}
	
	/*--------------functions to update the death rate--------------*/
	KnowledgeBase updateMap(KnowledgeBase kb)	// update the current map death chance = pit + wumpus
	{
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				if (kb.map[i][j] != -3)	// if not visited yet
					kb.map[i][j] = kb.pit[i][j] + kb.wumpus[i][j];
		
		return kb;
	}
	
	KnowledgeBase getBreeze(KnowledgeBase kb, int death_rate)	// set the death rate for each neighbor if it sense breeze
	{
		int sz = kb.notvisited.size();
		for (int i = 0; i < sz; ++i)
		{
			if (kb.pit[kb.notvisited[i].first][kb.notvisited[i].second] != -1)	// if the current square is safe already (-1) do not need to update
			{
				if (death_rate == -1)	// if the death_rate is -1, then replace the death chance in that square to be safe (-1)
					kb.pit[kb.notvisited[i].first][kb.notvisited[i].second] = death_rate;
				
				else	// else then accumulate the death rate
					kb.pit[kb.notvisited[i].first][kb.notvisited[i].second] += death_rate;
			}
		}
		
		return kb;
	}
	
	// set the percentage of death for each neighbor if it sense stench
	// the same idea from breeze also apply to stench
	KnowledgeBase getStench(KnowledgeBase kb, int death_rate)
	{
		int sz = kb.notvisited.size();
		for (int i = 0; i < sz; ++i)
		{
			if (kb.wumpus[kb.notvisited[i].first][kb.notvisited[i].second] != -1)
			{
				if (death_rate == -1)
					kb.wumpus[kb.notvisited[i].first][kb.notvisited[i].second] = death_rate;
			
				else
					kb.wumpus[kb.notvisited[i].first][kb.notvisited[i].second] += death_rate;
			}
		}
		
		return kb;
	}
	/*--------------functions to update the death rate--------------*/
	
	KnowledgeBase doAction(KnowledgeBase kb, action bestAction)	// update the status of current X Y
	{
		switch (bestAction)	// update the x, y and map for any move
		{
			case moveW:
				kb.x -= 1;	// then update with the new coordinate
				break;
			case moveS:
				kb.y -= 1;
				break;
			case moveN:
				kb.y += 1;
				break;
			case moveE:
				kb.x += 1;
				break;
			default:
				break;
		}
		
		// if we are backtracking, then do not push the move to the stack to avoid ifinite loop
		if (kb.backing == false)
			kb.path.push(bestAction);	// remember the move we taken
		
		return kb;
	}
	
	KnowledgeBase backtrack(KnowledgeBase kb)	// go backward until hits 0-0 and climbout
	{
		if (kb.x == 0 && kb.y == 0)
			kb.back = climbOut;
		else
		{
			kb.back = oppositeMove(kb.path.top());
			kb.path.pop();
			kb.backing = true;
		}
		
		// remember the square we just backtracked from
		kb.pastX = kb.x;
		kb.pastY = kb.y;
		
		// update status
		kb = doAction(kb, kb.back);
		return kb;
	}
}

// Rename and complete this agent function.
action htwAgentJackie(WumpusWorldSensor currentSensor, bool shouldReset)
{
	// Declare one static variable of type KnowledgeBase.
	static KnowledgeBase kb;
	// Only one static variable is allowed!
	
	// Your function must end up returning a valid action.
	// No random-number generation allowed!
	
	if (shouldReset)
	{
		// Create a fresh knowledge base for the new wumpus world.
		kb = KnowledgeBase();
		// Just return without taking an action.
		return doNothing;
	}
	
	// set the current square to have been visited (-3) and safe from pit and wumpus (-1)
	kb.map[kb.x][kb.y] = -3;
	kb.pit[kb.x][kb.y] = -1;
	kb.wumpus[kb.x][kb.y] = -1;
	
	action bestAction = doNothing;
	kb.backing = false;
	
	// start every iteration by evaluating where the neighbors that have not been visited yet
	kb = get_notvisited(kb);
	
	// if after all that and all the neighbors are visited
	if (kb.notvisited.size() == 0 || kb.possibleMove.size() == 0)
		kb.noNeighbor = true;
	
	// check for breeze
	if (currentSensor.perceiveBreeze() == true)
		kb = getBreeze(kb, 100 / kb.notvisited.size());	// assign the death rate to be 100 / the size of the neighboring square
	else
		kb = getBreeze(kb, -1);	// assign the neighbor square to be safe from pit
	
	// check for stench
	if (currentSensor.perceiveStench() == true) 
		kb = getStench(kb, 100 / kb.notvisited.size());	// assign the death rate to be 100 / the size of the neighboring square
	else
		kb = getStench(kb, -1);	// assign the neighbor square to be safe from wumpus
	
	kb = updateMap(kb);	// update the map after all that changes
	
	if (kb.noNeighbor == true)	// just backtracking when all the neighoring square is visited
	{
		kb = backtrack(kb);
		return kb.back;
	}
	
	//----------find the gold and return----------
	if (currentSensor.perceiveGlitter() == true && kb.haveGold == false)
	{
		kb.haveGold = true;
		return grab;
	}
	
	if (kb.haveGold == true)	// just going backtrack from where we got the gold
	{
		kb = backtrack(kb);
		return kb.back;
	}
	//----------find the gold and return----------
	
	if (kb.x == 0 && kb.y == 0 && (currentSensor.perceiveBreeze() == true || currentSensor.perceiveStench() == true || kb.haveGold == true))
		return climbOut;	// just to be safe
	
	//----------do some logic to see if the surrounding is death traps -> backtrack immediately----------
	uint countDeathNeighbor = 0;
	
	for (uint i = 0; i < kb.notvisited.size(); ++i)
		if (kb.map[kb.notvisited[i].first][kb.notvisited[i].second] > 30)
			countDeathNeighbor++;
	
	if (countDeathNeighbor == kb.notvisited.size())	// if the surrounding is all death -> backtrack
	{
		kb = backtrack(kb);
		return kb.back;
	}
	//----------do some logic to see if the surrounding is death traps -> backtrack immediately----------
	
	//----------move to the lowest death rate square----------
	bestAction = kb.possibleMove[0];
	int bestX = kb.notvisited[0].first, bestY = kb.notvisited[0].second;
	
	for (uint i = 1; i < kb.notvisited.size(); ++i)	// find the lowest death rate that was not the box we just backtracked from
		if (kb.map[bestX][bestY] > kb.map[kb.notvisited[i].first][kb.notvisited[i].second] && kb.notvisited[i].first != kb.pastX && kb.notvisited[i].second != kb.pastY)
		{
			bestAction = kb.possibleMove[i];
			bestX = kb.notvisited[i].first;
			bestY = kb.notvisited[i].second;
		}
	//----------move to the lowest death rate square----------
	
	// reset the previous backtrack square
	kb.pastX = -1;
	kb.pastY = -1;
	
	kb = doAction(kb, bestAction);
	return bestAction; // Replace this return statement.
}

/*

 - First, carefully comment your code above to clarify how it works.
 - Here, describe your approach and analyze it.  How exactly did you develop
   and test it?  What are its strengths and weaknesses?  Why do you expect
   it to do well against the other submitted agents?
 - Also make a note here if you talked about the assignment with anyone or
   gave or received any kind of help.
	
	I am creating an extremeely careful agent:
	it starts by using eval to get the RAW direction for directions where it can go
	next, filter it out by getting the neighbor square that has not been visited yet (!= -3)
	then check for percieve
	- always check for glitter, if glitter is true
		-> grab it and start backtracking to climb out
	- if it feels breeze: save it to the pit_map
	- if it feels stench: save it to the wumpus_map
		UPDATE the total map
	- If there is a box that is < 30% death chance or one with -2
		go to that quare
	- if the surrounding is all death trap, then backtrack immediately
	- else
		just keep exploring any square that have not been visited yet
	
	if backtrack hit 0-0 -> ClimbOut
*/
