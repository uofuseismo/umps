#!/usr/bin/env python3
import sys
import os
import glob
sys.path.append(os.getcwd())
import umpspy
import numpy as np
import datetime

################################################################################
#                                  Logging                                     #
################################################################################
def test_logging_standard_out():
    so = umpspy.Logging.StandardOut(umpspy.Logging.Level.Debug)
    assert so.level == umpspy.Logging.Level.Debug, 'level is wrong'
    so.error("Error test - okay")
    so.warn("Warning test - okay")
    so.info("Info test - okay")
    so.debug("Debug test - okay")

def test_logging_daily_file():
    level = umpspy.Logging.Level.Debug
    logger_name = "test_logger"
    file_name = "tempLogger.txt"
    d = umpspy.Logging.DailyFile()
    d.initialize(logger_name, file_name, level)
    d.error("Error test - okay")
    d.warn("Warn test - okay")
    d.info("Info test - okay")
    d.debug("Debug test - okay")

    output_file = glob.glob("tempLogger*")
    assert len(output_file) == 1, 'log file not created'
    os.remove(output_file[0])
    
################################################################################
#                                  Message Formats                             #
################################################################################
def test_message_formats_text():
    contents = "Contents"
    text = umpspy.MessageFormats.Text()
    text.contents = contents

    assert text.contents == contents, "contents failed"
    assert text.message_type == "UMPS::MessageFormats::Text", "message type failed"

def test_message_formats_failure():
    details = "unknown reason for failure"
    failure = umpspy.MessageFormats.Failure()
    failure.details = details

    assert failure.details == details, "details failed"
    assert failure.message_type == "UMPS::MessageFormats::Failure", "message type failed"

################################################################################
#                                 Authentication                               #
################################################################################
def test_authentication_username_password():
    user = 'user'
    password = 'password'
    cred = umpspy.Authentication.UserNameAndPassword() 
    cred.user_name = user
    cred.password = password
    assert cred.user_name == user, 'username failed'
    assert cred.password == password, 'password failed'

def test_authentication_keys():
    public_key = "x"*40
    private_key = "y"*40
    metadata = 'something about this key'
    keys = umpspy.Authentication.Keys()
    keys.public_key = public_key
    keys.private_key = private_key
    keys.metadata = metadata
    assert keys.public_key == public_key, 'public key failed'
    assert keys.private_key == private_key, 'private key failed'
    assert keys.metadata == metadata, 'metadata failed'

def test_authentication_zap_options():
    zap = umpspy.Authentication.ZAPOptions()

################################################################################
#                                   Messaging                                  #
################################################################################
def test_messaging_pubsub_publisher_options():
    send_address = "tcp://127.0.0.1:5555"
    send_time_out = datetime.timedelta(milliseconds = 20)
    send_high_water_mark = 150
    options = umpspy.Messaging.PublisherSubscriber.PublisherOptions()
    options.address = send_address
    options.high_water_mark = send_high_water_mark
    options.time_out = send_time_out

    assert options.address == send_address, 'send address failed'
    assert options.high_water_mark == send_high_water_mark, 'send hwm failed'
    assert options.time_out == send_time_out, 'send timeout failed'

def test_messaging_pubsub_subscriber_options():
    receive_address = "tcp://127.0.0.1:5555"
    receive_time_out = datetime.timedelta(milliseconds = 10)
    receive_high_water_mark = 120
    options = umpspy.Messaging.PublisherSubscriber.SubscriberOptions() 
    text_message = umpspy.MessageFormats.Text()
    failure_message = umpspy.MessageFormats.Failure()
    message_types = umpspy.MessageFormats.Messages()
    message_types.add(text_message)
    message_types.add(failure_message)
    options.message_types = message_types 
    options.address = receive_address 
    options.high_water_mark = receive_high_water_mark
    options.time_out = receive_time_out
    
    assert options.address == receive_address, 'receive address failed'
    assert options.high_water_mark == receive_high_water_mark, 'receive hwm failed'
    assert options.time_out == receive_time_out, 'receive timeout failed'
    assert options.message_types.size == 2, 'size is wrong'
    assert options.message_types.contains(text_message.message_type), 'text message missing'
    assert options.message_types.contains(failure_message.message_type), 'failure message missing'
    """
    EXPECT_NO_THROW(messageTypes.add(textMessage));
    EXPECT_NO_THROW(messageTypes.add(failureMessage));
    const std::string address = "tcp://127.0.0.1:5555";
    const int highWaterMark = 120;
    const std::chrono::milliseconds timeOut{10};
    const int zero = 0;
    PublisherSubscriber::SubscriberOptions options;
    EXPECT_NO_THROW(options.setAddress(address));
    EXPECT_NO_THROW(options.setMessageTypes(messageTypes));
    EXPECT_NO_THROW(options.setReceiveHighWaterMark(highWaterMark));
    EXPECT_NO_THROW(options.setReceiveTimeOut(timeOut));

    options.clear();
    EXPECT_EQ(options.getReceiveHighWaterMark(), zero);
    EXPECT_EQ(options.getReceiveTimeOut(), std::chrono::milliseconds{-1});
    EXPECT_FALSE(options.haveMessageTypes());
    """


"""
def test_messages_data_packet():
    packet = umpspy.MessageFormats.DataPacket()
    network = "WY"
    station = "YFT"
    channel = "EHZ"
    location_code = "01" 
    starttime_mus = 1050
    sampling_rate = 100.
    x = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])
    endtime_mus = starttime_mus + int( (len(x) - 1)/(sampling_rate)*1.e6 )

    packet.network = network
    packet.station = station
    packet.channel = channel
    packet.location_code = location_code
    packet.sampling_rate = sampling_rate
    packet.start_time_in_microseconds = starttime_mus
    packet.data = x
 
    assert packet.network == network, 'network failed'
    assert packet.station == station, 'station failed'
    assert packet.channel == channel, 'channel failed'
    assert packet.location_code == location_code, 'location failed'
    assert packet.start_time_in_microseconds == starttime_mus, 'start time failed'
    assert packet.end_time_in_microseconds == endtime_mus, 'end time failed'
    assert abs(packet.sampling_rate - sampling_rate) < 1.e-13, 'sampling rate failed'
    x_back = packet.data
    assert len(x) == len(x_back), 'inconsistent sizes'
    for i in range(len(x)):
        assert abs(x_back[i] - x[i]) < 1.e-14, 'x[%d] is wrong'%i
"""

#def test_messaging_pubsub_subscriber():
#    subscriber = umpspy.Messaging.PublisherSubscriber.Subscriber()
#    pick = umpspy.MessageFormats.Pick()
#    subscriber.add_subscription(pick)
#    #a = umpspy.MessageFormats.pick_base_class()
#    #print(type(pick))
#    #print(type(subscriber))
#    #print(type(pick))

if __name__ == "__main__":
    test_logging_standard_out()
    test_logging_daily_file()
    test_message_formats_text()
    test_message_formats_failure()
    test_authentication_username_password()
    test_authentication_keys()
    test_authentication_zap_options()
    test_messaging_pubsub_publisher_options()
    test_messaging_pubsub_subscriber_options()
