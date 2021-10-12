// CS 4318, spring 2021
// Agent Challenge 6: Hunt the wumpus
//
// Here's an example agent function.  You can do much better than this.

#include "htw.h"

namespace
{
   // A cellection of important information for future decision-making.
   // All members must be initialized for a new wumpus world.
   struct KnowledgeBase
   {
      // The agent starts in the lower-left corner.
      int whereX = 0; // which column the agent is in now
      int whereY = 0; // which row the agent is in now
      // The agent starts with no gold.
      bool haveGold = false; // true iff the agent has picked up the gold
   };
}

action htwAgentCoward(WumpusWorldSensor currentSensor, bool shouldReset)
{
   // Declare one static variable of type KnowledgeBase.
   static KnowledgeBase kb;
   // Only one static variable is allowed!
   action bestAction; // the action the agent decides to take

   if (shouldReset)
   {
      // Create a fresh knowledge base for the new wumpus world.
      kb = KnowledgeBase();
      // Just return without taking an action.
      return doNothing;
   }
   // At this point, shouldReset must be false.

   // First, learn new things using percepts and logic.
   // (You can save new knowledge in your knowledge base here.)

   // Then, figure out what to do next.
   // (This coward agent doesn't do much.)
   if (!kb.haveGold && currentSensor.perceiveGlitter())
   {
      // It glitters!  Grab it!
      bestAction = grab;
   }
   else if (kb.whereX > 0)
   {
      // Not on western edge, so move west.
      bestAction = moveW;
   }
   else if (kb.whereY > 0)
   {
      // Not on southern edge, so move south.
      bestAction = moveS;
   }
   else
   {
      // Must be in lower left corner then!
      bestAction = climbOut;
   }

   // Update the knowledge base according to the chosen action.
   switch (bestAction)
   {
   case climbOut:
      break;
   case grab:
      kb.haveGold = true;
      break;
   case moveW:
      kb.whereX -= 1;
      break;
   case moveS:
      kb.whereY -= 1;
      break;
   case moveN:
      kb.whereY += 1;
      break;
   case moveE:
      kb.whereX += 1;
      break;
   case shootW:
      break;
   case shootS:
      break;
   case shootN:
      break;
   case shootE:
      break;
   case doNothing:
      break;
   }

   // Return the chosen action.
   return bestAction;
}
