#include <iostream>  
#include <fstream> 
#include <string> 
#include <cstring>  
#include <cstdio>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <sys/time.h>
#include <time.h>

//CSCI 443, Evolutionary Algorithms
//Project 4
//Don Lieu
//Description: Given a limit of available funds and a list of investments of varying cost, risk, and potential, this program uses a genetic algorithm to find an optimal combination of investments using available funds given the following objectives:
////1. Maximize total potential
////2. Minimize total risk
////3. Minimize unused funds

using namespace std;

const int collection_size = 69, minRand = 1, maxRand = 2000, timeLimit = 5;
//Parameters can be manipulated by user
int availableFunds = ceil(maxRand * (collection_size / 4));
//Available funds as a function of maximum random value and collection size
int chromPop = ceil(1.65 * pow(2, (0.21 * collection_size)));
//Optimal population size by Goldberg
int terminationPop = ceil( log(chromPop) );
//Final Pareto front size increases logarithmically with population size
int mutationRate = 99;
//Chance mutation occurs as a percentage value from 1 to 100.  Can be changed if necessary.
int sigCoefficient = ceil( maxRand * .005 );
//How much greater the totalPotential/totalCost - availableFunds must be to be considered significantly dominant.  Prevents premature convergence over tiny differences in optimality.

struct chromosome
//Binary string representing a set of investment choices.  1 means the investment was chosen, 0 means it was not.  All other values calculated accordingly, but totalCost cannot exceed availableFunds.
{
	char sequence[collection_size];
	int totalCost;
	int totalRisk;
	int totalPotential;
	int unusedFunds;
	int dominance;
	bool dominant;
};

struct investment
{
	int cost;
	int risk;
	int potential;
};

bool cEquals(struct chromosome c1, struct chromosome c2)
//Determine if two chromosomes are equivalent
{
	for (int i = 0; i < collection_size; i++)
	{
		if (c1.sequence[i] != c2.sequence[i])
		{
			return false;
		}
	}
	return true;
}

void populate(struct investment investmentList[])
//Polulate initial list of investments and prints to terminal screen
{
	for(int i = 0; i < collection_size; i++)
	{
		int exp = rand() % 4 + 1;
		investmentList[i].cost = rand() % maxRand + minRand;
		investmentList[i].risk = rand() % 99 + 1;
		investmentList[i].potential = investmentList[i].cost + ceil( investmentList[i].cost * (pow( ((100 / (100 - investmentList[i].risk) ) ), exp) ) );

		cout << "Investment " << i+1 << ":" << endl;
		cout << "Cost: $" << investmentList[i].cost << endl;
		cout << "Risk: " << investmentList[i].risk << "%" << endl;
		cout << "Potential: $" << investmentList[i].potential << endl;
		cout << endl;
	}
}

struct chromosome initialChromosome(struct investment investmentList[])
//Create generation 0 of parent chromosomes by random assignment of investments.
{
	struct chromosome newChromosome;
	for(int j = 0; j < collection_size; j++)
	{
		int randomAllele = rand() % 100 + 1;
		if (randomAllele > 50)
		{
			newChromosome.sequence[j] = '1';

			//Initialize cost, risk, and potential
			newChromosome.totalCost += investmentList[j].cost;
			//Cannot exceed available funds

			if (newChromosome.totalCost > availableFunds)
			//Divest if funds exceeded
			{
				newChromosome.totalCost -= investmentList[j].cost;
				newChromosome.sequence[j] = '0';
			}
			else
			{
				newChromosome.totalRisk += (investmentList[j].risk * investmentList[j].cost);
				newChromosome.totalPotential += (investmentList[j].potential);
			}
			
		}
		else
		{
			newChromosome.sequence[j] = '0';
		}
	}

	//Calculate dominance
	if (newChromosome.totalRisk == 0)
	//Prevents division by zero
	{
		newChromosome.totalRisk = 1;
	}

	newChromosome.unusedFunds = availableFunds - newChromosome.totalCost; 

	newChromosome.dominance = ceil( ( (newChromosome.totalPotential / newChromosome.totalRisk) / sigCoefficient) - (newChromosome.unusedFunds / (sigCoefficient) ) );
	
	if (newChromosome.dominance < 0)
	{
		newChromosome.dominance = 0;
	}

	newChromosome.dominant = false;
	//Chromosomes will only be considered dominant after dominance values are compared to those of other chromosomes
	return newChromosome;
}

struct chromosome generateChild(struct chromosome parent1, struct chromosome parent2, struct investment investmentList[])
//Generate a child sequence given two parents.  Compare child sequence against original array to determine difference value
{
	struct chromosome child;
	int crossoverPoint = rand() % collection_size;

	//Perform single point crossover
	for (int i = 0; i < crossoverPoint; i++)
	{
		child.sequence[i] = parent1.sequence[i];

	}
	for (int i = crossoverPoint; i < collection_size; i++)
	{
		child.sequence[i] = parent2.sequence[i];
	}

	//Randomly mutate one chromosome at mutation rate
	int mutationChance = rand() % 100;
	if (mutationChance < mutationRate)
	{
		int mutationPoint = rand() % collection_size;
		if (child.sequence[mutationPoint] == '1')
		{
			child.sequence[mutationPoint] = '0';
		}
		else
		{
			child.sequence[mutationPoint] = '1';
		}
	}

	child.totalCost = 0;
	child.totalRisk = 0;
	child.totalPotential = 0;
	child.dominance = 0;

	for (int i = 0; i < collection_size; i++)
	{
		
		if (child.sequence[i] == '1')
		{
			//Calculate cost, risk, and potential
			child.totalCost += investmentList[i].cost;

			//Cannot exceed available funds
			if (child.totalCost > availableFunds)
			//Divest if funds exceeded.
			{
				child.totalCost -= investmentList[i].cost;
				child.sequence[i] = '0';
			}
			else
			{
				child.totalRisk += ceil( (investmentList[i].risk) * investmentList[i].cost );
				child.totalPotential += (investmentList[i].potential);
			}
			
		}
	}

	//Calculate dominance

	if (child.totalRisk == 0)
	//Prevents division by zero
	{
		child.totalRisk = 1;
	}

	child.unusedFunds = availableFunds - child.totalCost;

	child.dominance = ceil( ( (child.totalPotential / child.totalRisk)  / sigCoefficient) - (child.unusedFunds / (sigCoefficient) ) );

	if (child.dominance < 0)
	{
		child.dominance = 0;
	}

	child.dominant = false;
	//Chromosomes are only considered dominant after the dominance values are compared to those of other chromosomes
	return child;
}

int main ()
{

	int collection[collection_size], maxDominance = 0, paretoCount, optionCount = 1, generationCount = 0;
	struct chromosome parents[chromPop], children[chromPop];	
	struct investment investmentList[collection_size];
	bool done = false, timedOut = false;

	srand (time(NULL));
	//Pseudo-random number generator seeded with current time

	clock_t timeStart = clock(), timeEnd;

	populate(investmentList);
	//Populate collection of investments and print to terminal screen.
	
	cout << "Investment list created.  Optimizing portfolio..." << endl;

	for (int i = 0; i < chromPop; i++)
	//Determine max dominance value
	{
		parents[i] = initialChromosome(investmentList);
		if (parents[i].dominance > maxDominance)
		{
			maxDominance = parents[i].dominance;
		}
		
	}
	
	for (int i = 0; i < chromPop; i++)
	//Set parents with the max dominance value as dominant parents
	{
		if (parents[i].dominance >= maxDominance)
		{
			parents[i].dominant = true;
		}
	}

	while(!done)
	//Continue to create generations of children until population of dominant chromosomes is smaller than terminationPop or program times out.
	{
		paretoCount = 0;		
		int position = 0;

		if ( (clock() - timeStart) / CLOCKS_PER_SEC >= timeLimit)
		//Timeout.  Stop generating more children.
		{
			cout << "\nCOMPILE TIME LIMIT REACHED." << endl;
			cout << "No solutions were found to be significantly more optimal than others within allotted time." << endl;
			timeEnd = clock();
			timedOut = true;
			break;
		}

		for (int i = 0; i < chromPop; i++)
		//Select dominant parents from Pareto front only.
		{
			struct chromosome parent1, parent2;
			bool p1 = false, p2 = false;

			while (position < chromPop)
			{
				if (p1 == false)
				//Find first parent
				{
					if (parents[position].dominant == true)
					{
						parent1 = parents[position];
						p1 = true;
					}
					if (position < chromPop-1)
					{
						position++;
					}
					else
					{
						position = 0;
					}
				}
				else
				//Find second parent
				{
					if (parents[position].dominant == true)
					{
						parent2 = parents[position];
						p2 = true;
						break;
					}
					if (position < chromPop-1)
					{
						position++;
					}
					else
					{
						position = 0;
					}
				}
			}

			children[i] = generateChild(parent1, parent2, investmentList);
			//Create child from parents

			if (cEquals(children[i], children[i-1]) == true)
			//One chromosome cannot be the same as the previous.
			{
				i--;
			}
		}

		//Replace parent generation and calculate new maxDominance
		maxDominance = 0;
		for (int i = 0; i < chromPop; i++)
		{
			parents[i] = children[i];
			if (parents[i].dominance > maxDominance)
			{
				maxDominance = parents[i].dominance;
			}
		}

		generationCount++;
		//Determine size of new Pareto front
		for (int i = 0; i < chromPop; i++)
		{
			if (parents[i].dominance >= maxDominance)
			{
				parents[i].dominant = true;
				paretoCount++;
			}
		}

		if (paretoCount <= terminationPop)
		//If Pareto front is smaller than terminationPop, the final list of optimal solutions has been found.  Stop generating children.
		{
			done = true;
		}
	}
	cout << endl;

	if (timedOut == false)
	{
		cout << "Final list of optimal investment combinations compiled successfully within time limit.  These are:\n" << endl;
		for (int i = 0; i < chromPop; i++)
		{
			if (parents[i].dominant == true)
			{
				cout << "Option " << optionCount << ":" << endl;
				cout << "Investments: ";
				for (int j = 0; j < collection_size; j++)
				{
					if (parents[i].sequence[j] == '1')
					{
						cout << j+1 << " ";
					}
				}
				cout << endl;
				cout << "Total potential: $" << parents[i].totalPotential << endl;
				cout << "Total risk: $" << parents[i].totalRisk << endl;
				cout << "Unused funds: $" << parents[i].unusedFunds << endl;
				cout << "Dominance: " << parents[i].dominance << endl;
				cout << endl;
				optionCount++;
			}
		}
	}
	else
	//Time exceeded before a significantly optimal solution was found.  Program will try to offer the best solutions it has found within time limit.
	{
		cout << "Program offers the following solution(s):\n" << endl;
		for (int i = 0; i < terminationPop; i++)
		{
			if (parents[i].dominant == true)
			{
				cout << "Option " << optionCount << ":" << endl;
				cout << "Investments: ";
				for (int j = 0; j < collection_size; j++)
				{
					if (parents[i].sequence[j] == '1')
					{
						cout << j+1 << " ";
					}
				}
				cout << endl;
				cout << "Total potential: $" << parents[i].totalPotential << endl;
				cout << "Total risk: $" << parents[i].totalRisk << endl;
				cout << "Unused funds: $" << parents[i].unusedFunds << endl;
				cout << "Dominance: " << parents[i].dominance << endl;
				cout << endl;
				optionCount++;
			}
		}
	}
	cout << "Total funds available: $" << availableFunds << endl;	
	cout << "Population size: " << chromPop << endl;
	cout << "Pareto termination size: " << terminationPop << endl;
	cout << "Completed in " << generationCount << " generations and " << (timeStart - timeEnd) << " clock ticks" << endl;

	
	return 0;
	
}
