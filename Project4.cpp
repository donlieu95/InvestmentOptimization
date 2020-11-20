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

using namespace std;

const int collection_size = 60, minRand = 1, maxRand = 2000, timeLimit = 60;
int availableFunds = ceil(maxRand * (collection_size / 4));
int chromPop = ceil(1.65 * pow(2, (0.21 * collection_size)));
int terminationPop = ceil(chromPop * 0.00025);
int mutationRate = ceil(chromPop * 0.02);
int sigCoefficient = ceil( maxRand * .2 );

struct chromosome
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
//Polulate initial array
{
	srand (time(NULL));
	int multiplier;
	for(int i = 0; i < collection_size; i++)
	{
		int exp = rand() % 3 + 1;
		//multiplier = rand() % 5 + 0;
		investmentList[i].cost = rand() % maxRand + minRand;
		investmentList[i].risk = rand() % 100 + 1;
		//investmentList[i].potential = ceil( investmentList[i].cost * (pow( (1+(investmentList[i].risk / 100) ), exp) ) ); 
		//investmentList[i].potential = ceil( investmentList[i].cost + (investmentList[i].cost * (investmentList[i].risk / 100) * multiplier ) );
		investmentList[i].potential = ceil( investmentList[i].cost * (pow( ((100 / (100 - investmentList[i].risk) ) ), exp) ) );

		cout << "Investment " << i+1 << ":" << endl;
		cout << "Cost: $" << investmentList[i].cost << endl;
		cout << "Risk: " << investmentList[i].risk << "%" << endl;
		cout << "Potential: $" << investmentList[i].potential << endl;
		//cout << "Exp: " << exp << "Multipler: " << (pow( ((100 / (100 - investmentList[i].risk) ) ), exp) ) << endl;
		cout << endl;
	}
}

struct chromosome initialChromosome(struct investment investmentList[])
{
	struct chromosome newChromosome;
	//srand (time(NULL));
	for(int j = 0; j < collection_size; j++)
	{
		int randomAllele = rand() % 100 + 1;
		if (randomAllele > 50)
		{
			newChromosome.sequence[j] = '1';
			//Initialize cost, risk, and potential
			newChromosome.totalCost += investmentList[j].cost;
			if (newChromosome.totalCost > availableFunds)
			//Cannot exceed available funds
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
		//cout << newChromosome.sequence[j];
	}
	//Calculate dominance
	if (newChromosome.totalRisk == 0)
	{
		newChromosome.totalRisk = 1;
	}
	newChromosome.unusedFunds = availableFunds - newChromosome.totalCost; 
	newChromosome.dominance = ceil( ( (newChromosome.totalPotential / newChromosome.totalRisk) - (newChromosome.unusedFunds * ( collection_size/5 ) ) ) / sigCoefficient);
	
	if (newChromosome.dominance < 0)
	{
		newChromosome.dominance = 0;
	}

	newChromosome.dominant = false;
	return newChromosome;
}

struct chromosome generateChild(struct chromosome parent1, struct chromosome parent2, struct investment investmentList[])
//Generate a child sequence given two parents.  Compare child sequence against original array to determine difference value
{
	struct chromosome child;
	//int unusedFunds;
	int crossoverPoint = rand() % collection_size;
	//Perform single point crossover
	for (int i = 0; i < crossoverPoint; i++)
	{
		child.sequence[i] = parent1.sequence[i];

	}
	//cout << endl;
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

	/*for (int i = 0; i < collection_size; i++)
	{
		cout << child.sequence[i];
	}*/
	//cout << endl;
	for (int i = 0; i < collection_size; i++)
	{
		
		if (child.sequence[i] == '1')
		{
			//Calculate cost, risk, and potential
			child.totalCost += investmentList[i].cost;
			if (child.totalCost > availableFunds)
			//Cannot exceed available funds.  Divest.
			{
				child.totalCost -= investmentList[i].cost;
				child.sequence[i] = '0';
			}
			else
			{
				child.totalRisk += ceil( (investmentList[i].risk) * investmentList[i].cost );
				//cout << ceil( (investmentList[i].risk) * investmentList[i].cost ) << " added to total risk." << endl;
				child.totalPotential += (investmentList[i].potential);
				//cout << investmentList[i].potential << " added to total potential." << endl;
			}
			
		}
	}
	//cout << endl;
	//Calculate dominance value
	if (child.totalRisk == 0)
	{
		child.totalRisk = 1;
	}
	child.unusedFunds = availableFunds - child.totalCost;
	child.dominance = ceil( ( (child.totalPotential / child.totalRisk) - (child.unusedFunds * (collection_size/5) ) ) / sigCoefficient);

	if (child.dominance < 0)
	{
		child.dominance = 0;
	}

	//cout << "Total Risk: " << child.totalRisk << endl;
	//cout << "Total Potential: " << child.totalPotential << endl;
	//cout << "Unused Funds: " << child.unusedFunds << endl;
	//cout << "Dominance: " << child.dominance << endl;
	child.dominant = false;
	return child;
}

int main ()
{

	int collection[collection_size], maxDominance = 0, paretoCount, optionCount = 1, generationCount = 0;
	struct chromosome parents[chromPop], children[chromPop];	
	struct investment investmentList[collection_size];
	bool done = false, timedOut = false;

	clock_t timeStart = clock();
	populate(investmentList);
	//Populate collection of investments
	
	cout << "Investment list created.  Optimizing portfolio..." << endl;

	for (int i = 0; i < chromPop; i++)
	{
		parents[i] = initialChromosome(investmentList);
		//cout << endl;
		if (parents[i].dominance > maxDominance)
		{
			maxDominance = parents[i].dominance;
		}
	}
	
	
	//cout << "First parent generation successfully populated!  Max dominance: " << maxDominance << endl;
	//cout << "Dominant chromosomes: ";
	
	for (int i = 0; i < chromPop; i++)
	{
		if (parents[i].dominance >= maxDominance)
		{
			parents[i].dominant = true;
			//cout << i << " ";
		}
	}
	//cout << endl;

	while(!done)
	{
		paretoCount = 0;		
		int position = 0;
		//cout << "Generation " << generationCount << ": " << endl;
		if ( (clock() - timeStart) / CLOCKS_PER_SEC >= timeLimit)
		//Timeout
		{
			cout << "\nCOMPILE TIME LIMIT REACHED." << endl;
			timedOut = true;
			break;
		}
		for (int i = 0; i < chromPop; i++)
		{
			struct chromosome parent1, parent2;
			bool p1 = false, p2 = false;
			while (position < chromPop)
			{
				if (p1 == false)
				{
					if (parents[position].dominant == true)
					{
						parent1 = parents[position];
						p1 = true;
						//cout << "Parent 1 at: " << position << endl;
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
				{
					if (parents[position].dominant == true)
					{
						parent2 = parents[position];
						p2 = true;
						//cout << "Parent 2 at: " << position << endl;
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
			//cout << "Child " << i << ": ";
			children[i] = generateChild(parent1, parent2, investmentList);
			if (cEquals(children[i], children[i-1]) == true)
			//One chromosome cannot be the same as the last.
			{
				i--;
			}
			//cout << endl;
		}
		//Replace parent generation and calculate new maxDominance
		maxDominance = 0;
		for (int i = 0; i < chromPop; i++)
		{
			parents[i] = children[i];
			if (parents[i].dominance > maxDominance)
			{
				maxDominance = parents[i].dominance;
				//cout << "New max dominance: " << maxDominance << endl;
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
	{
		cout << "Program has converged on the following dominant solution(s):\n" << endl;
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
	cout << "Completed in " << generationCount << " generations." << endl;

	
	return 0;
	
}