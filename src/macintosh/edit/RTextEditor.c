/*
 *  R : A Computer Language for Statistical Data Analysis
 *  File RTextEditor.c
 *  Copyright (C) 1998-1999  Ross Ihaka
 *                2000-2001  Stefano M. Iacus and the R core team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *  This file is adapted from the public demos coming with the Waste library
 *  distribution:  WASTE Text Engine � 1993-2000 Marco Piovanelli.
 *   
 *  This file was originally written by: Wing Kwong (Tiki), WAN 3/2/99
 *  Updated to last version of WasteLib library: Stefano M. Iacus, 2001
 *  Added mac_load history, mac_savehistory: Stefano M. Iacus, 2000
 * 
 *  Original file was:
 *
 *	WASTE Demo Project:
 *	Dialog Utilities
 *
 *	Copyright � 1993-1998 Marco Piovanelli
 *	All Rights Reserved
 *
 *	C port by John C. Daub
 */

#include "RIntf.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "Defn.h"

void mac_savehistory(char *file);
void mac_loadhistory(char *file);


// Constant
#define   kLineto                         1
#define   kRectangle                      2
#define   MAXLONG		                      0x7FFFFFFF
#define   rNewWindow                      129
#define   rStringList		                  131
#define   sUntitled		                    1
#define   eMaxWindows	                    2
#define   eFailWindow	                    4
#define   eFailMenus		                  5
#define   eFailMemory	                    6
#define   cmdRecord                       100
#define   kMaxGraphic                     15


// Global Variables
Ptr                                       *Cmd_Hist;
int                                       g_cur_Cmd, g_start_Cmd, g_end_Cmd;
Boolean                                   g_Stop = false;
Boolean                                   g_down = true;
Boolean                                   g_not_first = false;
SInt32                                    gbuflen, gpmtLh;
SInt32                                    gChangeAble=0;
SInt16                                    gWindowNum =0, gtabSize;
Boolean                                   gfinishedInput;
char                             	  *gbuf;
SInt16                                    HISTORY;
Boolean		                          GWgDone;
extern Boolean		                  gInBackground;
Ptr				          gPreAllocatedBlockPtr;
SInt32		                          gUntitledWindowNumber  = 0;
SInt32		                          gCurrentNumberOfWindows = 0;
WindowPtr	                          gWindowPtrArray[kMaxWindows + 2];

// Extern Global Variables
extern   WindowPtr                        Console_Window, Edit_Window;
extern   Ptr                              gPreAllocatePointer;
extern RGBColor	                          gTypeColour;
extern RGBColor                           gFinishedColour;
extern RGBColor                           gComputerColour;
extern Boolean                            HaveContent;
extern Handle	                          myHandle;
extern SInt32                              curPaste, finalPaste;
// Functions protocol
void   doNewWindow                        (void);
void   GWdoConcatPStrings		  (Str255,Str255);
void   doSetStandardState                 (WindowPtr);
void   RnWrite                            (char*, SInt16);


// Function with empty content
void R_ResetConsole(void);
void R_FlushConsole(void);
void R_ClearerrConsole(void);
void R_Busy(int which);

void R_Suicide(char *msg);
void R_CleanUp(SA_TYPE, int, int);

char *R_ExpandFileName(char *s);

void R_RestoreGlobalEnv(void);
void R_SaveGlobalEnv(void);



void RWrite(char* buf)
{
    SInt32 buflen, i;

    buflen = strlen(buf);
   
    if(fileno(stdout)>1){
	fputs(buf, stdout);
	fflush(stdout);
    }
    else
	for ( i=0; i < buflen; i++)
	    WEKey ( buf[i], NULL, GetWindowWE (Console_Window) ) ;
}


void RnWrite(char* buf, SInt16 len)
{
    SInt32 i;
    char buf2[300];
   
    if(fileno(stdout)>1){
	strncpy(buf2,buf,len);
	fputs(buf2, stdout);
	fflush(stdout);
    }
    else
	for ( i=0; i < len; i++)
	    WEKey ( buf[i], NULL, GetWindowWE (Console_Window) ) ; 
}
/* R_ReadConsole
This is a platform dependent function.                           
This function prints the given prompt at the console and then does a gets(3)- like operation, 
transferring up to buflen characters into the buffer buf. The last character is set to \0 to
preserve sanity.                                         
*/

void R_ReadConsole1(char *prompt,  char *buf, int buflen, int hist)
{
    WEReference we ;
    EventRecord myEvent;
    SInt32      i;
    char        tempChar;
    char buffo[1000];

    we = GetWindowWE ( Console_Window ) ;
   
    // let have something about hist
    hist = hist + 1;
    // Print the prompt to stanard output


    RWrite(prompt);
   

    gpmtLh = strlen(prompt);
    gChangeAble = WEGetTextLength(we);
    // The Prompt is in different color (black)
    //>
    Change_Color_Range(gChangeAble-gpmtLh, gChangeAble, 
		       gComputerColour.red, gComputerColour.green, gComputerColour.blue, we);
    //The Command is in different color (red)
    //Change_Color(gTypeColour.red, gTypeColour.green, gTypeColour.blue, we);
   
    // gbuf is a ptr, which is used to point to the receive buffer
    gbuf = buf;
    gbuflen = buflen;


    // Call the Receive loop  

    if (HaveContent){
	HLock( myHandle );
	for ( i = curPaste+1 ; i <=finalPaste; i++){
	    tempChar = (*myHandle)[i];
	    if ((tempChar == '\r') || (i == finalPaste)){
		RnWrite(*myHandle + (curPaste + 1), i - (curPaste + 1));
		if (i != finalPaste){
		    myEvent.message = 140301;
		    DoKeyDown (&myEvent);
		}
		curPaste = i;
		break;
	    }
	}

	HUnlock( myHandle );
	if (finalPaste > i){
	    HaveContent = true;
	}
	if (finalPaste <= i){
	    DisposeHandle(myHandle);
	    HaveContent = ! HaveContent;
	}
    }
    if (!HaveContent){
	gfinishedInput = false;
	while(!gfinishedInput)
	{    
	    if (isTextWindow){ 
 
		ProcessEvent ( );
	    }
	}
    }   

    //It is green in color 
    Change_Color_Range(gChangeAble, WEGetTextLength(we),
		       gFinishedColour.red, gFinishedColour.green, gFinishedColour.blue, we);
    // It is blue in color
    Change_Color_Range(WEGetTextLength(we), WEGetTextLength(we),
		       gComputerColour.red, gComputerColour.green, gComputerColour.blue,  we);
    // ********* Don't try to change the content of buf
    if (strlen((const char *)buf) > 1)
	maintain_cmd_History(buf);
    // Mac_Command(buf);
   
    
}

/* R_ReadConsole2 experimental code
This is a platform dependent function.                           
This function prints the given prompt at the console and then does a gets(3)- like operation, 
transferring up to buflen characters into the buffer buf. The last character is set to \0 to
preserve sanity.                                         
*/

void R_ReadConsole2(char *prompt,  char *buf, int buflen, int hist)
{
    WEReference we ;
    EventRecord myEvent;
    SInt32      i;
    char        tempChar;
    char buffo[1000];

    we = GetWindowWE ( Console_Window ) ;

   
    // let have something about hist
    hist = hist + 1;
    // Print the prompt to stanard output


    RWrite(prompt);
   

    gpmtLh = strlen(prompt);
    gChangeAble = WEGetTextLength(we);
 
    // The Prompt is in different color (black)
    //>

/*   Change_Color_Range(gChangeAble-gpmtLh, gChangeAble, 
     gComputerColour.red, gComputerColour.green, gComputerColour.blue, we);

*/
    //The Command is in different color (red)
    //Change_Color(gTypeColour.red, gTypeColour.green, gTypeColour.blue, we);
   
    // gbuf is a ptr, which is used to point to the receive buffer
    gbuf = buf;
    gbuflen = buflen;


    // Call the Receive loop  

    if (HaveContent){
	HLock( myHandle );
	for ( i = curPaste+1 ; i <=finalPaste; i++){
	    tempChar = (*myHandle)[i];
	    if ((tempChar == '\r') || (i == finalPaste)){
		RnWrite(*myHandle + (curPaste + 1), i - (curPaste + 1));
		if (i != finalPaste){
		    myEvent.message = 140301;
		    DoKeyDown (&myEvent);
		}
		curPaste = i;
		break;
	    }
	}

	HUnlock( myHandle );
	if (finalPaste > i){
	    HaveContent = true;
	}
	if (finalPaste <= i){
	    DisposeHandle(myHandle);
	    HaveContent = ! HaveContent;
	}
    }
    if (!HaveContent){
	gfinishedInput = false;
	while(!gfinishedInput)
	{    
	    if (isTextWindow){ 
 
		ProcessEvent ( );
	    }
	}
    }   
/*
  //It is green in color 
  Change_Color_Range(gChangeAble, WEGetTextLength(we),
  gFinishedColour.red, gFinishedColour.green, gFinishedColour.blue, we);
  // It is blue in color
  Change_Color_Range(WEGetTextLength(we), WEGetTextLength(we),
  gComputerColour.red, gComputerColour.green, gComputerColour.blue,  we);
  // ********* Don't try to change the content of buf
*/
    
/*   ReadCharsFromConsole(buf,buflen);
 */
  
    if (strlen((const char *)buf) > 1)
	maintain_cmd_History(buf);
    // Mac_Command(buf);
   
    
}

void DRWrite(long in)
{
    char     temp[100];
    sprintf(temp, "%d", in);
    RWrite(temp);
   
}

void FRWrite(double in)
{
    char temp[100];
    sprintf(temp, "%f", in);
    RWrite(temp);
}

/* free_History
free the memory obtain the maintain the history list
*/
void free_History(void)
{
    int i;
    if (g_start_Cmd < g_end_Cmd)
	for(i = g_start_Cmd ; i < g_end_Cmd ; i++){
	    free(Cmd_Hist[i]);
	} 
    else
	for(i = 0; i < HISTORY; i++)
	    free(Cmd_Hist[i]);
}


SEXP do_savehistory(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP sfile;
    
    checkArity(op, args);
    sfile = CAR(args);
    if (!isString(sfile) || LENGTH(sfile) < 1)
	errorcall(call, "invalid file argument");
    mac_savehistory(CHAR(STRING_ELT(sfile, 0)));
    return R_NilValue;
}

SEXP do_loadhistory(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP sfile;
    
    checkArity(op, args);
    sfile = CAR(args);
    if (!isString(sfile) || LENGTH(sfile) < 1)
	errorcall(call, "invalid file argument");
    mac_loadhistory(CHAR(STRING_ELT(sfile, 0)));
    return R_NilValue;
}

/**********************************************
 mac_savehistory: save history command to a 
 specified file. Adapted from gl_savehistory
 for Windows
 Stefano M. Iacus Gen 2001
**********************************************/ 

void mac_savehistory(char *file)
{
    FILE *fp;
    int i;
    char hist_buff[1000];
    
    if (!file || !g_end_Cmd) return;
    
    fp = fopen(file, "w");
    if (!fp) {
	char msg[256];
	sprintf(msg, "Unable to open history file \"%s\" for writing", file);
	warning(msg);
	return;
    }
    
    if (g_start_Cmd < g_end_Cmd)
	for(i = g_start_Cmd ; i < g_end_Cmd ; i++){
	    fprintf(fp, "%s\n", Cmd_Hist[i]);
	} 
    else
	for(i = 0; i < HISTORY; i++)
	    fprintf(fp, "%s\n", Cmd_Hist[i]);
    fclose(fp); 
}

/**********************************************
 mac_loadhistory: load history command from a 
 specified file. Adapted from gl_loadhistory
 for Windows. It can read history files of
 Windowds porting.
 Stefano M. Iacus Gen 2001
**********************************************/ 
void mac_loadhistory(char *file)
{
    FILE *fp;
    int i,buflen,j;
    char buf[1002];

    if (!file) return;
    fp = fopen(file, "r");
    if (!fp) {
	REprintf("\nUnable to open history file \"%s\" for reading\n", file);
	return;
    }

    for(i = 0;; i++) {
	if(!fgets(buf, 1000, fp)) 
	    break;
	if( (buflen = strlen(buf)) > 1) {
	    if(buf[buflen-1]==0x0A) {
		if(buf[buflen-2]==0x0D)
		    buf[buflen-1]='\0'; 
		else { 
		    buf[buflen]='\0'; 
		    buf[buflen-1]=0x0D;
		}
	    }
	    maintain_cmd_History(buf);
	}
    }
    fclose(fp); 
}



/* maintain_cmd_History
Put the buf into the list of History                            
Maintain the position of the start, cur and end pointer of the list of the History
*/

void maintain_cmd_History(char *buf)
{
    char *temp;
    int numberOfChar;

    g_Stop = false;
    numberOfChar = strlen(buf);
    temp = (char *)malloc(numberOfChar);
    strcpy(temp, (const char *)buf);
    Cmd_Hist[g_end_Cmd] = temp;
    g_not_first = false;
    g_cur_Cmd = g_end_Cmd;
    g_end_Cmd++;
    if (g_end_Cmd <= g_start_Cmd){
	g_start_Cmd++;
    }
    if (g_start_Cmd == HISTORY){
	g_start_Cmd = 0;
    }
    if (g_end_Cmd == HISTORY){
	g_end_Cmd = 0;
	g_start_Cmd = 1;
    }
}


/* R_WriteConsole
This function writes the given buffer out to the console. No special actions are required. (Specify
the length of the buffer)
*/
void R_WriteConsole1(char *buf, SInt32 buflen)
{
    SInt32 i;
    SInt32 outlen, lastLen;
    Boolean ended = false;
    WEReference we;
    char stringona[1000];

    outlen =   strlen(buf);
 
    strncpy(stringona,buf,outlen);
    for ( i=0; i < outlen; i++)
	if (buf[i] == '\n') stringona[i]='\r';

    we = GetWindowWE ( Console_Window );
//   WESetSelection ( WEGetTextLength(we), WEGetTextLength(we), we );

    WEPut(kCurrentSelection,kCurrentSelection, stringona, outlen,kTextEncodingMultiRun, 0,0,nil,nil,we ); 
}  

void R_WriteConsoleX(Ptr buf, SInt32 buflen)
{
    SInt32 i, selStart, selEnd;
/*   WEHandle pWE;
 */
    WEReference pWE;

// Updated for Waste 2.X  Jago

/*typedef WEReference WEHandle;*/

    SInt32 outLen, lastLen;
    Boolean ended = false;
    WEReference we;

    lastLen = outLen = 0; 
    we = GetWindowWE ( Console_Window);
    WESetSelection ( WEGetTextLength(we), WEGetTextLength(we), we );
    for ( i=0; i < buflen; i++){
	if (buf[i] == '\n'){
	    if (outLen != 0){
//            WEInsert(&buf[lastLen], outLen, nil, nil, GetWindowWE(Console_Window));
		WEPut(kCurrentSelection,kCurrentSelection, &buf[lastLen], outLen,kTextEncodingMultiRun, 0,0,nil,nil,GetWindowWE(Console_Window) );   
		outLen =0;
	    }
	    WEKey ('\r', NULL,  GetWindowWE (Console_Window));
	    lastLen = i+1;
	}
	else{
	    outLen++;
	}
    }
    //  WEInsert(&buf[lastLen], outLen, nil, nil, GetWindowWE(Console_Window));
    WEPut(kCurrentSelection,kCurrentSelection, &buf[lastLen], 
	  outLen,kTextEncodingMultiRun, 0,0,nil,nil,
	  GetWindowWE(Console_Window) );   

}  



/* R_WriteConsole2 
Experimental code to use only Sioux
This function writes the given buffer out to the console. No special actions are required. (Specify
the length of the buffer)
*/
void R_WriteConsole2(char *buf, SInt32 buflen)
{
    SInt32 i;
    SInt32 outlen, lastLen;
    Boolean ended = false;
    WEReference we;
    char stringona[1000];


    WriteCharsToConsole(buf,buflen);
   
    return;
   
    outlen =   strlen(buf);
 
    strncpy(stringona,buf,outlen);
    for ( i=0; i < outlen; i++)
	if (buf[i] == '\n') stringona[i]='\r';

    we = GetWindowWE ( Console_Window );
//   WESetSelection ( WEGetTextLength(we), WEGetTextLength(we), we );
    WEPut(kCurrentSelection,kCurrentSelection, stringona, outlen,kTextEncodingMultiRun, 0,0,nil,nil,we ); 
}  

Boolean inRange(int start, int end , int back, int length)
{
    if(end > back){
	if (start > back) return false;
	else return true;
    } 
    if(end < (back-length)) return false;
    if (end > (back-length)) return true;
    return true;
}

void Change_Color_Range(SInt32 start, SInt32 end, long red, long green, 
			long blue, WEReference we)
{
    RGBColor			color = { 0x0000, 0x0000, 0x0000 } ;
/*  code mod Jago 08/28/00
    TextStyle			ts ;
    ts . tsColor . red = red;
    ts . tsColor . green = green ;
    ts . tsColor . blue = blue ;
    WESetSelection(start, end, we);
    WESetStyle ( weDoColor, & ts, we ); 
*/
   
    color.red = red;
    color.blue = blue;
    color.green = green;
	
    WESetOneAttribute ( start, end, weTagTextColor,
			& color, sizeof ( color ),  we  ) ;
    WESetSelection(WEGetTextLength(we), WEGetTextLength(we), we);
}

void Change_Color(long red, long green, long blue, WEReference we)
{
    Change_Color_Range(WEGetTextLength(we),WEGetTextLength(we), 
		       red, green, blue,  we);
   
}

Boolean isTextWindow(WindowPtr window)
{
    if ((window == Console_Window) || (window == Edit_Window))
        return true;
    else
        return false;    
}

// doErrorAlert
void  GWdoErrorAlert(SInt16 errorType)
{
    AlertStdAlertParamRec paramRec;
    Str255 labelText;
    Str255 narrativeText;
    SInt16 itemHit;
		
    paramRec.movable			= false;
    paramRec.helpButton			= false;
    paramRec.filterProc			= NULL;
    paramRec.defaultText		= (StringPtr) kAlertDefaultOKText;
    paramRec.cancelText			= NULL;
    paramRec.otherText			= NULL;
    paramRec.defaultButton		= kAlertStdAlertOKButton;
    paramRec.cancelButton		= 0;
    paramRec.position			= kWindowAlertPositionMainScreen;
    GetIndString(labelText,rStringList,errorType);
    if(errorType == eMaxWindows)
    {
	GetIndString(narrativeText,rStringList,errorType + 1);
	RWrite("No more window");
//		StandardAlert(kAlertCautionAlert,labelText,narrativeText,&paramRec,&itemHit);
    }
    else
    {
	Do_StandardAlert(labelText);
//		StandardAlert(kAlertStopAlert,labelText,0,&paramRec,&itemHit);
	if (errorType <7)
	    ExitToShell();
    }	
}

// doNewWindow
void  doNewWindow(void)
{
    WindowPtr windowPtr;
    Str255 untitledString;
    Str255 numberAsString;
    MenuHandle windowsMenu = NULL;
    if(gCurrentNumberOfWindows == kMaxWindows)
    {
	GWdoErrorAlert(eMaxWindows);
	return;
    }
    if(!(windowPtr = GetNewCWindow(rNewWindow,gPreAllocatedBlockPtr,(WindowPtr)-1)))
	GWdoErrorAlert(eFailWindow);
	
    gPreAllocatedBlockPtr = NULL;
    GetIndString(untitledString,rStringList,sUntitled);
    NumToString(++gUntitledWindowNumber,numberAsString);
    GWdoConcatPStrings(untitledString,numberAsString);
    SetWTitle(windowPtr,untitledString);
    doSetStandardState(windowPtr);
    if(gUntitledWindowNumber <10)
    {
	GWdoConcatPStrings(untitledString,"\p/");
	NumToString(gUntitledWindowNumber,numberAsString);
	GWdoConcatPStrings(untitledString,numberAsString);
    }
    InsertMenuItem(windowsMenu,untitledString,CountMenuItems(windowsMenu));		
    SetWRefCon(windowPtr,gCurrentNumberOfWindows);
    gCurrentNumberOfWindows ++;
    gWindowPtrArray[gCurrentNumberOfWindows] = windowPtr;	
}

// doSetStandardState
void  doSetStandardState(WindowPtr windowPtr)
{
    WindowPeek windowRecPtr;
    WStateData *winStateDataPtr;
    Rect tempRect;
    tempRect = qd.screenBits.bounds;	
    windowRecPtr = (WindowPeek) windowPtr;
    winStateDataPtr = (WStateData *) *(windowRecPtr->dataHandle);
    SetRect(&(winStateDataPtr->stdState),tempRect.left+40,tempRect.top+60,
	    tempRect.right-40,tempRect.bottom-40);
}

/* do_Down_Array
This procedure used to maintain the reponse when you click the down array key. (about display 
previous command in console window)                                            */
void do_Down_Array(void)
{
    WEReference we ;
    SInt32 textLength; 
    if (g_start_Cmd != g_end_Cmd) { 
	we = GetWindowWE ( Console_Window ) ;
	if (!g_down){
	    g_cur_Cmd--;
	}
	g_not_first = true;
	g_down = true;
	WESetSelection(gChangeAble,WEGetTextLength(we),we);
	if (g_start_Cmd == 0){ 
	    if (g_cur_Cmd < g_start_Cmd){
		SysBeep(10);
	    }else{
		textLength = strlen(Cmd_Hist[g_cur_Cmd]) - 1;
		RnWrite(Cmd_Hist[g_cur_Cmd] , textLength); 
		g_cur_Cmd--;   
	    }
	}else{
	    if (g_cur_Cmd == g_end_Cmd){
		SysBeep(10);
	    }else{
		if(g_cur_Cmd == -1) g_cur_Cmd = HISTORY - 1;
		textLength = strlen(Cmd_Hist[g_cur_Cmd]) - 1;
		RnWrite(Cmd_Hist[g_cur_Cmd] , textLength);
		g_cur_Cmd--;
		if(g_cur_Cmd == -1) g_cur_Cmd = HISTORY - 1;
	    }
	}
    }
}

void do_Up_Array(void)
{
    WEReference we ;
    SInt32 textLength;  
    if (g_start_Cmd != g_end_Cmd) {
 
	we = GetWindowWE ( Console_Window ) ;
	WESetSelection(gChangeAble,WEGetTextLength(we),we);
	if ((g_down) && (g_not_first)){
	    g_cur_Cmd++;
	    g_down = false;
	}
	if (g_start_Cmd == 0){
	    if (g_cur_Cmd == (g_end_Cmd-1))
		SysBeep(10);
	    else{
		g_cur_Cmd ++;
		textLength = strlen(Cmd_Hist[g_cur_Cmd]) - 1;
		RnWrite(Cmd_Hist[g_cur_Cmd] , textLength);
	    }
	}else{
	    if ((g_cur_Cmd == (HISTORY -1)) && (g_end_Cmd ==0)){
		SysBeep(10);
	    }else
		if ((g_cur_Cmd == g_end_Cmd) || (g_cur_Cmd == (g_end_Cmd -1))){
		    SysBeep(10);
		}else{
		    g_cur_Cmd ++;
		    if (g_cur_Cmd == HISTORY) g_cur_Cmd = 0;
		    textLength = strlen(Cmd_Hist[g_cur_Cmd]) - 1;
		    RnWrite(Cmd_Hist[g_cur_Cmd] , textLength);
		}
	}
    }
}
