/* -*- c++ -*- */
/*
 * Copyright 2019 Albin Stigo.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_AIRSPY_AIRSPYHF_IMPL_H
#define INCLUDED_AIRSPY_AIRSPYHF_IMPL_H

#include <airspy/airspyhf.h>
#include <libairspyhf/airspyhf.h>
#include <gnuradio/logger.h>
#include <mutex>
#include <condition_variable>

namespace gr {
namespace airspy {

class airspyhf_impl : public airspyhf
{
private:
    airspyhf_device_t *d_device;
    
    // Thread Communication and Synchronization
    // int d_noutput_items;
    gr_complex *d_output_items;
    std::mutex d_mutex;
    std::condition_variable d_work_ready;
    std::condition_variable d_callback_done;
    friend int airspyhf_callback(airspyhf_transfer_t* transfer_fn);
    
    int d_airspyhf_output_size;
    
    gr::logger_ptr d_logger;
    
public:
    airspyhf_impl(const std::string serial_number, int samplerate, int frequency);
    ~airspyhf_impl();
    
    void set_freq(const uint32_t freq_hz);
    void set_samplerate(const uint32_t samplerate);

    //bool is_streaming();
    //int set_calibration();
    //int calibration();
    
    // TODO: implement some of these
    // airspyhf_lib_version
    // airspyhf_list_devices
    // is_streaming
    // set_lib_dsp
    // get_samplerates
    // set_samplerate
    // airspyhf_get_calibration
    // airspyhf_set_calibration
    // airspyhf_set_optimal_iq_correction_point
    // airspyhf_iq_balancer_configure
    // airspyhf_flash_calibration
    // airspyhf_board_partid_serialno_read
    // airspyhf_version_string_read
    // airspyhf_set_user_output
    // airspyhf_set_hf_agc
    // airspyhf_set_hf_agc_threshold
    // airspyhf_set_hf_att
    // airspyhf_set_hf_lna

    
    bool start() override;
    bool stop() override;
    
    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star &input_items,
             gr_vector_void_star &output_items) override;
};

} // namespace airspy
} // namespace gr

#endif /* INCLUDED_AIRSPY_AIRSPYHF_IMPL_H */

/*

 extern ADDAPI int ADDCALL airspyhf_is_streaming(airspyhf_device_t* device);
 extern ADDAPI int ADDCALL airspyhf_is_low_if(airspyhf_device_t* device); /* Tells if the current sample rate is Zero-IF (0) or Low-IF (1)
 extern ADDAPI int ADDCALL airspyhf_set_freq(airspyhf_device_t* device, const uint32_t freq_hz);
 extern ADDAPI int ADDCALL airspyhf_set_lib_dsp(airspyhf_device_t* device, const uint8_t flag); /* Enables/Disables the IQ Correction, IF shift and Fine Tuning.
 extern ADDAPI int ADDCALL airspyhf_get_samplerates(airspyhf_device_t* device, uint32_t* buffer, const uint32_t len);
 extern ADDAPI int ADDCALL airspyhf_set_samplerate(airspyhf_device_t* device, uint32_t samplerate);
 extern ADDAPI int ADDCALL airspyhf_get_calibration(airspyhf_device_t* device, int32_t* ppb);
 extern ADDAPI int ADDCALL airspyhf_set_calibration(airspyhf_device_t* device, int32_t ppb);
 extern ADDAPI int ADDCALL airspyhf_set_optimal_iq_correction_point(airspyhf_device_t* device, float w);
 extern ADDAPI int ADDCALL airspyhf_iq_balancer_configure(airspyhf_device_t* device, int buffers_to_skip, int fft_integration, int fft_overlap, int correlation_integration);
 extern ADDAPI int ADDCALL airspyhf_flash_calibration(airspyhf_device_t* device);    /* streaming needs to be stopped
 extern ADDAPI int ADDCALL airspyhf_board_partid_serialno_read(airspyhf_device_t* device, airspyhf_read_partid_serialno_t* read_partid_serialno);
 extern ADDAPI int ADDCALL airspyhf_version_string_read(airspyhf_device_t* device, char* version, uint8_t length);
 extern ADDAPI int ADDCALL airspyhf_set_user_output(airspyhf_device_t* device, airspyhf_user_output_t pin, airspyhf_user_output_state_t value);
 extern ADDAPI int ADDCALL airspyhf_set_hf_agc(airspyhf_device_t* device, uint8_t flag);                /* 0 = off, 1 = on
 extern ADDAPI int ADDCALL airspyhf_set_hf_agc_threshold(airspyhf_device_t* device, uint8_t flag);    /* when agc on: 0 = low, 1 = high
 extern ADDAPI int ADDCALL airspyhf_set_hf_att(airspyhf_device_t* device, uint8_t value); /* Possible values: 0..8 Range: 0..48 dB Attenuation with 6 dB steps
 extern ADDAPI int ADDCALL airspyhf_set_hf_lna(airspyhf_device_t* device, uint8_t flag);    /* 0 or 1: 1 to activate LNA (alias PreAmp): 1 = +6 dB gain - compensated in digital */

