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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <boost/format.hpp>
#include <gnuradio/prefs.h>
#include "airspyhf_impl.h"
#include <stdexcept>
#include <cstring>

namespace gr {
namespace airspy {

airspyhf::sptr
airspyhf::make(const std::string serial_number, int samplerate, int frequency)
{
    return gnuradio::get_initial_sptr
    (new airspyhf_impl(serial_number, samplerate, frequency));
}


/*
 * The private constructor
 */
airspyhf_impl::airspyhf_impl(const std::string serial_number,
                             int samplerate,
                             int frequency)
: d_output_items(nullptr),
gr::sync_block("airspyhf",
                 gr::io_signature::make(0, 0, 0),
                 gr::io_signature::make(1, 1, sizeof(gr_complex)))
{
    int ret;
    ret = airspyhf_open(&d_device);
    if(ret) {
        throw std::runtime_error("airspyhf_open");
    }
    // Samples come in blocks from Airspy HF
    d_airspyhf_output_size = airspyhf_get_output_size(d_device);
    set_min_noutput_items(d_airspyhf_output_size);
    
    ret = airspyhf_set_samplerate(d_device, samplerate);
    if(ret) {
        throw std::runtime_error("airspyhf_set_samplerate");
    }
    
    ret = airspyhf_set_freq(d_device, frequency);
    if(ret) {
        throw std::runtime_error("airspyhf_set_freq");
    }
    
    // Logging
    // Borrowed from https://github.com/dl1ksv
    prefs *p = prefs::singleton();
    std::string config_file = p->get_string("LOG", "log_config", "");
    std::string log_level = p->get_string("LOG", "log_level", "off");
    std::string log_file = p->get_string("LOG", "log_file", "");
    
    GR_LOG_GETLOGGER(LOG, "gr_log." + alias());
    GR_LOG_SET_LEVEL(LOG, log_level);
    if(log_file.size() > 0) {
        if(log_file == "stdout") {
            GR_LOG_SET_CONSOLE_APPENDER(LOG, "cout","gr::log :%p: %c{1} - %m%n");
        }
        else if(log_file == "stderr") {
            GR_LOG_SET_CONSOLE_APPENDER(LOG, "cerr","gr::log :%p: %c{1} - %m%n");
        }
        else {
            GR_LOG_SET_FILE_APPENDER(LOG, log_file , true,"%r :%p: %c{1} - %m%n");
        }
    }

    d_logger = LOG;
}

/*
 * Our virtual destructor.
 */
airspyhf_impl::~airspyhf_impl()
{
    airspyhf_close(d_device);
}

int airspyhf_callback(airspyhf_transfer_t* transfer_fn) {
    airspyhf_impl* self = (airspyhf_impl*)transfer_fn->ctx;
    // Wait for a buffer to become ready
    std::unique_lock<std::mutex> lock(self->d_mutex);
    while (self->d_output_items == nullptr) self->d_work_ready.wait(lock);
    if (transfer_fn->dropped_samples) {
        GR_LOG_INFO(self->d_logger,
                    boost::format("dropped %1%") % transfer_fn->dropped_samples);
    }
    
    // Copy samples to GNURadio buffer
    std::memcpy(self->d_output_items,
                transfer_fn->samples,
                sizeof(gr_complex) * self->d_airspyhf_output_size);
    // Ok we are done, invalidate the buffer pointer and notify
    self->d_output_items = nullptr;
    self->d_callback_done.notify_one();
    
    return 0; // anything else is an error
}

bool airspyhf_impl::start() {
    int ret;
    ret = airspyhf_start(d_device, airspyhf_callback, (void*)this);
    if(ret) {
        throw std::runtime_error("airspyhf_start");
    }
    
    return true;
}

bool airspyhf_impl::stop() {
    int ret;
    ret = airspyhf_stop(d_device);
    // TODO: error check
    return true;
}

void airspyhf_impl::set_freq(const uint32_t freq_hz) {
    int ret = airspyhf_set_freq(d_device, freq_hz);
    if(ret) {
        throw std::runtime_error("airspyhf_set_freq");
    }
}

void airspyhf_impl::set_samplerate(const uint32_t samplerate) {
    int ret = airspyhf_set_samplerate(d_device, samplerate);
    if(ret) {
        throw std::runtime_error("airspyhf_set_samplerate");
    }
}

int
airspyhf_impl::work(int noutput_items,
                    gr_vector_const_void_star &input_items,
                    gr_vector_void_star &output_items)
{
    gr_complex *out = (gr_complex *) output_items[0];
    std::unique_lock<std::mutex> lock(d_mutex);
    d_output_items = out;
    // We are ready for the callback to fill the buffer
    d_work_ready.notify_one();
    // The callback will set this to nullptr
    while (d_output_items != nullptr) d_callback_done.wait(lock);
    
    // Tell runtime system how many output items we produced.
    return d_airspyhf_output_size;
}

} /* namespace airspy */
} /* namespace gr */

