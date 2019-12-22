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

#ifndef INCLUDED_AIRSPY_AIRSPYHF_H
#define INCLUDED_AIRSPY_AIRSPYHF_H

#include <airspy/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace airspy {

/*!
 * \brief <+description of block+>
 * \ingroup airspy
 *
 */
class AIRSPY_API airspyhf : virtual public gr::sync_block
{
public:
    typedef boost::shared_ptr<airspyhf> sptr;
    
    /*!
     * \brief Return a shared_ptr to a new instance of airspy::airspyhf.
     *
     * To avoid accidental use of raw pointers, airspy::airspyhf's
     * constructor is in a private implementation
     * class. airspy::airspyhf::make is the public interface for
     * creating new instances.
     */
    static sptr make(const std::string serial_number="",
                     int samplerate=768000,
                     int frequency=7000000);
};

} // namespace airspy
} // namespace gr

#endif /* INCLUDED_AIRSPY_AIRSPYHF_H */

