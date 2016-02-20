#ifndef RUBIK_DRAWING_H
#define RUBIK_DRAWING_H

#include "Singleton.h"

#define sDrawing Singleton<Drawing>::instance()

class RubikCube;

class EventReceiver : public IEventReceiver
{
    public:
        EventReceiver() { }

        struct SMouseState
        {
            core::position2di Position;
            bool LeftButtonDown;
            SMouseState() : LeftButtonDown(false) { }
        } MouseState;

        virtual bool OnEvent(const SEvent& event);

        const SMouseState & GetMouseState(void) const { return MouseState; }

    private:
        //
};

class Drawing
{
    friend class Singleton<Drawing>;
    public:
        ~Drawing();

        bool Init();
        bool Render();

        IrrlichtDevice* getDevice() { return m_irrDevice; };
        IVideoDriver* getDriver() { return m_irrDriver; };

        void mouseMoveCallback(int deltaX, int deltaY);

        void showMessage(char* message);

    private:
        Drawing();
        void updateCameraPosition();

        IrrlichtDevice* m_irrDevice;
        IVideoDriver* m_irrDriver;
        ISceneManager* m_irrScene;
        IGUIEnvironment* m_irrGui;
        IGUIFont* m_appFont;
        EventReceiver m_eventReceiver;

        ICameraSceneNode* m_mainCamera;
        float m_cameraAngleX, m_cameraAngleY;

        std::string m_messageToShow;
        unsigned int m_messageShowTimer;
};

#endif
