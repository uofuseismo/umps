#!/usr/bin/env python3
import umpspy
import numpy as np

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
    test_message_formats_text()
    test_message_formats_failure()
