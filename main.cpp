#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <coreinit/cache.h>
#include <coreinit/screen.h>
#include <coreinit/thread.h>
#include <vpad/input.h>
#include <whb/log.h>
#include <whb/log_cafe.h>
#include <whb/log_console.h>
#include <whb/log_udp.h>
#include <whb/proc.h>

/* Source: https://github.com/yawut/ProgrammingOnTheU */
/* Additional example: https://github.com/devkitPro/wut/blob/master/samples/make/helloworld/source/main.c */
int main(void)
{
    /* Init logging. We log to Cafe's internal logger (shows up in Decaf and some
    crash logs), over UDP for with udplogserver, and to the console. */
    WHBLogCafeInit();
    WHBLogUdpInit();
    /* WHBLogPrint and WHBLogPrintf add new line characters for you */
    WHBLogPrint("MungPlex-Gecko started.");

    /* Init WHB's ProcUI wrapper. This will manage all the little Cafe OS bits and
    pieces for us - home menu overlay, power saving features, etc. */
    WHBProcInit();

    /* Init OSScreen. This is the really simple graphics API we'll be using to
    draw some text! Also exit out of WHB console logging. */
    //WHBLogConsoleFree();
    OSScreenInit();

    /* OSScreen needs buffers for each display - get the size of those now.
    "DRC" is Nintendo's acronym for the GamePad. */
    size_t tvBufferSize = OSScreenGetBufferSizeEx(SCREEN_TV);
    size_t drcBufferSize = OSScreenGetBufferSizeEx(SCREEN_DRC);
    
    WHBLogPrintf("Will allocate 0x%X bytes for the TV, "
                 "and 0x%X bytes for the GamePad.",
                 tvBufferSize, drcBufferSize);

    /* Try to allocate an area for the buffers. According to OSScreenSetBufferEx's
    documentation, these need to be 0x100 aligned. */
    void *tvBuffer = memalign(0x100, tvBufferSize);
    void *drcBuffer = memalign(0x100, drcBufferSize);

    /* Make sure the allocation actually succeeded! */
    if (!tvBuffer || !drcBuffer)
    {
        WHBLogPrint("Out of memory!");

        /* It's vital to free everything - under certain circumstances, your memory
        allocations can stay allocated even after you quit. */
        if (tvBuffer)
            free(tvBuffer);
        if (drcBuffer)
            free(drcBuffer);

        /* Deinit everything */
        OSScreenShutdown();
        WHBLogPrint("Quitting.");
        WHBLogCafeDeinit();
        WHBLogUdpDeinit();
        WHBProcShutdown();

        /* Your exit code doesn't really matter, though that may be changed in
        future. Don't use -3, that's reserved for HBL. */
        return 1;
    }

    /* Buffers are all good, set them */
    OSScreenSetBufferEx(SCREEN_TV, tvBuffer);
    OSScreenSetBufferEx(SCREEN_DRC, drcBuffer);

    /* Finally, enable OSScreen for each display! */
    OSScreenEnableEx(SCREEN_TV, true);
    OSScreenEnableEx(SCREEN_DRC, true);

    /* Create the GamePad button press variables. */
    VPADStatus status;
    VPADReadError error;
    VPADTouchData touchData;
    bool vpad_fatal = false;
    
    /* WHBProcIsRunning will return false if the OS asks us to quit, so it's a
    good candidate for a loop */
    while (WHBProcIsRunning())
    {
        /* Clear each buffer - the 0x... is an RGBX colour */
        OSScreenClearBufferEx(SCREEN_TV, 0xAF58FF00);
        OSScreenClearBufferEx(SCREEN_DRC, 0xAF58FF00);

        /* Print some text. Coordinates are (columns, rows). */
        OSScreenPutFontEx(SCREEN_TV, 0, 0, "Henlo World! This is on the TV.");
        OSScreenPutFontEx(SCREEN_TV, 0, 1,
                          "If you can read this, the homebrew app is working (to some extend).");

        OSScreenPutFontEx(SCREEN_DRC, 0, 0, "Henlo World! This is on the GamePad.");
        OSScreenPutFontEx(SCREEN_DRC, 0, 1,
                          "If you can read this, the homebrew app is working (to some extend).");

        /* Read button, touch and sensor data from the Gamepad */
        VPADRead(VPAD_CHAN_0, &status, 1, &error);
        /* Check for any errors */
        switch (error)
        {
            case VPAD_READ_SUCCESS:
            {
                /* Everything worked, awesome! */
                OSScreenPutFontEx(SCREEN_DRC, 0, 10,
                                "Successfully read the state of the GamePad!");
                break;
            }
            /* No data available from the DRC yet - we're asking too often!
            This is really common, and nothing to worry about. */
            case VPAD_READ_NO_SAMPLES:
            {
                /* Just keep looping, we'll get data eventually */
                OSScreenPutFontEx(SCREEN_DRC, 0, 10, "Got no input this cycle.");
                continue;
            }
            /* Either our channel was bad, or the controller is. Since this app
            hard-codes channel 0, we can assume something's up with the
            controller - maybe it's missing or off? */
            case VPAD_READ_INVALID_CONTROLLER:
            {
                WHBLogPrint("GamePad disconnected!");
                OSScreenPutFontEx(SCREEN_TV, 0, 10, "GamePad disconnected!");
                /* Not much point testing buttons for a controller that's not
                    actually there */
                vpad_fatal = true;
                break;
            }
                /* If you hit this, good job! As far as we know VPADReadError will
                always be one of the above. */
            default:
            {
                WHBLogPrintf("Unknown VPAD error! %08X", error);
                OSScreenPutFontEx(SCREEN_TV, 0, 10, "Unknown error! Check logs.");
                vpad_fatal = true;
                break;
            }
        }
        /* If there was some kind of fatal issue reading the VPAD, break out of
        the ProcUI loop and quit. */
        if (vpad_fatal)
            break;

        if (status.hold & VPAD_BUTTON_X)
        {
            WHBLogPrint("X pressed to exit.");
            //run = false;
            OSScreenPutFontEx(SCREEN_DRC, 0, 4, "Exiting!");
        }

        if (status.hold & VPAD_BUTTON_A)
        {
            WHBLogPrint("Pressed A this cycle.");
            OSScreenPutFontEx(SCREEN_DRC, 0, 6, "Pressing A!");
        }
        else if (status.release & VPAD_BUTTON_A)
        {
            WHBLogPrint("Released A.");
            OSScreenPutFontEx(SCREEN_DRC, 0, 6, "Released A!");
        }
        else
        {
            OSScreenPutFontEx(SCREEN_DRC, 0, 6, "Not pressing A!");
        }

        /* Flush all caches - read the tutorial, please! */
        DCFlushRange(tvBuffer, tvBufferSize);
        DCFlushRange(drcBuffer, drcBufferSize);

        /* Flip buffers - the text is now on screen! Flipping is kinda like
        committing your graphics changes. */
        OSScreenFlipBuffersEx(SCREEN_TV);
        OSScreenFlipBuffersEx(SCREEN_DRC);

        OSSleepTicks(OSMillisecondsToTicks(13));
    }

    /* Once we get here, ProcUI said we should quit. */
    WHBLogPrint("Got shutdown request!");

    /* It's vital to free everything - under certain circumstances, your memory
    allocations can stay allocated even after you quit. */
    if (tvBuffer)
        free(tvBuffer);
    if (drcBuffer)
        free(drcBuffer);


    OSScreenShutdown();
    WHBProcShutdown();
    WHBLogPrint("Quitting.");
    WHBLogCafeDeinit();
    WHBLogUdpDeinit();

    /* Your exit code doesn't really matter, though that may be changed in
    future. Don't use -3, that's reserved for HBL. */
    return 1;
}
