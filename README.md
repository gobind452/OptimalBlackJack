# Optimal BlackJack

Calculates the optimal action for any player and dealer initial card configuration in BlackJack modeling the game as a Markov Decision Process and using policy iteration

Rules - https://wizardofodds.com/games/blackjack/basics/ (Surrender not allowed in the code)

## Details
+ The only variable is the probability of a face card (Jack,Knight,Queen,10). This can be between 0 and 0.5. Higher probabilities will lead to diverging rewards, and the program wont converge.
+ Certain actions are not allowed (Surrender, Insurance)
+ On running the code, a policy file will be created with all optimal actions. The actions are encoded as P(Pair Split), D(Double), H(Hit) and S( Stand). The top row will be the dealer totals, and the leftmost column will be the players hand. According to the optimal first action will be written.


## Schematics
+ main.cpp - Contains all the logic
+ compile.sh - Compile script
+ run.sh - Takes the probability of a face card and outputs a file with optimal actions for all possible player-dealer hands
