/*
 *  GamePadController.cpp
 *  glswarm
 *
 *  Created by Dan Swain on 3/16/08.
 *
 */

#include "GamePadController.h"

MT_GamePadController::MT_GamePadController()
    : MT_HIDGamePad()
{
    common_init();
    myXYRobot = NULL;
    myWZRobot = NULL;
}
  
MT_GamePadController::MT_GamePadController(MT_RobotBase* setXYRobot)
    : MT_HIDGamePad()
{
    common_init();
    if(SetXYRobot(setXYRobot) == MT_ROBOT_ASSIGNMENT_ERROR)
        printf("Could not assign XY robot.\n");
    myWZRobot = NULL;
}

MT_GamePadController::MT_GamePadController(MT_RobotBase* setXYRobot,
                                           MT_RobotBase* setWZRobot)
    : MT_HIDGamePad()
{
    common_init();
  
    printf("Assigning Robot 1\n");
    if(SetXYRobot(setXYRobot) == MT_ROBOT_ASSIGNMENT_ERROR)
        printf("Could not assign XY robot.\n");
  
    printf("Assigning Robot 2\n");
    if(SetWZRobot(setWZRobot) == MT_ROBOT_ASSIGNMENT_ERROR)
        printf("Could not assign WZ robot.\n");
  
}

void MT_GamePadController::common_init()
{
    // Do the gamepad's initializations
    connected = !(MT_HIDGamePad::Init());
  
    // Initially we have no robots
    AvailableRobots.resize(0);

	m_bDisableWZ = false;
  
}

void MT_GamePadController::SeizeControl()
{
  
    // check to see if we have an XY robot
    if( myXYRobot )
    {
        // if so, make it non-autonomous
        myXYRobot->SetAutonomousOff();
    }
  
    // same for WZ robot
    if( myWZRobot )
    {
        myWZRobot->SetAutonomousOff();
    }
  
}


void MT_GamePadController::ReleaseControl()
{
  
    // check to see if we have an XY robot
    if( myXYRobot )
    {
        // if so, make it non-autonomous
        myXYRobot->SetAutonomousOn();
    }
  
    // same for WZ robot
    if( myWZRobot )
    {
        myWZRobot->SetAutonomousOn();
    }
  
}

unsigned int MT_GamePadController::AddRobot(MT_RobotBase* newRobot)
{
    int bot_index = -1;
  
    for(unsigned int i = 0; i < AvailableRobots.size(); i++)
    {
        if(newRobot == AvailableRobots[i] || AvailableRobots[i] == NULL)
        {
            bot_index = i;
        }
    }
  
    // still = -1 -> need to add robot
    if(bot_index < 0)
    {
        AvailableRobots.push_back(newRobot);
        bot_index = AvailableRobots.size() - 1;
    }
  
    return bot_index;
  
}

unsigned int MT_GamePadController::RemoveRobot(MT_RobotBase* BotToRemove)
{
    // if the robot is already assigned to an axis, don't give it up
    if( (BotToRemove == myXYRobot) || (BotToRemove == myWZRobot) )
        return MT_ROBOT_IN_USE;
  
    // if the robot is found in our list, free it (carefully) and let 
    //   the calling function know
    for(unsigned int i = 0; i < AvailableRobots.size(); i++){
        if(BotToRemove == AvailableRobots[i]){
            // Set the robot pointer to NULL so we don't clobber it with the erase!
            AvailableRobots[i] = NULL;
            AvailableRobots.erase(AvailableRobots.begin() + i);
            return MT_ROBOT_FREED;
        }
    }
  
    // The robot wasn't found in our list, but tell the calling function we freed it.
    return MT_ROBOT_FREED;
  
}

unsigned int MT_GamePadController::RemoveRobot(unsigned int i)
{

    // Check against Axes robots to make sure we don't try to move them
    if(AvailableRobots[i] == myXYRobot)
    {
        myXYRobot = NULL;
    }
    if(AvailableRobots[i] == myWZRobot)
    {
        myWZRobot = NULL;
    }
  
    // make sure the pointer is null
    AvailableRobots[i] = NULL;
  
    return MT_ROBOT_FREED;
  
}

unsigned int MT_GamePadController::robotID(MT_RobotBase* QueryBot)
{
    for(unsigned int i = 0; i < AvailableRobots.size(); i++){
        if(QueryBot == AvailableRobots[i])
            return i;
    }
    return AvailableRobots.size();
}

unsigned char MT_GamePadController::SetXYRobot(MT_RobotBase* setXYRobot)
{
  
    // make the robot non-autonomous
    return AssignRobot(myXYRobot,setXYRobot);

}

unsigned char MT_GamePadController::SetWZRobot(MT_RobotBase* setWZRobot)
{

	if(m_bDisableWZ)
	{
		return MT_ROBOT_ASSIGNMENT_ERROR;
	}

    // make the robot non-autonomous
    return AssignRobot(myWZRobot,setWZRobot); 
  
}

unsigned char MT_GamePadController::AssignRobot(MT_RobotBase*& RobotToChange, MT_RobotBase* NewBot) 
{
    if(!NewBot)
    {
        RobotToChange = NULL;
    }
	DisplayAssignedRobots();
    
    // This is the same robot, treat as an error
    if(NewBot == RobotToChange){
        printf("C1\n");
        return MT_ROBOT_ASSIGNMENT_ERROR;
    }
  
    // This robot already has an axis, so just bail
    if(NewBot == myXYRobot || NewBot == myWZRobot){
        printf("C2\n");
        return MT_ROBOT_ASSIGNMENT_ERROR;
    }
  
    // This robot is not connected, so tell the user and bail
    if( !(NewBot->IsConnected()) ){
        printf("Robot on %s is not connected.\n", NewBot->getInfo());
        return MT_ROBOT_ASSIGNMENT_ERROR;
    }  

    NewBot->SetAutonomousOff();
    RobotToChange = NewBot;
    // Add it to the list if it's not already there
    AddRobot(NewBot);
    return MT_ROBOT_ASSIGNMENT_OK;
}

void MT_GamePadController::PollAndUpdate(bool DoControl)
{
    Poll();
  
    // Be careful if both buttons are pressed.  Let 5 take precedence.
    if( Button5State() || (Button5State() && Button7State()) ){
        /*MT_SETBIT(ButtonStates,MT_BUTTON5,0);
          MT_SETBIT(ButtonStates,MT_BUTTON7,0);*/
        NextXYRobot(-1);
    }
  
    if(Button7State() && !Button5State() ){
        //MT_SETBIT(ButtonStates,MT_BUTTON7,0);
        NextXYRobot(1);
    }
  
    if( Button6State() || (Button6State() && Button8State()) ){
        /*MT_SETBIT(ButtonStates,MT_BUTTON6,0);
          MT_SETBIT(ButtonStates,MT_BUTTON8,0);*/
        NextWZRobot(-1);
    }
  
    if(Button8State() && !Button6State() ){
        //MT_SETBIT(ButtonStates,MT_BUTTON7,0);
        NextWZRobot(1);
    }

    if(!DoControl)
    {
        return;
    }

    std::vector<double> js_axes;
    unsigned int js_buttons = ButtonStates;
    js_axes.resize(4);
    js_axes[0] = Xf;
    js_axes[1] = Yf;
    js_axes[2] = Wf;
    js_axes[3] = Zf;
    
    if(myXYRobot){
        myXYRobot->JoyStickControl(js_axes, js_buttons);
    }

    /* the WZ robot has XY/WZ swapped */
    js_axes[0] = Wf;
    js_axes[1] = Zf;
    js_axes[2] = Xf;
    js_axes[3] = Yf;
    if(!m_bDisableWZ && myWZRobot){
        myWZRobot->JoyStickControl(js_axes, js_buttons);
    }
  
}

void MT_GamePadController::NextXYRobot(char direction)
{
    //printf("Trying to cycle XY robot in the %s direction\n", direction > 0 ? "+" : "-" );
    cycle_robot(myXYRobot,direction); 
}

void MT_GamePadController::NextWZRobot(char direction)
{
    //printf("Trying to cycle WZ robot in the %s direction\n", direction > 0 ? "+" : "-" );
    cycle_robot(myWZRobot,direction);
}

void MT_GamePadController::cycle_robot(MT_RobotBase*& botToChange, char direction)
{
  
    MT_RobotBase* original_bot = botToChange;
    unsigned int myID = robotID(botToChange);
    direction = MT_SGN(direction);
  
    for(unsigned int i = myID; i < AvailableRobots.size() && i >= 0; i = i + direction){
        if( (i != myID) && (AssignRobot(botToChange,AvailableRobots[i]) == MT_ROBOT_ASSIGNMENT_OK) ){
      
            //DisplayAssignedRobots();
      
            original_bot->SafeStop();
      
            return;
        }
    }
  
}

void MT_GamePadController::DisplayAssignedRobots() const
{
    char Robots[8];
    char Assigned[8];
    char c[2];
  
    for(unsigned int i = 0; i < AvailableRobots.size(); i++){
        sprintf(c,"%1d",i+1);
        Robots[i] = c[0];
        if(AvailableRobots[i] == myXYRobot)
            Assigned[i] = 'X';
        else if(AvailableRobots[i] == myWZRobot)
            Assigned[i] = 'W';
        else
            Assigned[i] = ' ';
    }
  
    for(int i = AvailableRobots.size(); i < 8; i++){
        Robots[i] = ' ';
        Assigned[i] = ' ';
    }
  
    Robots[7] = '\0';
    Assigned[7] = '\0';
  
    printf("Robots Available to Joystick: [%s]\n",Robots);
    printf("Assigned to axis:             [%s]\n",Assigned);
  
}
