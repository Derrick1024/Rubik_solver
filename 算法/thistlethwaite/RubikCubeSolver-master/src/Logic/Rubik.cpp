#include "Global.h"
#include "bigint.h"
#include <queue>
#include "Rubik.h"
#include "Drawing.h"
#include "Application.h"

#include <time.h>
#include <string>
#include <utility>
#include <fstream>
using namespace std;

// constructor - just reset some variables to initial state
RubikCube::RubikCube()
{
    m_progressStart = 0;
    m_toProgress = FLIP_NONE;
    m_flipTiming = ANIM_TIMER_DEFAULT;
}

RubikCube::~RubikCube()
{
    //
}

// sets color of cube atom face - also manipulates mesh, if in GUI mode
void CubeAtomFace::setColor(RubikColor cl)
{
    if (sApplication->IsGraphicMode())
        meshManipulator->setVertexColors(meshNode->getMesh(), rubikColorMap[cl]);
    color = cl;
}

// builds cube face with specified parameters
CubeAtomFace* RubikCube::BuildFace(ISceneManager* scene, IVideoDriver* videoDriver, CubeFace side, vector3df basePosition)
{
    CubeAtomFace* face = new CubeAtomFace();

    // face is in fact built only in GUI mode, but we still have to have this record present due to
    // working with it
    if (sApplication->IsGraphicMode())
    {
        IMeshSceneNode* node;
        IMesh* mesh = scene->getGeometryCreator()->createPlaneMesh(dimension2d<f32>(ATOM_SIZE, ATOM_SIZE), dimension2d<u32>(1, 1), nullptr);
        node = scene->addMeshSceneNode(mesh, nullptr, -1, basePosition + cubeFaceOffset[side], cubeFaceRotation[side]);
        node->setMaterialTexture(0, m_faceTexture);
        face->meshNode = node;
        face->basePosition = basePosition + cubeFaceOffset[side];
        face->baseRotation = cubeFaceRotation[side];
    }

    return face;
}

// builds rubik cube atom (one of those little cubes)
CubeAtom* RubikCube::BuildCubeAtom(ISceneManager* scene, IVideoDriver* videoDriver, vector3df basePosition, vector3di cubeOffset)
{
    CubeAtom* atom = new CubeAtom();

    // builds all faces and sets them implicit "none" color
    for (int i = CF_BEGIN; i < CF_END; i++)
    {
        atom->faces[i] = BuildFace(scene, videoDriver, (CubeFace)i, basePosition);
        atom->faces[i]->setColor(CL_NONE);
    }

    // if it's the cube on some of edges/corners, assign color to it

    // these are implicit colors, but every cube load from file will overwrite it

    if (cubeOffset.X == 1)
        atom->faces[CF_RIGHT]->setColor(CL_YELLOW);
    else if (cubeOffset.X == -1)
        atom->faces[CF_LEFT]->setColor(CL_BLUE);

    if (cubeOffset.Z == -1)
        atom->faces[CF_FRONT]->setColor(CL_GREEN);
    else if (cubeOffset.Z == 1)
        atom->faces[CF_BACK]->setColor(CL_ORANGE);

    if (cubeOffset.Y == 1)
        atom->faces[CF_UP]->setColor(CL_RED);
    else if (cubeOffset.Y == -1)
        atom->faces[CF_DOWN]->setColor(CL_WHITE);

    return atom;
}

// stores cube atom to internal array
void RubikCube::SetCubeAtom(int x, int y, int z, CubeAtom* atom)
{
    m_cubeAtoms[(x + 1)][(y + 1)][(z + 1)] = atom;
}

// retrieves cube atom from internal array
CubeAtom* RubikCube::GetAtom(int x, int y, int z)
{
    return m_cubeAtoms[(x + 1)][(y + 1)][(z + 1)];
}

// caches cube to 3D array, so we can easily use it to text/graphical output
void RubikCube::CacheCube()
{
    CubeAtom* ca;

    for (int x = 0; x <= 2; x++)
    {
        for (int y = 0; y <= 2; y++)
        {
            for (int z = 0; z <= 2; z++)
            {
                // skip central cube - it's not visible at all
                if (x == 1 && y == 1 && z == 1)
                    continue;
                // retrieve atom (coordinates are from -1 to 1)
                ca = GetAtom(x-1, y-1, z-1);
                // if it does not exist (should not happen), skip
                if (ca == nullptr)
                    continue;

                // and cache colors depending on cube position

                if (z == 0)
                    m_cubeCache[CF_FRONT][x][y] = ca->faces[CF_FRONT]->getColor();
                else if (z == 2)
                    m_cubeCache[CF_BACK][x][y] = ca->faces[CF_BACK]->getColor();

                if (x == 2)
                    m_cubeCache[CF_RIGHT][y][z] = ca->faces[CF_RIGHT]->getColor();
                else if (x == 0)
                    m_cubeCache[CF_LEFT][2-y][z] = ca->faces[CF_LEFT]->getColor();

                if (y == 2)
                    m_cubeCache[CF_UP][x][z] = ca->faces[CF_UP]->getColor();
                else if (y == 0)
                    m_cubeCache[CF_DOWN][x][z] = ca->faces[CF_DOWN]->getColor();
            }
        }
    }
}

// restores cube cache to graphical representation; this comes in handy when loading from file
void RubikCube::RestoreCacheCube()
{
    CubeAtom* ca;

    for (int x = 0; x <= 2; x++)
    {
        for (int y = 0; y <= 2; y++)
        {
            for (int z = 0; z <= 2; z++)
            {
                // skip central cube (not visible at all)
                if (x == 1 && y == 1 && z == 1)
                    continue;
                // get cube atom at those coords
                ca = GetAtom(x - 1, y - 1, z - 1);
                if (ca == nullptr)
                    continue;

                // and restore state of each cube side from cache

                if (z == 0)
                    ca->faces[CF_FRONT]->setColor(m_cubeCache[CF_FRONT][x][y]);
                else if (z == 2)
                    ca->faces[CF_BACK]->setColor(m_cubeCache[CF_BACK][x][y]);

                if (x == 2)
                    ca->faces[CF_RIGHT]->setColor(m_cubeCache[CF_RIGHT][y][z]);
                else if (x == 0)
                    ca->faces[CF_LEFT]->setColor(m_cubeCache[CF_LEFT][2 - y][z]);

                if (y == 2)
                    ca->faces[CF_UP]->setColor(m_cubeCache[CF_UP][x][z]);
                else if (y == 0)
                    ca->faces[CF_DOWN]->setColor(m_cubeCache[CF_DOWN][x][z]);
            }
        }
    }
}

// main rendering function - called just in GUI mode
void RubikCube::Render()
{
    // no rendering in nogui mode
    if (!sApplication->IsGraphicMode())
        return;

    // if there is flips in processing (or in queue, ..)
    if (m_toProgress != FLIP_NONE && m_progressStart > 0)
    {
        // calculate progress (from progress start to now, should be number from 0 to 1)
        unsigned int diff = getMSTimeDiff(m_progressStart, getMSTime());
        float progress = ((float)diff) / (float)m_flipTiming;

        // ending flag is set when progress is greater or equal 1 (means we finished progress on this flip)
        bool endflag = false;
        if (progress >= 1.0f)
        {
            endflag = true;
            // note this line - we set progress to 0, so the cube is restored to original transformation
            // BUT, after all those transformation magic, we performs flip in colors, so the cube would not
            // be anyhow transformated, we just change colors at the end of transformation, and then restore
            // cubes to original state
            progress = 0;
        }

        int x, y, z, dirX, dirY, dirZ, rotX, rotY, rotZ;
        int group;

        // get all affected cubes
        CubeAtom* at;
        // we iterate through 2 dimensions, because the third one is fixed in all flips
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                // rotation is, at first, determined from orintation of flip (+ or -)
                rotX = (m_toProgress % 3) == 2 ? -1 : 1;
                rotY = rotX;
                rotZ = rotX;

                // Now, this is madness
                // we split flips to groups with two (four) members in the same axis, i.e. front and back are
                // in the same group, because the flip behaves nearly the same (just orientation and offsets are bit different)
                switch (m_toProgress)
                {
                    // GROUP 0 - front and back
                    case FLIP_F_P:
                    case FLIP_F_N:
                        x = i;
                        y = j;
                        z = -1;
                        rotX = 0;
                        rotY = 0;
                        rotZ *= 1;
                        dirX = rotZ;
                        dirY = 1;

                        group = 0;
                        break;
                    case FLIP_B_P:
                    case FLIP_B_N:
                        x = - i;
                        y = j;
                        z = 1;
                        rotX = 0;
                        rotY = 0;
                        rotZ *= -1;
                        dirX = rotZ;
                        dirY = 1;

                        group = 0;
                        break;
                    // GROUP 1 - left and right
                    case FLIP_L_P:
                    case FLIP_L_N:
                        x = -1;
                        y = j;
                        z = -i;
                        rotX *= 1;
                        rotY = 0;
                        rotZ = 0;
                        dirX = 1;
                        dirY = 1;
                        dirZ = -rotX;

                        group = 1;
                        break;
                    case FLIP_R_P:
                    case FLIP_R_N:
                        x = 1;
                        y = j;
                        z = i;
                        rotX *= -1;
                        rotY = 0;
                        rotZ = 0;
                        dirX = 1;
                        dirY = 1;
                        dirZ = -rotX;

                        group = 1;
                        break;
                    // GROUP 2 - up and down
                    case FLIP_U_P:
                    case FLIP_U_N:
                        x = i;
                        y = 1;
                        z = j;
                        rotX = 0;
                        rotY *= -1;
                        rotZ = 0;
                        dirX = 0;
                        dirY = rotY;
                        dirZ = 0;

                        group = 2;
                        break;
                    case FLIP_D_P:
                    case FLIP_D_N:
                        x = -i;
                        y = -1;
                        z = j;
                        rotX = 0;
                        rotY *= 1;
                        rotZ = 0;
                        dirX = 0;
                        dirY = -rotY;
                        dirZ = 0;

                        group = 2;
                        break;
                }

                // retrieve atom
                at = GetAtom(x, y, z);
                if (!at)
                    continue;

                // update all faces
                for (int f = CF_BEGIN; f < CF_END; f++)
                {
                    CubeAtomFace* caf = at->faces[f];
                    if (!caf->meshNode)
                        continue;
                    bool black = caf->color == CL_NONE;

                    // group 0 = front and back flip
                    if (group == 0)
                    {
                        float baseAngle = atan(caf->basePosition.Y / caf->basePosition.X);
                        if (x == -1)
                            baseAngle -= PI;

                        // otherwise glitches would appear
                        if (x == 0 && black && f == CF_LEFT)
                            baseAngle += PI;

                        float baseDist = sqrt(caf->basePosition.X*caf->basePosition.X + caf->basePosition.Y*caf->basePosition.Y);

                        // do not move center
                        if (x != 0 || y != 0)
                        {
                            // move face to transformed position
                            caf->meshNode->setPosition(
                                vector3df
                                (
                                    cos(baseAngle - rotZ * progress*PI / 2.0f)*baseDist - dirX * progress * ATOM_SIZE / 2.0f,
                                    sin(baseAngle - rotZ * progress*PI / 2.0f)*baseDist + dirY * progress * ATOM_SIZE / 2.0f,
                                    caf->basePosition.Z
                                )
                            );
                        }

                        // we also need to rotate it a bit
                        caf->meshNode->setRotation(
                            vector3df
                            (
                                caf->baseRotation.X - rotX * progress * 90.0f,
                                caf->baseRotation.Y - rotY * progress * 90.0f,
                                caf->baseRotation.Z - rotZ * progress * 90.0f
                            )
                        );
                    }
                    // group 1 = left and right flip
                    else if (group == 1)
                    {
                        float baseAngle = atan(caf->basePosition.Y / caf->basePosition.Z);
                        if (z == -1)
                            baseAngle -= PI;

                        // otherwise glitches would appear
                        if (z == 0 && black && f == CF_FRONT)
                            baseAngle += PI;

                        float baseDist = sqrt(caf->basePosition.Z*caf->basePosition.Z + caf->basePosition.Y*caf->basePosition.Y);
                        // do not move center
                        if (z != 0 || y != 0)
                        {
                            caf->meshNode->setPosition(
                                vector3df
                                (
                                    caf->basePosition.X,
                                    sin(baseAngle + rotX * progress*PI / 2.0f)*baseDist + dirY * progress * ATOM_SIZE / 2.0f,
                                    cos(baseAngle + rotX * progress*PI / 2.0f)*baseDist - dirZ * progress * ATOM_SIZE / 2.0f
                                )
                            );
                        }

                        if (f == CF_RIGHT || f == CF_LEFT)
                        {
                            caf->meshNode->setRotation(
                                vector3df
                                (
                                    caf->baseRotation.X,
                                    caf->baseRotation.Y + (((f == CF_LEFT) ? (rotX) : (-rotX)) * progress * 90.0f),
                                    caf->baseRotation.Z
                                )
                            );
                        }
                        else
                        {
                            caf->meshNode->setRotation(
                                vector3df
                                (
                                    caf->baseRotation.X - rotX * progress * 90.0f,
                                    caf->baseRotation.Y - rotY * progress * 90.0f,
                                    caf->baseRotation.Z - rotZ * progress * 90.0f
                                )
                            );
                        }
                    }
                    // group 2 = up and down flip
                    else if (group == 2)
                    {
                        float baseAngle = atan(caf->basePosition.Z / caf->basePosition.X);
                        if (x == -1)
                            baseAngle -= PI;

                        float baseDist = sqrt(caf->basePosition.Z*caf->basePosition.Z + caf->basePosition.X*caf->basePosition.X);

                        // do not move center
                        if (x != 0 || z != 0)
                        {
                            caf->meshNode->setPosition(
                                vector3df
                                    (
                                        cos(baseAngle + rotY * progress*PI / 2.0f)*baseDist + dirX * progress * ATOM_SIZE / 4.0f,
                                        caf->basePosition.Y,
                                        sin(baseAngle + rotY * progress*PI / 2.0f)*baseDist + dirZ * progress * ATOM_SIZE / 4.0f
                                    )
                                );
                        }

                        if (f == CF_UP || f == CF_DOWN)
                        {
                            caf->meshNode->setRotation(
                                vector3df
                                (
                                    caf->baseRotation.X,
                                    caf->baseRotation.Y - rotY * progress * 90.0f,
                                    caf->baseRotation.Z
                                )
                            );
                        }
                        else if (f == CF_RIGHT || f == CF_LEFT)
                        {
                            caf->meshNode->setRotation(
                                vector3df
                                (
                                    caf->baseRotation.X + ((f == CF_LEFT) ? (-rotY) : (rotY)) * progress * 90.0f,
                                    caf->baseRotation.Y,
                                    caf->baseRotation.Z
                                )
                            );
                        }
                        else // front / back
                        {
                            caf->meshNode->setRotation(
                                vector3df
                                (
                                    caf->baseRotation.X,
                                    caf->baseRotation.Y - rotY * progress * 90.0f,
                                    caf->baseRotation.Z
                                )
                            );
                        }
                    }
                }
            }
        }

        // if ending flag is set
        if (endflag)
        {
            // perform color change
            DoFlip(m_toProgress, true);

            // set next move from queue if any
            if (m_flipQueue.size() > 0)
            {
                cout << getStrForFlip(m_toProgress) << ", ";
                m_toProgress = m_flipQueue.front();
                m_flipQueue.pop();

                m_progressStart = getMSTime();
            }
            else // if not, end flipping
            {
                m_toProgress = FLIP_NONE;
                cout << "\b\b  " << endl;
            }
        }
    }

    // Now draw the 2D printout of rubik's cube

    int i, j;

    // up face (offset to be up to front side)
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            sDrawing->getDriver()->draw2DImage(m_faceMiniTexture, core::position2d<s32>(2 + 60 + i*20, 2 + 40 - j*20),
                core::rect<s32>(0, 0, 20, 20), nullptr,
                rubikColorMap[m_cubeCache[CF_UP][i][j]], false);
        }
    }

    // middle layer - left, front, right and back faces
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            sDrawing->getDriver()->draw2DImage(m_faceMiniTexture, core::position2d<s32>(2 + 40 - j * 20, 2 + 60 + i * 20),
                core::rect<s32>(0, 0, 20, 20), nullptr,
                rubikColorMap[m_cubeCache[CF_LEFT][i][j]], false);
        }
        for (j = 0; j < 3; j++)
        {
            sDrawing->getDriver()->draw2DImage(m_faceMiniTexture, core::position2d<s32>(2 + 60 + i * 20, 2 + 60 + 40 - j * 20),
                core::rect<s32>(0, 0, 20, 20), nullptr,
                rubikColorMap[m_cubeCache[CF_FRONT][i][j]], false);
        }
        for (j = 0; j < 3; j++)
        {
            sDrawing->getDriver()->draw2DImage(m_faceMiniTexture, core::position2d<s32>(2 + 2*60 + j * 20, 2 + 60 + 40 - i * 20),
                core::rect<s32>(0, 0, 20, 20), nullptr,
                rubikColorMap[m_cubeCache[CF_RIGHT][i][j]], false);
        }
        for (j = 0; j < 3; j++)
        {
            sDrawing->getDriver()->draw2DImage(m_faceMiniTexture, core::position2d<s32>(2 + 3*60 + 40 - i * 20, 2 + 60 + 40 - j * 20),
                core::rect<s32>(0, 0, 20, 20), nullptr,
                rubikColorMap[m_cubeCache[CF_BACK][i][j]], false);
        }
    }

    // down face - also have offset to be under front face
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            sDrawing->getDriver()->draw2DImage(m_faceMiniTexture, core::position2d<s32>(2 + 60 + i * 20, 2 + 2*60 + j * 20),
                core::rect<s32>(0, 0, 20, 20), nullptr,
                rubikColorMap[m_cubeCache[CF_DOWN][i][j]], false);
        }
    }
}

// generates random sequence of flips to mix the cube
void RubikCube::Scramble(std::list<CubeFlip> *target)
{
    CubeFlip a;
    int count = 20 + rand() % 10;
    for (int i = 0; i < count; i++)
    {
        a = (CubeFlip)(rand() % FLIP_MAX);
        target->push_back(a);
    }
}

// proceeds supplied flip sequence, or pushes it into queue to be animated
void RubikCube::ProceedFlipSequence(std::list<CubeFlip> *source, bool animate)
{
    // if not animating, just flip the cube instantly
    if (!animate)
    {
        for (std::list<CubeFlip>::iterator itr = source->begin(); itr != source->end(); ++itr)
            DoFlip(*itr, true);
    }
    else // if yes, put it into queue and animate
    {
        // if no moves specified, return
        if (!source || source->empty())
            return;

        // copy flips to queue
        CubeFlip fl;
        for (std::list<CubeFlip>::iterator itr = source->begin(); itr != source->end(); ++itr)
        {
            fl = *itr;

            if (fl % 3 == 1)
            {
                fl = (CubeFlip)(fl - 1);
                m_flipQueue.push(fl);
            }
            m_flipQueue.push(fl);
        }

        // pop first capable flip, and set it to processing state
        m_toProgress = m_flipQueue.front();
        m_flipQueue.pop();

        // show something on console
        cout << getStrForFlip(m_toProgress) << ", ";

        // and set progress start time
        m_progressStart = getMSTime();
    }
}

// builds cube using supplied scene manager and video driver
void RubikCube::BuildCube(ISceneManager* scene, IVideoDriver* videoDriver)
{
    if (sApplication->IsGraphicMode())
    {
        // get mesh manipulator
        meshManipulator = scene->getMeshManipulator();

        // build textures and store pointer to their structures
        m_faceTexture = videoDriver->getTexture(DATA_DIR "face.bmp");
        m_faceMiniTexture = videoDriver->getTexture(DATA_DIR "mini_face.bmp");
    }

    CubeAtom* tmp;

    // build all cube atoms
    for (int ix = -1; ix <= 1; ix++)
    {
        for (int iy = -1; iy <= 1; iy++)
        {
            for (int iz = -1; iz <= 1; iz++)
            {
                // skip the center (not visible cube)
                if (ix == 0 && iy == 0 && iz == 0)
                    continue;

                tmp = BuildCubeAtom(scene, videoDriver, vector3df(ix*(ATOM_SIZE + ATOM_SPACING), iy*(ATOM_SIZE + ATOM_SPACING), iz*(ATOM_SIZE + ATOM_SPACING)),
                    vector3di(ix, iy, iz));

                SetCubeAtom(ix, iy, iz, tmp);
            }
        }
    }

    CacheCube();
}

// circular swap of cube atom faces
void RubikCube::AtomCircularSwap(int ax, int ay, int az, CubeFace a, int bx, int by, int bz, CubeFace b, int cx, int cy, int cz, CubeFace c, int dx, int dy, int dz, CubeFace d, bool reverse /* = false */)
{
    // get atoms
    CubeAtom* _a = GetAtom(ax, ay, az);
    CubeAtom* _b = GetAtom(bx, by, bz);
    CubeAtom* _c = GetAtom(cx, cy, cz);
    CubeAtom* _d = GetAtom(dx, dy, dz);

    // get their original values
    RubikColor c_a = _a->faces[a]->getColor();
    RubikColor c_b = _b->faces[b]->getColor();
    RubikColor c_c = _c->faces[c]->getColor();
    RubikColor c_d = _d->faces[d]->getColor();

    // yes, this may be a bit more optimalized, but this step is done only when we know what we are doing
    // - means we use different flipping (swapping, ..) method for critical applications like cube solving

    // and flip them, in requested direction
    if (!reverse)
    {
        _a->faces[a]->setColor(c_d);
        _b->faces[b]->setColor(c_a);
        _c->faces[c]->setColor(c_b);
        _d->faces[d]->setColor(c_c);
    }
    else
    {
        _a->faces[a]->setColor(c_b);
        _b->faces[b]->setColor(c_c);
        _c->faces[c]->setColor(c_d);
        _d->faces[d]->setColor(c_a);
    }
}

// perform swap on visual cube, and prepare inputs for solving mechanism as well (same source)
void RubikCube::DoFlip(CubeFlip flip, bool draw)
{
    switch (flip)
    {
        case FLIP_F_P:
        {
            AtomCircularSwap(-1, 1, -1, CF_UP,   1, 1, -1, CF_RIGHT, 1, -1, -1, CF_DOWN,  -1, -1, -1, CF_LEFT);
            AtomCircularSwap( 0, 1, -1, CF_UP,   1, 0, -1, CF_RIGHT, 0, -1, -1, CF_DOWN,  -1,  0, -1, CF_LEFT);
            AtomCircularSwap(-1, 1, -1, CF_LEFT, 1, 1, -1, CF_UP,    1, -1, -1, CF_RIGHT, -1, -1, -1, CF_DOWN);

            AtomCircularSwap(-1, 1, -1, CF_FRONT, 1, 1, -1, CF_FRONT, 1, -1, -1, CF_FRONT, -1, -1, -1, CF_FRONT);
            AtomCircularSwap(0, 1, -1,  CF_FRONT, 1, 0, -1, CF_FRONT, 0, -1, -1, CF_FRONT, -1, 0, -1,  CF_FRONT);
            break;
        }
        case FLIP_F_2:
        {
            DoFlip(FLIP_F_P, false);
            DoFlip(FLIP_F_P, draw);
            break;
        }
        case FLIP_F_N:
        {
            AtomCircularSwap(-1, 1, -1, CF_UP, 1, 1, -1, CF_RIGHT, 1, -1, -1, CF_DOWN, -1, -1, -1, CF_LEFT, true);
            AtomCircularSwap(0, 1, -1, CF_UP, 1, 0, -1, CF_RIGHT, 0, -1, -1, CF_DOWN, -1, 0, -1, CF_LEFT, true);
            AtomCircularSwap(-1, 1, -1, CF_LEFT, 1, 1, -1, CF_UP, 1, -1, -1, CF_RIGHT, -1, -1, -1, CF_DOWN, true);

            AtomCircularSwap(-1, 1, -1, CF_FRONT, 1, 1, -1, CF_FRONT, 1, -1, -1, CF_FRONT, -1, -1, -1, CF_FRONT, true);
            AtomCircularSwap(0, 1, -1, CF_FRONT, 1, 0, -1, CF_FRONT, 0, -1, -1, CF_FRONT, -1, 0, -1, CF_FRONT, true);
            break;
        }
        case FLIP_U_P:
        {
            AtomCircularSwap(-1, 1, -1, CF_FRONT, 1, 1, -1, CF_RIGHT, 1, 1, 1, CF_BACK, -1, 1, 1, CF_LEFT, true);
            AtomCircularSwap(0, 1, -1, CF_FRONT, 1, 1, 0, CF_RIGHT, 0, 1, 1, CF_BACK, -1, 1, 0, CF_LEFT, true);
            AtomCircularSwap(-1, 1, -1, CF_LEFT, 1, 1, -1, CF_FRONT, 1, 1, 1, CF_RIGHT, -1, 1, 1, CF_BACK, true);

            AtomCircularSwap(-1, 1, -1, CF_UP, 1, 1, -1, CF_UP, 1, 1, 1, CF_UP, -1, 1, 1, CF_UP, true);
            AtomCircularSwap(0, 1, -1, CF_UP, 1, 1, 0, CF_UP, 0, 1, 1, CF_UP, -1, 1, 0, CF_UP, true);
            break;
        }
        case FLIP_U_2:
        {
            DoFlip(FLIP_U_P, draw);
            DoFlip(FLIP_U_P, draw);
            break;
        }
        case FLIP_U_N:
        {
            AtomCircularSwap(-1, 1, -1, CF_FRONT, 1, 1, -1, CF_RIGHT, 1, 1, 1, CF_BACK, -1, 1, 1, CF_LEFT);
            AtomCircularSwap(0, 1, -1, CF_FRONT, 1, 1, 0, CF_RIGHT, 0, 1, 1, CF_BACK, -1, 1, 0, CF_LEFT);
            AtomCircularSwap(-1, 1, -1, CF_LEFT, 1, 1, -1, CF_FRONT, 1, 1, 1, CF_RIGHT, -1, 1, 1, CF_BACK);

            AtomCircularSwap(-1, 1, -1, CF_UP, 1, 1, -1, CF_UP, 1, 1, 1, CF_UP, -1, 1, 1, CF_UP);
            AtomCircularSwap(0, 1, -1, CF_UP, 1, 1, 0, CF_UP, 0, 1, 1, CF_UP, -1, 1, 0, CF_UP);
            break;
        }
        case FLIP_R_P:
        {
            AtomCircularSwap(1, 1, -1, CF_FRONT, 1, 1, 1, CF_UP, 1, -1, 1, CF_BACK, 1, -1, -1, CF_DOWN);
            AtomCircularSwap(1, 1, 0, CF_UP, 1, 0, 1, CF_BACK, 1, -1, 0, CF_DOWN, 1, 0, -1, CF_FRONT);
            AtomCircularSwap(1, 1, -1, CF_UP, 1, 1, 1, CF_BACK, 1, -1, 1, CF_DOWN, 1, -1, -1, CF_FRONT);

            AtomCircularSwap(1, 1, -1, CF_RIGHT, 1, 1, 1, CF_RIGHT, 1, -1, 1, CF_RIGHT, 1, -1, -1, CF_RIGHT);
            AtomCircularSwap(1, 1, 0, CF_RIGHT, 1, 0, 1, CF_RIGHT, 1, -1, 0, CF_RIGHT, 1, 0, -1, CF_RIGHT);
            break;
        }
        case FLIP_R_2:
        {
            DoFlip(FLIP_R_P, draw);
            DoFlip(FLIP_R_P, draw);
            break;
        }
        case FLIP_R_N:
        {
            AtomCircularSwap(1, 1, -1, CF_FRONT, 1, 1, 1, CF_UP, 1, -1, 1, CF_BACK, 1, -1, -1, CF_DOWN, true);
            AtomCircularSwap(1, 1, 0, CF_UP, 1, 0, 1, CF_BACK, 1, -1, 0, CF_DOWN, 1, 0, -1, CF_FRONT, true);
            AtomCircularSwap(1, 1, -1, CF_UP, 1, 1, 1, CF_BACK, 1, -1, 1, CF_DOWN, 1, -1, -1, CF_FRONT, true);

            AtomCircularSwap(1, 1, -1, CF_RIGHT, 1, 1, 1, CF_RIGHT, 1, -1, 1, CF_RIGHT, 1, -1, -1, CF_RIGHT, true);
            AtomCircularSwap(1, 1, 0, CF_RIGHT, 1, 0, 1, CF_RIGHT, 1, -1, 0, CF_RIGHT, 1, 0, -1, CF_RIGHT, true);
            break;
        }
        case FLIP_L_P:
        {
            AtomCircularSwap(-1, 1, -1, CF_FRONT, -1, 1, 1, CF_UP, -1, -1, 1, CF_BACK, -1, -1, -1, CF_DOWN, true);
            AtomCircularSwap(-1, 1, 0, CF_UP, -1, 0, 1, CF_BACK, -1, -1, 0, CF_DOWN, -1, 0, -1, CF_FRONT, true);
            AtomCircularSwap(-1, 1, -1, CF_UP, -1, 1, 1, CF_BACK, -1, -1, 1, CF_DOWN, -1, -1, -1, CF_FRONT, true);

            AtomCircularSwap(-1, 1, -1, CF_LEFT, -1, 1, 1, CF_LEFT, -1, -1, 1, CF_LEFT, -1, -1, -1, CF_LEFT, true);
            AtomCircularSwap(-1, 1, 0, CF_LEFT, -1, 0, 1, CF_LEFT, -1, -1, 0, CF_LEFT, -1, 0, -1, CF_LEFT, true);
            break;
        }
        case FLIP_L_2:
        {
            DoFlip(FLIP_L_P, draw);
            DoFlip(FLIP_L_P, draw);
            break;
        }
        case FLIP_L_N:
        {
            AtomCircularSwap(-1, 1, -1, CF_FRONT, -1, 1, 1, CF_UP, -1, -1, 1, CF_BACK, -1, -1, -1, CF_DOWN);
            AtomCircularSwap(-1, 1, 0, CF_UP, -1, 0, 1, CF_BACK, -1, -1, 0, CF_DOWN, -1, 0, -1, CF_FRONT);
            AtomCircularSwap(-1, 1, -1, CF_UP, -1, 1, 1, CF_BACK, -1, -1, 1, CF_DOWN, -1, -1, -1, CF_FRONT);

            AtomCircularSwap(-1, 1, -1, CF_LEFT, -1, 1, 1, CF_LEFT, -1, -1, 1, CF_LEFT, -1, -1, -1, CF_LEFT);
            AtomCircularSwap(-1, 1, 0, CF_LEFT, -1, 0, 1, CF_LEFT, -1, -1, 0, CF_LEFT, -1, 0, -1, CF_LEFT);
            break;
        }
        case FLIP_B_P:
        {
            AtomCircularSwap(-1, 1, 1, CF_UP, 1, 1, 1, CF_RIGHT, 1, -1, 1, CF_DOWN, -1, -1, 1, CF_LEFT, true);
            AtomCircularSwap(0, 1, 1, CF_UP, 1, 0, 1, CF_RIGHT, 0, -1, 1, CF_DOWN, -1, 0, 1, CF_LEFT, true);
            AtomCircularSwap(-1, 1, 1, CF_LEFT, 1, 1, 1, CF_UP, 1, -1, 1, CF_RIGHT, -1, -1, 1, CF_DOWN, true);

            AtomCircularSwap(-1, 1, 1, CF_BACK, 1, 1, 1, CF_BACK, 1, -1, 1, CF_BACK, -1, -1, 1, CF_BACK, true);
            AtomCircularSwap(0, 1, 1, CF_BACK, 1, 0, 1, CF_BACK, 0, -1, 1, CF_BACK, -1, 0, 1, CF_BACK, true);
            break;
        }
        case FLIP_B_2:
        {
            DoFlip(FLIP_B_P, draw);
            DoFlip(FLIP_B_P, draw);
            break;
        }
        case FLIP_B_N:
        {
            AtomCircularSwap(-1, 1, 1, CF_UP, 1, 1, 1, CF_RIGHT, 1, -1, 1, CF_DOWN, -1, -1, 1, CF_LEFT);
            AtomCircularSwap(0, 1, 1, CF_UP, 1, 0, 1, CF_RIGHT, 0, -1, 1, CF_DOWN, -1, 0, 1, CF_LEFT);
            AtomCircularSwap(-1, 1, 1, CF_LEFT, 1, 1, 1, CF_UP, 1, -1, 1, CF_RIGHT, -1, -1, 1, CF_DOWN);

            AtomCircularSwap(-1, 1, 1, CF_BACK, 1, 1, 1, CF_BACK, 1, -1, 1, CF_BACK, -1, -1, 1, CF_BACK);
            AtomCircularSwap(0, 1, 1, CF_BACK, 1, 0, 1, CF_BACK, 0, -1, 1, CF_BACK, -1, 0, 1, CF_BACK);
            break;
        }
        case FLIP_D_P:
        {
            AtomCircularSwap(-1, -1, -1, CF_FRONT, 1, -1, -1, CF_RIGHT, 1, -1, 1, CF_BACK, -1, -1, 1, CF_LEFT);
            AtomCircularSwap(0, -1, -1, CF_FRONT, 1, -1, 0, CF_RIGHT, 0, -1, 1, CF_BACK, -1, -1, 0, CF_LEFT);
            AtomCircularSwap(-1, -1, -1, CF_LEFT, 1, -1, -1, CF_FRONT, 1, -1, 1, CF_RIGHT, -1, -1, 1, CF_BACK);

            AtomCircularSwap(-1, -1, -1, CF_DOWN, 1, -1, -1, CF_DOWN, 1, -1, 1, CF_DOWN, -1, -1, 1, CF_DOWN);
            AtomCircularSwap(0, -1, -1, CF_DOWN, 1, -1, 0, CF_DOWN, 0, -1, 1, CF_DOWN, -1, -1, 0, CF_DOWN);
            break;
        }
        case FLIP_D_2:
        {
            DoFlip(FLIP_D_P, draw);
            DoFlip(FLIP_D_P, draw);
            break;
        }
        case FLIP_D_N:
        {
            AtomCircularSwap(-1, -1, -1, CF_FRONT, 1, -1, -1, CF_RIGHT, 1, -1, 1, CF_BACK, -1, -1, 1, CF_LEFT, true);
            AtomCircularSwap(0, -1, -1, CF_FRONT, 1, -1, 0, CF_RIGHT, 0, -1, 1, CF_BACK, -1, -1, 0, CF_LEFT, true);
            AtomCircularSwap(-1, -1, -1, CF_LEFT, 1, -1, -1, CF_FRONT, 1, -1, 1, CF_RIGHT, -1, -1, 1, CF_BACK, true);

            AtomCircularSwap(-1, -1, -1, CF_DOWN, 1, -1, -1, CF_DOWN, 1, -1, 1, CF_DOWN, -1, -1, 1, CF_DOWN, true);
            AtomCircularSwap(0, -1, -1, CF_DOWN, 1, -1, 0, CF_DOWN, 0, -1, 1, CF_DOWN, -1, -1, 0, CF_DOWN, true);
            break;
        }
    }

    if (draw)
        CacheCube();
}

// prints out cube as string lines sequence
void RubikCube::PrintOut()
{
    int i, j;

    cout << endl;

    // upper side, padded by spaces
    for (i = 0; i < 3; i++)
    {
        cout << "   ";
        for (j = 0; j < 3; j++)
            cout << rubikColorCode[m_cubeCache[CF_UP][i][j]];
        cout << endl;
    }

    // left, front, right and back sides
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
            cout << rubikColorCode[m_cubeCache[CF_LEFT][i][j]];
        for (j = 0; j < 3; j++)
            cout << rubikColorCode[m_cubeCache[CF_FRONT][i][j]];
        for (j = 0; j < 3; j++)
            cout << rubikColorCode[m_cubeCache[CF_RIGHT][i][j]];
        for (j = 0; j < 3; j++)
            cout << rubikColorCode[m_cubeCache[CF_BACK][i][j]];
        cout << endl;
    }

    // down side, padded by spaces
    for (i = 0; i < 3; i++)
    {
        cout << "   ";
        for (j = 0; j < 3; j++)
            cout << rubikColorCode[m_cubeCache[CF_DOWN][i][j]];
        cout << endl;
    }
}

// performs flip in linearized state and returns new state
bigint RubikCube::DoLinearFlip(int move, bigint state)
{
    // how many turns do we have to do (i.e. for F2 we do 2 flips, for F3 we do 3, although its the same
    // as F-, which is 1 flip in negative direction, but this is more generic way to do it)
    int turns = move % 3 + 1;
    // which face are we about to turn
    int face = move / 3;
    bigint oldState;

    // as far as we have turns left
    while (turns)
    {
        turns--;
        // copy old state
        oldState.copy(state);
        for (int i = 0; i < 8; i++)
        {
            // if it's corner, the behaviour is different
            int isCorner = i > 3;

            // what index do we modify
            int target = flipCubeEffect[face][i] + isCorner * 12;

            // with what value (at which index) we will substitute
            int source = flipCubeEffect[face][(i & 3) == 3 ? i - 3 : i + 1] + isCorner * 12;

            // just determines the orientation of flipped edge/corner (index of cyclic permutation)
            int orientationDelta = isCorner ?
                                        ((face < 2) ? 0 : (2 - (i & 1))) // if its corner, it has possible permutation indexes of 0, 1 and 2
                                        :
                                        (face > 1 && face < 4); // otherwise the permutation index changes only in F and B flips

            state.d[target] = oldState.d[source];
            // add orientation delta, regardless of mow much the change would exceed limit; the orientation will be cut off by module on last turn
            state.d[target + 20] = oldState.d[source + 20] + orientationDelta;

            // at last turn, strip the hash portion to 1-2 bits
            if (!turns)
                state.d[target + 20] %= 2 + isCorner;
        }
    }
    return state;
}

// retrieve inverse move to current (just inverses the offset within move group)
// (see CubeFlip enumerator)
int inverse(int move)
{
    return move + 2 - 2 * (move % 3);
}

// hashes supplied state depending on solution stage we are in
// - it will hash only those things, we are about to solve this stage, so the rest
// could be almost anything (we don't care about the rest in our stage, maybe later)
bigint RubikCube::GetStateHash(bigint &state)
{
    // in stage 1 - we hash only edges (their orientation)
    if (m_solveStage == 1)
        return bigint(state, 20, 32);

    // in stage 2 - we hash corner orientations, and middle slice edges
    if (m_solveStage == 2){
        bigint result(state, 31, 40);
        // hash middle edges including corners
        for (int e = 0; e < 12; e++)
            result.d[0] |= (state.d[e] / 8) << e;
        return result;
    }

    // in stage 3 - we hash middle, center edges, and L+R edges+corners to fix parity
    if (m_solveStage == 3)
    {
        bigint result(3);

        int base = 0;

        // middle strip (12 sides)
        for (int e = 0; e < 12; e++)
            result.d[0] |= ((state.d[e] > 7) ? 2 : (state.d[e] & 1)) << (2 * e);

        base = 12;
        // center edges
        for (int c = 0; c < 8; c++)
            result.d[1] |= ((state.d[c + base] - base) & 5) << (3 * c);

        // edge+corners in L+R sides
        for (int i = 12; i < 20; i++)
            for (int j = i + 1; j < 20; j++)
                result.d[2] ^= state.d[i] > state.d[j];

        return result;
    }

    // in stage 4 we do not hash at all - we are now heading to complete solution,
    // so we have to "hash everything" (and that does not make much sense, so return whole state)
    return bigint(state);
}

// this will return permutation for current edge (means - we have to have red on upper side
// and yellow on right side, so we append U and R together and return)
char* RubikCube::GetEdgeCode(int x, int y, int z, CubeFace frst, CubeFace scnd)
{
    CubeAtom* ca;
    char* ret = new char[3];
    ret[2] = '\0';

    // retrieve desired atom
    ca = GetAtom(x, y, z);

    // and encode inputs
    ret[0] = rubikFaceCode[colorFaceMap[ca->faces[frst]->getColor()]];
    ret[1] = rubikFaceCode[colorFaceMap[ca->faces[scnd]->getColor()]];

    return ret;
}

// this will return permutation for current corner (means - we have to have red on upper side,
// yellow on right side and green on front side, so we append U, R and F together and return)
char* RubikCube::GetCornerCode(int x, int y, int z, CubeFace frst, CubeFace scnd, CubeFace thrd)
{
    CubeAtom* ca;
    char* ret = new char[4];
    ret[3] = '\0';

    ca = GetAtom(x, y, z);

    ret[0] = rubikFaceCode[colorFaceMap[ca->faces[frst]->getColor()]];
    ret[1] = rubikFaceCode[colorFaceMap[ca->faces[scnd]->getColor()]];
    ret[2] = rubikFaceCode[colorFaceMap[ca->faces[thrd]->getColor()]];

    return ret;
}

// this will convert current state of Rubik's cube to permutation of edges
// - this is very important step, because as permutation table, the state is
// linearized, and we are able to work with it much faster
void RubikCube::ConvertToPermutationTable(std::vector<std::string> &dst)
{
    // upper edges
    dst.push_back(std::string(GetEdgeCode(0, 1, -1, CF_UP, CF_FRONT)));
    dst.push_back(std::string(GetEdgeCode(1, 1,  0, CF_UP, CF_RIGHT)));
    dst.push_back(std::string(GetEdgeCode(0, 1,  1, CF_UP, CF_BACK)));
    dst.push_back(std::string(GetEdgeCode(-1, 1, 0, CF_UP, CF_LEFT)));

    // down edges
    dst.push_back(std::string(GetEdgeCode(0, -1, -1, CF_DOWN, CF_FRONT)));
    dst.push_back(std::string(GetEdgeCode(1, -1, 0, CF_DOWN, CF_RIGHT)));
    dst.push_back(std::string(GetEdgeCode(0, -1, 1, CF_DOWN, CF_BACK)));
    dst.push_back(std::string(GetEdgeCode(-1, -1, 0, CF_DOWN, CF_LEFT)));

    // frontal side edges
    dst.push_back(std::string(GetEdgeCode(1, 0, -1, CF_FRONT, CF_RIGHT)));
    dst.push_back(std::string(GetEdgeCode(-1, 0, -1, CF_FRONT, CF_LEFT)));

    // back side edges
    dst.push_back(std::string(GetEdgeCode(1, 0, 1, CF_BACK, CF_RIGHT)));
    dst.push_back(std::string(GetEdgeCode(-1, 0, 1, CF_BACK, CF_LEFT)));


    // top corners
    dst.push_back(std::string(GetCornerCode(1,  1, -1, CF_UP, CF_FRONT, CF_RIGHT)));
    dst.push_back(std::string(GetCornerCode(1, 1, 1, CF_UP, CF_RIGHT, CF_BACK)));
    dst.push_back(std::string(GetCornerCode(-1, 1, 1, CF_UP, CF_BACK, CF_LEFT)));
    dst.push_back(std::string(GetCornerCode(-1, 1, -1, CF_UP, CF_LEFT, CF_FRONT)));

    // bottom corners
    dst.push_back(std::string(GetCornerCode(1, -1, -1, CF_DOWN, CF_RIGHT, CF_FRONT)));
    dst.push_back(std::string(GetCornerCode(-1, -1, -1, CF_DOWN, CF_FRONT, CF_LEFT)));
    dst.push_back(std::string(GetCornerCode(-1, -1, 1, CF_DOWN, CF_LEFT, CF_BACK)));
    dst.push_back(std::string(GetCornerCode(1, -1, 1, CF_DOWN, CF_BACK, CF_RIGHT)));
}

void RubikCube::Solve(std::list<CubeFlip> *target)
{
    if (!target)
        return;

    // reset solve stage
    m_solveStage = 0;

    // permutation format inspired by: https://www.speedsolving.com/wiki/index.php/ACube

    // convert current cube to permutation table
    std::vector<std::string> permTable;
    ConvertToPermutationTable(permTable);

    // just convert current state, and destination state to hashed structures
    bigint currentState(40);
    bigint solvedState(40);
    std::string atom;

    for (int i = 0; i < STATE_STRING_LENGTH; i++)
    {
        solvedState.d[i] = i;

        atom = permTable[i];
        // this little funky part of code will lookup the atom (means UF, UL, .. formatted permutation) in
        // goal state. If it's not found, permutate it (in case of edge atom, just switch UF to FU, etc.),
        // and verify that again - it has to be there now, otherwise it's error in preformatter, and we got
        // another equivalence group of states, thus no solution
        // in upper part of hash (indexes 20 to 39) we store permutation index (0 = no permutation, 1 = first permutation, ..)
        int limit = atom.length() + 1;
        while ((currentState.d[i] = find(solvedPermutation, solvedPermutation + 20, atom) - solvedPermutation) == STATE_STRING_LENGTH)
        {
            atom = atom.substr(1) + atom[0];
            currentState.d[i + STATE_STRING_LENGTH]++;
            limit--;

            // no permutation of expected input found - there's no solution
            if (limit == 0)
            {
                target->clear();
                return;
            }
        }
    }

    queue<bigint> q;
    std::map<bigint, bigint> predecessor;
    std::map<bigint, int> direction;
    std::map<bigint, int> lastMove;

    // start five stage Thistlethwaite algorithm
    while (++m_solveStage < 5)
    {
        // hash current state and goal state
        // the hash is different in every state! It depends on what are we about to solve this stage
        // i.e. in stage 1 we hash only edges due to their orientation, etc.
        bigint currentId = GetStateHash(currentState);
        bigint solvedId = GetStateHash(solvedState);

        // if we are there, skip and end
        if (currentId == solvedId)
            continue;

        // clear BFS queue (pop what left, if neccessarry)
        while (!q.empty())
            q.pop();

        // at the bottom, push goal state, and over it, push current state
        q.push(currentState);
        q.push(solvedState);

        // init helper maps to be able to return / go forward when finding solution ("path" in state graph)
        predecessor.clear(); // map of predecessors, to determine return path
        direction.clear();   // here we will store directions of bidirectional BFS
        lastMove.clear();    // here we will store last move made on specific state - this will help us determine HOW
                             // we got from state A to state B

        // direction map (matches contents of bBFS queue)
        // 1 = go forward (towards solution)
        // 2 = go backwards (away from solution, towards current state)
        direction[currentId] = 1;
        direction[solvedId] = 2;

        // run bidirectional BFS
        while (true)
        {
            // queue is empty = there are no connections between two states
            // (the user may have entered odd permutation of faces, and that cannot be solved)
            if (q.empty())
            {
                target->clear();
                return;
            }

            // get state from queue
            bigint currState = q.front();
            q.pop();

            // compute its ID and get direction
            bigint currId = GetStateHash(currState);
            int &currDir = direction[currId];

            // try all allowed moves in specified stage
            // move types in stages are restricted using Thistletwaite's algorithm
            for (int move = FLIP_BEGIN; move < FLIP_MAX; move++)
            {
                if ((stageAllowedFlips[m_solveStage - 1] & (1 << move)) != 0)
                {
                    // flips the cube (linearized state)
                    bigint newState = DoLinearFlip(move, currState);
                    // computes new state id
                    bigint newId = GetStateHash(newState);
                    // and retrieves reference to direction record in dir map
                    int &newDir = direction[newId];

                    // if we already have been in new state, and its direction is different from current ("previous") state
                    // direction, we found a connection between forward and backward search of bidirectional BFS
                    if (newDir && newDir != currDir)
                    {
                        // swap those states to allow unified path search
                        if (currDir > 1)
                        {
                            swap(newId, currId);
                            move = inverse(move);
                        }

                        // reconstruct path using stored predecessor map and lastMove map
                        vector<int> path(1, move);
                        // forwards path
                        while (currId != currentId)
                        {
                            // insert to beginning, to have first move on head
                            path.insert(path.begin(), lastMove[currId]);
                            currId = predecessor[currId];
                        }
                        // backwards path
                        while (newId != solvedId)
                        {
                            // we need to inverse the last move to get the opposite direction
                            // push to the back, so we have last move 
                            path.push_back(inverse(lastMove[newId]));
                            newId = predecessor[newId];
                        }

                        // when we have our path complete, convert it to flips and push it to solution list
                        for (int i = 0; i < (int)path.size(); i++)
                        {
                            CubeFlip cfl = (CubeFlip)((5 - path[i] / 3) * 3 + path[i] % 3);
                            target->push_back(cfl);
                            currentState = DoLinearFlip(path[i], currentState);
                        }

                        // and then jump to next stage
                        // here's the case, when goto comes in handy - instead of setting two state booleans to
                        // be able to "continue" on outer loop, we use goto to bypass these needs
                        goto nextStage;
                    }

                    // we haven't been in this state yet, so save it
                    if (!newDir)
                    {
                        // to queue
                        q.push(newState);
                        // we came from "currdir" direction (newDir is reference, so it will pass to the map also)
                        newDir = currDir;
                        // save the move we used to get there
                        lastMove[newId] = move;
                        // and save its predecessor
                        predecessor[newId] = currId;
                    }
                }
            }
        }

        // label to jump to, when we are ready for next stage of Thistletwaite algorithm
    nextStage:
        ;
    }

    //
}

// loads cube configuration from file
bool RubikCube::LoadFromFile(char* filename)
{
    std::ifstream f(filename);
    if (f.fail() || !f.is_open())
    {
        cerr << "File " << filename << " does not exist." << endl;
        return false;
    }

    // stage one - read all non-empty lines, that does not start with hash mark (that's comment)
    std::vector<std::string> lines;
    std::string line;
    while (!f.eof() && getline(f, line))
    {
        if (line.length() > 0 && line.at(0) != '#')
            lines.push_back(std::string(line));
    }

    f.close();

    // now the only valid strings, that should be present are:
    // 1) at first 3 lines, there should be definition of spaces-preceded upper side
    // 2) at next 3 lines, there should be definition of faces L (left), F (front), R (right) and B (back)
    // 3) at next 3 lines, there should be definition of spaces-preceded down side

    // so there should be 9 lines in total, but to provide more precise error message,
    // check before every action

    if (lines.size() == 0)
    {
        cerr << "Invalid input file - the file " << filename << " is empty!" << endl;
        return false;
    }

    // first three lines should contain 6 characters (three spaces, and three letter definitions)
    char c;
    RubikColor rc;

    // load upper side
    for (int i = 0; i < 3; i++)
    {
        line = lines[i];
        // exactly 6 characters long, uses 3 spaces as indenting
        if (line.length() != 6 || line.at(0) != ' ' || line.at(1) != ' ' || line.at(2) != ' ')
        {
            cerr << "Invalid definition of cube upper side - make sure you used 3 spaces, and 3 valid characters" << endl;
            return false;
        }

        // load color char by char
        for (int j = 0; j < 3; j++)
        {
            c = line.at(3 + j);
            rc = getColorForCode(c);
            // if no color with this code found, report error
            if (rc == CL_NONE)
            {
                cerr << "Invalid color code " << c << " in upper side definition" << endl;
                return false;
            }

            // save to cache
            m_cubeCache[CF_UP][j][2-i] = rc;
        }
    }

    // the next 3 lines consist of 4 sides definition, so load them one by one
    for (int i = 0; i < 3; i++)
    {
        line = lines[i+3];
        // they have to be exactly 12 characters long (3 for each of 4 sides)
        if (line.length() != 12)
        {
            cerr << "Invalid definition of cube left+front+right+back side - make sure you used 12 valid characters" << endl;
            return false;
        }

        // load left side
        for (int j = 0; j < 3; j++)
        {
            c = line.at(j);
            rc = getColorForCode(c);
            if (rc == CL_NONE)
            {
                cerr << "Invalid color code " << c << " in left side definition" << endl;
                return false;
            }

            m_cubeCache[CF_LEFT][i][2-j] = rc;
        }

        // front side
        for (int j = 0; j < 3; j++)
        {
            c = line.at(j + 3);
            rc = getColorForCode(c);
            if (rc == CL_NONE)
            {
                cerr << "Invalid color code " << c << " in front side definition" << endl;
                return false;
            }

            m_cubeCache[CF_FRONT][j][2-i] = rc;
        }

        // right side
        for (int j = 0; j < 3; j++)
        {
            c = line.at(j + 6);
            rc = getColorForCode(c);
            if (rc == CL_NONE)
            {
                cerr << "Invalid color code " << c << " in right side definition" << endl;
                return false;
            }

            m_cubeCache[CF_RIGHT][2-i][j] = rc;
        }

        // back side
        for (int j = 0; j < 3; j++)
        {
            c = line.at(j + 9);
            rc = getColorForCode(c);
            if (rc == CL_NONE)
            {
                cerr << "Invalid color code " << c << " in back side definition" << endl;
                return false;
            }

            m_cubeCache[CF_BACK][2-j][2-i] = rc;
        }
    }

    // and finally load down side
    for (int i = 0; i < 3; i++)
    {
        line = lines[i+6];
        if (line.length() != 6 || line.at(0) != ' ' || line.at(1) != ' ' || line.at(2) != ' ')
        {
            cerr << "Invalid definition of cube down side - make sure you used 3 spaces, and 3 valid characters" << endl;
            return false;
        }

        for (int j = 0; j < 3; j++)
        {
            c = line.at(3 + j);
            rc = getColorForCode(c);
            if (rc == CL_NONE)
            {
                cerr << "Invalid color code " << c << " in down side definition" << endl;
                return false;
            }

            m_cubeCache[CF_DOWN][j][i] = rc;
        }
    }

    // check color frequency - there has to be 9 of every color
    int counter[CL_COUNT];
    // put zeros at start
    for (int i = 0; i < CL_COUNT; i++)
        counter[i] = 0;
    // count them all
    for (int i = 0; i < CF_COUNT; i++)
        for (int j = 0; j < 3; j++)
            for (int k = 0; k < 3; k++)
                counter[m_cubeCache[i][j][k]]++;

    // go through all counters and determine counts
    for (int i = 0; i < CL_COUNT; i++)
    {
        if (counter[i] != 9)
        {
            cerr << "Invalid cube definition - there are " << counter[i] << " occurencies of " << rubikColorCode[i] << " color, but there should be 9!" << endl;
            return false;
        }
    }

    // now check, if all side centers are distinct - side centers are constant elements in rubik's cube,
    // and they have to be unique (one side = one color)

    // reuse counter array - after this, there should be 10 of every color
    for (int i = 0; i < CF_COUNT; i++)
        counter[m_cubeCache[i][1][1]]++;

    // go through all counters and check counts
    for (int i = 0; i < CL_COUNT; i++)
    {
        if (counter[i] < 10)
        {
            cerr << "Invalid cube definition - there is no occurence of " << rubikColorCode[i] << " color center face!" << endl;
            return false;
        }
    }

    // now when everything seems valid (at least from basic point of view), proceed to propagate cache to cube itself
    RestoreCacheCube();

    // set cube faces and their colors
    for (int i = 0; i < CF_COUNT; i++)
        colorFaceMap[m_cubeCache[i][1][1]] = (CubeFace)i;

    return true;
}
