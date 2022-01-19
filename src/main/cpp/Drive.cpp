#include "Drive.h"
#include "rev/CANSparkMax.h"
#include "frc/DoubleSolenoid.h"

using namespace std;

Drive::Drive(){
    int leftdriveID = 9;
    int leftdriveID2 = 10;
    int rightdriveID = 7;
    int rightdriveID2 = 8;
    int leftclimbID = 4;
    int rightclimbID = 5;   
    int climberlockIDa = 0;
    int climberlockIDb = 1;
    int climber_tilta = 2;
    int climber_tiltb = 3;

// Define motors, sensors, and pneumatics here
    // Drive motors, sensors, and pneumatics

        m_leftdrive = new rev::CANSparkMax{leftdriveID, rev::CANSparkMax::MotorType::kBrushless}; //actually 1
        m_leftdrive2 = new rev::CANSparkMax{leftdriveID2, rev::CANSparkMax::MotorType::kBrushless};
        m_leftdrive->SetInverted(true);
        m_leftdrive2->SetInverted(true);
        m_rightdrive = new rev::CANSparkMax{rightdriveID, rev::CANSparkMax::MotorType::kBrushless};
        m_rightdrive2 = new rev::CANSparkMax{rightdriveID2, rev::CANSparkMax::MotorType::kBrushless};

    // Climb motors, sensors, and pneumatics
    
        m_leftclimb = new rev::CANSparkMax{leftclimbID, rev::CANSparkMax::MotorType::kBrushless};
        m_rightclimb = new rev::CANSparkMax{rightclimbID, rev::CANSparkMax::MotorType::kBrushless};
        m_leftclimb->SetInverted(true);  

        p_climberlock = new frc::DoubleSolenoid{frc::PneumaticsModuleType::REVPH, climberlockIDa, climberlockIDb};  
        p_climbertilt = new frc::DoubleSolenoid{frc::PneumaticsModuleType::REVPH, climberlockIDa, climberlockIDb};
}

/* JOYSTICK DRIVE */
/* This function provides basic joystick control of the drive base*/
    
    void Drive::Joystick_Drive(double LeftStick, double RightStick){

    double left_out = LeftStick*LeftStick*LeftStick;
    double right_out = RightStick*RightStick*RightStick;

    m_leftdrive -> Set(left_out);
    m_leftdrive2 -> Set(left_out);
    m_rightdrive -> Set(right_out);
    m_rightdrive2 -> Set(right_out);
}

/* CLIMBING */

        void Drive::climber_extend(){
           
            //unlock climbers and extend
            p_climberlock-> Set(frc::DoubleSolenoid::Value::kReverse);
            
            m_leftclimb -> Set(1);
            m_rightclimb -> Set(1);
        }

        void Drive::climber_retract(){
           
            //unlock climbers and retract
            p_climberlock-> Set(frc::DoubleSolenoid::Value::kReverse);
            
            m_leftclimb -> Set(-1);
            m_rightclimb -> Set(-1);
        }

        void Drive::climber_hold(){
           
            //unlock climbers and hold
            p_climberlock-> Set(frc::DoubleSolenoid::Value::kForward);
            
            m_leftclimb -> Set(0);
            m_rightclimb -> Set(0);
        }

        void Drive::climber_tiltin(){  
            p_climbertilt-> Set(frc::DoubleSolenoid::Value::kForward);
      
        }


