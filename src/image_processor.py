#!/usr/bin/env python

"""
This script contains a ROS node that will receive Image messages over the topic 'raw_image', convert them into OpenCV images, perform computer vision tasks, currently placeholder, convert them back into Image messages and publish them on the topic 'processed_image'.
"""

import rospy
import cv2
from sensor_msgs.msg import Image
from cv_bridge import CvBridge, CvBridgeError


# Use this publisher to send the output
pub = rospy.Publisher('processed_image',Image,queue_size=1)

# Create a translator object to convert images
converter = CvBridge()

# Set threshold for color processing
threshold = 90


def callback(input):
    # The callback accepts an Image message, performs visual processing and masking on it, and publishes the result. Exact processing is placeholder. I'm going to start by assuming the presence of material at every location in the frame except where darker colors are present. Darker pixels will then be changed to black, non-dark pixels will be made white.

    # Translate Image message into a cv2 image using existing encoding. The image will revert to a more general CV encoding, like 8UC3.
    cv_image = converter.imgmsg_to_cv2(input,"passthrough")

    # Make a copy, as the original image will not be editable after conversion
    cv_image = cv_image.copy()

    # Perform visual processing on the image to extract relevant data.
    # Extract a tuple containing the number of (rows, columns, channels).
    (rows,cols,chans)  = cv_image.shape

    # Cycle through each row
    for row in range(rows):
        # Cycle through each column
        for col in range(cols):
            total = 0
            # Cycle through each channel
            for chan in range(chans):
                # Sum the intensity of the channels
                total += cv_image.item(row,col,chan)

            # Cycle through each channel
            for chan in range(chans):
                # If the total intensity is less than the threshold
                if (total < threshold):
                    # Set color to black
                    cv_image.itemset((row,col,chan),0)
                else:
                    # Set the color to white
                    cv_image.itemset((row,col,chan),255)


    # Translate cv2 image back to an Image message using existing encoding. This will be a generic encoding like 8UC3.
    msg = converter.cv2_to_imgmsg(cv_image,"passthrough")

    # Assert encoding. CV generally uses bgr8.
    msg.encoding = "bgr8"

    # Publish image message
    pub.publish(msg)





def main():
    # Initialize the ROS node and Subscriber
    rospy.init_node("image_processor_node")
    rospy.Subscriber('raw_image',Image,callback)

    # Spin until shut down
    rospy.spin()





if __name__ == '__main__':
    main()
