#include<iostream>
#include<vector>
#include<string>
#include<cstdlib>
#include<unordered_map>
#include<cmath>
#include<ctime>
#include<algorithm>
#include<fstream>

using namespace std;

double faceProb;
double normalProb;

double minf = -1000;

double getProb(int value){
	if(value == 10){
		return faceProb;
	}
	return normalProb;
}

string createHand(int card1,int card2){
	if(card1 == 11 and card2 == 11){
		return "AA";
	}
	else if(card1+card2 == 21){
		return "BL";
	}
	else if(card1 == 11 or card2 == 11){
		int a = (card1 == 11)?card2:card1;
		return "A" + to_string(a);
	}
	else if(card1 == card2){
		return to_string(card1) + to_string(card2);
	}
	else {
		return to_string(card1 + card2);
	}
}

typedef struct Neighbor{
	Neighbor(double prob,int state):prob(prob),state(state){}
	double prob;
	int state;
}Neighbor;

typedef struct Action{
	Action(double reward = 0):reward(reward){};
	double reward;
	vector<Neighbor> neighbors;
}Action;

unordered_map<int,string> intToString;
unordered_map<string,int> stringToInt;
unordered_map<int,string> actionsMap;

void initMaps(){
	for(int i=0;i<17;i++){
		int state = i+5;
		intToString[i] = to_string(state);
		stringToInt[to_string(state)] = i;
	}
	int count = 2;
	for(int i=17;i<26;i++){
		string state = "A" + to_string(count);
		intToString[i] = state;
		stringToInt[state] = i;
		count++;
	}
	count = 2;
	for(int i=26;i<35;i++){
		string state = to_string(count)+to_string(count);
		intToString[i] = state;
		stringToInt[state] = i;
		count++;
	}
	intToString[35] = "AA";
	stringToInt["AA"] = 35;
	intToString[36] = "B";
	stringToInt["B"] = 36;
	intToString[37] = "BL";
	stringToInt["BL"] = 37;
	actionsMap[0] = "H";
	actionsMap[1] = "S";
	actionsMap[2] = "D";
	actionsMap[3] = "P";
}

double dealerMatrix[10][7]; // 17,18,19,20,21,B,BL

class Hand{
public:
	string state;
	int value;
	Hand(string state){
		this->state = state;
		if(state == "BL"){
			value = 21;
		}
		else if(state == "B"){
			value = 0;
		}
		else if(state.at(0) == 'A'){ // Ace
			if(state.at(1) == 'A'){
				value = 12;
				return;
			}
			string temp = state.substr(1);
			value = 11 + atoi(temp.c_str());
		}
		else if(state.length() == 4){ // 1010
			value = 20;
		}
		else if(atoi(state.c_str())%11 == 0){ // 11,22,33,44,55,66..
			if(atoi(state.c_str()) == 11){ //11
				value = 11;
			}
			else value = 2*(atoi(state.c_str())/11); // 22,33,44.. 
		}
		else value = atoi(state.c_str()); // 1,2,3,4..
	}
	void addNewCard(int card){
		if(state == "B"){
			return;
		}
		else if(state.at(0) == 'A'){
			if(state.at(1) == 'A'){
				card = (card == 11)?1:card; // Only one ace can be 11
				card++;
				if(card >=11){
					state = to_string(++card); // Flip the one ace to 1 
					value = card;
					return;
				}
				else{
					state = "A" + to_string(card);
					value = 11 + card;
					return;
				}
			}
			string temp = state.substr(1);
			int score = atoi(temp.c_str());
			card = (card == 11)?1:card; // Only one ace can be 11
			score = score+card;
			if(score > 10){
				state = to_string(score+1); // Take the initial ace to be 1
				value = score+1;
				return;
			}
			else {
				state = "A"+ to_string(score);
				value = score+11;
			}
		}
		else if(atoi(state.c_str())%11 == 0){
			int str = atoi(state.c_str());
			if(str == 1010){
				card = (card == 11)?1:card;
				if(card>1){
					state = "B";
					value = 0;
					return;
				}
				state = "21";
				value = 21;
			}
			else if(str == 11){
				card = (card == 11)?1:card;
				state = to_string(11 + card);
				value = 11 + card;
			}
			else{
				card = (card == 11 and value > 10)?1:card;
				value = value + card;
				if(value >21){
					state = "B";
					value = 0;
					return;
				}
				if(card == 11){
					state = "A" + to_string(value-card);
				}
				else state = to_string(value);
			}
		}
		else{
			card = (card == 11 and value > 10)?1:card;
			value = value+card;
			if(value > 21){
				state = "B";
				value = 0;
				return;
			}
			if(card == 11){
				state = "A" + to_string(value -card);
			}
			else state = to_string(value);
		}
	}
	double standReward(int dealerCard){
		if(state == "Bl"){
			return 1.5;
		}
		else if(state == "B"){
			return -1;
		}
		else if(value < 17){
			return (dealerMatrix[dealerCard-2][5]) - (1-dealerMatrix[dealerCard-2][5]); // Reward only if bust; 
		}
		else{
			double reward = 0;
			int pos = value-17;
			for(int i=0;i<pos;i++){
				reward = reward + dealerMatrix[dealerCard-2][i]; // Win 
			}
			for(int i=pos+1;i<5;i++){
				reward = reward - dealerMatrix[dealerCard-2][i]; //Lose
			}
			reward = reward + dealerMatrix[dealerCard-2][5]; // Dealer busts
			reward = reward - dealerMatrix[dealerCard-2][6]; // Dealer blackjack
		}
	}
	Hand split(int newCard1,int newCard2){
		if(state == "AA"){
			string newHand1 = createHand(11,newCard1);
			if(newHand1 == "BL"){
				newHand1 = "21";
			}
			string newHand2 = createHand(11,newCard2);
			if(newHand2 == "BL"){
				newHand2 = "21";
			}
			Hand temp = Hand(newHand1);
			state = temp.state;
			value = temp.value;
			return Hand(newHand2);
		}
		int card = value/2;
		string newHand1 = createHand(card,newCard1);
		string newHand2 = createHand(card,newCard2);
		Hand temp = Hand(newHand1);
		state = temp.state;
		value = temp.value;
		return Hand(newHand2);
	}
};

bool sortHands(Hand& hand1,Hand& hand2){
	if(hand1.value <= hand2.value){
		return true;
	}
	return false;
}

bool sortAces(Hand& hand1,Hand& hand2){
	if(hand1.state.at(0) == 'A' and hand2.state.at(0) == 'A'){
		return sortHands(hand1,hand2);
	}
	else if(hand1.state.at(0) == 'A'){
		return true;
	}
	return false;
}
void initDealer(){
	double probMatrix[38][7] = {0};
	vector<Hand> hands;
	for(int i=0;i<37;i++){
		hands.push_back(Hand(intToString[i]));
		if(hands[i].value > 16){
			probMatrix[i][hands[i].value-17] = 1;
		}
	}
	probMatrix[stringToInt["B"]][5] = 1;
	sort(hands.begin(),hands.end(),sortHands);
	hands.push_back(Hand("BL"));
	probMatrix[stringToInt["BL"]][6] = 1;
	sort(hands.begin()+12,hands.begin()+24,sortAces);	
	for(int i =24;i>=0;i--){
		Hand temp = hands[i];
		int pos1 = stringToInt[temp.state];
		for(int addCard = 2;addCard<=11;addCard++){
			temp.addNewCard(addCard);
			int pos2 = stringToInt[temp.state];
			for(int j=0;j<7;j++){
				probMatrix[pos1][j] = probMatrix[pos1][j] + getProb(addCard)*probMatrix[pos2][j];
			}
			temp = hands[i];
		}
	}
	for(int i=2;i<=11;i++){
		for(int secondCard = 2;secondCard<=11;secondCard++){
			string hand = createHand(i,secondCard);
			for(int j=0;j<7;j++){
				dealerMatrix[i-2][j] = dealerMatrix[i-2][j] + getProb(secondCard)*probMatrix[stringToInt[hand]][j];
			}
		}
	}
	return;
}

class MDP{
public:
	int policy2[38]; // For 2 card states
	int policy3[37]; // For 3 or more card states
	double value2[38]; // For 2 card states
	double value3[37]; // For 3 or more card states
	Action encoding2[38][4];
	Action encoding3[37][2];
	MDP(int dealerCard){
		initPolicy();
		initEncoding();
		initFixedRewards(dealerCard);
	}
	void initPolicy(){
		for(int i=0;i<26;i++){
			policy2[i] = rand()%3; // Hit,stand or double down
		}
		for(int i=26;i<36;i++){
			policy2[i] = rand()%4; // Hit stand double or spllit
		}
		policy2[36] = 1;
		policy2[37] = 1;
		for(int i=0;i<36;i++){
			policy3[i] = rand()%2; // Hit or stand
		}
		policy3[36] = 1;
	}
	void initEncoding(){
		for(int i=0;i<36;i++){ // Hit equations
			vector<Neighbor>& tempVector = encoding2[i][0].neighbors;
			Hand hand = Hand(intToString[i]);
			for(int j=2;j<=11;j++){
				Hand temp = hand;
				temp.addNewCard(j);
				tempVector.push_back(Neighbor(getProb(j),stringToInt[temp.state]));
			}
		}
		for(int i=26;i<36;i++){ // Split equations
			vector<Neighbor>& tempVector = encoding2[i][3].neighbors;
			Hand hand = Hand(intToString[i]);
			for(int j=2;j<=11;j++){
				for(int k=2;k<=11;k++){
					Hand temp = hand;
					Hand temp1 = temp.split(j,k);
					tempVector.push_back(Neighbor(getProb(j)*getProb(k),stringToInt[temp.state]));
					tempVector.push_back(Neighbor(getProb(j)*getProb(k),stringToInt[temp1.state]));
				}
			}
		}
		for(int i=0;i<36;i++){ // Only the hit equations
			vector<Neighbor>& tempVector = encoding3[i][0].neighbors;
			Hand hand = Hand(intToString[i]);
			for(int j=2;j<=11;j++){
				Hand temp = hand;
				temp.addNewCard(j);
				tempVector.push_back(Neighbor(getProb(j),stringToInt[temp.state]));
			}
		}
	}
	void initFixedRewards(int dealerCard){
		for(int i=0;i<37;i++){ // Stand Rewards
			Hand hand = Hand(intToString[i]);
			encoding2[i][1].reward = hand.standReward(dealerCard);
			encoding3[i][1].reward = hand.standReward(dealerCard); // Stand rewards same for 2 and more than 2
		}
		encoding2[37][1].reward = 1.5; // Blackjack reward
		for(int i=0;i<38;i++){
			Hand hand = Hand(intToString[i]);
			vector<Neighbor>& tempVector = encoding2[i][0].neighbors;
			double answer = 0;
			for(int j=0;j<tempVector.size();j++){
				answer = answer + 2*tempVector[j].prob*(encoding3[tempVector[j].state][1].reward); // Double down rewards			}
			}
			encoding2[i][2].reward = answer; // Double down rewards
		}
		for(int i=0;i<3;i++){
			encoding2[stringToInt["B"]][i].reward = -1;
			encoding2[stringToInt["BL"]][i].reward = 1.5;
		}
		for(int i=0;i<2;i++){
			encoding3[stringToInt["B"]][i].reward = -1;
		}
		double answer = 0; // Fixed reward for AA split
		vector<Neighbor>& tempVector = encoding2[stringToInt["AA"]][3].neighbors;
		for(int j=0;j<tempVector.size();j++){
			answer = answer + tempVector[j].prob*(encoding2[tempVector[j].state][1].reward);
		}
		encoding2[stringToInt["AA"]][3].reward = answer;
	}
	double calculateValueAction(int state,int action,double* tempValue2 = 0,double* tempValue3 = 0){
		if(action == 1 or action == 2){
			return encoding2[state][action].reward;
		}
		else if(intToString[state] == "B" or intToString[state] == "BL"){
			return encoding2[state][action].reward;
		}
		else if(action == 0){ // Hit
			if(tempValue3 == 0){
				tempValue3 = value3;
			}
			vector<Neighbor>& tempVector = encoding2[state][action].neighbors;
			double answer = 0;
			for(int i=0;i<tempVector.size();i++){
				answer = answer + tempVector[i].prob*(tempValue3[tempVector[i].state]); // Hit always maps to 3 cards
			}
			return answer;
		}
		else if(action == 3){
			if(intToString[state] == "AA"){
				return encoding2[stringToInt["AA"]][3].reward;
			}
			if(tempValue2 == 0){
				tempValue2 = value2;
			}
			vector<Neighbor>& tempVector = encoding2[state][action].neighbors;
			double answer = 0;
			for(int i=0;i<tempVector.size();i++){
				answer = answer + tempVector[i].prob*(tempValue2[tempVector[i].state]); // Split always maps to 2 cards
			}
			return answer;
		}
	}
	void evaluatePolicy(){
		double tempValue2[2][38];
		double tempValue3[2][37];
		for(int i=0;i<38;i++){
			tempValue2[0][i] = rand()%100;
		}
		for(int i=0;i<37;i++){
			tempValue3[0][i] = rand()%100;
		}
		int a = 0;
		int b = 1;
		bool converged = 0;
		double maxDiff = 0;
		double parameter = 0.1;
		while(converged == 0){
			for(int i=0;i<38;i++){
				tempValue2[b][i] = calculateValueAction(i,policy2[i],tempValue2[a],tempValue3[a]); 
				if(abs(tempValue2[b][i] - tempValue2[a][i])>maxDiff){
					maxDiff = abs(tempValue2[b][i] - tempValue2[a][i]);
				}
			}
			for(int i=0;i<37;i++){
				tempValue3[b][i] = calculateValueAction(i,policy3[i],tempValue2[a],tempValue3[a]);
				if(abs(tempValue3[b][i] - tempValue3[a][i])>maxDiff){
					maxDiff = abs(tempValue3[b][i] - tempValue3[a][i]);
				}
			}
			//decoyAAValue = calculateValueAction(stringToInt["AA"],decoyAA,tempValue2[a],tempValue3[a]);
			if(maxDiff < parameter){
				converged = 1;
			}
			maxDiff = 0;
			a = (a+1)%2;
			b = (b+1)%2;
		}
		for(int i=0;i<38;i++){
			value2[i] = tempValue2[0][i];
		}
		for(int i=0;i<37;i++){
			value3[i] = tempValue3[0][i];
		}
		return;
	}
	int bestAction(int state,int cards){
		int maxAction = 0;
		double max = minf;
		int actions = 3;
		if(cards == 3){
			actions = 2;
		}
		else if(cards == 2 and state >=26 and state <36){
			actions = 4;
		}
		for(int i=0;i<actions;i++){
			double answer = calculateValueAction(state,i);
			if(answer>max){
				max = answer;
				maxAction = i;
			}
		}
		return maxAction;
	}
	bool oneLookPolicy(){
		bool changed = 0;
		for(int i=0;i<36;i++){
			int action = bestAction(i,2);
			if(action != policy2[i]){
				changed = 1;
				policy2[i] = action;
			}
		}
		for(int i=0;i<36;i++){
			int action = bestAction(i,3);
			if(action != policy3[i]){
				changed = 1;
				policy3[i] = action;
			}
		}
		return changed;
	}
	void printPolicy(){
		for(int i=0;i<38;i++){
			cout << intToString[i] << " " << actionsMap[policy2[i]] << " ";
		}
		cout << endl;
	}
	void printValues(){
		for(int i=0;i<38;i++){
			cout << value2[i] << " ";
		}
		cout << endl;
	}
	void policyIteration(){
		bool changed = 1;
		while(changed == 1){
			evaluatePolicy();
			changed = oneLookPolicy();
		}
	}
};

unordered_map<string,vector<string> > optimal;

int main(int argc,char* argv[]){
	srand(time(0));
	faceProb = atof(argv[1]);
	normalProb = (1-faceProb)/9;
	initMaps();
	initDealer();
	for(int j=2;j<12;j++){
		MDP mdp = MDP(j);
		mdp.policyIteration();
		for(int i=5;i<20;i++){
			optimal[to_string(i)].push_back(actionsMap[mdp.policy2[stringToInt[to_string(i)]]]);
		}
		for(int i=2;i<10;i++){
			optimal["A"+to_string(i)].push_back(actionsMap[mdp.policy2[stringToInt["A"+to_string(i)]]]);
		}
		for(int i=2;i<11;i++){
			optimal[to_string(i)+to_string(i)].push_back(actionsMap[mdp.policy2[stringToInt[to_string(i)+to_string(i)]]]);
		}
		optimal["AA"].push_back(actionsMap[mdp.policy2[stringToInt["AA"]]]);
	}
	ofstream output;
	output.open("Policy.txt");
	for(int i=5;i<20;i++){
		output << to_string(i) << "\t";
		for(int j=0;j<10;j++){
			output << optimal[to_string(i)][j] << " ";
		}
		output << endl;
	}
	for(int i=2;i<10;i++){
		output << "A"+to_string(i) << "\t";
		for(int j=0;j<10;j++){
			output << optimal["A"+to_string(i)][j] << " ";
		}
		output << endl;
	}
	for(int i=2;i<11;i++){
		output << to_string(i)+to_string(i) << "\t";
		for(int j=0;j<10;j++){
			output << optimal[to_string(i)+to_string(i)][j] << " ";
		}
		output << endl;
	}
	output << "AA\t";
	for(int j=0;j<10;j++){
		output << optimal["AA"][j] << " ";
	}
	output.close();
	return 0;
}