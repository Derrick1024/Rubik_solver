#include "Global.h"
#include "bigint.h"
#include "Rubik.h"
#include "Drawing.h"

// empty constructor
Drawing::Drawing()
{
    m_messageToShow = "";
    m_messageShowTimer = 0;
}

// empty destructor
Drawing::~Drawing()
{
    //
}

// initializes drawing
bool Drawing::Init()
{
    // create display device and set some parameters
    SIrrlichtCreationParameters params = SIrrlichtCreationParameters();
    // TODO: some kind of config?
    params.AntiAlias = 8;
    params.Bits = 32;
    params.DriverType = video::EDT_OPENGL;
    params.WindowSize = core::dimension2d<u32>(800, 600);
    // hook event receiver
    params.EventReceiver = &m_eventReceiver;
    m_irrDevice = createDeviceEx(params);

    if (!m_irrDevice)
        return false;

    // create driver, scene and gui
    m_irrDriver = m_irrDevice->getVideoDriver();
    m_irrScene = m_irrDevice->getSceneManager();
    m_irrGui = m_irrDevice->getGUIEnvironment();

    // default camera rotation
    m_cameraAngleX = - PI / 4.0f;
    m_cameraAngleY = PI / 4.0f;

    // add main camera
    m_mainCamera = m_irrScene->addCameraSceneNode(0, vector3df(25, 25, -25), vector3df(0, 0, 0));
    // update camera position by mouse position
    updateCameraPosition();

    // adds light
    m_irrScene->addLightSceneNode(m_mainCamera);

    // build cube using created scene and driver
    sCube->BuildCube(m_irrScene, m_irrDriver);

    // load font and set spacing (not kerning! bad function naming; doc. Kohout would be very angry about that)
    m_appFont = m_irrGui->getFont(DATA_DIR "appfont.png");
    m_appFont->setKerningWidth(-3);

    return true;
}

// rendering method
bool Drawing::Render()
{
    if (!m_irrDevice)
        return false;

    if (!m_irrDevice->run())
        return false;

    // clears scene
    m_irrDriver->beginScene(true, true, SColor(255, 120, 190, 130));

    // draws everything that should be there (3D)
    m_irrScene->drawAll();
    // renders cube
    sCube->Render();

    // print some info about controls
    m_appFont->draw(L"CONTROL\nR\t\t\t\t\tmix up the cube\nS\t\t\t\t\tsolve\n+ -\t\t\tspeed up/down flips", rect<s32>(5, 600 - 24*5, 100, 100), SColor(255, 0, 0, 127));

    // about flipping speed...
    stringw repstr = "Flipping speed: ";
    std::string str = std::to_string(sCube->GetFlipTiming() / 1000.0f);
    str.erase(str.find_last_not_of('0') + 1, std::string::npos);
    repstr += str.c_str();
    repstr += " s";

    m_appFont->draw(repstr, rect<s32>(5, 600 - 24, 100, 100), SColor(255, 0, 50, 240));

    if (m_messageShowTimer > 0 && m_messageToShow.length() > 0)
    {
        if (getMSTimeDiff(m_messageShowTimer, getMSTime()) > 3000)
        {
            m_messageShowTimer = 0;
        }

        m_appFont->draw(m_messageToShow.c_str(), rect<s32>(5, 600 - 24*7, 100, 100), SColor(255, 100, 40, 40));
    }

    m_irrGui->drawAll();

    m_irrDriver->endScene();

    return true;
}

// updates camera position regarding stored angles
void Drawing::updateCameraPosition()
{
    m_mainCamera->setPosition(vector3df(35 * cos(m_cameraAngleX) * cos(m_cameraAngleY), 35 * sin(m_cameraAngleY), 35 * sin(m_cameraAngleX) * cos(m_cameraAngleY)));
}

// callback on mouse move
void Drawing::mouseMoveCallback(int deltaX, int deltaY)
{
    // when left button is down, that means, we are dragging view
    if (m_eventReceiver.GetMouseState().LeftButtonDown)
    {
        m_cameraAngleX -= (float)deltaX * PI / 280.0f;
        m_cameraAngleY += (float)deltaY * PI / 280.0f;

        updateCameraPosition();
    }
}

// shows message on screen to let user know about something
void Drawing::showMessage(char* message)
{
    m_messageToShow = std::string(message);
    m_messageShowTimer = getMSTime();
}

// catches events from irrlicht engine
bool EventReceiver::OnEvent(const SEvent& event)
{
    // mouse state should be remembered to determine several things
    if (event.EventType == irr::EET_MOUSE_INPUT_EVENT)
    {
        switch (event.MouseInput.Event)
        {
            // left button pressed
            case EMIE_LMOUSE_PRESSED_DOWN:
                MouseState.LeftButtonDown = true;
                break;
            // left button released
            case EMIE_LMOUSE_LEFT_UP:
                MouseState.LeftButtonDown = false;
                break;
            // the mouse was moved
            case EMIE_MOUSE_MOVED:
                sDrawing->mouseMoveCallback(event.MouseInput.X - MouseState.Position.X, event.MouseInput.Y - MouseState.Position.Y);
                MouseState.Position.X = event.MouseInput.X;
                MouseState.Position.Y = event.MouseInput.Y;
                break;
            default:
                break;
        }
    }
    // key events
    else if (event.EventType == irr::EET_KEY_INPUT_EVENT && event.KeyInput.PressedDown)
    {
        switch (event.KeyInput.Key)
        {
            // R - scramble cube
            case KEY_KEY_R:
            {
                if (sCube->IsFlipSequenceInProgress())
                    break;

                cout << "Randomly mixing up cube:" << endl;
                std::list<CubeFlip> fliplist;
                sCube->Scramble(&fliplist);
                sCube->ProceedFlipSequence(&fliplist, true);
                break;
            }
            // S - solve cube
            case KEY_KEY_S:
            {
                if (sCube->IsFlipSequenceInProgress())
                    break;

                cout << "Finding solution..." << endl;
                std::list<CubeFlip> fliplist;
                sCube->Solve(&fliplist);
                // if no solution found, we can't do anything (the list would be empty)
                if (fliplist.empty())
                {
                    cout << "No solution found" << endl;
                    sDrawing->showMessage("No solution found");
                }
                else
                {
                    cout << "Solving cube:" << endl;
                    sCube->ProceedFlipSequence(&fliplist, true);
                }
                break;
            }
            // minus means "slow down"
            case KEY_MINUS:
            case KEY_SUBTRACT:
            {
                sCube->UpdateFlipTiming(+100);
                break;
            }
            // plus means "faster!"
            case KEY_PLUS:
            case KEY_ADD:
            {
                sCube->UpdateFlipTiming(-100);
                break;
            }
        }
    }
    // log messages - we don't use logs for now!
    else if (event.EventType == irr::EET_LOG_TEXT_EVENT)
    {
        return true;
    }

    return false;
}
