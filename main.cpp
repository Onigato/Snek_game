/*
COPYRIGHT Onigato, using the OLC-3 License
SNEK v0.1.0
Published Feb 24, 2020

License (OLC-3)
	~~~~~~~~~~~~~~~

	Copyright 2018 - 2019 OneLoneCoder.com

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions or derivations of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.

	2. Redistributions or derivative works in binary form must reproduce the above
	copyright notice. This list of conditions and the following	disclaimer must be
	reproduced in the documentation and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors may
	be used to endorse or promote products derived from this software without specific
	prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS	"AS IS" AND ANY
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
	SHALL THE COPYRIGHT	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL,	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
	TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
	BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
	ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
	SUCH DAMAGE.

*/

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <vector>

struct snake
{
	//A snake body piece, using olc 2D vectors for position, and direction
	//Constructor needs to know where to make the new piece, and what way it is going
	snake(olc::vi2d inPos, olc::vi2d inDir)
	{
		pos = inPos;
		dir = inDir;
	}
	//just a getter. Which technically isn't necessary, since pos is public, buuuuuuuttttt...
	olc::vi2d getPos()
	{
		return pos;
	}
	//adds current direction to current position
	void update()
	{
		pos += dir;
	}

	//KEEP PUBLIC OR ELSE!! You do NOT want to have to make getters for dir and head.
	//pos and dir have two member variables, x and y and all standard operations EXCEPT ==
	//pos == position
	//dir == direction
	olc::vi2d pos;
	olc::vi2d dir;
	//Is this bit the head of the snake?
	bool head;
};

struct food  
{
	//The bit of food for the snake to get and grow
	//Has an olc vi2d for position, and a boolean for if it has been eaten
	food(int x, int y)
	{
		pos.x = x;
		pos.y = y;
	}
	void newLoc(int x, int y)
	{
		pos.x = x;
		pos.y = y;
		exists = true;
	}
	olc::vi2d pos;
	bool exists = true;
};

struct sTurns
{
	olc::vi2d pos;
	olc::vi2d dir;
	bool touches = false;
	sTurns(olc::vi2d inPos, olc::vi2d inDir)
	{
		pos = inPos;
		dir = inDir;
	}

};

// Override base class with your custom functionality
class Snek : public olc::PixelGameEngine
{
private:
	//A couple holding vectors, vecCrawlie for the snake bits, vecTurns for the list of all the turns the game holds for the snake bits
	std::vector<snake*> vecCrawlie;
	std::vector<sTurns*> vecTurns;
	//Initiallize a few other things
	//A pointer to the food object
	food* noms = nullptr;
	//cycleTime is the number of frames between play updates. countTime is the number of frames SINCE the last play update
	int cycleTime = 1000;
	int countTime = 0;
	//score has not yet been implemented
	int score = 0;
	//Two olc 2d vectors used to give things a position and direction. Each contains a .x member variable and a .y member variable
	olc::vi2d newPos;
	olc::vi2d newDir;


	void addSnekBit(olc::vi2d inPos, olc::vi2d inDir)
	{
		//make a new snake bit, add it to the snake. Takes an olc 2d vector for position and direction
		snake* temp = new snake(inPos, inDir);
		//New bit is the head, flag it as such
		temp->head = true;
		//All the other bits are NOT, flag them as not
		for (auto& a : vecCrawlie)
			a->head = false;
		//Add the new bit to the actual vector. Do NOT do this before flagging auto-for loop operation.
		vecCrawlie.push_back(temp);
	}

	void addTurn(olc::vi2d inDir)
	{
		//a storage place for all turn orders given by the player, input is an olc 2d vector
		//Wherever the head is, that's where the turn order will be
		for (auto& a : vecCrawlie)
			if (a->head)
			{
				newPos = a->getPos();
			}
		//create a pointer to the new turning spot with the correct location, and the input direction, then add it to the holding vector
		sTurns* temp = new sTurns(newPos, inDir);
		vecTurns.push_back(temp);
	}

	void clearTurns()
	{
		//Since the turns are being held not in the snake, but in the play field, old turn orders MUST be removed over time
		//Holding vector to hold the KEPT turn orders
		std::vector<sTurns*> vecHold;
		//If the snake is touching the turn order point, save it in the temp vector
		for (auto& tp : vecTurns)
			for (auto& bit : vecCrawlie)
				if (tp->pos.x == bit->pos.x && tp->pos.y == bit->pos.y)
					vecHold.push_back(tp);
		//clear out ALL the orders in the main vector, then copy the actual kept turns back in, and clean up after yourself in the temp, just in case
		vecTurns.clear();
		vecTurns = vecHold;
		vecHold.clear();
	}

	void snekTurnsUpdate()
	{
		//Fancy auto for loop. Update dir on all the bits of snake, based on IF they currently hit a turning point
		//Probably inefficient, especially as the number of turns and bits IS going to get big over time
		for (auto& tempBit : vecCrawlie)
		{
			for (auto& tempPoint : vecTurns)
				if (tempBit->pos.x == tempPoint->pos.x && tempBit->pos.y == tempPoint->pos.y)
					tempBit->dir = tempPoint->dir;
		}
	}

	void snekChkEat(snake* snekBit)
	{
		//Takes a snake* bit in
		//Did the snake eat some food?
		if (snekBit->pos.x == noms->pos.x && snekBit->pos.y == noms->pos.y && noms->exists && snekBit->head)
		{
			//Food ceases to exist for now
			noms->exists = false;
			//add a new bit of snake IN FRONT OF the current snake head. 
			//Yes. This means you have to go ALONG walls to get food safely. Corners, I shudder to think about.
			addSnekBit((snekBit->pos + snekBit->dir), snekBit->dir);
			//Shorten the cycle time, making the game slightly faster with each bit of food eaten, but clamp it so it never gets TOO fast
			cycleTime -= 10;
			if (cycleTime <= 50)
				cycleTime = 50;
		}
	}

	bool snekCollide()
	{
		//Initialize a couple temp snake bits
		//head is verifiably the head
		snake* head = nullptr;
		//piece is the active piece
		snake* piece = nullptr;
		//Reverse iterator, C++17 or higher ONLY
		for (auto it = vecCrawlie.crbegin(), end = vecCrawlie.crend(); it != end; ++it)
		{
			//Active piece is whatever the iterator is looking at
			piece = *it;
			//Is it the head? Set head as well!
			if (piece->head) head = *it;
			//Risky AF~ if the last element of the vector ISN'T the head piece, call to nullptr exceptions.
			//But it works. If the current piece ISN'T the head, and IS in the same space as the head, End Game
			if (piece->pos.x == head->pos.x && piece->pos.y == head->pos.y && piece->head == false) 
			{
				return true;
			}
			//Hit a wall? End Game
			if (piece->pos.x < 1 || piece->pos.x >(ScreenWidth() - 1) || piece->pos.y < 1 || piece->pos.y >(ScreenHeight() - 1))
			{ 
				return true; 
			}
		}
		//Didn't hit anything you weren't supposed to? Keep playing!
		return false;
	}

public:
	Snek()
	{
		sAppName = "Snek";
	}

public:
	bool OnUserCreate() override
	{
		//User starts in the middle, and with NO motion intially
		newDir.x = 0;
		newDir.y = 0;
		newPos.x = (ScreenWidth() / 2);
		newPos.y = (ScreenHeight() / 2);
		//Actually make the first bit of snake
		addSnekBit(newPos, newDir);
		
		//"randomly" put a bit of food somewhere on the screen, inside the walls. I think.
		noms = new food((rand() % (ScreenWidth() - 2) + 1), (rand() % (ScreenHeight() - 2) + 1));
			
		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override
	{
		//Direction keys. Needs to be refactored to state machine, and probably a switch
		if (GetKey(olc::W).bPressed || GetKey(olc::UP).bPressed)
		{
			newDir.x = 0;
			newDir.y = -1;
			addTurn(newDir);
		}
		if (GetKey(olc::A).bPressed || GetKey(olc::LEFT).bPressed)
		{
			newDir.x = -1;
			newDir.y = 0;
			addTurn(newDir);
		}
		if (GetKey(olc::S).bPressed || GetKey(olc::DOWN).bPressed)
		{
			newDir.x = 0;
			newDir.y = 1;
			addTurn(newDir);
		}
		if (GetKey(olc::D).bPressed || GetKey(olc::RIGHT).bPressed)
		{
			newDir.x = 1;
			newDir.y = 0;
			addTurn(newDir);
		}
		if (GetKey(olc::ESCAPE).bPressed) return false;//Esc to close
		
		//Needs to be tied to FPS somehow, but FPS is a private variable
		//Slows the game down to reasonable levels on a fast machine, too much on a slower machine
		if (countTime >= cycleTime)
		{
			Clear(olc::BLACK);
			//Reset the clock
			countTime = countTime - (cycleTime + fElapsedTime);
			//Set all the bits .dir data correctly
			snekTurnsUpdate();

			for (auto &snekBit : vecCrawlie)
			{
				//snake bits move
				snekBit->update();
				//And the HEAD checks to see if any food has been impacted.
				if (snekBit->head)
					snekChkEat(snekBit);
			}
			//If the snake actually hit something, End Game
			if (snekCollide())
				return false;
		}
		//Draw the snake
		for (auto &snekBit : vecCrawlie)
			Draw(snekBit->pos.x, snekBit->pos.y, olc::WHITE);
		//Draw the walls
		DrawRect(0, 0, ScreenWidth()-1, ScreenHeight()-1, olc::BLUE);
		//If food exists, draw it, otherwise create a new food. This gives a one frame delay on the food reappearing, WAI
		if (noms->exists)
		{
			Draw(noms->pos.x, noms->pos.y, olc::RED);
		}
		else
		{
			noms->newLoc((rand() % (ScreenWidth() - 2) + 1), (rand() % (ScreenHeight() - 2) + 1));
		}
		//increment the time for actual updates to occur
		countTime++;
		//Clear out any turn orders that are no longer relevant, as the snake has already moved past the points
		clearTurns();
		//Keep playing!
		return true;
	}
};


int main()
{
	Snek core;
	if (core.Construct(32, 32, 8, 8))
		core.Start();
	return 0;
}