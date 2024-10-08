/*
 *  FormatHelper.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */
 
char *FormatTimeAsDateTime(time_t _time);
char *FormatUptime(time_t *bootTime);
char *FormatFrequency(float frequency);
char *FormatIPAddress(IPAddress ipAddress);
char *FormatWithTriple(int v, char *tempBuffer);
