// CS 4318, spring 2021
// Agent Challenge 6: Hunt the wumpus
//
// Here's the main function that runs all agents on many wumpus worlds and
// summarizes the results.

#include "htw.h"

extern const int numAgents;
extern action (*agentFunc[])(WumpusWorldSensor, bool);
extern string agentStr[];

int main()
{
   const int numWorlds = 5000;   // Change if you like.
   action nextAction;
   int arrowsShot[numAgents], highScore, i, j, order[numAgents],
       roomsExplored[numAgents], score[numAgents], timesBest[numAgents],
       timesKilled[numAgents], timesLeftWithGold[numAgents],
       timesLeftWithoutGold[numAgents], timesStarved[numAgents], temp,
       totalScore[numAgents], wumpiKilled[numAgents];
   string actionStr;
   WumpusWorld world;

   srandom(time(0));

   cout << "Agent Challenge 6: Hunt the wumpus\n"
        << "Iteration results\n\n";
   for (i = 0; i < numAgents; i += 1)
   {
      totalScore[i] = 0;
      timesBest[i] = 0;
      timesLeftWithGold[i] = 0;
      timesLeftWithoutGold[i] = 0;
      timesKilled[i] = 0;
      timesStarved[i] = 0;
      roomsExplored[i] = 0;
      arrowsShot[i] = 0;
      wumpiKilled[i] = 0;
   }
   for (i = 0; i < numWorlds; i += 1)
   {
      if (i == 0)
      {
         world.setTextbookWorld(false);
      }
      else if (i == 1)
      {
         world.setTextbookWorld(true);
      }
      else if (i == 2)
      {
         world.setRiskyWorld();
      }
      else
      {
         world.randomizeWorld();
      }
      cout << "Wumpus world " << i + 1 << ":\n";
      world.printWorld();
      cout << "\n";
      for (j = 0; j < numAgents; j += 1)
      {
         cout << setw(20) << left << agentStr[j] << flush;
         (*agentFunc[j])(WumpusWorldSensor(), true);
         world.resetWorld();
         actionStr = "";
         do
         {
            nextAction = (*agentFunc[j])(world.getSensor(), false);
            actionStr += actionChar(nextAction);
            world.applyAction(nextAction);
         }
         while (world.isPlayerStillExploring());
         score[j] = world.getPlayerScore();
         totalScore[j] += score[j];
         cout << " " << setw(5) << right << score[j]
              << " [" << actionStr << "] ";
         if (world.hasPlayerLeftWithGold())
         {
            timesLeftWithGold[j] += 1;
            cout << "rich\n";
         }
         else if (world.hasPlayerLeftWithoutGold())
         {
            timesLeftWithoutGold[j] += 1;
            cout << "gave up\n";
         }
         else if (world.wasPlayerKilled())
         {
            timesKilled[j] += 1;
            cout << "killed\n";
         }
         else if (world.didPlayerStarve())
         {
            timesStarved[j] += 1;
            cout << "starved\n";
         }
         roomsExplored[j] += world.getNumRoomsExplored();
         if (world.didPlayerShootArrow())
         {
            arrowsShot[j] += 1;
         }
         if (world.didPlayerKillWumpus())
         {
            wumpiKilled[j] += 1;
         }
      }
      cout << "\n";
      highScore = score[0];
      for (j = 1; j < numAgents; j += 1)
      {
         if (score[j] > highScore)
         {
            highScore = score[j];
         }
      }
      for (j = 0; j < numAgents; j += 1)
      {
         if (score[j] >= highScore)
         {
            timesBest[j] += 1;
         }
      }
   }

   for (i = 0; i < numAgents; i += 1)
   {
      order[i] = i;
   }
   for (i = 0; i < numAgents - 1; i += 1)
   {
      for (j = i + 1; j < numAgents; j += 1)
      {
         if (totalScore[order[i]] < totalScore[order[j]] || (totalScore[order[i]] == totalScore[order[j]] &&
             (timesLeftWithGold[order[i]] < timesLeftWithGold[order[j]] || (timesLeftWithGold[order[i]] == timesLeftWithGold[order[j]] &&
              (wumpiKilled[order[i]] < wumpiKilled[order[j]] || (wumpiKilled[order[i]] == wumpiKilled[order[j]] &&
               (timesStarved[order[i]] > timesStarved[order[j]] || (timesStarved[order[i]] == timesStarved[order[j]] &&
                agentStr[order[i]] > agentStr[order[j]]))))))))
         {
            temp = order[i];
            order[i] = order[j];
            order[j] = temp;
         }
      }
   }
   cout << "\n"
        << "Overall standings:           average    times    times    times    times    times      rooms    wumpi   arrows     arrow\n"
        << "                              points     best     rich  gave up   killed  starved   explored   killed     shot     hit %\n"
        << fixed;
   for (i = 0; i < numAgents; i += 1)
   {
      cout << setw(24) << left << agentStr[order[i]]
           << " " << setprecision(5) << setw(11) << right << static_cast<double>(totalScore[order[i]]) / numWorlds
           << " " << setw(8) << right << timesBest[order[i]]
           << " " << setw(8) << right << timesLeftWithGold[order[i]]
           << " " << setw(8) << right << timesLeftWithoutGold[order[i]]
           << " " << setw(8) << right << timesKilled[order[i]]
           << " " << setw(8) << right << timesStarved[order[i]]
           << " " << setw(10) << right << roomsExplored[order[i]]
           << " " << setw(8) << right << wumpiKilled[order[i]]
           << " " << setw(8) << right << arrowsShot[order[i]];
      if (arrowsShot[order[i]] > 0)
      {
         cout << " " << setprecision(2) << setw(8) << right << 100.0 * static_cast<double>(wumpiKilled[order[i]]) / arrowsShot[order[i]] << "%";
      }
      cout << "\n";
   }

   return 0;
}
