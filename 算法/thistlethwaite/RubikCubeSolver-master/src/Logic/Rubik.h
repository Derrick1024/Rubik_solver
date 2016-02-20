#ifndef RUBIK_RUBIK_H
#define RUBIK_RUBIK_H

#include <queue>
#include "bigint.h"

#include "Singleton.h"

// size of cube in graphics units
#define CUBE_SIZE 18.0f
// size of one atom (small cube)
#define ATOM_SIZE CUBE_SIZE/3.0f
// spacing between atoms
#define ATOM_SPACING 0.2f
// default animation timer
#define ANIM_TIMER_DEFAULT 200

// stored static reference to mesh manipulator to simplify working with meshes
static IMeshManipulator* meshManipulator = nullptr;

// state permutation string array length
#define STATE_STRING_LENGTH 20

// all possible colors
enum RubikColor
{
    CL_RED = 0,
    CL_GREEN = 1,
    CL_BLUE = 2,
    CL_YELLOW = 3,
    CL_WHITE = 4,
    CL_ORANGE = 5,

    CL_NONE = 6,    // "no color", typically initial state or non-visible side
    CL_COUNT = 6
};

// each color's ARGB representation (index matches value from enumerator RubikColor)
static SColor rubikColorMap[] = {
    { 255, 200,   0,   0 },
    { 255,   0, 200,   0 },
    { 255,   0,   0, 200 },
    { 255, 200, 200,   0 },
    { 255, 255, 255, 255 },
    { 255, 207,  64,   0 },
    { 255,   0,   0,   0 }
};

// color code used for each color (index matches value from enumerator RubikColor)
static char rubikColorCode[] = { 'R', 'G', 'B', 'Y', 'W', 'O', '-' };

// translates color code to RubikColor enum value
static RubikColor getColorForCode(char code)
{
    for (int i = 0; i < CL_COUNT; i++)
        if (code == rubikColorCode[i])
            return (RubikColor)i;
    return CL_NONE;
}

// all possible cube faces
enum CubeFace
{
    CF_UP = 0,
    CF_DOWN = 1,
    CF_BACK = 2,
    CF_FRONT = 3,
    CF_RIGHT = 4,
    CF_LEFT = 5,

    CF_BEGIN = CF_UP,
    CF_END = CF_LEFT + 1,
    CF_COUNT = CF_END
};

// face codes for each cube face (index matches value from enumerator CubeFace)
static char rubikFaceCode[] = { 'U', 'D', 'B', 'F', 'R', 'L' };

static std::string solvedPermutation[] = { "UF", "UR", "UB", "UL", "DF", "DR", "DB", "DL", "FR", "FL", "BR", "BL", "UFR", "URB", "UBL", "ULF", "DRF", "DFL", "DLB", "DBR" };

// this will then depend on user input (these are default values)
static CubeFace colorFaceMap[] = {
    /* CL_RED */    CF_UP,
    /* CL_GREEN */  CF_FRONT,
    /* CL_BLUE */   CF_LEFT,
    /* CL_YELLOW */ CF_RIGHT,
    /* CL_WHITE */  CF_DOWN,
    /* CL_ORANGE */ CF_BACK
};

// XYZ offsets of faces relative to cube fixed point ("position"), index matches value from enum CubeFace
static vector3df cubeFaceOffset[] = {
    { 0,                    ATOM_SIZE,           0 },
    { 0,                    0,                   0 },
    { 0,                    ATOM_SIZE / 2.0f,   ATOM_SIZE / 2.0f },
    { 0,                    ATOM_SIZE / 2.0f,  -ATOM_SIZE / 2.0f },
    { ATOM_SIZE / 2.0f,     ATOM_SIZE / 2.0f, 0 },
    { -ATOM_SIZE / 2.0f,    ATOM_SIZE / 2.0f, 0 }
};

// XYZ rotation of every cube face (index matches value from enum CubeFace)
static vector3df cubeFaceRotation[] = {
    { 0, 0, 0 },
    { -180.0f, 0, 0 },
    { 90.0f, 0.0f, 0.0f },
    { -90.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, -90.0f },
    { 0.0f, 0.0f, 90.0f }
};

class RubikCube;

// structure of cube face
struct CubeAtomFace
{
    // graphics element of this atom face
    IMeshSceneNode* meshNode;
    // base position (at start, used for animation)
    vector3df basePosition;
    // base rotation (at start, used for animation)
    vector3df baseRotation;
    // assigned color
    RubikColor color;

    // sets color to cube face
    void setColor(RubikColor cl);
    // retrieves color
    RubikColor getColor() { return color; }
};

// structure of cube atom
struct CubeAtom
{
    // all faces it contains
    CubeAtomFace* faces[CF_COUNT];
};

// half-turn metric flips
enum CubeFlip
{
    FLIP_R_P = 0,
    FLIP_R_2 = 1,
    FLIP_R_N = 2,

    FLIP_L_P = 3,
    FLIP_L_2 = 4,
    FLIP_L_N = 5,

    FLIP_B_P = 6,
    FLIP_B_2 = 7,
    FLIP_B_N = 8,

    FLIP_F_P = 9,
    FLIP_F_2 = 10,
    FLIP_F_N = 11,

    FLIP_D_P = 12,
    FLIP_D_2 = 13,
    FLIP_D_N = 14,

    FLIP_U_P = 15,
    FLIP_U_2 = 16,
    FLIP_U_N = 17,

    FLIP_MAX = FLIP_U_N + 1,
    FLIP_BEGIN = FLIP_R_P,

    FLIP_NONE       // used just as "flag", not real turn
};

// strings representing each flip (index matches value from CubeFlip enumerator)
static char* cubeFlipStr[] = { "R+", "R2", "R-", "L+", "L2", "L-", "B+", "B2", "B-", "F+", "F2", "F-", "D+", "D2", "D-", "U+", "U2", "U-" };

// Thistletwaithe's algorithm restricts flips in each stage of cube solving
// these are bitmasks for each of stage
static int stageAllowedFlips[] = {
    1 << FLIP_U_P | 1 << FLIP_D_P | 1 << FLIP_F_P | 1 << FLIP_B_P | 1 << FLIP_L_P | 1 << FLIP_R_P | 1 << FLIP_U_N | 1 << FLIP_D_N | 1 << FLIP_F_N | 1 << FLIP_B_N | 1 << FLIP_L_N | 1 << FLIP_R_N,
    1 << FLIP_U_P | 1 << FLIP_D_P | 1 << FLIP_F_2 | 1 << FLIP_B_2 | 1 << FLIP_L_P | 1 << FLIP_R_P | 1 << FLIP_U_N | 1 << FLIP_D_N | 1 << FLIP_L_N | 1 << FLIP_R_N,
    1 << FLIP_U_2 | 1 << FLIP_D_2 | 1 << FLIP_F_2 | 1 << FLIP_B_2 | 1 << FLIP_L_P | 1 << FLIP_R_P | 1 << FLIP_L_N | 1 << FLIP_R_N,
    1 << FLIP_U_2 | 1 << FLIP_D_2 | 1 << FLIP_F_2 | 1 << FLIP_B_2 | 1 << FLIP_L_2 | 1 << FLIP_R_2,
};

// array of affected cubes by specific flip - index matches value from CubeFace,
// because the only thing we want to know is permutation, and number of those permutations
// needed to proceed specific flip is determined from enumerator value
static int flipCubeEffect[][8] = {
    { 0, 1, 2, 3, 0, 1, 2, 3 },    // U
    { 4, 7, 6, 5, 4, 5, 6, 7 },    // D
    { 0, 9, 4, 8, 0, 3, 5, 4 },    // F
    { 2, 10, 6, 11, 2, 1, 7, 6 },  // B
    { 3, 11, 7, 9, 3, 2, 6, 5 },   // L
    { 1, 8, 5, 10, 1, 0, 4, 7 },   // R
};

// retrieves flip for supplied string identifier
static CubeFlip getFlipForStr(char* str)
{
    for (int i = 0; i < FLIP_MAX; i++)
    {
        if (strcmp(str, cubeFlipStr[i]) == 0)
            return (CubeFlip)i;
    }

    return FLIP_NONE;
}

// retrieves string representing current flip
static char* getStrForFlip(CubeFlip fl)
{
    if (fl < FLIP_MAX)
        return cubeFlipStr[fl];
    return nullptr;
}

// rubik's cube class
class RubikCube
{
    friend class Singleton<RubikCube>;
    public:
        ~RubikCube();

        // loads cube from file
        bool LoadFromFile(char* filename);

        // renders cube and 2D drawing
        void Render();

        // creates cube and its faces, assigns default values
        void BuildCube(ISceneManager* scene, IVideoDriver* videoDriver);
        // retrieves atom at specified position
        CubeAtom* GetAtom(int x, int y, int z);

        // performs flip; may propagate changes to cache (parameter draw)
        void DoFlip(CubeFlip flip, bool draw);

        // generates random sequence of flips
        void Scramble(std::list<CubeFlip> *target);
        // generate solution to current state
        void Solve(std::list<CubeFlip> *target);

        // processes flip sequence, instantly or animated (pushed to queue)
        void ProceedFlipSequence(std::list<CubeFlip> *source, bool animate);

        // determines, if there are some flips in queue
        bool IsFlipSequenceInProgress() { return !m_flipQueue.empty(); };

        // prints cube to console
        void PrintOut();

        // updates timing of flip animation
        void UpdateFlipTiming(int howmuch) { if (m_flipTiming + howmuch >= 100 && m_flipTiming + howmuch <= 5000) m_flipTiming += howmuch; };
        // retrieves limit of flip animation
        int GetFlipTiming() { return m_flipTiming; };

    private:
        // private constructor, only friend class could create this class instance
        RubikCube();

        // atom storage
        CubeAtom* m_cubeAtoms[3][3][3];
        // cube cache by faces
        RubikColor m_cubeCache[CF_COUNT][3][3];
        // stored textures for faces
        ITexture* m_faceTexture, *m_faceMiniTexture;
        // value indicating in which solving stage are we right now
        unsigned char m_solveStage;
        // timing to proceed flips
        int m_flipTiming;

        // flip actually being progressed
        CubeFlip m_toProgress;
        // start of counter (ms)
        unsigned int m_progressStart;
        // queue of flips to be done
        std::queue<CubeFlip> m_flipQueue;

        // sets cube atom to internal array
        void SetCubeAtom(int x, int y, int z, CubeAtom* atom);
        // builds cube atom, its faces, etc.
        CubeAtom* BuildCubeAtom(ISceneManager* scene, IVideoDriver* videoDriver, vector3df basePosition, vector3di cubeOffset);
        // builds face of cube atom
        CubeAtomFace* BuildFace(ISceneManager* scene, IVideoDriver* videoDriver, CubeFace side, vector3df basePosition);
        // caches cube to internal array
        void CacheCube();
        // restores internal array to cube visually
        void RestoreCacheCube();

        // retrieves hash of current state, regarding current solving stage
        bigint GetStateHash(bigint &state);
        // converts cube configuration to permutation table
        void ConvertToPermutationTable(std::vector<std::string> &dstList);
        // retrieves code of edge (permutation)
        char* GetEdgeCode(int x, int y, int z, CubeFace frst, CubeFace scnd);
        // retrieves code for corner (permutation)
        char* GetCornerCode(int x, int y, int z, CubeFace frst, CubeFace scnd, CubeFace thrd);
        // performs flip on linearized cube state
        bigint DoLinearFlip(int move, bigint state);

        // circulary swaps four elements
        void AtomCircularSwap(int ax, int ay, int az, CubeFace a, int bx, int by, int bz, CubeFace b, int cx, int cy, int cz, CubeFace c, int dx, int dy, int dz, CubeFace d, bool reverse = false);
};

#define sCube Singleton<RubikCube>::instance()

#endif
