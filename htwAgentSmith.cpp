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

namespace
{
	// If you need to define any new types or functions, put them here in
	// this unnamed namespace.  But no variables allowed!
	
	struct KnowledgeBase
	{
		// Declare and initialize member variables here.
		// No dynamic memory allocation is allowed, but arrays are okay.
		int x = 0, y = 0;
		
		int map[4][4] = {0};	// keep a copy of the map that has the percentage of death
		
		int wumpus[4][4] = {0}, wumpusX = -1, wumpusY = -1;	// the location of the wumpus
		
		int pit[4][4] = {0};	// the location of the pits

		stack<action> path;	// keep the current path as a stack
		
		vector<action> possibleMove;	// keep a list of all possible move: moveW, moveS, moveN, moveE
		
		vector<pair<int, int>> possibleNeighbor;	// keep a list of neighbor square that have not visited
		
		bool
			setDefault = true, 	// 1 time set default
			haveGold = false,	// if the player have gold or not
			wumpusDie = false,	// if the wumpus has died or not
			noNeighbor = false,	// if all the neighboring square are already visited
			haveArrow = true,	// remember we still have an arrow
			backing = false;	// trigger when we are backtracking
		
		action back, shootDir;
	};
	
	/*----------functions to get the neighboring square----------*/
	KnowledgeBase getMoves(KnowledgeBase kb)	// evaluate all the possible move we can take from where we are right now
	{
		if (kb.x == 0)	// if it is at the left most side of the map
		{
			if (kb.y == 0)	// lower left corner of the map
				kb.possibleMove = {moveN, moveE};
			else if (kb.y == 3)	// upper-left corner
				kb.possibleMove = {moveE, moveS};
			else	// left-most column
				kb.possibleMove = {moveN, moveE, moveS};
		}
		
		else if (kb.x == 3)	// if it is at the right-most side of the map
		{
			if (kb.y == 0)	// lower right corner
				kb.possibleMove = {moveN, moveW};
			else if (kb.y == 3)	// upper right corner
				kb.possibleMove = {moveW, moveS};
			else
				kb.possibleMove = {moveN, moveW, moveS};
		}
		
		else	// we are at the 2 middle column
		{
			if (kb.y == 0)	// at the bottom row
				kb.possibleMove = {moveW, moveN, moveE};
			else if (kb.y == 3)	// at the top row
				kb.possibleMove = {moveW, moveS, moveE};
			else	// somewhere in the middle of the whole map
				kb.possibleMove = {moveW, moveS, moveN, moveE};
		}
		
		return kb;
	}
	
	// chek for all the not visited neighbor (!= -1)
	// and get the (x, y) of all the neighbor
	KnowledgeBase getNeighbor(KnowledgeBase kb)
	{
		// clear the old neighbors as the player have taken an action
		kb.possibleNeighbor.clear();
		
		kb = getMoves(kb);
		
		// a function to check all the possible neighbors that have not been visited
		for (uint i = 0; i < kb.possibleMove.size(); ++i)
		{
			if (kb.possibleMove[i] == moveN && kb.map[kb.x][kb.y + 1] != -1)
				kb.possibleNeighbor.push_back(make_pair(kb.x, kb.y + 1));
			
			else if (kb.possibleMove[i] == moveS && kb.map[kb.x][kb.y - 1] != -1)
				kb.possibleNeighbor.push_back(make_pair(kb.x, kb.y - 1));
			
			else if (kb.possibleMove[i] == moveW && kb.map[kb.x - 1][kb.y] != -1)
				kb.possibleNeighbor.push_back(make_pair(kb.x - 1, kb.y));
			
			else if (kb.possibleMove[i] == moveE && kb.map[kb.x + 1][kb.y] != -1)
				kb.possibleNeighbor.push_back(make_pair(kb.x + 1, kb.y));
			
			else	// erase if the move is going to an already visited square
			{
				kb.possibleMove.erase(kb.possibleMove.begin() + i);
				--i;
			}
		}
		
		return kb;
	}
	/*----------functions to get the neighboring square----------*/
	
	/*--------------functions to update the death rate--------------*/
	KnowledgeBase updateMap(KnowledgeBase kb)	// update the current map death chance = pit + wumpus
	{
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				if (kb.map[i][j] != -1)	// if not visited yet
					kb.map[i][j] = kb.pit[i][j] + kb.wumpus[i][j];
		
		return kb;
	}
	
	KnowledgeBase getBreeze(KnowledgeBase kb)	// set the percentage of death for each neighbor if it sense breeze
	{
		int sz = kb.possibleNeighbor.size();
		for (int i = 0; i < sz; ++i)
		{
			kb.pit[kb.possibleNeighbor[i].first][kb.possibleNeighbor[i].second] += 100 / sz;
			
			if (kb.pit[kb.possibleNeighbor[i].first][kb.possibleNeighbor[i].second] > 80)
				kb.map[kb.possibleNeighbor[i].first][kb.possibleNeighbor[i].second] = INT_MAX;	// definitely do not go there
		}
		
		// map death chance = pit + wumpus
		kb = updateMap(kb);
		
		return kb;
	}
	
	KnowledgeBase getStench(KnowledgeBase kb)	// set the percentage of death for each neighbor if it sense stench
	{
		int sz = kb.possibleNeighbor.size();
		for (int i = 0; i < sz; ++i)
		{
			kb.wumpus[kb.possibleNeighbor[i].first][kb.possibleNeighbor[i].second] += 100 / sz;
			
			// if the chance of wumpus appearing in that square is > 80% => found it
			if (kb.wumpus[kb.possibleNeighbor[i].first][kb.possibleNeighbor[i].second] > 80)
			{
				kb.wumpusX = kb.possibleNeighbor[i].first;
				kb.wumpusY = kb.possibleNeighbor[i].second;
				kb.map[kb.possibleNeighbor[i].first][kb.possibleNeighbor[i].second] = INT_MAX;	// definitely do not go there
			}
		}
		
		// map death chance = pit + wumpus
		kb = updateMap(kb);
		
		return kb;
	}
	/*--------------functions to update the death rate--------------*/
	
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
	
	KnowledgeBase backtrack(KnowledgeBase kb)	// go backward until hits 0-0 and climbout
	{
		if (kb.x == 0 && kb.y == 0)
			kb.back = climbOut;
		else
		{
			kb.back = oppositeMove(kb.path.top());
			kb.path.pop();
		}
		
		kb.backing = true;
		return kb;
	}
	
	KnowledgeBase doAction(KnowledgeBase kb, action bestAction)	// update the status
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
		
		if (kb.backing == false)
			kb.path.push(bestAction);	// remember the move we taken
		else
			kb.backing = false;	// we are done backtracking for this turn
		
		return kb;
	}
	
	KnowledgeBase doShoot(KnowledgeBase kb)
	{
		if (kb.shootDir == shootN)
			for (int y = kb.y + 1; y <= 3; ++y)
				kb.wumpus[kb.x][y] = 0;
		
		else if (kb.shootDir == shootS)
			for (int y = kb.y - 1; y >= 0; --y)
				kb.wumpus[kb.x][y] = 0;
		
		else if (kb.shootDir == shootW)
			for (int x = kb.x - 1; x >= 0; --x)
				kb.wumpus[x][kb.y] = 0;
		
		else
			for (int x = kb.x + 1; x <= 3; ++x)
				kb.wumpus[x][kb.y] = 0;
		
		kb = updateMap(kb);
		return kb;
	}
	
	void outputMap(KnowledgeBase kb)	// output the statistic of my Map
	{
		cout << endl;
		for (int y = 3; y >= 0; --y)
		{
			for (int x = 0; x <= 3; ++x)
				cout << setw(3) << kb.map[x][y] << " | ";
			cout << endl;
		}
	}
}

// Rename and complete this agent function.
action htwAgentSmith(WumpusWorldSensor currentSensor, bool shouldReset)
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
	
	// set the current square to have been visited
	kb.map[kb.x][kb.y] = -1;	
	kb.pit[kb.x][kb.y] = -1;
	kb.wumpus[kb.x][kb.y] = -1;
	
	//-----------initialize default values---------------
	if (kb.setDefault == true)
	{
		kb.map[0][0] = -1;	// already visited
		kb.pit[0][0] = -1;	// no pit at start
		kb.wumpus[0][0] = -1;	// no wumpus at start
		kb.setDefault = false;
	}
	//-----------initialize default values---------------
	
	// start every iteration by evaluating where the neighbors that have not been visited yet
	kb = getNeighbor(kb);
	
	if (kb.possibleNeighbor.size() == 0 || kb.possibleMove.size() == 0)	// if after all that and all the neighbors are visited
		kb.noNeighbor = true;
	
	action bestAction = climbOut;
	
	if (currentSensor.perceiveGlitter() == true)
		if (kb.haveGold == false)	// if found gold, return immediately
		{
			kb.haveGold = true;
			return grab;
		}
	
	if (kb.haveGold == true)	// just going back from where we got the gold
	{
		kb = backtrack(kb);
		bestAction = kb.back;
	}
	
	// only proceed if we have not found the gold
	else
	{
		// if we feel the breeze -> update the map and backtrack
		if (currentSensor.perceiveBreeze() == true)
		{
			kb = getBreeze(kb);
			
			// play safe - backtrack immediately when meet a pit
			kb = backtrack(kb);
			bestAction = kb.back;
			
			// update status
			kb = doAction(kb, bestAction);
			return bestAction;
		}
		
		// if the wumpus is not dead and we still have the arrow
		if (kb.wumpusDie == false)
		{
			if (currentSensor.perceiveStench() == true)
			{
				kb = getStench(kb);
				
				// play safe - backtrack immediately when smell stench
				if (kb.wumpusX == -1 && kb.wumpusY == -1)
				{
					kb = backtrack(kb);
					bestAction = kb.back;
			
					// update status
					kb = doAction(kb, bestAction);
					return bestAction;
				}
			}
			
			if (kb.haveArrow == true && (kb.wumpusX != -1 || kb.wumpusY != -1))	// if we already found the wumpus -> kill it
			{
				kb.haveArrow = false;	// we're gonna shoot the arrow
				
				if (kb.x == kb.wumpusX)
				{
					if (kb.y > kb.wumpusY)
						kb.shootDir = shootS;
					else
						kb.shootDir = shootN;
				}
				else if (kb.y == kb.wumpusY)
				{
					if (kb.x > kb.wumpusX)
						kb.shootDir = shootW;
					else
						kb.shootDir = shootE;
				}
				
				kb = doShoot(kb);
				return kb.shootDir;
			}
		}
		
		// check to see if the Wumpus die when we shoot the arrow
		if (kb.haveArrow == false)
		{
			// if it is dead, update for other square to be free of wumpus
			if (currentSensor.perceiveScream() == true)
			{
				kb.wumpusDie = true;
				
				for (int x = 0; x <= 3; ++x)
					for (int y = 0; y <= 3; ++y)
						if (kb.wumpus[x][y] != -1)
							kb.wumpus[x][y] = 0;
				
				kb = updateMap(kb);
			}
			else	// else, the other neighbors got + 50%
			{
				for (uint i = 0; i < kb.possibleNeighbor.size(); ++i)
					if (kb.wumpus[kb.possibleNeighbor[i].first][kb.possibleNeighbor[i].second] > 30)
						kb.wumpus[kb.possibleNeighbor[i].first][kb.possibleNeighbor[i].second] += 50;
			}
		}
		
		kb = updateMap(kb);
		
		/*----------if we made it here without any action----------*/
		// if there we are at start and all neighbor square are all visited
		if (kb.noNeighbor == true && kb.x == 0 && kb.y == 0)
			bestAction = climbOut;
		
		//	backtrack if all the neighboring square is visited
		else if (kb.noNeighbor == true)
		{
			kb = backtrack(kb);
			bestAction = kb.back;
		}
		// move to the lowest death rate square
		else
		{
			bestAction = kb.possibleMove[0];
			int bestX = kb.possibleNeighbor[0].first, bestY = kb.possibleNeighbor[0].second;
			
			for (uint i = 1; i < kb.possibleNeighbor.size(); ++i)	// find the lowest death rate
				if (kb.map[bestX][bestY] > kb.map[kb.possibleNeighbor[i].first][kb.possibleNeighbor[i].second])
				{
					bestAction = kb.possibleMove[i];
					bestX = kb.possibleNeighbor[i].first;
					bestY = kb.possibleNeighbor[i].second;
				}
		}
		/*----------if we made it here without any action----------*/
	}
	
	if (currentSensor.perceiveGlitter() == true)
		if (kb.haveGold == false)	// if found gold, return immediately
		{
			kb.haveGold = true;
			return grab;
		}
	
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
	- always check for glitter, if glitter is true
		-> grab it and start backtracking to climb out
	- if it feels breeze: back track immediately
	- if it feels stench: backtrack
		- if any box has > 80% wumpus => we found where the wumpus is
			=> shoot arrow to kill it
	- else
		just keep exploring any square that have not been visited (-1) yet
*/
