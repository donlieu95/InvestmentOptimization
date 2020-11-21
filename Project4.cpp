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
//Speed and optimality will be compared to the hill-climbing heuristic.

using namespace std;

const int collection_size = 30, minRand = 1, maxRand = 2000, timeLimit = 5;
//Parameters can be manipulated by user
int availableFunds = ceil(maxRand * (collection_size / 4));
//Available funds as a function of maximum random value and collection size
int chromPop = ceil(1.65 * pow(2, (0.21 * collection_size)));
//Optimal population size by Goldberg
int terminationPop = ceil( log(chromPop) );
//Final Pareto front size increases logarithmically with population size
int mutationRate = 50;
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
		int exp = rand() % 2 + 1;
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
				newChromosome.totalRisk += ( (investmentList[j].risk * investmentList[j].cost) / 100);
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
				child.totalRisk += ceil( ( (investmentList[i].risk) * investmentList[i].cost ) / 100 );
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

struct chromosome hillClimb(struct chromosome climber, int position, struct investment investmentList[])
{
	struct chromosome option1 = climber, option2 = climber;
	//Set both options identical to original candidate
	//Modify second option at passed position

	if (option2.sequence[position] == '1')
	{
		option2.sequence[position] = '0';
	}
	else
	{
		option2.sequence[position] = '1';
	}

	for (int i = 0; i < collection_size; i++)
	{
		
		if (option2.sequence[i] == '1')
		{
			//Calculate cost, risk, and potential
			option2.totalCost += investmentList[i].cost;

			//Cannot exceed available funds
			if (option2.totalCost > availableFunds)
			//Return original candidate.
			{
				return option1;
			}
			else
			{
				option2.totalRisk += ceil( ( (investmentList[i].risk) * investmentList[i].cost ) / 100 );
				option2.totalPotential += (investmentList[i].potential);
			}
			
		}
	}

	//Calculate dominance

	if (option2.totalRisk == 0)
	//Prevents division by zero
	{
		option2.totalRisk = 1;
	}

	option2.unusedFunds = availableFunds - option2.totalCost;

	option2.dominance = ceil( ( (option2.totalPotential / option2.totalRisk)  / sigCoefficient) - (option2.unusedFunds / (sigCoefficient) ) );

	if (option2.dominance < 0)
	{
		option2.dominance = 0;
	}

	option2.dominant = false;
	//Chromosomes are only considered dominant after the dominance values are compared to those of other chromosomes

	if (option2.dominance > option1.dominance)
	{
		return option2;
	}
	else
	{
		return option1;
	}
}

int main ()
{

	int collection[collection_size], maxDominance = 0, paretoCount, optionCount = 1, generationCount = 0, geneticTime, hillClimbingTime;
	struct chromosome parents[chromPop], children[chromPop];	
	struct investment investmentList[collection_size];
	bool done = false, timedOut = false;

	srand (time(NULL));
	//Pseudo-random number generator seeded with current time

	clock_t timeStart = clock(), timeEnd;

	populate(investmentList);
	//Populate collection of investments and print to terminal screen.
	
	cout << "Investment list created.  Optimizing portfolio with genetic algorithm..." << endl;

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
			cout << "Could not find a set of " << terminationPop << " solutions significantly more optimal than others within allotted time." << endl;
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
	timeEnd = clock();
	geneticTime = (timeEnd - timeStart);
	cout << endl;
	
	//Print genetic algorithm results
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
	cout << "Pareto termination size: " << terminationPop << endl;
	cout << "Completed in " << generationCount << " generations and " << geneticTime << " clock ticks" << endl;
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
	cout << "Pareto termination size: " << terminationPop << endl;
	cout << "Completed in " << generationCount << " generations and " << geneticTime << " clock ticks" << endl;
	}

	//Now perform hill-climbing to compare results
	cout << "\nOptimizing portfolio with hill-climbing heuristic..." << endl;
	struct chromosome hillClimber = initialChromosome(investmentList);
	//Start with a random assignment of investments

	timedOut = false;
	timeStart = clock(), timeEnd;

	for (int i = 0; i < collection_size; i++)
	//Perform hill-climbing until done or time limit exceeded.
	{
		if ( (clock() - timeStart) / CLOCKS_PER_SEC >= timeLimit)
		//Timeout.  Stop hill-climbing.
		{
			cout << "\nCOMPILE TIME LIMIT REACHED." << endl;
			cout << "Could not find local maxima solution within allotted time." << endl;
			timedOut = true;
			break;
		}
		hillClimber = hillClimb(hillClimber, i, investmentList);
	}
	timeEnd = clock();
	hillClimbingTime = (timeEnd - timeStart);

	//Print hill-climbing results
	if (timedOut == false)
	{
		cout << "Local maxima solution found successfully within time limit:\n" << endl;
		cout << "Investments: ";
		for (int j = 0; j < collection_size; j++)
		{
			if (hillClimber.sequence[j] == '1')
			{
				cout << j+1 << " ";
			}
		}
		cout << endl;
		cout << "Total potential: $" << hillClimber.totalPotential << endl;
		cout << "Total risk: $" << hillClimber.totalRisk << endl;
		cout << "Unused funds: $" << hillClimber.unusedFunds << endl;
		cout << "Dominance: " << hillClimber.dominance << endl;
		cout << "Completed in " << hillClimbingTime << " clock ticks" << endl;
		cout << endl;
	}
	else
	//Time exceeded before a significantly optimal solution was found.  Program will try to offer the best solutions it has found within time limit.
	{
		cout << "Program offers the following solution:\n" << endl;
		cout << "Investments: ";
		for (int j = 0; j < collection_size; j++)
		{
			if (hillClimber.sequence[j] == '1')
			{
				cout << j+1 << " ";
			}
		}
		cout << endl;
		cout << "Total potential: $" << hillClimber.totalPotential << endl;
		cout << "Total risk: $" << hillClimber.totalRisk << endl;
		cout << "Unused funds: $" << hillClimber.unusedFunds << endl;
		cout << "Dominance: " << hillClimber.dominance << endl;
		cout << "Completed in " << hillClimbingTime << " clock ticks" << endl;
		cout << endl;
	}

	//Compare optimality of genetic algorithm and hill-climbing
	if (maxDominance > hillClimber.dominance)
	{
		cout << "Genetic algorithm found a more optimal solution within time limit." << endl;
	}
	else if (hillClimber.dominance > maxDominance)
	{
		cout << "Hill-climbing found a more optimal solution within time limit." << endl;
	}
	else
	{
		cout << "No significant difference in optimality was found between genetic algorithm and hill-climbing within time limit." << endl;
	}

	//Compare execution time of genetic algorithm and hill-climbing
	if (geneticTime < hillClimbingTime)
	{
		cout << "Genetic algorithm finished " << (hillClimbingTime - geneticTime) << " clock ticks faster than hill-climbing.\n" << endl;
	}
	else if (hillClimbingTime < geneticTime)
	{
		cout << "Hill-climbing finished " << (geneticTime - hillClimbingTime) << " clock ticks faster than genetic algorithm.\n" << endl;
	}
	else
	{
		cout << "Genetic algorithm and hill-climbing both finished in " << geneticTime << " clock ticks.\n" << endl;
	}

	
	cout << "Total funds available: $" << availableFunds << endl;	
	cout << "Population size: " << chromPop << endl;

	
	return 0;
	
}
