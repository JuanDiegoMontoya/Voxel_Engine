/*==============================================================================
Object Panning Example
Copyright (c), Firelight Technologies Pty, Ltd 2015-2018.

This example demonstrates the FMOD object panner. The usage is completely
transparent to the API, the only difference is how the event is authored in the
FMOD Studio tool.

To hear the difference between object panning and normal panning this example
has two events (one configured with the normal panner, and one with the object
panner). As they move around the listener you may toggle between panning method
and two different sounds.

Object panning requires compatible hardware such as a Dolby Atmos amplifier or
a Playstation VR headset. For cases when the necessary hardware is not available
FMOD will fallback to standard 3D panning.

NOTE! Currently the Atmos output mode requires a dll to be supplied via support.
Atmos access needs to be granted via Dolby first.  Write to support@fmod.com 
for more.

==============================================================================*/
#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "common.h"
#include <math.h>

int FMOD_Main()
{
    bool isNoise = false;
    bool isObject = true;
    bool isPlaying = false;
    
    void *extraDriverData = NULL;
    Common_Init(&extraDriverData);

    FMOD::Studio::System *system = NULL;
    ERRCHECK( FMOD::Studio::System::create(&system) );

    // The example Studio project is authored for 5.1 sound, so set up the system output mode to match
    FMOD::System* lowLevelSystem = NULL;
    ERRCHECK( system->getLowLevelSystem(&lowLevelSystem) );
    ERRCHECK( lowLevelSystem->setSoftwareFormat(0, FMOD_SPEAKERMODE_5POINT1, 0) );

    // Attempt to initialize with a compatible object panning output
    FMOD_RESULT result = lowLevelSystem->setOutput(FMOD_OUTPUTTYPE_AUDIO3D);
    if (result != FMOD_OK)
    {
        result = lowLevelSystem->setOutput(FMOD_OUTPUTTYPE_WINSONIC);
        if (result == FMOD_OK)
        {
            ERRCHECK( lowLevelSystem->setSoftwareFormat(0, FMOD_SPEAKERMODE_7POINT1POINT4, 0) );
        }
    }

    int numDrivers = 0;
    ERRCHECK( lowLevelSystem->getNumDrivers(&numDrivers) );

    if (numDrivers == 0)
    {
        ERRCHECK( lowLevelSystem->setDSPBufferSize(512, 4) );
        ERRCHECK( lowLevelSystem->setOutput(FMOD_OUTPUTTYPE_AUTODETECT) );
    }

    ERRCHECK( system->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, extraDriverData) );
    
    // Load everything needed for playback
    FMOD::Studio::Bank *masterBank = NULL;
    FMOD::Studio::Bank *sfxBank = NULL;
    FMOD::Studio::Bank *stringsBank = NULL;
    FMOD::Studio::EventDescription *objectPannerDescription = NULL;
    FMOD::Studio::EventDescription *normalPannerDescription = NULL;
    FMOD::Studio::EventInstance *objectInstance = NULL;
    FMOD::Studio::EventInstance *normalInstance = NULL;

    ERRCHECK( system->loadBankFile(Common_MediaPath("Master Bank.bank"), FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank) );
    ERRCHECK( system->loadBankFile(Common_MediaPath("SFX.bank"), FMOD_STUDIO_LOAD_BANK_NORMAL, &sfxBank) );
    ERRCHECK( system->loadBankFile(Common_MediaPath("Master Bank.strings.bank"), FMOD_STUDIO_LOAD_BANK_NORMAL, &stringsBank) );
    ERRCHECK( system->getEvent("event:/Object Panning/Object 3D Panner", &objectPannerDescription) );
    ERRCHECK( system->getEvent("event:/Object Panning/Normal 3D Panner", &normalPannerDescription) );
    ERRCHECK( normalPannerDescription->createInstance(&normalInstance) );
    ERRCHECK( objectPannerDescription->createInstance(&objectInstance) );
    ERRCHECK( normalInstance->start() );
    ERRCHECK( objectInstance->start() );
    ERRCHECK( normalInstance->setPaused(true) );
    ERRCHECK( objectInstance->setPaused(true) );

    do
    {
        bool isChanged = false;

        Common_Update();

        if (Common_BtnPress(BTN_ACTION1))
        {
            isPlaying = !isPlaying;
            isChanged = true;
        }
        if (Common_BtnPress(BTN_ACTION2))
        {
            isNoise = !isNoise;
            isChanged = true;
        }
        if (Common_BtnPress(BTN_ACTION3))
        {
            isObject = !isObject;
            isChanged = true;
        }

        if (isChanged)
        {
            ERRCHECK( objectInstance->setPaused(true) );
            ERRCHECK( normalInstance->setPaused(true) );

            if (isPlaying)
            {
                FMOD::Studio::EventInstance *instance = (isObject ? objectInstance : normalInstance);

                ERRCHECK( instance->setParameterValueByIndex(0, isNoise ? 0.0f : 1.0f) );
                ERRCHECK( instance->setPaused(false) );
            }
        }

        FMOD_3D_ATTRIBUTES vec = { };
        vec.forward.z = 1.0f;
        vec.up.y = 1.0f;

        if (isNoise)
        {
            static float t = 0;
            vec.position.x = sinf(t) * 1.0f;        /* Rotate sound in a circle */
            vec.position.y = 0;                     /* At ground level */
            vec.position.z = cosf(t) * 1.0f;        /* Rotate sound in a circle */
            t += 0.03f;
        }
        else
        {
            static float t = 0;
            vec.position.x = sinf(t) * 5.0f;        /* Pan left and right */
            vec.position.y = 1.0f;                  /* Up high */
            vec.position.z = 0.2f;                  /* In front a bit */
            t += 0.02f;
        }

        ERRCHECK( objectInstance->set3DAttributes(&vec) );
        ERRCHECK( normalInstance->set3DAttributes(&vec) );

        FMOD_3D_ATTRIBUTES listener_vec = { };
        listener_vec.forward.z = 1.0f;
        listener_vec.up.y = 1.0f;

        ERRCHECK( system->setListenerAttributes(0, &listener_vec) );
        ERRCHECK( system->update() );

        Common_Draw("==================================================");
        Common_Draw("Object Panning Example.");
        Common_Draw("Copyright (c) Firelight Technologies 2015-2018.");
        Common_Draw("==================================================");
        Common_Draw("");
        Common_Draw("Press %s to %s playback.", Common_BtnStr(BTN_ACTION1), isPlaying ? "stop" : "start");
        Common_Draw("Press %s to switch to %s.", Common_BtnStr(BTN_ACTION2), isNoise ? "rain" : "noise");
        Common_Draw("Press %s to switch panning to %s.", Common_BtnStr(BTN_ACTION3), isObject ? "Standard 3D Panner" : "Object Panner");
        Common_Draw("");
        Common_Draw("Press %s to quit", Common_BtnStr(BTN_QUIT));

        Common_Sleep(50);
    } while (!Common_BtnPress(BTN_QUIT));
    
    ERRCHECK( stringsBank->unload() );
    ERRCHECK( sfxBank->unload() );
    ERRCHECK( system->release() );

    Common_Close();

    return 0;
}
