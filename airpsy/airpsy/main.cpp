//
//  main.cpp
//  airpsy
//
//  Created by Albin Stigö on 2019-12-22.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#include <iostream>

#include <gnuradio/top_block.h>
#include <gnuradio/blocks/null_sink.h>

#include <airspy/airspyhf.h>


int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    auto tb = gr::make_top_block("airspyhf");
    auto as = gr::airspy::airspyhf::make(0);
    auto ns = gr::blocks::null_sink::make(sizeof(gr_complex));
    
    tb->connect(as, 0, ns, 0);

    tb->start();
    sleep(5);
    tb->stop();
    
    return 0;
}
