/*
 *  MT_JoyStickFrame.cpp
 *
 *  Created by Daniel Swain on 12/2/08.
 *
 */

#include <stdlib.h>
#include "MT_JoyStickFrame.h"

#include "MT/MT_Core/fileio/XMLSupport.h"

enum
{
    ID_ROBOT_MENU_CONNECT = wxID_HIGHEST + 1,
    ID_MENU_ROBOTS_COMMAND,
    ID_MAXTURNINGRATE,
    ID_MAXSPEED,
    ID_XYROBOTCHC,
    ID_BUTTONTXT,
    ID_WZROBOTCHC,
    ID_BUTTON_TOGGLE_JOYSTICK 
};

class MT_JoyStickFrameTimer : public wxTimer
{
private:
    MT_JoyStickFrame* m_pFrame;

public:
    MT_JoyStickFrameTimer(MT_JoyStickFrame* parent) : m_pFrame(parent) {};

    void Notify(){m_pFrame->DoTimedEvents();};
};

BEGIN_EVENT_TABLE(MT_JoyStickFrame, wxFrame)
EVT_MENU(wxID_EXIT, MT_JoyStickFrame::OnMenuFileExit)
EVT_MENU(ID_ROBOT_MENU_CONNECT, MT_JoyStickFrame::OnMenuRobotConnections)
EVT_MENU(ID_MENU_ROBOTS_COMMAND, MT_JoyStickFrame::OnMenuRobotCommand)
EVT_TEXT(ID_MAXSPEED, MT_JoyStickFrame::CheckMaxSpeed)
EVT_TEXT(ID_MAXTURNINGRATE, MT_JoyStickFrame::CheckMaxTurningRate)
EVT_TEXT_ENTER(ID_MAXTURNINGRATE, MT_JoyStickFrame::OnMaxTurningRateChange)
EVT_TEXT_ENTER(ID_MAXSPEED, MT_JoyStickFrame::OnMaxSpeedChange)
EVT_CHOICE(ID_XYROBOTCHC, MT_JoyStickFrame::OnXYChoice)
EVT_CHOICE(ID_WZROBOTCHC, MT_JoyStickFrame::OnWZChoice)
EVT_BUTTON(ID_BUTTON_TOGGLE_JOYSTICK, MT_JoyStickFrame::OnButtonToggleJoyStick)
EVT_CLOSE(MT_JoyStickFrame::OnWindowClosed)
END_EVENT_TABLE()

MT_JoyStickFrame::MT_JoyStickFrame(wxFrame* parent,
                                   MT_AllRobotContainer* inRobots,
                                   bool asChild,
                                   wxWindowID id)
: MT_FrameWithInit(parent, 
                   id, 
                   wxT("Joystick Controller"), 
                   wxDefaultPosition, 
                   wxSize(375,160), 
                   wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN),
    myGamePadController(),
    m_bOwnRobots(false),
    m_RobotConfigXML(),
    m_pTimer(NULL)
{
    if(!inRobots)
    {
        m_bOwnRobots = true;
        TheRobots = new MT_AllRobotContainer();
    }
    else
    {
        TheRobots = inRobots;
    }

    float maxspeed = MT_DEFAULT_MAX_SPEED;
    float maxturningrate = MT_DEFAULT_MAX_TURNING_RATE;

    // This needs to be initialized false - e.g. MSW does not do this for us!
    DoEvents = false;

    DoControl = true;

    GamePadValid = true;  


    // associate robots w/ gamepad
    AttachRobotsToGamePad();

    /**************************************
     * Menu Construction
     **************************************/
    if(asChild == MT_JSF_STANDALONE)
    {
        float maxspeed = MT_DEFAULT_MAX_SPEED;
        float maxturningrate = MT_DEFAULT_MAX_TURNING_RATE;
        m_RobotConfigXML.SetFilename(MT_GetXMLPath(wxT("robotconfig.xml")).mb_str());
        MT_ReadRobotXML(TheRobots, &maxspeed, &maxturningrate, &m_RobotConfigXML);
        TheRobots->MaxGamePadSpeed = maxspeed;
        TheRobots->MaxGamePadTurningRate = maxturningrate;
        myGamePadController.SetMaxRates(maxspeed, maxturningrate);

        wxMenu *fileMenu = new wxMenu;
#ifdef _WIN32
        fileMenu->Append(wxID_EXIT, wxT("E&xit\tF4"));
#elif defined(__APPLE__)
        fileMenu->Append(wxID_EXIT, wxT("E&xit\tALT-X"));
#endif
        wxMenu* robotMenu = new wxMenu;
        robotMenu->Append(ID_ROBOT_MENU_CONNECT, wxT("Connections..."));
        robotMenu->Append(ID_MENU_ROBOTS_COMMAND, wxT("Send Commands..."));

        wxMenuBar* menuBar = new wxMenuBar;
        menuBar->Append(fileMenu, wxT("&File"));
        menuBar->Append(robotMenu, wxT("Robots"));
        SetMenuBar(menuBar);

        IsChild = false;
    }
    else
    {
        myGamePadController.SetMaxRates(TheRobots->MaxGamePadSpeed, 
                                        TheRobots->MaxGamePadTurningRate);
        IsChild = true;
    }


    /**************************************
     * GUI Construction
     **************************************/

    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer* vbox0 = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* hbox0 = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer* xybox = new wxBoxSizer(wxVERTICAL);

    xybox->Add(new wxStaticText(this, -1, wxT("X,Y")));
    XYPosText = new wxStaticText(this, -1, wxT("-1.00,-1.00"));
    xybox->Add(XYPosText);
    //XYChoices.Add("----");
    XYRobotChoice = new wxChoice(this, ID_XYROBOTCHC, wxDefaultPosition, wxSize(100,20), XYChoices);
    xybox->Add(XYRobotChoice, 0, wxTOP, 10);

    wxBoxSizer* wzbox = new wxBoxSizer(wxVERTICAL);

    wzbox->Add(new wxStaticText(this, -1, wxT("W,Z")));
    WZPosText = new wxStaticText(this, -1, wxT("-1.00,-1.00"));
    wzbox->Add(WZPosText);
    //WZChoices.Add("----");
    WZRobotChoice = new wxChoice(this, ID_WZROBOTCHC, wxDefaultPosition, wxSize(100,20), WZChoices);
    wzbox->Add(WZRobotChoice, 0, wxTOP, 10);

    hbox0->Add(xybox, 0, wxALL, 10);
    hbox0->Add(wzbox, 0, wxALL, 10);

    vbox0->Add(hbox0);
    ButtonText = new wxStaticText(this, ID_BUTTONTXT, wxT("Buttons: "));
    vbox0->Add(ButtonText, 0, wxLEFT, 10);

    wxBoxSizer* vbox1 = new wxBoxSizer(wxVERTICAL);

    wxString MaxString;
    MaxString.Printf(wxT("%3.2f"), myGamePadController.GetMaxSpeedMPS());
    vbox1->Add(new wxStaticText(this, -1, wxT("Max Speed")));
    MaxSpeedCtrl = new wxTextCtrl(this, ID_MAXSPEED, MaxString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    vbox1->Add(MaxSpeedCtrl);

    MaxString.Printf(wxT("%4.2f"), myGamePadController.GetMaxTurningRateRADPS());
    vbox1->Add(new wxStaticText(this, -1, wxT("Max Turning Rate")), 0, wxTOP, 10);
    MaxTurningRateCtrl = new wxTextCtrl(this, ID_MAXTURNINGRATE, MaxString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    vbox1->Add(MaxTurningRateCtrl);

    ButtonToggleJoyStick = new wxButton(this, ID_BUTTON_TOGGLE_JOYSTICK, wxT("Disable"));
    vbox1->Add(ButtonToggleJoyStick, 0, wxTOP | wxALIGN_CENTER, 10);

    hbox->Add(vbox0);
    hbox->Add(vbox1, 0, wxALL, 10);

    /* For some reason without a panel the background color is wrong.  This forces
     * it to be the right color, using "Own" version so we don't change the colors
     * of the other controls, too. */
    SetOwnBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    
    SetSizerAndFit(hbox);

    /* for some reason EVT_KILL_FOCUS needs to be connected from the control itself
     * (as opposed to from the form)... The last parameter (this) is very important,
     * because it says which object should handle the event */
    MaxSpeedCtrl->Connect(ID_MAXSPEED,
        wxEVT_KILL_FOCUS,
        wxCommandEventHandler(MT_JoyStickFrame::OnMaxSpeedChange),
        NULL,
        this);
    MaxTurningRateCtrl->Connect(ID_MAXTURNINGRATE,
        wxEVT_KILL_FOCUS,
        wxCommandEventHandler(MT_JoyStickFrame::OnMaxTurningRateChange),
        NULL,
        this);

    UpdateRobotChoices(true);

    if(!IsChild)
    {
        registerDialogForXML(this);
        DoEvents = true;
    }

    m_pTimer = new MT_JoyStickFrameTimer(this);
    m_pTimer->Start(MT_JOYSTICK_REFRESH_PERIOD);

}

MT_JoyStickFrame::~MT_JoyStickFrame()
{
    DoEvents = false;

    if(m_pTimer)
    {
        delete m_pTimer;
    }

    if(m_bOwnRobots)
    {
        delete TheRobots;
    }

}

void MT_JoyStickFrame::OnWindowClosed(wxCloseEvent& event)
{

    // stop doing events - free robots from joystick
    DoEvents = false;

    if(IsChild) // If this is a child window, we don't want to destroy it, just hide it
    {
        if (!event.CanVeto()) // Test if we can veto this deletion
        {
            this->Destroy();    // If not, destroy the window anyway.
        }
        else
        {
            this->Hide();
            event.Veto();     // Notify the calling code that we didn't delete the frame.
        }
    }
    else
    {
        /* if the "x" is clicked and this is not a child window, quit the app */
        DoQuit();
    }

}

void MT_JoyStickFrame::AttachRobotsToGamePad()
{

    // if there's no gamepad, there's nothing to do!
    if(!GamePadValid || !myGamePadController.IsConnected())
    {
        return;
    }

    /* things that need to get taken care of here:
       1. new robots might have been added by connection dialog, they
       need to be added to the gamepad - happily the gamepad takes
       care of duplicates automatically
       2. Some robots may have been deleted by the connection dialog,
       this is tricky b/c the pointer is now invalid
    */
    bool FlaggedChanges = false;  // look for changes
    for(unsigned int i = 0; i < MT_MAX_NROBOTS; i++)
    {
        switch(TheRobots->StatusChange[i])
        {
        case MT_ROBOT_JUST_CONNECTED:
            TheRobots->GamepadIndex[i] = myGamePadController.AddRobot(TheRobots->GetRobot(i));
            // See if there is an open slot on the controller to assign this to
            if(myGamePadController.getXYRobot() == NULL)
            {
                myGamePadController.SetXYRobot(TheRobots->GetRobot(i));
            }
            if( (myGamePadController.getXYRobot() != NULL) && (myGamePadController.getWZRobot() == NULL) )
            {
                myGamePadController.SetWZRobot(TheRobots->GetRobot(i));
            }
            FlaggedChanges = true;
            break;
        case MT_ROBOT_JUST_DISCONNECTED:
            myGamePadController.RemoveRobot(TheRobots->GamepadIndex[i]);
            FlaggedChanges = true;
            break;
        case MT_ROBOT_NO_CHANGE:
            break;
        }
    }

    // if changes happened, clear the choice strings and let UpdateRobotChoices rebuild them
    if(FlaggedChanges)
    {
        XYChoices.Clear();
        WZChoices.Clear();
    }

    TheRobots->ClearStatusChanges();

}

void MT_JoyStickFrame::UpdateRobotChoices(bool FlaggedChanges)
{

    // make these static so we can find out if they changed
    static unsigned int xychoiceindex = 0;
    static unsigned int wzchoiceindex = 0;

    for(unsigned int i = 0; i < MT_MAX_NROBOTS; i++)
    {
        if(TheRobots->IsPhysical(i))
        {
            // Add to the available lists
            FlaggedChanges |=    // flag a change only if we added a robot to both lists
                (MT_UniqueAddString(&XYChoices, MT_StringToWxString(TheRobots->RobotName[i])) & 
                 MT_UniqueAddString(&WZChoices, MT_StringToWxString(TheRobots->RobotName[i])));
            if(TheRobots->GetRobot(i) == myGamePadController.getXYRobot())
            {
                // if this is the XY robot, remove it from the WZ list
                MT_SafeRemoveString(&WZChoices, MT_StringToWxString(TheRobots->RobotName[i]));
                // also make sure it is "selected" in the XY list
                if(xychoiceindex != i)
                {
                    FlaggedChanges = true;
                    xychoiceindex = i;
                }
            }
            if(TheRobots->GetRobot(i) == myGamePadController.getWZRobot())
            {
                // if this is the WZ robot, remove it from the XY list
                MT_SafeRemoveString(&XYChoices, MT_StringToWxString(TheRobots->RobotName[i]));
                // also make sure it is "selected" in the WZ list
                if(wzchoiceindex != i)
                {
                    FlaggedChanges = true;
                    wzchoiceindex = i;
                }
            }
        }
    }

    // if there were no changes, then we don't do any control updating
    if(!FlaggedChanges)
    {
        return;
    }

    // update the controls
    XYRobotChoice->Clear();
    WZRobotChoice->Clear();
    for(unsigned int i = 0; i < XYChoices.Count(); i++)
    {
        XYRobotChoice->Append(XYChoices.Item(i));
    }
    for(unsigned int i = 0; i < WZChoices.Count(); i++)
    {
        WZRobotChoice->Append(WZChoices.Item(i));
    }

    // update the choices:
    // re-index to the available list
    wxString botname = MT_StringToWxString(TheRobots->RobotName[xychoiceindex]);
    unsigned int xychoiceindex_inlist = XYChoices.Index(botname);
    botname = MT_StringToWxString(TheRobots->RobotName[wzchoiceindex]);
    unsigned int wzchoiceindex_inlist = WZChoices.Index(botname);
    // set the selection in the control
    XYRobotChoice->SetSelection(xychoiceindex_inlist);
    WZRobotChoice->SetSelection(wzchoiceindex_inlist);

}

void MT_JoyStickFrame::OnMaxSpeedChange(wxCommandEvent& event)
{

    // if DoEvents is cancelled, we are exiting -> avoid memory access errors
    if(!DoEvents)
    {
        return;
    }

    // force value into limits - automatically updates text if clamped
    float value = MT_ClampTextCtrlFloat(MaxSpeedCtrl, 0, MT_MAX_ALLOWED_SPEED, "%3.2f");

    if(GamePadValid)
    {
        myGamePadController.SetMaxSpeedMPS(value);
    }

    TheRobots->MaxGamePadSpeed = value;

    /* We need to let the wx base classes process this event, especially if it
     * was a EVT_KILL_FOCUS - this takes care of, e.g. moving the cursor */
    event.Skip();

}

void MT_JoyStickFrame::OnMaxTurningRateChange(wxCommandEvent& event)
{

    // if DoEvents is cancelled, we are exiting -> avoid memory access errors
    if(!DoEvents)
    {
        return;
    }

    // force value into limits - automatically updates text if clamped
    float value = MT_ClampTextCtrlFloat(MaxTurningRateCtrl, 0, MT_MAX_ALLOWED_TURNING_RATE, "%4.2f");

    if(GamePadValid)
    {
        myGamePadController.SetMaxTurningRateRADPS(value);
    }

    TheRobots->MaxGamePadTurningRate = value;

    /* We need to let the wx base classes process this event, especially if it
     * was a EVT_KILL_FOCUS - this takes care of, e.g. moving the cursor */
    event.Skip();

}

void MT_JoyStickFrame::OnMenuFileExit(wxCommandEvent& WXUNUSED(event))
{
    DoQuit();
}


void MT_JoyStickFrame::OnMenuRobotConnections(wxCommandEvent& WXUNUSED(event))
{

    // we need to stop doing events until this is done
    DoEvents = false;

    MT_RobotConnectDialog* RoboDlg = new MT_RobotConnectDialog(TheRobots, this);
    registerDialogForXML(RoboDlg);
    RoboDlg->ShowModal();
    RoboDlg->Destroy();
    delete RoboDlg;
    RoboDlg = NULL;

    AttachRobotsToGamePad();
    UpdateRobotChoices(true);

    // OK to start again
    DoEvents = true;

}

void MT_JoyStickFrame::OnMenuRobotCommand(wxCommandEvent& WXUNUSED(event))
{

    // we need to stop doing events until this is done
    DoEvents = false;

    MT_RobotCommandDialog* CommandDlg = new MT_RobotCommandDialog(TheRobots, this);
    registerDialogForXML(CommandDlg);
    CommandDlg->ShowModal();
    CommandDlg->Destroy();
    delete CommandDlg;
    CommandDlg = NULL;

    // OK to start again
    DoEvents = true;

}

void MT_JoyStickFrame::OnButtonToggleJoyStick(wxCommandEvent& event)
{

    if(DoControl)
    {
        ButtonToggleJoyStick->SetLabel(wxT("Enable"));
        DoControl = false;
    }
    else
    {
        ButtonToggleJoyStick->SetLabel(wxT("Disable"));
        DoControl = true;
    }


}


void MT_JoyStickFrame::CheckMaxTurningRate(wxCommandEvent& WXUNUSED(event))
{

    // Avoid an infinite loop b/c validator can change text value
    if(!DoEvents || !(MaxTurningRateCtrl->IsModified()))
    {
        return;
    }

    MT_ValidateTextCtrlNumeric(MaxTurningRateCtrl);

}

void MT_JoyStickFrame::OnXYChoice(wxCommandEvent& WXUNUSED(event))
{

    // no choices => something odd happened
    if(XYChoices.Count())
    {
        // otherwise, get the selected name
        wxString choice = XYRobotChoice->GetStringSelection();
        // match it to the correct robot
        int rchoice = -1;
        for(unsigned int i = 0; i < MT_MAX_NROBOTS; i++)
        {
            if(MT_StringToWxString(TheRobots->RobotName[i]) == choice)
            {
                rchoice = i;
            }
        }

        // rchoice = -1 means not found
        if(rchoice >= 0)
        {
            // otherwise set
            myGamePadController.SetXYRobot(TheRobots->GetRobot(rchoice));
        }
    }

}

void MT_JoyStickFrame::OnWZChoice(wxCommandEvent& WXUNUSED(event))
{

    // no choices => something odd happened
    if(WZChoices.Count())
    {
        // otherwise, get the selected name
        wxString choice = WZRobotChoice->GetStringSelection();
        // match it to the correct robot
        int rchoice = -1;
        for(unsigned int i = 0; i < MT_MAX_NROBOTS; i++)
        {
            if(MT_StringToWxString(TheRobots->RobotName[i]) == choice)
            {
                rchoice = i;
            }
        }

        // rchoice = -1 means not found
        if(rchoice >= 0)
        {
            // otherwise set
            myGamePadController.SetWZRobot(TheRobots->GetRobot(rchoice));
        }
    }

}

void MT_JoyStickFrame::CheckMaxSpeed(wxCommandEvent& WXUNUSED(event))
{

    // Avoid an infinite loop b/c validator can change text value
    if(!DoEvents || !(MaxSpeedCtrl->IsModified()))
    {
        return;
    }

    MT_ValidateTextCtrlNumeric(MaxSpeedCtrl);

}

void MT_JoyStickFrame::DoTimedEvents()
{

    // if DoEvents is false, then we don't want to do any of this stuff
    //  - this is used not only for program control but also for safety,
    //     since a false DoEvents can indicate that we are quitting and
    //     therefore some pointers may be invalid
    if(!DoEvents)
    {
        return;
    }

    // we want to store previous values of these so that
    //   we only update the controls if the values change
    //    this avoids flickering (esp. on MSW)
    static int bprev = -1;
    static float xprev;
    static float yprev;
    static float wprev;
    static float zprev;

    UpdateRobotChoices();

    if(!GamePadValid || !myGamePadController.IsConnected())
    {
        return;
    }

    myGamePadController.PollAndUpdate(DoControl);

    float x = myGamePadController.xf();
    float y = myGamePadController.yf();
    float w = myGamePadController.wf();
    float z = myGamePadController.zf();
    unsigned int b = myGamePadController.buttons();

    wxString ValString;

    if((bprev < 0) || (x != xprev) || (y != yprev))
    {
        ValString.Printf(wxT("%+3.2f, %+3.2f"),x,y);
        XYPosText->SetLabel(ValString);
        xprev = x;
        yprev = y;
    }

    if((bprev < 0) || (w != wprev) || (z != zprev))
    {
        ValString.Printf(wxT("%+3.2f, %+3.2f"),w,z);
        WZPosText->SetLabel(ValString);
        wprev = w;
        zprev = z;
    }

    if(bprev < 0 || b != bprev)
    {
        ButtonText->SetLabel(wxT("Buttons: ") + MT_UIntToBitString(b,12));
        bprev = b;
    }

}

void MT_JoyStickFrame::DoQuit()
{
    DoEvents = false; // stop the timer!
    GamePadValid = false;
    myGamePadController.Disconnect();

    if(IsChild)
    {
        Close(true);
    }
    else
    {
        MT_WriteRobotXML(TheRobots, myGamePadController.GetMaxSpeedMPS(), myGamePadController.GetMaxTurningRateRADPS(), &m_RobotConfigXML);
        m_RobotConfigXML.SaveFile();

        Destroy();
    }

}

void MT_JoyStickFrame::EnableEvents()
{
    DoEvents = true;
    AttachRobotsToGamePad();
    UpdateRobotChoices();
}


void MT_JoyStickFrame::registerDialogForXML(wxWindow* dialog)
{
    if(!dialog || IsChild)
    {
        return;
    }

    MT_ReadWindowDataFromXML(m_RobotConfigXML, 
                          dialog->GetLabel(), 
                          dialog); 
    dialog->Connect(wxID_ANY, 
                    wxEVT_MOVE, 
                    wxCommandEventHandler(MT_JoyStickFrame::onChildMove),
                    NULL,
                    this); /* it's important that the "this" parameter
                              is passed to the Connect call so that
                              it knows which instance of MT_FrameBase
                              to refer to */
}


void MT_JoyStickFrame::onChildMove(wxCommandEvent& event)
{
    if(IsChild)
    {
        return;
    }

    wxWindow* win = dynamic_cast<wxWindow*>(event.GetEventObject());
    MT_WriteWindowDataToXML(&m_RobotConfigXML, win->GetLabel(), win);
}

