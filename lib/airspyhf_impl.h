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
    
    void set_freq(const uint32_t freq_hz) override;
    void set_samplerate(const uint32_t samplerate) override;
    std::vector<uint32_t> get_samplerates() override;
    bool is_streaming() override;
    void set_hf_agc(uint8_t flag) override;
    void set_hf_agc_threshold(uint8_t flag) override;
    void set_hf_att(uint8_t value) override;
    void set_hf_lna(uint8_t flag) override;
    void set_lib_dsp(uint8_t flag) override;
    void board_partid_serialno_read() override;
    std::string version_string_read() override;
    
    // TODO: implement some of these
    //int set_calibration();
    //int get_calibration();
    // airspyhf_set_optimal_iq_correction_point
    // airspyhf_iq_balancer_configure
    // airspyhf_flash_calibration
    // airspyhf_set_user_output
    
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
