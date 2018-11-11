// Includes
#include<ros/ros.h>
#include<sensor_msgs/PointCloud2.h>
#include<sensor_msgs/Image.h>
#include<cv_bridge/cv_bridge.h>
#include<pcl_conversions/pcl_conversions.h>
#include<pcl/point_cloud.h>
#include<pcl/point_types.h>
#include<pcl/visualization/pcl_visualizer.h>

//TODO
/*
Probably don't need to copy image
*/

// Declare publisher
ros::Publisher pub;

// Declare image pointer
cv_bridge::CvImagePtr cv_ptr;

// Create a visualizer
boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(new pcl::visualization::PCLVisualizer("3D Viewer"));

float scale = .01;
// float scale = 1;


void callback(const sensor_msgs::Image &msg)
{
    // Accepts an image message, generates a pointcloud from it, translates that pointcloud into a pointcloud message, and publishes it.

    // Translate the image message to a cv_bridge image container
    try
    {
        // Returns a container with a cv::Mat
        cv_ptr = cv_bridge::toCvCopy(msg, "bgr8");
    }
    catch (cv_bridge::Exception& e)
    {
        ROS_ERROR("cv_bridge exception: %s", e.what());
        return;
    }


    // Declare a point cloud pointer and a point
    pcl::PointCloud<pcl::PointXYZ>::Ptr basic_cloud_ptr (new pcl::PointCloud<pcl::PointXYZ>);
    pcl::PointXYZ basic_point;
    // Set z to constant 0
    basic_point.z = 0;


    // Cycle through cv_ptr->image.cols, cv_ptr->image.rows pixels and read intensity
    int colnum = cv_ptr->image.cols;
    int rownum = cv_ptr->image.rows;
    cv::Vec3b intensity;

    for(int i = 0; i < rownum; i++)
    {
        for(int j = 0; j < colnum; j++)
        {
            // Row, Col
            intensity = cv_ptr->image.at<cv::Vec3b>(i, j);

            // Access in intensity.val[i], with i being 0,1,2 for b,g,r. uchar format. If a pixel is not black
            if(intensity.val[0] > 0)
            {
                // Add a point to the cloud representing the current pixel
                basic_point.x = scale * j;
                basic_point.y = scale * i;
                basic_cloud_ptr->points.push_back(basic_point);

            }
        }
    }

    // Set some properties of the cloud to make a valid message?
    basic_cloud_ptr->width = (int) basic_cloud_ptr->points.size ();
    basic_cloud_ptr->height = 1;


    // Translate the cloud into a PointCloud2 message
    pcl::PCLPointCloud2 cl;
    sensor_msgs::PointCloud2 output;
    pcl::toPCLPointCloud2(*basic_cloud_ptr,cl);
    pcl_conversions::fromPCL(cl, output);

    // Set the frame info
    output.header.frame_id = "camera_depth_optical_frame";

    // Publish the message
    pub.publish(output);


    // Update the viewer
    viewer->updatePointCloud<pcl::PointXYZ> (basic_cloud_ptr, "main cloud");
}


int main(int argc, char **argv)
{
    // Initialize node and declare NodeHandle
    ros::init(argc,argv,"image_translator_node");
    ros::NodeHandle nh;

    // Subscribe to topic with 1 message buffer
    ros::Subscriber sub = nh.subscribe("processed_image", 1, callback);

    // Announce publisher with 1 message buffer
    pub = nh.advertise<sensor_msgs::PointCloud2>("camera/depth/points", 1);

    // Prime the viewer with a point cloud and Initialize it
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_ptr (new pcl::PointCloud<pcl::PointXYZ>);
    viewer->addPointCloud<pcl::PointXYZ> (cloud_ptr, "main cloud");
    viewer->setBackgroundColor (0, 0, 0);
    viewer->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 1, "main cloud");
    viewer->addCoordinateSystem (1.0);
    viewer->initCameraParameters ();

    ros::Rate r(100);  // Run at 100Hz

    // Spin until shutdown
    while(ros::ok())
    {
        ros::spinOnce();
        viewer->spinOnce(100);
        r.sleep();
    }

    return 0;
}
