#include <QMainWindow>
#include <QPixmap>
#include <QThread>
#include <QTimer>
#include <QtGui/QFont>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include <chrono>
#include <memory>

#include "ros/ros.h"
#include "geometry_msgs/PoseStamped.h"

#include <sstream>
// uic -o untitled.h untitled.ui
#include "untitled.h"

#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

class RosControllerThread : protected QThread
{
public:
	RosControllerThread()
	{
		publisherSetPose = n.advertise<geometry_msgs::PoseStamped>("/move_base_simple/goal", 1);
	}

	void run()
	{
		ros::Rate loop_rate(10);
		while (ros::ok())
		{
			ros::spinOnce();
			loop_rate.sleep();
		}
	}

	void Start(std::shared_ptr<RosControllerThread> shared)
	{
		this_shared = shared;
		start();
	}

	~RosControllerThread()
	{
		ros::shutdown();
	}

	ros::Publisher publisherSetPose;
	std::shared_ptr<RosControllerThread> this_shared;
	ros::NodeHandle n;
};

class WindowManager : protected Ui_MainWindow
{
public:
	WindowManager(int argc, char **argv)
	{
		RosProcess(argc, argv);
		WindowProcess();
	}

	void WindowProcess()
	{
		setupUi(&window);

		QObject::connect(pushButton, &QPushButton::clicked, [this]()
						 {
			MoveBaseClient ac("move_base", true);

			while (!ac.waitForServer(ros::Duration(5.0)))
			{
				ROS_INFO("Waiting for the move_base action server to come up");
			}
 			move_base_msgs::MoveBaseGoal goal;
			
  			goal.target_pose.header.frame_id = "map";
  			goal.target_pose.header.stamp = ros::Time::now();

			goal.target_pose.pose.position.x = doubleSpinBox_X->value();
			goal.target_pose.pose.position.y = doubleSpinBox_Y->value();
			goal.target_pose.pose.orientation.w = 1.0;
  
    		ac.sendGoal(goal);

			// ac.waitForResult(); 
			// if (ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
			// 	ROS_INFO("Hooray, the base moved 1 meter forward");
			// else
			// 	ROS_INFO("The base failed to move forward 1 meter for some reason");

			auto posMessage = geometry_msgs::PoseStamped();
			posMessage.pose.position.x = doubleSpinBox_X->value();
			posMessage.pose.position.y = doubleSpinBox_Y->value();
			posMessage.pose.orientation.w = 1;
			posMessage.header.frame_id = "map";

			ros->publisherSetPose.publish(posMessage); });

		window.show();
	}

	void RosProcess(int argc, char **argv)
	{
		ros::init(argc, argv, "controller_node");
		ros = std::make_shared<RosControllerThread>();
		ros->Start(ros);
	}

	~WindowManager()
	{
	}
	QMainWindow window;
	std::shared_ptr<RosControllerThread> ros;
};

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	WindowManager wm(argc, argv);
	return app.exec();
}
