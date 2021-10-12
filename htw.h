// CS 4318, spring 2021
// Agent Challenge 6: Hunt the wumpus
//
// Here are the #includes and definitions available to each agent.

#ifndef HTW_H
#define HTW_H

#include <climits>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

const int pitProbability = 20; // The chance a room will have a pit, out of 100.
const int worldSize = 4; // The number of rooms square of each wumpus world.
const int actionLimit = 10 * worldSize * worldSize; // The player starves after this many actions.

// The allowed actions for agents to take.
enum action {climbOut, grab, moveW, moveS, moveN, moveE,
             shootW, shootS, shootN, shootE, doNothing};

// Convert an action to a representative character to be printed out.
char actionChar(action a);

// The things an agent can sense when deciding what to do.
class WumpusWorldSensor
{
private:
   bool isBreeze;
   bool isBump;
   bool isGlitter;
   bool isScream;
   bool isStench;
public:
   WumpusWorldSensor();
   bool perceiveBreeze() const {return isBreeze;}
   bool perceiveBump() const {return isBump;}
   bool perceiveGlitter() const {return isGlitter;}
   bool perceiveScream() const {return isScream;}
   bool perceiveStench() const {return isStench;}
   void resetAll();
   void setBreeze() {isBreeze = true;}
   void setBump() {isBump = true;}
   void setGlitter() {isGlitter = true;}
   void setScream() {isScream = true;}
   void setStench() {isStench = true;}
};

// A cave to explore with gold to find and a hungry wumpus to avoid.
class WumpusWorld
{
private:
   WumpusWorldSensor currentSensor;
   bool doesPlayerHaveArrow;
   bool doesPlayerHaveGold;
   bool hasGold[worldSize][worldSize];
   bool hasPit[worldSize][worldSize];
   bool hasPlayerBeenInRoom[worldSize][worldSize];
   bool hasPlayerExited;
   bool hasWumpus[worldSize][worldSize];
   bool isPlayerDead;
   bool isPlayerOutOfTime;
   bool isWumpusDead;
   int numActionsTaken;
   int playerScore;
   int whereX;
   int whereY;
   static int randomInt(int);
public:
   WumpusWorld();
   void applyAction(action a);
   bool didPlayerKillWumpus() {return isWumpusDead;}
   bool didPlayerShootArrow() {return !doesPlayerHaveArrow;}
   bool didPlayerStarve() {return isPlayerDead && isPlayerOutOfTime;}
   int getNumRoomsExplored() const;
   int getPlayerScore() const {return playerScore;}
   WumpusWorldSensor getSensor() const {return currentSensor;}
   bool hasPlayerLeftWithGold() {return hasPlayerExited && doesPlayerHaveGold;}
   bool hasPlayerLeftWithoutGold() {return hasPlayerExited && !doesPlayerHaveGold;}
   bool isPlayerStillExploring() {return !hasPlayerExited && !isPlayerDead;}
   void printWorld() const;
   void randomizeWorld();
   void resetWorld();
   void setRiskyWorld();
   void setTextbookWorld(bool useMirrorImage);
   bool wasPlayerKilled() {return isPlayerDead && !isPlayerOutOfTime;}
};

#endif // #ifndef HTW_H
