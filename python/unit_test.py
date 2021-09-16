#!/usr/bin/env python3
import pyurts
import numpy as np

def test_messages_pick():
    pick = pyurts.MessageFormats.Pick()
    pick.network = "UU"
    pick.station = "ICU"
    pick.channel = "EHZ"
    pick.time = 1034
    pick.location_code = "01"
    pick.identifier = 101
    pick.polarity = pyurts.MessageFormats.Polarity.up
    pick.phase_hint = "P"
    pick.algorithm = "test_algorithm"

    assert pick.network == "UU", "network failed"
    assert pick.station == "ICU", "station failed"
    assert pick.channel == "EHZ", "channel failed"
    assert pick.location_code == "01", "location failed"
    assert abs(pick.time - 1034) < 1.e-14, 'time failed'
    assert pick.identifier == 101, "id failed"
    assert pick.polarity == pyurts.MessageFormats.Polarity.up, "polarity failed"
    assert pick.phase_hint == "P", "phase hint failed"
    assert pick.algorithm == "test_algorithm", "algorithm failed"

def test_messages_data_packet():
    packet = pyurts.MessageFormats.DataPacket()
    a = 1

if __name__ == "__main__":
    test_messages_pick()
    test_messages_data_packet()
