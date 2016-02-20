#include "cube.h"
#include <iostream>
#include <algorithm>
#include <queue>
#include <ctime>

using namespace std;

// Vectors containing numbers that correspond to faces
const vector<char> Cube::FRONT_EDGES({ YG, OG, WG, RG });
const vector<char> Cube::RIGHT_EDGES({ YO, OB, WO, OG });
const vector<char> Cube::BACK_EDGES({ YB, RB, WB, OB });
const vector<char> Cube::LEFT_EDGES({ YR, RG, WR, RB });
const vector<char> Cube::UP_EDGES({ YB, YO, YG, YR });
const vector<char> Cube::DOWN_EDGES({ WG, WO, WB, WR });

const vector<char> Cube::FRONT_CORNERS({ YRG, YOG, WOG, WRG });
const vector<char> Cube::RIGHT_CORNERS({ YOG, YOB, WOB, WOG });
const vector<char> Cube::BACK_CORNERS({ YOB, YRB, WRB, WOB });
const vector<char> Cube::LEFT_CORNERS({ YRB, YRG, WRG, WRB });
const vector<char> Cube::UP_CORNERS({ YRB, YOB, YOG, YRG });
const vector<char> Cube::DOWN_CORNERS({ WRG, WOG, WOB, WRB });

// Corresponding edges in the slices
const vector<char> Cube::LR_SLICE({ YB, YG, WB, WG });
const vector<char> Cube::FB_SLICE({ YR, YO, WR, WO });
const vector<char> Cube::UD_SLICE({ RB, OB, RG, OG });

// Hashmaps for each step
unordered_map<int, Cube::TurnVec> Cube::STEP1MAP;
unordered_map<int, Cube::TurnVec> Cube::STEP2MAP;
unordered_map<int, Cube::TurnVec> Cube::STEP3MAP;
unordered_map<int, Cube::TurnVec> Cube::STEP4MAP;

// Vector of allowable turns for each step (easy to iterate through)
const Cube::TurnVec Cube::OK_TURNS1({
	Cube::front, Cube::right, Cube::back, Cube::left, Cube::up, Cube::down,
	Cube::frontI, Cube::rightI, Cube::backI, Cube::leftI, Cube::upI, Cube::downI });

const Cube::TurnVec Cube::OK_TURNS2({
	Cube::front, Cube::right, Cube::back, Cube::left, Cube::up2, Cube::down2,
	Cube::frontI, Cube::rightI, Cube::backI, Cube::leftI });

const Cube::TurnVec Cube::OK_TURNS3({
	Cube::front2, Cube::right, Cube::back2, Cube::left, Cube::up2, Cube::down2,
	Cube::rightI, Cube::leftI });

const Cube::TurnVec Cube::OK_TURNS4({
	Cube::front2, Cube::right2, Cube::back2, Cube::left2, Cube::up2, Cube::down2 });

// ok turns for the two-turn solve
const Cube::TurnVec Cube::OK_TURNS_2FACE({ Cube::front, Cube::right, Cube::frontI, Cube::rightI });


// Cube constructor
// TODO: figure out how to get rid of edgeOrbits.  They're unnecessary
Cube::Cube(char eColors[12], char eOrients[12], char eOrbits[12],
	char cColors[8], char cOrients[8])
{
	for (int i = 0; i < NUM_EDGES; ++i) {
		edgeColors_[i] = eColors[i];
		edgeOrients_[i] = eOrients[i];
		edgeOrbits_[i] = eOrbits[i];
	}
	for (int i = 0; i < NUM_CORNERS; ++i) {
		cornerColors_[i] = cColors[i];
		cornerOrients_[i] = cOrients[i];
	}
}

int Cube::fac(int n)
{
	return n == 0 ? 1 : n*fac(n - 1);
}

void Cube::initChoose()
{
	// Memoize values for FAC and CHOOSE tables
	for (int n = 0; n < 12; ++n) {
		FAC[n] = fac(n);
	}

	for (int n = 0; n < 12; ++n) {
		for (int k = 0; k <= n; ++k) {
			CHOOSE[n][k] = FAC[n] / (FAC[k] * FAC[n - k]);
		}
	}
}

void Cube::init()
{
	initChoose();

	// Create a solved cube, with corresponding empty TurnVec queue.
	// These are the starting points for hashmaps in steps 1, 2, and 4
	Cube cube = solvedCube();
	queue<Cube> cubeQueue;
	cubeQueue.push(cube);

	queue<TurnVec> turnsQueue;
	turnsQueue.push(TurnVec());

	cout << "filling map 1/4";

	// MAX_SIZE: 2^11 = 2048
	buildMap(cubeQueue, turnsQueue, STEP1MAP, &step1Code, OK_TURNS1, 2048);
	cout << endl;

	cout << "filling map 2/4";

	// MAX_SIZE: 3^7 * (12 choose 4) = 1082565
	buildMap(cubeQueue, turnsQueue, STEP2MAP, &step2Code, OK_TURNS2, 1082565);
	cout << endl;

	cout << "filling map 3/4";

	// Build the 96 unique starting points and corresponding queue for step 3
	queue<Cube> endPoints = step4ValidCorners();
	queue<TurnVec> step3TurnsQueue;
	for (int i = 0; i < endPoints.size(); ++i) {
		step3TurnsQueue.push(TurnVec());
	}

	// MAX_SIZE: 8c4 * 8c4 * 6 * 96 = 2822400
	// (could be 8c4*8c4*6 = 29400 if we have only one starting point
	buildMap(endPoints, step3TurnsQueue, STEP3MAP, &step3Code, OK_TURNS3, 2822400);
	cout << endl;

	cout << "filling map 4/4";

	// MAX_SIZE: (4!)^3 / 2 * 96 = 663552
	buildMap(cubeQueue, turnsQueue, STEP4MAP, &step4Code, OK_TURNS4, 663552);
	cout << endl;
}


void Cube::init2()
{
	initChoose();

	Cube cube = solvedCube();
	queue<Cube> cubeQueue;
	cubeQueue.push(cube);

	queue<TurnVec> turnsQueue;
	turnsQueue.push(TurnVec());

	cout << "filling map 1/4";

	// MAX_SIZE: 2^11 = 2048
	buildMap(cubeQueue, turnsQueue, STEP2MAP, &code2, OK_TURNS_2FACE, 2048);
	cout << endl;
}

Cube Cube::front(Cube cube)
{
	cube.turn(FRONT_EDGES, FRONT_CORNERS);
	cube.orientCorners(FRONT_CORNERS);
	return cube;
}

Cube Cube::right(Cube cube)
{
	cube.turn(RIGHT_EDGES, RIGHT_CORNERS);
	return cube;
}

Cube Cube::back(Cube cube)
{
	cube.turn(BACK_EDGES, BACK_CORNERS);
	cube.orientCorners(BACK_CORNERS);
	return cube;
}

Cube Cube::left(Cube cube)
{
	cube.turn(LEFT_EDGES, LEFT_CORNERS);
	return cube;
}

Cube Cube::up(Cube cube)
{
	cube.turn(UP_EDGES, UP_CORNERS);
	cube.orientEdges(UP_EDGES);
	cube.orientCorners(UP_CORNERS);
	return cube;
}

Cube Cube::down(Cube cube)
{
	cube.turn(DOWN_EDGES, DOWN_CORNERS);
	cube.orientEdges(DOWN_EDGES);
	cube.orientCorners(DOWN_CORNERS);
	return cube;
}

void Cube::turn(const vector<char>& faceEdges,
	const vector<char>& faceCorners)
{
	forwardCycle(faceEdges, edgeColors_);
	forwardCycle(faceEdges, edgeOrients_);
	forwardCycle(faceEdges, edgeOrbits_);

	forwardCycle(faceCorners, cornerColors_);
	forwardCycle(faceCorners, cornerOrients_);
}

void Cube::forwardCycle(const vector<char>& positions, char* cubies)
{
	char temp = cubies[positions[3]];
	for (size_t i = 3; i > 0; --i) {
		cubies[positions[i]] = cubies[positions[i - 1]];
	}
	cubies[positions[0]] = temp;
}

Cube Cube::frontI(Cube cube)
{
	cube.turnI(FRONT_EDGES, FRONT_CORNERS);
	cube.orientCorners(FRONT_CORNERS);
	return cube;
}

Cube Cube::rightI(Cube cube)
{
	cube.turnI(RIGHT_EDGES, RIGHT_CORNERS);
	return cube;
}

Cube Cube::backI(Cube cube)
{
	cube.turnI(BACK_EDGES, BACK_CORNERS);
	cube.orientCorners(BACK_CORNERS);
	return cube;
}

Cube Cube::leftI(Cube cube)
{
	cube.turnI(LEFT_EDGES, LEFT_CORNERS);
	return cube;
}

Cube Cube::upI(Cube cube)
{
	cube.turnI(UP_EDGES, UP_CORNERS);
	cube.orientEdges(UP_EDGES);
	cube.orientCorners(UP_CORNERS);
	return cube;
}

Cube Cube::downI(Cube cube)
{
	cube.turnI(DOWN_EDGES, DOWN_CORNERS);
	cube.orientEdges(DOWN_EDGES);
	cube.orientCorners(DOWN_CORNERS);
	return cube;
}

void Cube::turnI(const vector<char>& faceEdges,
	const vector<char>& faceCorners)
{
	backwardCycle(faceEdges, edgeColors_);
	backwardCycle(faceEdges, edgeOrients_);
	backwardCycle(faceEdges, edgeOrbits_);

	backwardCycle(faceCorners, cornerColors_);
	backwardCycle(faceCorners, cornerOrients_);
}

void Cube::backwardCycle(const vector<char>& positions, char* cubies)
{
	char temp = cubies[positions[0]];
	for (size_t i = 0; i < 3; ++i) {
		cubies[positions[i]] = cubies[positions[i + 1]];
	}
	cubies[positions[3]] = temp;
}



Cube Cube::front2(Cube cube)
{
	cube.turn2(FRONT_EDGES, FRONT_CORNERS);
	return cube;
}

Cube Cube::right2(Cube cube)
{
	cube.turn2(RIGHT_EDGES, RIGHT_CORNERS);
	return cube;
}

Cube Cube::back2(Cube cube)
{
	cube.turn2(BACK_EDGES, BACK_CORNERS);
	return cube;
}

Cube Cube::left2(Cube cube)
{
	cube.turn2(LEFT_EDGES, LEFT_CORNERS);
	return cube;
}

Cube Cube::up2(Cube cube)
{
	cube.turn2(UP_EDGES, UP_CORNERS);
	return cube;
}

Cube Cube::down2(Cube cube)
{
	cube.turn2(DOWN_EDGES, DOWN_CORNERS);
	return cube;
}

void Cube::turn2(const vector<char>& faceEdges, const vector<char>& faceCorners)
{
	// do double turn by swapping pairs of cubies.  No change in orientation.
	swap(edgeColors_[faceEdges[0]], edgeColors_[faceEdges[2]]);
	swap(edgeColors_[faceEdges[1]], edgeColors_[faceEdges[3]]);
	swap(edgeOrients_[faceEdges[0]], edgeOrients_[faceEdges[2]]);
	swap(edgeOrients_[faceEdges[1]], edgeOrients_[faceEdges[3]]);
	swap(edgeOrbits_[faceEdges[0]], edgeOrbits_[faceEdges[2]]);
	swap(edgeOrbits_[faceEdges[1]], edgeOrbits_[faceEdges[3]]);

	swap(cornerColors_[faceCorners[0]], cornerColors_[faceCorners[2]]);
	swap(cornerColors_[faceCorners[1]], cornerColors_[faceCorners[3]]);
	swap(cornerOrients_[faceCorners[0]], cornerOrients_[faceCorners[2]]);
	swap(cornerOrients_[faceCorners[1]], cornerOrients_[faceCorners[3]]);
}

void Cube::print() const
{
	cout << "//// EDGES ////" << endl;

	// cast to an int, since they're chars
	for (size_t i = 0; i < NUM_EDGES; ++i) {
		cout << (int)edgeColors_[i] << "   ";
		cout << (int)edgeOrients_[i] << "   ";
		cout << (int)edgeOrbits_[i] << endl;

	}
	cout << endl;

	cout << " /// CORNERS ////" << endl;
	for (size_t i = 0; i < NUM_CORNERS; ++i) {
		cout << (int)cornerColors_[i] << "   ";
		cout << (int)cornerOrients_[i] << endl;
	}

	cout << endl << endl;
}

void Cube::orientEdges(const vector<char>& edges)
{
	// edge orientations are changed identically (wrt the face) for U,D,U',D'
	for (char pos : edges) {
		edgeOrients_[pos] = !edgeOrients_[pos];
	}
}

void Cube::orientCorners(const vector<char>& pos)
{
	// corner orientations also change the same for U,D,U',D',L,R,L',R'
	cornerOrients_[pos[0]] = (cornerOrients_[pos[0]] + 1) % 3;
	cornerOrients_[pos[1]] = (cornerOrients_[pos[1]] + 2) % 3;
	cornerOrients_[pos[2]] = (cornerOrients_[pos[2]] + 1) % 3;
	cornerOrients_[pos[3]] = (cornerOrients_[pos[3]] + 2) % 3;
}


int Cube::step1Code() {

	// Two codes have the same code if the edge orientations are the same
	// Essentially creates a unique base 2 number
	int sum = 0;
	for (int i = 0; i < NUM_EDGES - 1; ++i) {
		sum += edgeOrients_[i] * pow(2, i);
	}
	return sum;
}

int Cube::step2Code() {

	// Two codes have the same code if the corner orientations are the same
	// and if the UD_SLICE edges are in the same place

	// Essentially creates a unique base 3 number for corner orients
	int sum1 = 0;
	for (int i = 0; i < NUM_CORNERS - 1; ++i) {
		sum1 += cornerOrients_[i] * pow(3, i);
	}

	// Hashes {0,0,0,0,_,_...} to 0, {0,0,0,_,0,_...} to 1,
	// ... and {...,_,0,0,0,0} to 494 = (12 choose 4) - 1.
	int sum2 = 0;
	int offset = 3;
	for (int i = 0; offset >= 0; ++i) {
		if (edgeOrbits_[i] == 0) {
			--offset;
		}
		else {
			sum2 += CHOOSE[12 - i - 1][offset];
		}
	}
	return sum1 * 495 + sum2;
}

int Cube::step3Code()
{
	// TODO: simplify code for step 3

	// certain edge orbit spots are known to be filled with 0's
	int openSpots[8] = { 1,2,4,5,6,7,9,10 };

	// creates a code for the edges of the FB_ORBIT
	// similar to second part of step2Code
	int sum1 = 0;
	int offset = 3;
	for (int i = 0; offset >= 0; ++i) {

		if (edgeOrbits_[openSpots[i]] == 1) {
			--offset;
		}
		else {
			sum1 += CHOOSE[8 - i - 1][offset];
		}
	}

	// creates a code for all the corners, similar to step4Help
	int sum2 = 0;
	for (int i = 0; i < 7; ++i) {

		int numGreater = 0;
		for (int j = i + 1; j < 8; ++j) {

			if (cornerColors_[j] < cornerColors_[i]) {
				++numGreater;
			}
		}
		sum2 += (numGreater * FAC[8 - i - 1]);
	}

	// some multiplication to ensure unique codes for different cubes
	return sum1 * 40320 + sum2;
}

int Cube::step4Help(vector<char> orbit, char* cubies)
{
	// Creates a unique code for the cubies of an orbit
	// essentially, 0,1,2,3 => 0, 0,1,3,2 => 1, ... 3,2,1,0 => 23
	int sum = 0;
	for (int i = 0; i < 3; ++i) {

		int numGreater = 0;
		for (int j = i + 1; j < 4; ++j) {

			if (cubies[orbit[j]] < cubies[orbit[i]]) {
				++numGreater;
			}
		}
		sum += (numGreater * FAC[4 - i - 1]);
	}
	return sum;
}

int Cube::step4Code()
{
	// TODO: simplify code for step 4

	// creates a unique code based on the positions of the edges within
	// their orbits
	int sum = 0;
	sum += step4Help(UD_SLICE, edgeColors_);
	sum += step4Help(LR_SLICE, edgeColors_) * 24;
	sum += step4Help(FB_SLICE, edgeColors_)*pow(24, 2);

	// takes into the account the overall position of the corners
	int sum2 = 0;
	for (int i = 0; i < 7; ++i) {

		int numGreater = 0;
		for (int j = i + 1; j < 8; ++j) {

			if (cornerColors_[j] < cornerColors_[i]) {
				++numGreater;
			}
		}
		sum2 += (numGreater * FAC[8 - i - 1]);
	}
	sum += sum2*pow(24, 3);

	return sum;
}

int Cube::code2()
{
	char edges[7] = { YO, YG, RG, OG, OB, WG, WO };
	char corners[6] = { YOB, YRG, YOG, WOB, WRG, WOG };

	int sum1 = 0;
	for (int i = 0; i < 5; ++i) {
		sum1 += cornerOrients_[corners[i]] * pow(3, i);
	}

	int sum2 = 0;
	for (int i = 0; i < 5; ++i) {
		int numGreater = 0;
		for (int j = i + 1; j < 6; ++j) {

			if (cornerColors_[corners[j]] < cornerColors_[corners[i]]) {
				++numGreater;
			}
		}
		sum2 += (numGreater * FAC[6 - i - 1]);
	}

	int sum3 = 0;
	for (int i = 0; i < 6; ++i) {
		int numGreater = 0;
		for (int j = i + 1; j < 7; ++j) {

			if (edgeColors_[edges[j]] < edgeColors_[edges[i]]) {
				++numGreater;
			}
		}
		sum3 += (numGreater * FAC[7 - i - 1]);
	}
	return sum1 + 243*sum2 + 2430000*sum3;
}

void Cube::solve(Cube cube)
{
	//cout << "step 1 1 1 1 1 1 1 1 1 1 1" << endl;
	TurnVec step1 = doStep(cube, STEP1MAP, &step1Code, OK_TURNS1);
	cout << endl;

	//cout << "step 2 2 2 2 2 2 2 2 2 2 2" << endl;
	TurnVec step2 = doStep(cube, STEP2MAP, &step2Code, OK_TURNS2);
	cout << endl;

	//cout << "step 3 3 3 3 3 3 3 3 3 3 3" << endl;
	TurnVec step3 = doStep(cube, STEP3MAP, &step3Code, OK_TURNS3);
	cout << endl;

	//cout << "step 4 4 4 4 4 4 4 4 4 4 4" << endl;
	TurnVec step4 = doStep(cube, STEP4MAP, &step4Code, OK_TURNS4);
	cout << endl;

	cout << "SUCCESS!!!!" << endl;

	printSteps(step1);
	printSteps(step2);
	printSteps(step3);
	printSteps(step4);
}

void Cube::solve2(Cube cube)
{

	TurnVec step2 = doStep(cube, STEP2MAP, &code2, OK_TURNS_2FACE);
	cout << endl;

	cout << "SUCCESS!!!!" << endl;

	printSteps(step2);

}


Cube::TurnVec Cube::doStep(Cube& cube, unordered_map<int, TurnVec>& stepTable, int (Cube::*code)(),
	TurnVec okSteps)
{
	TurnVec turns = cube.findTurns(stepTable, code, okSteps);
	for (auto turn : turns) {
		cube = turn(cube);
	}
	//printSteps(turns);

	//cube.print();

	return turns;
}

Cube::TurnVec Cube::findTurns(unordered_map<int, TurnVec>& stepTable, int (Cube::*code)(),
	TurnVec okSteps)
{
	// keep a set of the cubes seen so far, so no duplicates
	unordered_set<int> seenCubes;

	// check to see if the first cube matches
	int firstHash = (this->*code)();

	seenCubes.insert(firstHash);

	// if the cube can already has a value in the table
	// use the corresponding turns
	if (stepTable.count(firstHash) > 0) {
		TurnVec lastSteps = stepTable[firstHash];
		TurnVec flipped = invert(lastSteps);

		return flipped;
	}

	// create a cubeQueue and turnsQueue
	queue<Cube> cubeQueue;
	cubeQueue.push(*this);

	queue<TurnVec> turnsQueue;
	turnsQueue.push(TurnVec());

	int i = 0;
	while (true) {
		if (i % 100 == 0) {
			cout << ".";
		}

		// pop the first cube and turn vector
		Cube currCube = cubeQueue.front();
		cubeQueue.pop();

		TurnVec currSteps = turnsQueue.front();
		turnsQueue.pop();

		// push back a dummy turn that will immediately be popped
		// (to simplify the loop)
		currSteps.push_back(front);

		for (Cube(*turn)(Cube) : okSteps) {

			// apply a turn to the cube.  Check if it matches
			Cube turnedCube = turn(currCube);
			int cubeCode = (turnedCube.*code)();

			// only use this cube if we've never seen it before
			if (seenCubes.count(cubeCode) == 0) {
				seenCubes.insert(cubeCode);

				currSteps.pop_back();
				currSteps.push_back(turn);

				// if this cube is in the step table, combine these
				// steps with the inverted steps from the step table
				if (stepTable.count(cubeCode) > 0) {
					TurnVec lastSteps = stepTable[cubeCode];
					TurnVec flipped = invert(lastSteps);

					currSteps.insert(currSteps.end(), flipped.begin(), flipped.end());

					return currSteps;
				}
				// otherwise, push onto the queue and keep searching
				else {
					cubeQueue.push(turnedCube);
					turnsQueue.push(currSteps);
				}
			}
		}
		++i;
	}
}


queue<Cube> Cube::step4ValidCorners()
{
	// add a solved Cube to the queue
	Cube cube = solvedCube();
	vector<Cube> cubeQueue;
	cubeQueue.push_back(cube);

	unordered_set<int> cubeCodes; // for easy access to the cubeCodes
	cubeCodes.insert(cube.step4CornerCode());

	// generate the 96 unique corner orientations
	for (int i = 0; cubeCodes.size() < 96; ++i) {

		Cube currCube = cubeQueue[i];

		for (auto turn : OK_TURNS4) {

			Cube turnedCube = turn(currCube);
			int code = turnedCube.step4CornerCode();

			if (cubeCodes.count(code) == 0) {
				cubeQueue.push_back(turnedCube);
				cubeCodes.insert(code);
			}
		}

	}

	queue<Cube> queueToReturn;

	for (auto c : cubeQueue) {
		queueToReturn.push(c);
	}

	return queueToReturn;
}

int Cube::step4CornerCode()
{
	int sum = 0;

	for (int i = 0; i < 7; ++i) {

		int numGreater = 0;
		for (int j = i + 1; j < 8; ++j) {

			if (cornerColors_[j] < cornerColors_[i]) {
				++numGreater;
			}
		}
		sum += (numGreater * FAC[8 - i - 1]);
	}
	return sum;
}

void Cube::buildMap(queue<Cube> cubeQueue, queue<TurnVec> turnsQueue, unordered_map<int, TurnVec>& stepList,
	int (Cube::*code)(), TurnVec okTurns, int tableSize)
{
	while (stepList.size() < tableSize) {

		Cube currCube = cubeQueue.front();
		TurnVec currSteps = turnsQueue.front();

		cubeQueue.pop();
		turnsQueue.pop();

		// push back a dummy turn that will immediately be popped
		// (to simplify the loop)
		currSteps.push_back(front);

		for (auto turn : okTurns) {
			
			Cube turnedCube = turn(currCube);
			int cubeCode = (turnedCube.*code)();

			if (stepList.count(cubeCode) == 0) {
				
				currSteps.pop_back();
				currSteps.push_back(turn);
				
				cubeQueue.push(turnedCube);
				turnsQueue.push(currSteps);
				
				stepList[cubeCode] = currSteps;

				if (stepList.size() % (tableSize / 10) == 0) {
					cout << ".";
				}
			}
			
		}
	}
}

Cube Cube::solvedCube()
{
	// creates a solved cube
	char eColors[12] = { 0,1,2,3,4,5,6,7,8,9,10,11 };
	char eOrients[12] = { 0,0,0,0,0,0,0,0,0,0,0,0 };
	char eOrbits[12] = { 0,1,1,0,2,2,2,2,0,1,1,0 };

	char cColors[8] = { 0,1,2,3,4,5,6,7 };
	char cOrients[8] = { 0,0,0,0,0,0,0,0 };



	Cube cube(eColors, eOrients, eOrbits, cColors, cOrients);

	return cube;
}


void Cube::printSteps(TurnVec steps)
{
	for (auto step : steps) {
		if (step == front) cout << "F";
		else if (step == right) cout << "R";
		else if (step == back) cout << "B";
		else if (step == left) cout << "L";
		else if (step == up) cout << "U";
		else if (step == down) cout << "D";

		else if (step == front2) cout << "F2";
		else if (step == right2) cout << "R2";
		else if (step == back2) cout << "B2";
		else if (step == left2) cout << "L2";
		else if (step == up2) cout << "U2";
		else if (step == down2) cout << "D2";

		else if (step == frontI) cout << "F'";
		else if (step == rightI) cout << "R'";
		else if (step == backI) cout << "B'";
		else if (step == leftI) cout << "L'";
		else if (step == upI) cout << "U'";
		else if (step == downI) cout << "D'";

		cout << " ";
	}
	cout << endl;
}

Cube::TurnVec Cube::invert(TurnVec steps)
{
	reverse(steps.begin(), steps.end());

	TurnVec flipped;


	for (auto step : steps) {

		if (step == front) step = frontI;
		else if (step == right) step = rightI;
		else if (step == back) step = backI;
		else if (step == left) step = leftI;
		else if (step == up) step = upI;
		else if (step == down) step = downI;

		else if (step == frontI) step = front;
		else if (step == rightI) step = right;
		else if (step == backI) step = back;
		else if (step == leftI) step = left;
		else if (step == upI) step = up;
		else if (step == downI) step = down;

		flipped.push_back(step);
	}

	return flipped;
}

void Cube::test()
{
	Cube cube = solvedCube();
	//cube.print();

	/*
	TurnVec maneuver({
	up2, front, down, up, left2, down2, right, backI,
	right, leftI, up, left2, down, front2, left, right,
	up, right2, frontI, upI, down, left2, front, up,
	left2, frontI, upI, frontI });
	*/

	

	TurnVec maneuver({ right });

	for (int i = 0; i < 32; ++i) {
		maneuver.push_back(front);
		maneuver.push_back(right);
	}
	// test steps 1
	for (auto turn : maneuver) {
		cube = turn(cube);
	}

	//cube.print();
	cout << "STARTING" << endl;

	solve2(cube);
}

int main()
{
	Cube::init2();

	clock_t t;
	t = clock();

	Cube::test();

	cout << endl << endl;

	t = clock() - t;
	cout << "time: " << (float)t / CLOCKS_PER_SEC << endl;

	cout << endl << endl;

	system("pause");
	return 0;
}

/*
To do:

Serialize hash tables
Simplify cube codes for steps 3 and 4
Do coset math to decrease step 3 table by a factor of 96
Account for symmetries to decrease step 2 table by a factor of 48
Get rid of edge orbits to decrease size by 1/3
If we're doing really well, combine steps 1 and 2
Move combo stuff to different file
Turn vectors into char arrays, if you want
*/