#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>


// Declare function prototypes
char rolldice(void);
void displaydice(void);
void rerolldice(int numtoreroll);
void displayscores(void);
int qsort_compare (const void *num1, const void *num2);
int sumdice(void);


int dice_driver; 									// Holds the dice driver
int uppersectionbonus = 0;							// Holds user's upper section bonus if the upper section sum is greater than 62
char dice[5] = {0,0,0,0,0};							// Holds the player's dice	
int totalscore = 0; 								// Holds the user's total score
int upperscores[6] = {-1,-1,-1,-1,-1,-1};			// Holds user's scores for upper section
int lowerscores[7] = {-1,-1,-1,-1,-1,-1,-1};		// Holds user's scores for lower section


int main(void)
{
	dice_driver = open("/dev/dice", O_RDONLY); // open dice_driver
	if (dice_driver == NULL) 
    {
    	printf("ERROR OPENING DRIVER.\n");
        return 1;
    }

	int choices[13] = {0};			// Holds the player's choices
	int numChoices = 0;				// Holds the number of choices player has made. 
	int numtoreroll;				// Holds the number of dice user wants to reroll
	int upperlower;					// Holds the user's selection for upper or lower
	int selectionupper = 0;			// Holds the user's selection for the upper section
	int selectionlower = 0;			// Holds the user's selection for the lower section

	// Continue the game while the player has not made all
	// 13 choices yet.
	while( numChoices < 14)
	{
		printf("\nYour roll: \n");

		// Roll 5 dice
		int i;
		for(i = 0;  i < 5 ; i = i + 1)
		{
			dice[i] = rolldice();
		}

		displaydice();

		// Get num dice user wants to reroll 
		printf("\nHow many dice do you want to reroll?(Enter # 0 to 5)   ");
		scanf(" %d",&numtoreroll);
		while(numtoreroll < 0 || numtoreroll > 5)
		{
			printf("INPUT ERROR: How many dice do you want to reroll? Enter an integer in the range 0 to 5.   ");
			scanf(" %d",&numtoreroll);
		}

		// Reroll Dice
		if(numtoreroll !=0)
		{
			rerolldice(numtoreroll);
			printf("\nYour second roll:\n");
			displaydice();

			// Get num dice user wants to reroll 
			printf("\nHow many dice do you want to reroll?(Enter # 0 to 5)   ");
			scanf(" %d",&numtoreroll);
			while(numtoreroll < 0 || numtoreroll > 5)
			{
				printf("INPUT ERROR: How many dice do you want to reroll? Enter an integer in the range 0 to 5.   ");
				scanf(" %d",&numtoreroll);
			}

			if(numtoreroll !=0)
			{
				rerolldice(numtoreroll);
				printf("\nYour third roll:\n");
				displaydice();
			}
		}																				
		
		// Ask user which category they'd like to place their points
		printf("\nPlace dice into:\n1) Upper Section\n2) Lower Section\n");
		printf("Selection? Enter 1 or 2:   \n" );
		scanf(" %d",&upperlower);	 

		while(upperlower != 1 && upperlower !=2)
		{
			printf("INPUT ERROR: Selection? Enter 1 or 2:   \n" );
			scanf(" %d",&upperlower);	
		}

		// For this choice, a user will select 1,2,3,4,5,or 6. Based on the choice
		// we will sum how many dice they have that correspond to their choice, and then
		// update the user score accordingly. 
		if(upperlower == 1)
		{
			printf("\nPlace dice into:\n");
			printf("1) Ones\n2) Twos\n3) Threes\n4) Fours\n5) Fives\n6) Sixes\n");
			printf("If you don't have any dice of the selection you make, you'll be given a 0 for that category.\n");
			printf("Selection? Enter a number from 1 to 6:   ");
			scanf(" %d",&selectionupper);
			while(selectionupper < 1 || selectionupper > 6)
			{
				printf("INPUT ERROR:Selection? Enter a number from 1 to 6:   \n");
				scanf(" %d",&selectionupper);
			}
			
			if(upperscores[selectionupper-1] == -1)
			{
				// Sum the user's dice based on selection and update scores
				int m;
				for(m = 0; m < 5; m = m + 1)
				{
					if(upperscores[selectionupper-1] == -1)
					{
						upperscores[selectionupper-1] = 0;
					}

					if(dice[m] == selectionupper)
					{
						totalscore = totalscore + selectionupper;
						upperscores[selectionupper - 1] = upperscores[selectionupper - 1] + selectionupper;
					}
				}
			}

		}
		else
		{
			printf("\nPlace dice into:\n");
			printf("1) Three of a Kind\n2) Four of a Kind\n3) Full House\n4) Small straight\n5) Large straight\n6) Yahtzee\n7) Change\n");
			printf("If you don't have the correct dice for a specific category you pick, you'll be given a 0 for that category.\n");
			printf("Selection? Enter a number from 1 to 7:   ");
			scanf(" %d",&selectionlower);
			while(selectionlower < 1 || selectionlower > 7)
			{
				printf("INPUT ERROR:Selection? Enter a number from 1 to 7:   \n");
				scanf(" %d",&selectionlower);
			}


			//DEBUG
			printf("\n\nPRINTING UNSORTED DICE: [%d,%d,%d,%d,%d]\n\n",dice[0],dice[1],dice[2],dice[3],dice[4]);


			// Sort the dice 
			qsort(dice,5,sizeof(char),qsort_compare);


			//DEBUG
			printf("\n\nPRINTING SORTED DICE: [%d,%d,%d,%d,%d]\n\n",dice[0],dice[1],dice[2],dice[3],dice[4]);



			// No we must calculate the appropirate score based on the user's selection
			// 3 or 4 of a kind: total of all 5 dice
			// full house = 25, small straight = 30, large straight = 40
			// yahtzee = 5, chance = total of all 5 dice
			switch(selectionlower)
			{
				case 1: // Three of a kind
				{
					// Since the dice are sorted, the only way 3 dice can be ordered 
					// in sorted order in a set of five is if dice 1,2,3 or dice 2,3,4 
					// or dice 3,4,5 are equal.
					if( (dice[0]==dice[1] && dice[1]==dice[2]) || 
						(dice[1]==dice[2] && dice[2]==dice[3]) ||
						(dice[2]==dice[3] && dice[3]==dice[4]) )
					{
						// If player already has a score for this category, don't update it
						if(lowerscores[0] == -1)
						{
							totalscore = totalscore + sumdice();
							lowerscores[0] = sumdice();
						}
					}
					else // if they pick this category but they don't meet the condition, give them 0 points
					{
						lowerscores[0] = 0;
					}
					break;
				}
				case 2: // Four of a kind
				{
					// Since the dice are sorted, the only way 4 dice can be ordered 
					// in sorted order in a set of five is if dice 1,2,3,4 or dice 2,3,4,5 
					// are equal.
					if( ((dice[0]==dice[1] && dice[1]==dice[2]) && (dice[2]==dice[3]))  || 
						((dice[1]==dice[2] && dice[2]==dice[3]) && (dice[3]==dice[4])) )
					{
						// If player already has a score for this category, don't update it
						if(lowerscores[1] == -1)
						{		
							totalscore = totalscore + sumdice();
							lowerscores[1] = sumdice();	
						}
					}
					else // if they pick this category but they don't meet the condition, give them 0 points
					{
						lowerscores[1] = 0;
					}
					break;
				}
				case 3: // Full house
				{
					// Since the dice are sorted, the only way a 3 of a kind and two of a kind can
					// occur is if the first two dice are equal and the next 3 are equal OR if the first three
					// dice are equal and the last two dice are equal.
					if( (dice[0]==dice[1] && (dice[2]==dice[3] && dice[3]==dice[4])) ||
						((dice[0]==dice[1] && dice[1]==dice[2]) && dice[3]==dice[4]) )
					{
						// If player already has a score for this category, don't update it
						if(lowerscores[2] == -1)
						{
							totalscore = totalscore + 25;
							lowerscores[2] = 25;
						}
					}
					else // if they pick this category but they don't meet the condition, give them 0 points
					{
						lowerscores[2] = 0;
					}
					break;
				}
				case 4: // Small straight
				{
					// This occurs when dice 1 2 3 and 4 are in sorted order or when dice 2 3 4 5 are in sorted order.
					if( ((dice[1]==dice[0]+1) && (dice[2]==dice[0]+2) && (dice[3]==dice[0]+3)) ||
						((dice[2]==dice[1]+1) && (dice[3]==dice[1]+2) && (dice[4]==dice[1]+3)) )
					{
						// If player already has a score for this category, don't update it
						if(lowerscores[3] == -1)
						{
							totalscore = totalscore + 30;
							lowerscores[3] = 30;
						}
					}
					else // if they pick this category but they don't meet the condition, give them 0 points
					{
						lowerscores[3] = 0;
					}
					break;
				}
				case 5: //Large straight
				{
					// This occurs when all 5 dice are in sorted order
					if( (dice[1]==dice[0]+1) && (dice[2]==dice[0]+2) && (dice[3]==dice[0]+3) && (dice[4]==dice[0]+4) )
					{
						// If player already has a score for this category, don't update it
						if(lowerscores[4] == -1)
						{
							totalscore = totalscore + 40;
							lowerscores[4] = 40;
						}
					}
					else // if they pick this category but they don't meet the condition, give them 0 points
					{
						lowerscores[4] = 0;
					}
					break;
				}
				case 6: //Yahtzee
				{	
					// Occurs when all dice are the same
					if(dice[0]==dice[1] && dice[1]==dice[2] && dice[2]==dice[3] && dice[3]==dice[4])
					{
						// If player already has a score for this category, don't update it
						if(lowerscores[5] == -1)
						{
							totalscore = totalscore + 50;
							lowerscores[5] = 50;
						}
					}
					else // if they pick this category but they don't meet the condition, give them 0 points
					{
						lowerscores[5] = 0;
					}
					break;
				}
				default: // chance
				{	
					// add sum of dice to total sum
					if(lowerscores[6] == -1)
					{
						totalscore = totalscore + sumdice();
						lowerscores[6] = sumdice();
					}
					else // if they pick this category but they don't meet the condition, give them 0 points
					{
						lowerscores[6] = 0;
					}
					break;
				}
			}
		} 


		// Add bonus if upper section points are greater than or equal to 63
		if(sumdice() > 62)
		{
			if(uppersectionbonus == 0)
			{
				uppersectionbonus = 35;
				totalscore = totalscore + 35;
			}
		} 
		displayscores();

		numChoices = numChoices + 1;
	}


}


// This function will generate dice rolls
// and pass a byte value of 0 to 5 to main.
char rolldice(void)
{	
	unsigned char randbyte;

	// generate rand byte from 0 to 5
	read(dice_driver,&randbyte,1);

	return randbyte + 1;
}


// This funciton display's the user's die
// to the console
void displaydice(void)
{
	printf("%d %d %d %d %d\n",dice[0],dice[1],dice[2],dice[3],dice[4]);
}


// This function will reroll the user's dice
void rerolldice(int numtoreroll)
{
	// Holds the index of the dice the user wants to reroll
	int diceindex;
	int j;
	for(j = 0; j < numtoreroll; j = j + 1)
	{
		switch (j)
		{
			case 0:
			{
				printf("What is the first dice you want to reroll?   ");
				scanf(" %d",&diceindex);
	
				// We subtract one from the user's choice becuase our dice[]
				// array starts at index 0
				diceindex = diceindex - 1;
				while(diceindex < 0 || diceindex > 5)
				{
					printf("INPUT ERROR: What is the first dice you want to reroll?   ");
					scanf(" %d",&diceindex);
				}

				dice[diceindex] = rolldice();
				break;		
			}
			case 1:
			{
				printf("What is the second dice you want to reroll?   ");
				scanf(" %d",&diceindex);

				// We subtract one from the user's choice becuase our dice[]
				// array starts at index 0
				diceindex = diceindex - 1;
				while(diceindex < 0 || diceindex > 5)
				{
					printf("INPUT ERROR: What is the second dice you want to reroll?   ");
					scanf(" %d",&diceindex);
				}

				dice[diceindex] = rolldice();
				break;		
			}
			case 2:
			{
				printf("What is the third dice you want to reroll?   ");
				scanf(" %d",&diceindex);

				// We subtract one from the user's choice becuase our dice[]
				// array starts at index 0
				diceindex = diceindex - 1;
				while(diceindex < 0 || diceindex > 5)
				{
					printf("INPUT ERROR: What is the third dice you want to reroll?   ");
					scanf(" %d",&diceindex);
				}

				dice[diceindex] = rolldice();
				break;		
			}
			case 3:
			{
				printf("What is the fourth dice you want to reroll?   ");
				scanf(" %d",&diceindex);

				// We subtract one from the user's choice becuase our dice[]
				// array starts at index 0
				diceindex = diceindex - 1;
				while(diceindex < 0 || diceindex > 5)
				{
					printf("INPUT ERROR: What is the fourth dice you want to reroll?   ");
					scanf(" %d",&diceindex);
				}

				dice[diceindex] = rolldice();
				break;		
			}
			default:
			{
				printf("What is the fifth dice you want to reroll?   ");
				scanf(" %d",&diceindex);

				// We subtract one from the user's choice becuase our dice[]
				// array starts at index 0
				diceindex = diceindex - 1;
				while(diceindex < 0 || diceindex > 5)
				{
					printf("INPUT ERROR: What is the fifth dice you want to reroll?   ");
					scanf(" %d",&diceindex);
				}

				dice[diceindex] = rolldice();
				break;		
			}
		}
	}	
}


// This function will display the user's score and the score tables
void displayscores(void)
{
	// Print current results to user and updata scores
	printf("\nNOTE: -1 represents categories you have not chosen yet.\n");
	printf("Your score so far is: %d\n",totalscore);
	printf("\nOnes: %d\t\tFours: %d\nTwos: %d\t\tFives: %d\nThrees: %d\t\tSixes: %d\n",
			upperscores[0],upperscores[3],upperscores[1],upperscores[4],upperscores[2],upperscores[5]);
	
	printf("Upper section bonus: %d\n",uppersectionbonus);
	printf("Three of a Kind: %d\tFour of a Kind: %d\nSmall Straight: %d\tLarge Straight:%d\nFull House: %d\t\tYahtzee: %d\nChance: %d\n",
		lowerscores[0],lowerscores[1],lowerscores[3],lowerscores[4],lowerscores[2],lowerscores[5],lowerscores[6]);
}


int qsort_compare (const void * a, const void * b)
{
    if( *(char *)a == *(char *)b) return 0;
    if( *(char *)a > *(char *)b) return 1;
    if( *(char *)a < *(char *)b) return -1;
}

// This function will return the sum of the dice
int sumdice(void)
{
	return dice[0] + dice[1] + dice[2] + dice[3] + dice[4];
}







