// CS 4318, spring 2021
// Agent Challenge 6: Hunt the wumpus
//
// Here are the functions available to each agent.

#include "htw.h"

char actionChar(action a)
{
   // Convert an action to a representative character to be printed out.
   switch (a)
   {
   case climbOut:
      return 'C';
   case grab:
      return 'G';
   case moveW:
      return 'W';
   case moveS:
      return 'S';
   case moveN:
      return 'N';
   case moveE:
      return 'E';
   case shootW:
      return 'L';
   case shootS:
      return 'D';
   case shootN:
      return 'U';
   case shootE:
      return 'R';
   case doNothing:
      return 'X';
   default:
      return '*';
   }
}

WumpusWorldSensor::WumpusWorldSensor()
{
   // Create new sensor with all percepts off.
   resetAll();
}

void WumpusWorldSensor::resetAll()
{
   // Turn all percepts off.
   isBreeze = false;
   isBump = false;
   isGlitter = false;
   isScream = false;
   isStench = false;
}

WumpusWorld::WumpusWorld()
{
   // Create a new and random wumpus world.
   randomizeWorld();
}

void WumpusWorld::applyAction(action a)
{
   // Update the wumpus world given player's action.
   int whereArrow;
   numActionsTaken += 1;
   playerScore -= 1;
   currentSensor.resetAll();
   switch (a)
   {
   case climbOut:
      if (whereX == 0 && whereY == 0)
      {
         if (doesPlayerHaveGold)
         {
            playerScore += 1000;
         }
         hasPlayerExited = true;
         return;
      }
      break;
   case grab:
      if (hasGold[whereX][whereY])
      {
         doesPlayerHaveGold = true;
      }
      break;
   case moveW:
      if (whereX > 0)
      {
         whereX -= 1;
      }
      else
      {
         currentSensor.setBump();
      }
      break;
   case moveS:
      if (whereY > 0)
      {
         whereY -= 1;
      }
      else
      {
         currentSensor.setBump();
      }
      break;
   case moveN:
      if (whereY < worldSize - 1)
      {
         whereY += 1;
      }
      else
      {
         currentSensor.setBump();
      }
      break;
   case moveE:
      if (whereX < worldSize - 1)
      {
         whereX += 1;
      }
      else
      {
         currentSensor.setBump();
      }
      break;
   case shootW:
      if (doesPlayerHaveArrow)
      {
         playerScore -= 10;
         doesPlayerHaveArrow = false;
         for (whereArrow = whereX - 1; whereArrow >= 0 && !hasWumpus[whereArrow][whereY]; whereArrow -= 1)
         {
         }
         if (whereArrow >= 0)
         {
            isWumpusDead = true;
            currentSensor.setScream();
         }
      }
      break;
   case shootS:
      if (doesPlayerHaveArrow)
      {
         playerScore -= 10;
         doesPlayerHaveArrow = false;
         for (whereArrow = whereY - 1; whereArrow >= 0 && !hasWumpus[whereX][whereArrow]; whereArrow -= 1)
         {
         }
         if (whereArrow >= 0)
         {
            isWumpusDead = true;
            currentSensor.setScream();
         }
      }
      break;
   case shootN:
      if (doesPlayerHaveArrow)
      {
         playerScore -= 10;
         doesPlayerHaveArrow = false;
         for (whereArrow = whereY + 1; whereArrow < worldSize && !hasWumpus[whereX][whereArrow]; whereArrow += 1)
         {
         }
         if (whereArrow < worldSize)
         {
            isWumpusDead = true;
            currentSensor.setScream();
         }
      }
      break;
   case shootE:
      if (doesPlayerHaveArrow)
      {
         playerScore -= 10;
         doesPlayerHaveArrow = false;
         for (whereArrow = whereX + 1; whereArrow < worldSize && !hasWumpus[whereArrow][whereY]; whereArrow += 1)
         {
         }
         if (whereArrow < worldSize)
         {
            isWumpusDead = true;
            currentSensor.setScream();
         }
      }
      break;
   case doNothing:
      break;
   default: // suicide
      isPlayerDead = true;
      playerScore -= 3000;
      return;
   }
   hasPlayerBeenInRoom[whereX][whereY] = true;
   isPlayerDead = hasPit[whereX][whereY] || (hasWumpus[whereX][whereY] && !isWumpusDead);
   if (isPlayerDead)
   {
      playerScore -= 1000;
   }
   else if (numActionsTaken >= actionLimit)
   {
      isPlayerDead = true;
      isPlayerOutOfTime = true;
      playerScore -= 2000;
   }
   if (hasWumpus[whereX][whereY] ||
       (whereX > 0 && hasWumpus[whereX - 1][whereY]) ||
       (whereY > 0 && hasWumpus[whereX][whereY - 1]) ||
       (whereY < worldSize - 1 && hasWumpus[whereX][whereY + 1]) ||
       (whereX < worldSize - 1 && hasWumpus[whereX + 1][whereY]))
   {
      currentSensor.setStench();
   }
   if ((whereX > 0 && hasPit[whereX - 1][whereY]) ||
       (whereY > 0 && hasPit[whereX][whereY - 1]) ||
       (whereY < worldSize - 1 && hasPit[whereX][whereY + 1]) ||
       (whereX < worldSize - 1 && hasPit[whereX + 1][whereY]))
   {
      currentSensor.setBreeze();
   }
   if (hasGold[whereX][whereY] && !doesPlayerHaveGold)
   {
      currentSensor.setGlitter();
   }
}

int WumpusWorld::getNumRoomsExplored() const
{
   int column, numRoomsExplored, row;
   numRoomsExplored = 0;
   for (column = 0; column < worldSize; column += 1)
   {
      for (row = 0; row < worldSize; row += 1)
      {
         if (hasPlayerBeenInRoom[column][row])
         {
            numRoomsExplored += 1;
         }
      }
   }
   return numRoomsExplored;
}

void WumpusWorld::printWorld() const
{
   // Output a concise representation of the current wumpus world.
   int column, row;
   for (row = worldSize - 1; row >= 0; row -= 1)
   {
      for (column = 0; column < worldSize; column += 1)
      {
         cout << "[" << (hasWumpus[column][row] ? "W" : " ")
              << (hasPit[column][row] ? "P" : " ")
              << (hasGold[column][row] ? "G" : " ") << "]";
      }
      cout << "\n";
   }
}

int WumpusWorld::randomInt(int n)
{
   // Return a random nonnegative integer less than n.
   int r;
   if (n <= 0)
   {
      return 0;
   }
   do
   {
      r = random();
   }
   while (r >= INT_MAX / n * n);
   return r / (INT_MAX / n);
}

void WumpusWorld::randomizeWorld()
{
   // Create a new and random wumpus world.
   int column, row;
   for (column = 0; column < worldSize; column += 1)
   {
      for (row = 0; row < worldSize; row += 1)
      {
         hasWumpus[column][row] = false;
         hasPit[column][row] = randomInt(100) < pitProbability;
         hasGold[column][row] = false;
      }
   }
   do
   {
      column = randomInt(worldSize);
      row = randomInt(worldSize);
   }
   while (column == 0 && row == 0);
   hasWumpus[column][row] = true;
   hasPit[0][0] = false;
   hasGold[randomInt(worldSize)][randomInt(worldSize)] = true;
   resetWorld();
}

void WumpusWorld::resetWorld()
{
   // Reset the current wumpus world for a new player to explore.
   int column, row;
   doesPlayerHaveArrow = true;
   doesPlayerHaveGold = false;
   for (column = 0; column < worldSize; column += 1)
   {
      for (row = 0; row < worldSize; row += 1)
      {
         hasPlayerBeenInRoom[column][row] = column == 0 && row == 0;
      }
   }
   hasPlayerExited = false;
   isPlayerDead = false;
   isPlayerOutOfTime = false;
   isWumpusDead = false;
   numActionsTaken = 0;
   playerScore = 0;
   whereX = 0;
   whereY = 0;
   currentSensor.resetAll();
   if (hasWumpus[0][1] || hasWumpus[1][0])
   {
      currentSensor.setStench();
   }
   if (hasPit[0][1] || hasPit[1][0])
   {
      currentSensor.setBreeze();
   }
   if (hasGold[0][0])
   {
      currentSensor.setGlitter();
   }
}

void WumpusWorld::setRiskyWorld()
{
   // Create a wumpus world that rewards intelligent risk-taking.
   int column, row;
   for (column = 0; column < worldSize; column += 1)
   {
      for (row = 0; row < worldSize; row += 1)
      {
         hasWumpus[column][row] = false;
         hasPit[column][row] = false;
         hasGold[column][row] = false;
      }
   }
   hasWumpus[2][2] = true;
   hasPit[2][2] = true;
   hasGold[3][3] = true;
   resetWorld();
}

void WumpusWorld::setTextbookWorld(bool useMirrorImage)
{
   // Create the example wumpus world in the textbook.
   int column, row;
   for (column = 0; column < worldSize; column += 1)
   {
      for (row = 0; row < worldSize; row += 1)
      {
         hasWumpus[column][row] = false;
         hasPit[column][row] = false;
         hasGold[column][row] = false;
      }
   }
   if (useMirrorImage)
   {
      hasWumpus[2][0] = true;
      hasPit[0][2] = true;
      hasPit[2][2] = true;
      hasPit[3][3] = true;
      hasGold[2][1] = true;
   }
   else
   {
      hasWumpus[0][2] = true;
      hasPit[2][0] = true;
      hasPit[2][2] = true;
      hasPit[3][3] = true;
      hasGold[1][2] = true;
   }
   resetWorld();
}
