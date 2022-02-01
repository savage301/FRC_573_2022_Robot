// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

/* ---------------------------------------------------------------------------
------------------------------TEAM 573 NAMING CONVENTION---------------------------------
MOTORS: m_function  ex: m_leftdrive
SENSORS: s_type  ex: s_gyro
CONTROLLERS: c#_type_name  ex: c1_btn_a
PNEUMATICS: p_function ex: p_intake
BRANCHES OF CODE: section/what you're working on     ex: Drive/JoystickControl
also: merge into main before each event, Before event create event branch and merge every night
--------------------------------------------------------------------------------
*/

#include "Robot.h"
#include "Drive.h"
#include "Appendage.h"
#include "Led.h"
#include <fmt/core.h>
#include "networktables/NetworkTable.h"
#include "networktables/NetworkTableInstance.h"
#include "networktables/NetworkTableEntry.h"
#include "networktables/NetworkTableValue.h"
#include "wpi/span.h"
#include <frc/smartdashboard/SmartDashboard.h>
#include <tuple>

void Robot::RobotInit()
{
  m_chooser.SetDefaultOption(kAutoNameDefault, kAutoNameDefault);
  m_chooser.AddOption(kAutoNameCustom, kAutoNameCustom);
  m_chooser.AddOption(kAutoNameCustom1, kAutoNameCustom1);

  frc::SmartDashboard::PutData("Auto Modes", &m_chooser);
  alliance_color = "red";  // Default evaluated in auto and teleop inits
  turret_direction = true; // Initial turrent scan direction

  // Dashboard input creations
  MyAppendage.DashboardCreate();
}

/**
 * This function is called every robot packet, no matter the mode. Use
 * this for items like diagnostics that you want ran during disabled,
 * autonomous, teleoperated and test.
 *
 * <p> This runs after the mode specific periodic functions, but before
 * LiveWindow and SmartDashboard integrated updating.
 */
void Robot::RobotPeriodic() {}

/**
 * This autonomous (along with the chooser code above) shows how to select
 * between different autonomous modes using the dashboard. The sendable chooser
 * code works with the Java SmartDashboard. If you prefer the LabVIEW Dashboard,
 * remove all of the chooser code and uncomment the GetString line to get the
 * auto name from the text box below the Gyro.
 *
 * You can add additional auto modes by adding additional comparisons to the
 * if-else structure below with additional strings. If using the SendableChooser
 * make sure to add them to the chooser code above as well.
 */
void Robot::AutonomousInit()
{

  counter = 0;

  // Get alliance station color
  static auto color = frc::DriverStation::GetAlliance();
  if (color == frc::DriverStation::Alliance::kBlue)
  {
    alliance_color = "blue";
  }

  m_autoSelected = m_chooser.GetSelected();
  // m_autoSelected = SmartDashboard::GetString("Auto Selector",
  //     kAutoNameDefault);

  fmt::print("Auto selected: {}\n", m_autoSelected);

  if (m_autoSelected == kAutoNameCustom)
  {
    // Custom Auto goes here
  }
  else
  {
    // Default Auto goes here
  }
}

void Robot::AutonomousPeriodic()
{

  //Reset shooter variables
  bool align = false;
  bool atspeed = false;
  bool rotate = false;

  // -------- Read in Shooter camera Stuff -----------------------------------------------

  std::shared_ptr<nt::NetworkTable> table_s = nt::NetworkTableInstance::GetDefault().GetTable("Limelight_Shooter");
  table_s->PutNumber("ledMode", 1);
  table_s->PutNumber("camMode", 0);

  // -----------PIPELINE STUFF-----------//
  table_s->PutNumber("pipeline", 0);

  //--------CAMERA VALUES-----------------//
  float shooter_camera_x = table_s->GetNumber("tx", 0);

  float shooter_camera_exist = table_s->GetNumber("tv", 0);
  // float image_size = table->GetNumber("ta", 0);
  float shooter_camera_y = table_s->GetNumber("ty", 0);

  double distance = MyAppendage.Get_Distance(shooter_camera_y);

  // ----------------------------------------------------------

  // -------- Read in Intake camera Stuff -----------------------------------------------

  std::shared_ptr<nt::NetworkTable> table_i = nt::NetworkTableInstance::GetDefault().GetTable("Limelight_Intake");
  table_i->PutNumber("ledMode", 1);
  table_i->PutNumber("camMode", 0);

  // -----------PIPELINE STUFF-----------//
  if(alliance_color == "blue"){
    table_i->PutNumber("pipeline", 0); // Blue color detection pipeline
  }
  else{
    table_i->PutNumber("pipeline", 1); // Red color detection pipeline
  }

  //--------CAMERA VALUES-----------------//
  float intake_camera_x = table_i->GetNumber("tx", 0);

  float intake_camera_exist = table_i->GetNumber("tv", 0);
  // float image_size = table->GetNumber("ta", 0);
  //float intake_camera_y = table_i->GetNumber("ty", 0);

  // ----------------------------------------------------------

  if (m_autoSelected == kAutoNameCustom)
  {
    // 2 Ball Autonomous

    if (intake_camera_exist)
    {
      MyDrive.camera_intake(intake_camera_x, 0.8);
      MyAppendage.Intake_Down();
      MyAppendage.Intake_In();
    }
    else
    {
      double distance = MyAppendage.Get_Distance(shooter_camera_y);
      tie(align,turret_direction) = MyAppendage.Rotate(shooter_camera_exist, shooter_camera_x, turret_direction);
      bool rotate = MyAppendage.Articulate(distance);
      bool atspeed = MyAppendage.Shooter_Encoder();
      MyAppendage.Feeder_Off();

      if (align && rotate && atspeed)
      {
        MyAppendage.Feeder_In();
      }
    }
  }

  else if (m_autoSelected == kAutoNameCustom1)
  {
    // 4  Ball Autonomous
  }

  else
  {

    if (counter < 100)
    { // 100 = 2 seconds
      MyDrive.Joystick_Drive(0.5, 0.5);
    }

    else
    {
      MyDrive.Joystick_Drive(0, 0);
    }
  }

  counter++;
}

void Robot::TeleopInit()
{

  // Setting teleop variables
  drive_straight_first = true;
  endgame_unlock = false;

  // Get alliance station color
  static auto color = frc::DriverStation::GetAlliance();
  if (color == frc::DriverStation::Alliance::kBlue)
  {
    alliance_color = "blue";
  }
}
void Robot::TeleopPeriodic()
{

  //Reset shooter variables
  bool align = false;
  bool atspeed = false;
  bool rotate = false;

  //********** Read in Joystick Values ******************************************
  //------------- Driver Controller ---------------------------------------------

  double c1_joy_leftdrive = controller1.GetRawAxis(1);
  double c1_joy_rightdrive = controller1.GetRawAxis(5);
  bool c1_btn_back = controller1.GetRawButton(7);
  bool c1_btn_start = controller1.GetRawButton(8);
  double c1_righttrigger = controller1.GetRawAxis(3);
  double c1_lefttrigger = controller1.GetRawAxis(2);
  bool c1_leftbmp = controller1.GetRawButton(5);
  bool c1_rightbmp = controller1.GetRawButton(6);
  bool c1_btn_b = controller1.GetRawButton(2);
  // bool c1_btn_x = controller1.GetRawButton(3);
  bool c1_btn_a = controller1.GetRawButton(1);

  //-----------------------------------------------------------------------------
  //------------ Operator Controller --------------------------------------------
  // double c2_joy_left = controller2.GetRawAxis(1);
  // bool c2_btn_a = controller2.GetRawButton(1);
  // bool c2_btn_b = controller2.GetRawButton(2);
  bool c2_btn_y = controller2.GetRawButton(4);
  // bool c2_btn_x = controller2.GetRawButton(3);
  // bool c2_btn_lb = controller2.GetRawButton(5);
  // bool c2_btn_rb = controller2.GetRawButton(6);
  // double c2_dpad = controller2.GetPOV(0);
  // bool c2_btn_back = controller2.GetRawButton(7);
  // bool c2_btn_start = controller2.GetRawButton(8);

  bool c2_rightbumper = controller2.GetRawButton(6);
  bool c2_leftbumper = controller2.GetRawButton(5);

  // bool c2_right_trigger = controller2.GetRawAxis(3);
  double c2_left_trigger = controller2.GetRawAxis(2);
  //----------------------------------------------------------------------------

  // -------- Read in Shooter camera Stuff -----------------------------------------------

  std::shared_ptr<nt::NetworkTable> table_s = nt::NetworkTableInstance::GetDefault().GetTable("Limelight_Shooter");
  table_s->PutNumber("ledMode", 1);
  table_s->PutNumber("camMode", 0);

  // -----------PIPELINE STUFF-----------//
  table_s->PutNumber("pipeline", 0);

  //--------CAMERA VALUES-----------------//
  float shooter_camera_x = table_s->GetNumber("tx", 0);

  float shooter_camera_exist = table_s->GetNumber("tv", 0);
  // float image_size = table->GetNumber("ta", 0);
  float shooter_camera_y = table_s->GetNumber("ty", 0);

  double distance = MyAppendage.Get_Distance(shooter_camera_y);

  // ----------------------------------------------------------

  // -------- Read in Intake camera Stuff -----------------------------------------------

  std::shared_ptr<nt::NetworkTable> table_i = nt::NetworkTableInstance::GetDefault().GetTable("Limelight_Intake");
  table_i->PutNumber("ledMode", 1);
  table_i->PutNumber("camMode", 0);

  // -----------PIPELINE STUFF-----------//

  if(alliance_color == "blue"){
    table_i->PutNumber("pipeline", 0); // Blue color detection pipeline
  }
  else{
    table_i->PutNumber("pipeline", 1); // Red color detection pipeline
  }
  

  //--------CAMERA VALUES-----------------//
  float intake_camera_x = table_i->GetNumber("tx", 0);

  float intake_camera_exist = table_i->GetNumber("tv", 0);
  // float image_size = table->GetNumber("ta", 0);
  //float intake_camera_y = table_i->GetNumber("ty", 0);

  // ----------------------------------------------------------

  /*--------------------- DRIVE CODE -------------------------- */

  if (c1_btn_b)
  {
    MyDrive.drive_straight(drive_straight_first, c1_joy_leftdrive);

    drive_straight_first = false;
  }

  else if (c1_btn_a)
  {
    MyDrive.camera_intake(intake_camera_x, c1_joy_leftdrive);
  }

  else
  {
    drive_straight_first = true;

    MyDrive.Joystick_Drive(c1_joy_leftdrive, c1_joy_rightdrive);
  }
  /* ---------------------- CLIMBER CODE -----------------------------*/

  // Climber lock / unlock check
  if (c1_btn_back && c1_btn_start)
  {
    endgame_unlock = true;
  }

  if (endgame_unlock)
  {

    if (c1_righttrigger > 0.5)
    {
      MyDrive.climber_extend();
    }

    else if (c1_lefttrigger > 0.5)
    {
      MyDrive.climber_retract();
    }

    else
    {
      MyDrive.climber_hold();
    }

    if (c1_leftbmp)
    {
      MyDrive.climber_tiltin();
    }

    else if (c1_rightbmp)
    {
      MyDrive.climber_tiltout();
    }
  }
  // -------------------------------------------------------------------

  //--------------------Intake Code -----------------------------------
  if (c2_leftbumper)
  {
    MyAppendage.Intake_Down();
  }
  else
  {
    MyAppendage.Intake_Up();
  }

  if (c2_rightbumper)
  {
    MyAppendage.Intake_In();
  }
  else
  {
    MyAppendage.Intake_Off();
  }

  if (c2_btn_y)
  {
    MyAppendage.Intake_Out();
  }

  //--------------------Shooter Code -----------------------------------

  if (c2_left_trigger >= 0.5)
  {
    atspeed = MyAppendage.Shooter_Encoder();
    tie(align,turret_direction) = MyAppendage.Rotate(shooter_camera_exist, shooter_camera_x, turret_direction);
    rotate = MyAppendage.Articulate(distance);
  }
  else
  {
    MyAppendage.Shooter_Off();
    MyAppendage.Rotate_Off();
    // turret_direction = MyAppendage.Rotate(camera_exist, camera_x, turret_direction);
  }

  // -------------------------------------------------------------------

  //---------------------LED CODE----------------------------------

  if (endgame_unlock){
    MyLed.led_control("Rainbow");
  }
    else if (intake_camera_exist){
      MyLed.led_control("White");
    }

    else if (align && rotate && !atspeed){
      MyLed.led_control("Yellow");
    }

    else if (align && rotate && atspeed){
      MyLed.led_control("Green");
    }

    else{
       MyLed.led_control("Black");
    }

  // -------------------------------------------------------------------

  // --------- dashboard code ---------------

  // MyLog.Dashboard();
  // MyLog.PDPTotal();
  MyDrive.dashboard();

  MyAppendage.dashboard();

  // ------------------------------------------
} // end of teleop periodic

void Robot::DisabledInit() {}

void Robot::DisabledPeriodic() {}

void Robot::TestInit() {}

void Robot::TestPeriodic() {}

#ifndef RUNNING_FRC_TESTS
int main()
{
  return frc::StartRobot<Robot>();
}
#endif
