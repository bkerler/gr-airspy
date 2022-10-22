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

#define MAX_DEVICES 10

namespace gr {
namespace airspy {

airspyhf::sptr
airspyhf::make(const std::string serial_number, int samplerate, int frequency)
{
    return gnuradio::get_initial_sptr
    (new airspyhf_impl(serial_number, samplerate, frequency));
}

std::vector<std::string>
airspyhf::list_devices() {
    int nd;

    nd = airspyhf_list_devices(0, 0);
    if (nd < 0) {
        throw std::runtime_error("airspyhf_list_devices(0, 0)");
    }
    std::vector<uint64_t> serials(nd);
    nd = airspyhf_list_devices(serials.data(), nd);
    if (nd < 0) {
        throw std::runtime_error("airspyhf_list_devices()");
    }
    std::vector<std::string> devices(nd);
    // Convert to hex strings
    for (int n = 0; n < nd; n++) {
        // TODO: fix
        devices[n] = boost::str(boost::format("%1$#x") % serials[n]);
    }
    
    return devices;
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
    // Setup logging
    // Borrowed from https://github.com/dl1ksv
    prefs *p = prefs::singleton();
    std::string config_file = p->get_string("LOG", "log_config", "");
    std::string log_level = p->get_string("LOG", "log_level", "off");
    std::string log_file = p->get_string("LOG", "log_file", "");
    
    //GR_LOG_GETLOGGER(LOG, "gr_log." + alias());
    //GR_LOG_SET_LEVEL(LOG, log_level);
    //d_logger = LOG;
    
    /*if(log_file.size() > 0) {
        if(log_file == "stdout") {
            GR_LOG_SET_CONSOLE_APPENDER(LOG, "cout","gr::log :%p: %c{1} - %m%n");
        }
        else if(log_file == "stderr") {
            GR_LOG_SET_CONSOLE_APPENDER(LOG, "cerr","gr::log :%p: %c{1} - %m%n");
        }
        else {
            GR_LOG_SET_FILE_APPENDER(LOG, log_file , true,"%r :%p: %c{1} - %m%n");
        }
    }*/
    
    airspyhf_lib_version_t ver;
    airspyhf_lib_version(&ver);
    GR_LOG_INFO(d_logger,
                boost::format("libairspyhf %d.%d.%d") % ver.major_version % ver.minor_version % ver.revision);
    
    // Open and configure device
    if(airspyhf_open(&d_device)) {
        throw std::runtime_error("airspyhf_open");
    }
    // Samples come in blocks from Airspy HF
    d_airspyhf_output_size = airspyhf_get_output_size(d_device);
    set_min_noutput_items(d_airspyhf_output_size);
    
    if(airspyhf_set_samplerate(d_device, samplerate)) {
        throw std::runtime_error("airspyhf_set_samplerate");
    }
    
    if(airspyhf_set_freq(d_device, frequency)) {
        throw std::runtime_error("airspyhf_set_freq");
    }
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
                    boost::format("dropped %d") % transfer_fn->dropped_samples);
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

bool
airspyhf_impl::start() {
    int ret;
    ret = airspyhf_start(d_device, airspyhf_callback, (void*)this);
    if(ret) {
        throw std::runtime_error("airspyhf_start");
    }
    
    return true;
}

bool
airspyhf_impl::stop() {
    int ret;
    ret = airspyhf_stop(d_device);
    // TODO: error check
    return true;
}

bool
airspyhf_impl::is_streaming() {
    return airspyhf_is_streaming(d_device);
}

void
airspyhf_impl::set_freq(const uint32_t freq_hz) {
    if(airspyhf_set_freq(d_device, freq_hz)) {
        throw std::runtime_error("airspyhf_set_freq");
    }
}

void
airspyhf_impl::set_samplerate(const uint32_t samplerate) {
    if(airspyhf_set_samplerate(d_device, samplerate)) {
        throw std::runtime_error("airspyhf_set_samplerate");
    }
}

std::vector<uint32_t>
airspyhf_impl::get_samplerates() {
    uint32_t nrates;
    // Get number of rates
    if(airspyhf_get_samplerates(d_device, &nrates, 0)) {
        throw std::runtime_error("airspyhf_get_samplerates(0)");
    }
    // Vector to store rates
    std::vector<uint32_t> rates(nrates);
    if(airspyhf_get_samplerates(d_device, rates.data(), nrates)) {
        throw std::runtime_error("airspyhf_get_samplerates()");
    }
    
    return rates;
}

void
airspyhf_impl::set_hf_agc(uint8_t flag) {
    if(airspyhf_set_hf_agc(d_device, flag)) {
        throw std::runtime_error("airspyhf_set_hf_agc");
    }
}

                    
void
airspyhf_impl::set_hf_agc_threshold(uint8_t flag) {
    if(airspyhf_set_hf_agc_threshold(d_device, flag)) {
        throw std::runtime_error("airspyhf_set_hf_agc_threshold");
    }
}


void
airspyhf_impl::set_hf_att(uint8_t value) {
    if(airspyhf_set_hf_att(d_device, value)) {
        throw std::runtime_error("airspyhf_set_hf_att");
    }
}


void
airspyhf_impl::set_hf_lna(uint8_t flag) {
    if(airspyhf_set_hf_lna(d_device, flag)) {
        throw std::runtime_error("airspyhf_set_hf_lna");
    }
}

void
airspyhf_impl::set_lib_dsp(uint8_t flag) {
    if(airspyhf_set_lib_dsp(d_device, flag)) {
        throw std::runtime_error("airspyhf_set_lib_dsp");
    }
}

void
airspyhf_impl::board_partid_serialno_read() {
    airspyhf_read_partid_serialno_t serialno;
    if(airspyhf_board_partid_serialno_read(d_device, &serialno)) {
        throw std::runtime_error("airspyhf_board_partid_serialno_read");
    }
}

std::string
airspyhf_impl::version_string_read() {
    char version[MAX_VERSION_STRING_SIZE];
    if(airspyhf_version_string_read(d_device, version, MAX_VERSION_STRING_SIZE)) {
        throw std::runtime_error("airspyhf_version_string_read");
    }
    return std::string(version);
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

