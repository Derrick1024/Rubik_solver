#pragma once
/**
* \file cube.h
* \brief interface for Cube class
*/

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <queue>

/// Cube colors
static const char YB = 0;
static const char YR = 1;
static const char YO = 2;
static const char YG = 3;
static const char RB = 4;
static const char OB = 5;
static const char RG = 6;
static const char OG = 7;
static const char WB = 8;
static const char WR = 9;
static const char WO = 10;
static const char WG = 11;

static const char YRB = 0;
static const char YOB = 1;
static const char YRG = 2;
static const char YOG = 3;
static const char WRB = 4;
static const char WOB = 5;
static const char WRG = 6;
static const char WOG = 7;

/// Memoized the necessary factorials and combinations
static int FAC[12];
static int CHOOSE[12][12];

/**
* \class Cube
* \brief Contains the implementation for a Rubik's Cube
* \details A rubik's cube is identified by it's edge colors,
*		   edge orientations, corner colors, and corner orientations
*		   the ith index of an array corresponds to the cubie
*		   in the space that is home to cubie i.
*/
class Cube {

public:

	/// Typedefs to make code easier to read
	typedef std::vector<Cube(*)(Cube)> TurnVec;

	/// Initializes constants and loads the hash tables
	static void init();
	static void initChoose();

	/// for two-face solve
	static void init2();

	/// Scramble and solve a test cube
	static void test();

	Cube() = delete;

	/// Four-argument parameterized constructor.  Constructs a cube
	/// out of edgeColors, edgeOrients, cornerColors, and cornerOrients
	Cube(char eColors[12], char eOrients[12], char eOrbits[12],
		char cColors[8], char cOrients[8]);

	Cube(const Cube& rhs) = default; ///< Default copy constructor


	/**
	* \brief Turning methods.  Each of the following static methods takes in a
	* copy of a cube.  It then applies the appropriate turn by rearranging the
	* relevant color and orientation vectors and by changing the orientations
	* properly.  The first six methods represent clockwise turns, the following
	* are CC (inverse) turns, then double turns.  All the methods require cycling the
	* arrays.  up and down require edge reorientation, and up down front and back
	* require corner orientation.
	*
	*/
	static Cube front(Cube cube);
	static Cube right(Cube cube);
	static Cube back(Cube cube);
	static Cube left(Cube cube);
	static Cube up(Cube cube);
	static Cube down(Cube cube);

	static Cube frontI(Cube cube);
	static Cube rightI(Cube cube);
	static Cube backI(Cube cube);
	static Cube leftI(Cube cube);
	static Cube upI(Cube cube);
	static Cube downI(Cube cube);

	static Cube front2(Cube cube);
	static Cube right2(Cube cube);
	static Cube back2(Cube cube);
	static Cube left2(Cube cube);
	static Cube up2(Cube cube);
	static Cube down2(Cube cube);


	/**
	* \fn		solve
	* \brief	Solves the cube by running steps 1 through 4.
	* \details	Uses Thistlethwaite's method.  First uses F,R,B,L,U,D to orient
	*			the edges.  Then uses F,R,B,L,U2,D2 to finish step 2 (and this
	*			combination of moves will prevent the edges from becoming
	*			unoriented.  Then uses F2,R,B2,L,U2,D2 to finish step 3.
	*			Finally, uses F2,R2,B2,L2,U2,D2 to solve the rest.
	*			In the end, it prints out the steps taken to solve the cube
	*/
	static void solve(Cube cube);

	/// for two-turn solve
	static void solve2(Cube cube);

	/// Creates an instance of a solved Cube
	static Cube solvedCube();


	/// Prints the cube in the form edgeColors, edgeOrients, cornerColors, cornerOrients
	void print() const;




private:

	/// Some useful constants
	static const char NUM_EDGES = 12;
	static const char NUM_CORNERS = 8;

	/// vectors containing numbers that correspond to faces
	static const std::vector<char> FRONT_EDGES;
	static const std::vector<char> RIGHT_EDGES;
	static const std::vector<char> LEFT_EDGES;
	static const std::vector<char> BACK_EDGES;
	static const std::vector<char> UP_EDGES;
	static const std::vector<char> DOWN_EDGES;

	static const std::vector<char> FRONT_CORNERS;
	static const std::vector<char> RIGHT_CORNERS;
	static const std::vector<char> LEFT_CORNERS;
	static const std::vector<char> BACK_CORNERS;
	static const std::vector<char> UP_CORNERS;
	static const std::vector<char> DOWN_CORNERS;

	/// Vectors corresponding to specific edge orbits
	static const std::vector<char> UD_SLICE;
	static const std::vector<char> FB_SLICE;
	static const std::vector<char> LR_SLICE;

	/// Hash maps from cube codes to the required turns to solve
	static std::unordered_map<int, TurnVec> STEP1MAP;
	static std::unordered_map<int, TurnVec> STEP2MAP;
	static std::unordered_map<int, TurnVec> STEP3MAP;
	static std::unordered_map<int, TurnVec> STEP4MAP;

	/// Vectors with function pointers to allowable turns
	static const TurnVec OK_TURNS1;
	static const TurnVec OK_TURNS2;
	static const TurnVec OK_TURNS3;
	static const TurnVec OK_TURNS4;

	static const TurnVec OK_TURNS_2FACE;

	/**
	* \fn		turn
	* \brief	Calls forwardCycle on the data members of Cube.
	*/
	void turn(const std::vector<char>& edges, const std::vector<char>& corners);

	/**
	* \fn		turnI
	* \brief	Calls backwardCycle on the data members of Cube.
	*/
	void turnI(const std::vector<char>& edges, const std::vector<char>& corners);

	/**
	* \fn		forwardCycle
	* \brief	Cycles the numbers at the given indices forward (i.e. clockwise on the cube)
	* \details	For example, then numbers at indices {1,3,5,6} will now be at {6,1,3,5}
	*/
	void forwardCycle(const std::vector<char>& positions, char* cubies);

	/**
	* \fn		forwardCycle
	* \brief	Cycles the numbers at the given indices backward (i.e. CC on the cube)
	* \details	For example, then numbers at indices {1,3,5,6} will now be at {3,5,6,1}
	*/
	void backwardCycle(const std::vector<char>& positions, char* cubies);


	/**
	* \fn		orientEdges
	* \brief	Inverts the orientation of the edges at a given face.
	* \details	Used in the turning methods up and down.
	*/
	void orientEdges(const std::vector<char>& edges);

	/**
	* \fn		orientCorners
	* \brief	Changes the orientations of the corners at given face to represent a turn.
	* \details	Used in the turning methods up, down, left, and right.  Conveniently,
	*			the orientations for respective pieces changes in the same way for
	*			each of these turns.
	*/
	void orientCorners(const std::vector<char>& corners);

	/**
	* \fn		turn2
	* \brief	Swaps different indices of the cube data members to represent a double
	*			turn on a Cube.
	* \details	Helper method for front2, right2, etc.
	*/
	void turn2(const std::vector<char>& edges, const std::vector<char>& corners);

	/**
	* \fn		step1Code
	* \brief	Hashes the cube into an integer
	* \details	Cubes had unique codes based only on edge orientation.
	* \return	int
	*/
	int step1Code();

	/**
	* \fn		step2Code
	* \brief	Hashes the cube into a relevant integer for step 2
	* \details	Cubes have unique codes based only on corner orientation
	*			and the position of the UD_SLICE edges
	* \return	int
	*/
	int step2Code();


	/**
	* \fn		step3Code
	* \brief	Hashes the cube into a relevant integer for step 2
	* \details	Cubes have unique codes based only on edge orbits
	*			and corner colors (corners must be in one of 96 possibilities)
	* \return	int
	*/
	int step3Code();

	/**
	* \fn		step4Code
	* \brief	Hashes the cube into a relevant integer for step 4
	* \details	Cube have unique codes based only on edge colors
	*/
	int step4Code();

	int code2();

	/// Helper method to determine the code for cubies in a specific orbit
	int step4Help(std::vector<char> orbit, char* cubies);

	/**
	* \fn		doStep
	* \brief	Maneuvers the cube to complete a given step
	* \details	Calls doStep to determine how to most quickly finish the step
	*			Then turns the cube with the given function pointers.  Also prints
	*			The steps and the cube.  Returns the turn vector.
	* \param	Cube& cube					a reference to the cube
	* \param	bool(Cube::*solved)()		the method to see if it's solved
	* \param	TurnVec okSteps				the vector of allowable turns for this step
	*/
	static TurnVec doStep(Cube& cube, std::unordered_map<int, TurnVec>& stepTable, int (Cube::*hashFunction)(), TurnVec okSteps);

	/**
	* \fn		findTurns
	* \brief	Uses a breadth-first search to find the fastest way to finish a step
	* \details	Given a method to see if the cube is solved and a vector of allowable
	*			turns, findTurns creates a queue of Cubes and a queue of TurnVec.
	*			Using a bfs, it continues to try every possible turn, check if it
	*			is "solved", and if not, adds it to the queue.  If it is "solved,"
	*			it returns the associated TurnVec.
	* \param	bool (Cube::*solved)()		Method to check if the cube is done with step
	* \param	TurnVec okSteps				A vector of allowable turns for this step
	*/
	TurnVec findTurns(std::unordered_map<int, TurnVec>& stepTable, int (Cube::*code)(), TurnVec okSteps);

	/**
	* \fn		printSteps
	* \brief	Given a vector of function pointers to turns, prints them in readable form
	*/
	static void printSteps(TurnVec steps);

	/// Inverts the step in a TurnVec.  For example, F R U2 becomes U2 R' F'
	static TurnVec invert(TurnVec steps);

	/// Builds the hashmaps holding the cubes
	static void buildMap(std::queue<Cube> cubeQueue, std::queue<TurnVec> turnsQueue,
		std::unordered_map<int, TurnVec>& stepList, int (Cube::*hashFunction)(), TurnVec okTurns, int tableSize);

	/// Helper method to hash the corners to a unique number.  Used to determine
	/// The only 96 possible corner positions using double turns
	int step4CornerCode();

	/// Generates a queue with all 96 possible corner positions before step 4.
	/// Used as a starting point to build the step 3 tables
	static std::queue<Cube> step4ValidCorners();

	static int fac(int n);

	/// data members that define a cube.  For the following, indices represent positions
	char edgeColors_[12];	///< Numbers represent colors (see table of consts above)
	char edgeOrients_[12];  ///< 0 is oriented, 1 is unoriented
	char cornerColors_[8];	///< Numbers represent colors (see table of consts above)
	char cornerOrients_[8];	///< 0 is oriented, 1 is clockwise, 2 is counterclockwise

	char edgeOrbits_[12];  ///< 0, 1, or 2 depending on whether the edge is at home

};


