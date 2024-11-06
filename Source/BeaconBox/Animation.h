/*
 *  Animation.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

// Define routines and functions

void animationSetUp(void _modeChangeHandler(bool manualEvent));
void animationSetState(bool _active, int _modeHeardTimeoutSeconds, int _modeNcdxfIaruTimeoutSeconds, int _modeDaylightTimeoutSeconds, int _manualEventTimeoutSeconds);
void animationAnimate();
void animationManualEvent();

 
