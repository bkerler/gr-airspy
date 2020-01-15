#!/usr/bin/env python3

import airspy

dev = airspy.airspyhf()

print(dev)
print(dev.list_devices())
print(dev.get_samplerates())

dev.set_hf_agc(1)
dev.set_hf_agc_threshold(1)
dev.set_hf_att(1)
dev.set_hf_lna(1)
dev.set_lib_dsp(1)

print(dev.board_partid_serialno_read())
print(dev.version_string_read())